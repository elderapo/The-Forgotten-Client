/*
  Tibia CLient
  Copyright (C) 2019 Saiyans King

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

#include "staticText.h"
#include "engine.h"

extern Engine g_engine;
extern Uint32 g_frameTime;

StaticText::StaticText(const Position& pos)
{
	m_textWidth = 0;
	m_position = pos;
	m_mode = MessageNone;
	m_red = 255;
	m_green = 255;
	m_blue = 255;
	m_isCompleted = false;
}

void StaticText::addMessage(const std::string& name, MessageMode mode, std::string text)
{
	if(m_messages.empty())
	{
		m_name = name;
		m_mode = mode;
		m_isCompleted = false;
	}
	else if(m_messages.size() >= 10)
		m_messages.erase(m_messages.begin());

	Uint32 time = UTIL_max<Uint32>(STATICTEXT_PER_CHAR_STAY_DURATION * SDL_static_cast(Uint32, text.length()), STATICTEXT_MINIMUM_STAY_DURATION);
	if(mode == MessageYell || mode == MessageMonsterYell || mode == MessageBarkLoud)
		time *= 2;

	StaticTextMessage newMessage;
	newMessage.m_message = std::move(text);
	newMessage.m_endTime = g_frameTime+time;
	m_messages.push_back(newMessage);
	composeText();
}

void StaticText::composeText()
{
	m_text.clear();
	m_messagesLine.clear();
	switch(m_mode)
	{
		case MessageSay:
		{
			m_text.append(m_name);
			m_text.append(" says:\n");
			m_red = 240;
			m_green = 240;
			m_blue = 0;
		}
		break;
		case MessageWhisper:
		{
			m_text.append(m_name);
			m_text.append(" whispers:\n");
			m_red = 240;
			m_green = 240;
			m_blue = 0;
		}
		break;
		case MessageYell:
		{
			m_text.append(m_name);
			m_text.append(" yells:\n");
			m_red = 240;
			m_green = 240;
			m_blue = 0;
		}
		break;
		case MessageSpell:
		{
			m_text.append(m_name);
			m_text.append(" casts:\n");
			m_red = 240;
			m_green = 240;
			m_blue = 0;
		}
		break;
		case MessageMonsterSay:
		case MessageMonsterYell:
		case MessageBarkLow:
		case MessageBarkLoud:
		{
			m_red = 255;
			m_green = 100;
			m_blue = 0;
		}
		break;
		case MessageNpcFrom:
		case MessageNpcFromStartBlock:
		{
			m_text.append(m_name);
			m_text.append(":\n");
			m_red = 96;
			m_green = 248;
			m_blue = 248;
		}
		break;
		default: return;
	}

	size_t messages = m_messages.size();
	for(size_t i = 0; i < messages; ++i)
	{
		if(i != 0)
			m_text.append(1, '\n');

		m_text.append(m_messages[i].m_message);
	}

	m_textWidth = 0;
	Uint32 allowWidth = 275;
	size_t goodPos = 0;
	size_t start = 0;
	size_t i = 0;
	size_t strLen = m_text.length();
	while(i < strLen)
	{
		StaticTextMessageLine newLine;
		if(m_text[i] == '\n')
		{
			newLine.lineStart = start;
			newLine.lineLength = i-start+1;
			newLine.lineWidth = g_engine.calculateFontWidth(CLIENT_FONT_OUTLINED, m_text, newLine.lineStart, newLine.lineLength)/2;
			m_textWidth = UTIL_max<Uint32>(m_textWidth, newLine.lineWidth);
			m_messagesLine.push_back(newLine);
			start = i+1;
			i = start;
			goodPos = 0;
			continue;
		}

		Uint32 width = g_engine.calculateFontWidth(CLIENT_FONT_OUTLINED, m_text, start, i-start+1);
		if(width >= allowWidth)
		{
			if(goodPos != 0)
			{
				newLine.lineStart = start;
				newLine.lineLength = goodPos-start+1;
				newLine.lineWidth = g_engine.calculateFontWidth(CLIENT_FONT_OUTLINED, m_text, newLine.lineStart, newLine.lineLength)/2;
				m_textWidth = UTIL_max<Uint32>(m_textWidth, newLine.lineWidth);
				m_messagesLine.push_back(newLine);
				start = goodPos+1;
				goodPos = 0;
				continue;
			}
			else
			{
				m_text.insert(i-1, 1, '-');
				++strLen;
				newLine.lineStart = start;
				newLine.lineLength = i-start;
				newLine.lineWidth = g_engine.calculateFontWidth(CLIENT_FONT_OUTLINED, m_text, newLine.lineStart, newLine.lineLength)/2;
				m_textWidth = UTIL_max<Uint32>(m_textWidth, newLine.lineWidth);
				m_messagesLine.push_back(newLine);
				start = i;
			}
		}
		else
		{
			if(m_text[i] == ' ')
				goodPos = i;
		}
		++i;
	}
	if(i > start)
	{
		StaticTextMessageLine newLine;
		newLine.lineStart = start;
		newLine.lineLength = i-start+1;
		newLine.lineWidth = g_engine.calculateFontWidth(CLIENT_FONT_OUTLINED, m_text, newLine.lineStart, newLine.lineLength)/2;
		m_textWidth = UTIL_max<Uint32>(m_textWidth, newLine.lineWidth);
		m_messagesLine.push_back(newLine);
	}
}

void StaticText::update()
{
	if(m_messages.empty())
	{
		m_isCompleted = true;
		return;
	}

	StaticTextMessage& currentMessage = m_messages.front();
	if(g_frameTime >= currentMessage.m_endTime)
	{
		m_messages.erase(m_messages.begin());
		if(m_messages.empty())
			m_isCompleted = true;
		else
			composeText();
	}
}

void StaticText::render(Sint32 posX, Sint32 posY, Sint32 boundLeft, Sint32 boundTop, Sint32 boundRight, Sint32 boundBottom)
{
	update();
	if(m_isCompleted)
		return;

	Sint32 lines = UTIL_min<Sint32>(10, SDL_static_cast(Sint32, m_messagesLine.size()));
	Sint32 lineHeight = (lines-1)*14;
	posY -= lineHeight;
	if(posX-SDL_static_cast(Sint32, m_textWidth+1) <= boundLeft)
		posX = boundLeft+m_textWidth+1;
	if(posY <= boundTop)
		posY = boundTop+1;
	if(posX+SDL_static_cast(Sint32, m_textWidth+1) >= boundRight)
		posX = boundRight-m_textWidth-1;
	if(posY+lineHeight+14 >= boundBottom)
		posY = boundBottom-lineHeight-15;

	for(Sint32 i = 0; i < lines; ++i)
	{
		StaticTextMessageLine& currentLine = m_messagesLine[i];
		g_engine.drawFont(CLIENT_FONT_OUTLINED, posX-currentLine.lineWidth, posY, m_text, m_red, m_green, m_blue, CLIENT_FONT_ALIGN_LEFT, currentLine.lineStart, currentLine.lineLength);
		posY += 14;
	}
}
