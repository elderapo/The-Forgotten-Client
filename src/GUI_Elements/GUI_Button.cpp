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

#include "GUI_Button.h"
#include "../engine.h"

extern Engine g_engine;

GUI_Button::GUI_Button(iRect boxRect, const std::string labelName, Uint32 internalID, const std::string description)
{
	m_sx[0] = m_sx[1] = 0;
	m_sy[0] = m_sy[1] = 0;

	setRect(boxRect);
	m_description = std::move(description);
	m_label = std::move(labelName);
	m_startX = g_engine.calculateFontWidth(CLIENT_FONT_SMALL, m_label)/2;
	m_pressed = 0;
	m_evtParam = 0;
	m_eventHandlerFunction = NULL;
	m_internalID = internalID;
}

void GUI_Button::setRect(iRect& NewRect)
{
	switch(NewRect.x2)
	{
		case GUI_UI_BUTTON_34PX_GRAY_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_34PX_GRAY_UP_X;
			m_sy[0] = GUI_UI_BUTTON_34PX_GRAY_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_34PX_GRAY_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_34PX_GRAY_DOWN_Y;
		}
		break;
		case GUI_UI_BUTTON_42PX_GRAY_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_42PX_GRAY_UP_X;
			m_sy[0] = GUI_UI_BUTTON_42PX_GRAY_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_42PX_GRAY_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_42PX_GRAY_DOWN_Y;
		}
		break;
		case GUI_UI_BUTTON_43PX_GRAY_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_43PX_GRAY_UP_X;
			m_sy[0] = GUI_UI_BUTTON_43PX_GRAY_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_43PX_GRAY_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_43PX_GRAY_DOWN_Y;
		}
		break;
		case GUI_UI_BUTTON_43PX_GREEN_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_43PX_GREEN_UP_X;
			m_sy[0] = GUI_UI_BUTTON_43PX_GREEN_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_43PX_GREEN_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_43PX_GREEN_DOWN_Y;
			NewRect.x2 = 43;//width correction
		}
		break;
		case GUI_UI_BUTTON_43PX_RED_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_43PX_RED_UP_X;
			m_sy[0] = GUI_UI_BUTTON_43PX_RED_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_43PX_RED_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_43PX_RED_DOWN_Y;
			NewRect.x2 = 43;//width correction
		}
		break;
		case GUI_UI_BUTTON_48PX_GRAY_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_48PX_GRAY_UP_X;
			m_sy[0] = GUI_UI_BUTTON_48PX_GRAY_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_48PX_GRAY_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_48PX_GRAY_DOWN_Y;
		}
		break;
		case GUI_UI_BUTTON_58PX_GRAY_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_58PX_GRAY_UP_X;
			m_sy[0] = GUI_UI_BUTTON_58PX_GRAY_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_58PX_GRAY_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_58PX_GRAY_DOWN_Y;
		}
		break;
		case GUI_UI_BUTTON_75PX_GRAY_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_75PX_GRAY_UP_X;
			m_sy[0] = GUI_UI_BUTTON_75PX_GRAY_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_75PX_GRAY_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_75PX_GRAY_DOWN_Y;
		}
		break;
		case GUI_UI_BUTTON_86PX_GRAY_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_86PX_GRAY_UP_X;
			m_sy[0] = GUI_UI_BUTTON_86PX_GRAY_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_86PX_GRAY_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_86PX_GRAY_DOWN_Y;
		}
		break;
		case GUI_UI_BUTTON_86PX_GREEN_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_86PX_GREEN_UP_X;
			m_sy[0] = GUI_UI_BUTTON_86PX_GREEN_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_86PX_GREEN_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_86PX_GREEN_DOWN_Y;
			NewRect.x2 = 86;//width correction
		}
		break;
		case GUI_UI_BUTTON_86PX_GOLD_UP_W:
		{
			m_sx[0] = GUI_UI_BUTTON_86PX_GOLD_UP_X;
			m_sy[0] = GUI_UI_BUTTON_86PX_GOLD_UP_Y;
			m_sx[1] = GUI_UI_BUTTON_86PX_GOLD_DOWN_X;
			m_sy[1] = GUI_UI_BUTTON_86PX_GOLD_DOWN_Y;
			NewRect.x2 = 86;//width correction
		}
		break;
		default:
		{
			m_sx[0] = GUI_UI_BACKGROUND_GREY_X;
			m_sy[0] = GUI_UI_BACKGROUND_GREY_Y;
			m_sx[1] = GUI_UI_BACKGROUND_GREY_X;
			m_sy[1] = GUI_UI_BACKGROUND_GREY_Y;
		}
		break;
	}
	m_tRect = NewRect;
}

void GUI_Button::setButtonEventCallback(void (*eventHandlerFunction)(Uint32,Sint32), Uint32 mEvent)
{
	m_evtParam = mEvent;
	m_eventHandlerFunction = eventHandlerFunction;
}

void GUI_Button::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	bool inside = (isInsideParent && m_tRect.isPointInside(x, y));
	if(m_pressed > 0)
	{
		if(m_pressed == 1 && !inside)
			m_pressed = 2;
		else if(m_pressed == 2 && inside)
			m_pressed = 1;
	}
	if(!m_description.empty() && inside)
		g_engine.showDescription(x, y, m_description);
}

void GUI_Button::onLMouseDown(Sint32, Sint32)
{
	m_pressed = 1;
}

void GUI_Button::onLMouseUp(Sint32, Sint32)
{
	Uint8 pressed = m_pressed;
	if(pressed > 0)
	{
		m_pressed = 0;
		if(pressed == 1)
		{
			if(m_eventHandlerFunction)
				UTIL_SafeEventHandler(m_eventHandlerFunction, m_evtParam, 1);
		}
	}
}

void GUI_Button::render()
{
	bool pressed = (m_pressed == 1 ? true : false);

	Surface* renderer = g_engine.getRender();
	renderer->drawPicture(GUI_UI_IMAGE, m_sx[(pressed ? 1 : 0)], m_sy[(pressed ? 1 : 0)], m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
	g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1+(m_tRect.x2/2)+(pressed ? 1 : 0)-m_startX, m_tRect.y1+(pressed ? 7 : 6), m_label, 255, 255, 255, CLIENT_FONT_ALIGN_LEFT);
}

GUI_RadioButton::GUI_RadioButton(iRect boxRect, const std::string labelName, Uint32 internalID, const std::string description):
	GUI_Button(boxRect, labelName, internalID, description)
{
	m_eventRadioChecked = NULL;
}

void GUI_RadioButton::setRadioEventCallback(bool (*eventRadioChecked)(void), const std::string description)
{
	m_eventRadioChecked = eventRadioChecked;
	m_radioDescription = std::move(description);
}

void GUI_RadioButton::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	bool inside = (isInsideParent && m_tRect.isPointInside(x, y));
	if(m_pressed > 0)
	{
		if(m_pressed == 1 && !inside)
			m_pressed = 2;
		else if(m_pressed == 2 && inside)
			m_pressed = 1;
	}
	if(inside)
	{
		if(!m_radioDescription.empty() && m_eventRadioChecked && m_eventRadioChecked())
			g_engine.showDescription(x, y, m_radioDescription);
		else if(!m_description.empty())
			g_engine.showDescription(x, y, m_description);
	}
}

void GUI_RadioButton::render()
{
	bool pressed = (m_pressed == 1 ? true : false);
	if(m_eventRadioChecked && m_eventRadioChecked())
		pressed = true;
	
	Surface* renderer = g_engine.getRender();
	renderer->drawPicture(GUI_UI_IMAGE, m_sx[(pressed ? 1 : 0)], m_sy[(pressed ? 1 : 0)], m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
	g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1+(m_tRect.x2/2)+(pressed ? 1 : 0)-m_startX, m_tRect.y1+(pressed ? 7 : 6), m_label, 255, 255, 255, CLIENT_FONT_ALIGN_LEFT);
}
