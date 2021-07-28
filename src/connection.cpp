/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "protocol.h"

#include <curl/curl.h>

Connection::Connection(const char* host, Uint16 port, const char* proxy, const char* proxyAuth, Protocol* protocol) : m_protocol(protocol)
{
	m_host = SDL_strdup(host);
	m_port = port;
	m_proxy = SDL_strdup(proxy);
	m_proxyAuth = SDL_strdup(proxyAuth);
}

Connection::~Connection()
{
	if(m_host)
		SDL_free(m_host);

	if(m_proxy)
		SDL_free(m_proxy);

	if(m_proxyAuth)
		SDL_free(m_proxyAuth);

	closeConnection();
}

void Connection::closeConnection()
{
	if(m_curlEasyHandle)
	{
		curl_multi_remove_handle(SDL_reinterpret_cast(CURLM*, m_curlHandle), SDL_reinterpret_cast(CURL*, m_curlEasyHandle));
		curl_easy_cleanup(SDL_reinterpret_cast(CURL*, m_curlEasyHandle));
		m_curlEasyHandle = NULL;
	}
	if(m_curlHandle)
	{
		curl_multi_cleanup(SDL_reinterpret_cast(CURLM*, m_curlHandle));
		m_curlHandle = NULL;
	}

	m_connectionState = CONNECTION_STATE_CLOSED;
	if(m_protocol)
	{
		m_protocol->onDisconnect();
		m_protocol = NULL;
	}
}

void Connection::closeConnectionError(ConnectionError error)
{
	if(m_protocol)
		m_protocol->onConnectionError(error);

	closeConnection();
	m_connectionState = CONNECTION_STATE_ERROR;
}

void Connection::updateConnection()
{
	switch(m_connectionState)
	{
		case CONNECTION_STATE_INIT:
		{
			m_curlHandle = SDL_reinterpret_cast(void*, curl_multi_init());
			CURL* curl = curl_easy_init();
			m_curlEasyHandle = SDL_reinterpret_cast(void*, curl);
			//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(curl, CURLOPT_URL, m_host);
			curl_easy_setopt(curl, CURLOPT_PORT, m_port);
			curl_easy_setopt(curl, CURLOPT_PROXY, m_proxy);
			curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, m_proxyAuth);
			curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
			curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L);
			//curl_easy_setopt(curl, CURLOPT_TCP_FASTOPEN, 1L);
			curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);
			curl_multi_add_handle(SDL_reinterpret_cast(CURLM*, m_curlHandle), curl);
			m_connectionState = CONNECTION_STATE_CONNECTING;
		}
		case CONNECTION_STATE_CONNECTING:
		{
			Sint32 still_running = 0;
			CURLMcode code = curl_multi_perform(SDL_reinterpret_cast(CURLM*, m_curlHandle), &still_running);
			if(code == CURLM_OK)
			{
				if(!still_running)
				{
					CURLMsg* msg = curl_multi_info_read(SDL_reinterpret_cast(CURLM*, m_curlHandle), &still_running);
					if(msg)
					{
						switch(msg->data.result)
						{
							case CURLE_OK:
							{
								m_connectionState = CONNECTION_STATE_CONNECTED;
								m_messageSize = 2;
								m_inputMessage.reset();
								if(m_protocol)
									m_protocol->onConnect();
							}
							break;
							case CURLE_COULDNT_RESOLVE_HOST: closeConnectionError(CONNECTION_ERROR_RESOLVE_HOST); return;
							case CURLE_OPERATION_TIMEDOUT: closeConnectionError(CONNECTION_ERROR_TIMEOUT); return;
							case CURLE_COULDNT_CONNECT: closeConnectionError(CONNECTION_ERROR_REFUSED_CONNECT); return;
							default: closeConnectionError(CONNECTION_ERROR_FAIL_CONNECT); return;
						}
					}
					else
						return;
				}
				else
					return;
			}
			else
			{
				closeConnectionError(CONNECTION_ERROR_CANNOT_CONNECT);
				return;
			}
		}
		case CONNECTION_STATE_CONNECTED:
		{
			while(m_connectionState == CONNECTION_STATE_CONNECTED)
			{
				switch(m_readState)
				{
					case READ_SIZE:
					{
						Sint32 ret = internalRead(m_messageSize);
						if(ret == -1)
						{
							closeConnectionError(CONNECTION_ERROR_RECV_FAIL);
							return;
						}
						else if(ret == 0)//Don't have any data this frame
							return;

						m_messageSize -= SDL_static_cast(Uint16, ret);
						if(m_messageSize == 0)
						{
							m_inputMessage.setReadPos(0);
							m_messageSize = m_inputMessage.getU16();
							if(m_messageSize == 0 || m_messageSize > INPUTMESSAGE_SIZE)
							{
								closeConnectionError(CONNECTION_ERROR_PROTOCOL_FAIL);
								return;
							}

							m_readState = READ_BUFFER;
						}
						else//Let's wait for the incoming data
							return;
					}
					case READ_BUFFER:
					{
						Sint32 ret = internalRead(m_messageSize);
						if(ret == -1)
						{
							closeConnectionError(CONNECTION_ERROR_RECV_FAIL);
							return;
						}
						else if(ret == 0)//Don't have any data this frame
							return;

						m_messageSize -= SDL_static_cast(Uint16, ret);
						if(m_messageSize == 0)
						{
							m_inputMessage.setReadPos(2);
							if(m_protocol && !m_protocol->onRecv(m_inputMessage))
							{
								closeConnectionError(CONNECTION_ERROR_PROTOCOL_FAIL);
								return;
							}

							m_messageSize = 2;
							m_inputMessage.reset();
							m_readState = READ_SIZE;
						}
					}
					break;
					default: break;
				}
			}
		}
		break;
		case CONNECTION_STATE_SEND_ERROR: closeConnectionError(CONNECTION_ERROR_SEND_FAIL); break;
		default:
			//Ignore
			break;
	}
}

Sint32 Connection::internalRead(Uint16 n)
{
	Sint32 len = 0;
	while(n > 0)
	{
		size_t ret;
		CURLcode result = curl_easy_recv(SDL_reinterpret_cast(CURL*, m_curlEasyHandle), SDL_reinterpret_cast(char*, m_inputMessage.getReadBuffer() + len), n, &ret);
		if(result != CURLE_OK)
		{
			if(result == CURLE_AGAIN)
				goto ReturnLen;
			else
				return -1;
		}
		else if(ret == 0)
			return -1;

		len += SDL_static_cast(Sint32, ret);
		n -= SDL_static_cast(Uint16, ret);
	}

	ReturnLen:
	m_inputMessage.setReadPos(m_inputMessage.getReadPos() + SDL_static_cast(Uint16, len));
	m_inputMessage.setMessageSize(m_inputMessage.getMessageSize() + SDL_static_cast(Uint16, len));
	return len;
}

void Connection::sendMessage(OutputMessage& msg)
{
	if(m_connectionState != CONNECTION_STATE_CONNECTED || msg.getMessageSize() == 0)//sillently ignore
		return;

	Sint32 msgSize = SDL_static_cast(Sint32, msg.getMessageSize());
	Sint32 sendBytes = 0;
	while(sendBytes < msgSize)
	{
		size_t ret;
		CURLcode result = curl_easy_send(SDL_reinterpret_cast(CURL*, m_curlEasyHandle), SDL_reinterpret_cast(char*, msg.getBuffer() + sendBytes), UTIL_min<Sint32>(msgSize - sendBytes, 1000), &ret);
		if(result != CURLE_OK)
		{
			if(result != CURLE_AGAIN)
			{
				//Closing socket here is unsafe - schedule it to a safe switch
				m_connectionState = CONNECTION_STATE_SEND_ERROR;
				return;
			}
			else
				continue;
		}

		sendBytes += SDL_static_cast(Sint32, ret);
	}
}
