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

#ifndef __FILE_SCREENTEXT_h_
#define __FILE_SCREENTEXT_h_

#include "position.h"
#include "const.h"

struct ScreenTextMessage
{
	std::string m_message;
	Uint32 m_endTime;
	MessageMode m_mode;
};

struct ScreenTextMessageLine
{
	size_t lineStart;
	size_t lineLength;
	Uint32 lineWidth;
};

class ScreenText
{
	public:
		ScreenText(OnscreenMessages position) : m_screenPosition(position) {}

		// non-copyable
		ScreenText(const ScreenText&) = delete;
		ScreenText& operator=(const ScreenText&) = delete;

		// non-moveable
		ScreenText(ScreenText&&) = delete;
		ScreenText& operator=(ScreenText&&) = delete;

		void addMessage(MessageMode mode, std::string text);
		void composeText();

		bool update();
		void render(Sint32 boundLeft, Sint32 boundTop, Sint32 boundRight, Sint32 boundBottom);

	protected:
		std::vector<ScreenTextMessage> m_messages;
		std::vector<ScreenTextMessageLine> m_messagesLine;
		std::string m_text;

		Uint32 m_textWidth = 0;
		Uint32 m_allowedWidth = 240;
		OnscreenMessages m_screenPosition;

		MessageMode m_mode = MessageNone;
		Uint8 m_red = 255;
		Uint8 m_green = 255;
		Uint8 m_blue = 255;
		bool m_dirty = true;
};

#endif /* __FILE_SCREENTEXT_h_ */
