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

#include "engine.h"
#include "surfaceSoftware.h"
#include "surfaceDirect3D9.h"
#include "surfaceDirect3D11.h"
#include "surfaceDirectDraw.h"
#include "surfaceOpenglCore.h"
#include "surfaceOpengl.h"
#include "surfaceOpengles.h"
#include "surfaceOpengles2.h"
#include "surfaceVulkan.h"
#include "item.h"
#include "map.h"
#include "tile.h"
#include "thingManager.h"
#include "spriteManager.h"
#include "animator.h"
#include "protocollogin.h"
#include "protocolgame.h"
#include "creature.h"
#include "game.h"
#include "config.h"

#include "GUI_Elements/GUI_Window.h"
#include "GUI_Elements/GUI_Panel.h"
#include "GUI_Elements/GUI_PanelWindow.h"
#include "GUI_Elements/GUI_Description.h"
#include "GUI_Elements/GUI_ContextMenu.h"
#include "GUI_Elements/GUI_Log.h"
#include "GUI/itemUI.h"
#include "GUI/Chat.h"

extern Engine g_engine;
extern Map g_map;
extern ThingManager g_thingManager;
extern SpriteManager g_spriteManager;
extern Connection* g_connection;
extern GUI_Log g_logger;
extern Game g_game;
extern Chat g_chat;

extern bool g_running;
extern bool g_inited;
extern Sint32 g_actualCursor;
extern Uint32 g_frameTime;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;
extern Uint16 g_ping;

GUI_Window* g_mainWindow = NULL;

Engine::Engine()
{
	m_engines.emplace_back(CLIENT_ENGINE_SOFTWARE);
	#if defined(SDL_VIDEO_VULKAN)
	m_engines.emplace_back(CLIENT_ENGINE_VULKAN);
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D11)
	m_engines.emplace_back(CLIENT_ENGINE_DIRECT3D11);
	#endif
	#if defined(SDL_VIDEO_RENDER_DDRAW)
	m_engines.emplace_back(CLIENT_ENGINE_DIRECT3D7);
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL)
	m_engines.emplace_back(CLIENT_ENGINE_OPENGLCORE);
	m_engines.emplace_back(CLIENT_ENGINE_OPENGL);
	#else
	#if defined(SDL_VIDEO_RENDER_OGL_ES)
	m_engines.emplace_back(CLIENT_ENGINE_OPENGLES);
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES2)
	m_engines.emplace_back(CLIENT_ENGINE_OPENGLES2);
	#endif
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D)
	m_engines.emplace_back(CLIENT_ENGINE_DIRECT3D);
	#endif

	m_engine = m_engines.back();
	for(Sint32 i = Skills_Fist; i < Skills_LastSkill; ++i)
		m_showSkillsBar[i] = true;

	m_leftPanel = GUI_PANEL_RANDOM;
	m_rightPanel = GUI_PANEL_MAIN;
}

void Engine::loadCFG()
{
	Config cfg;
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%sconfig.cfg", g_prefPath.c_str());
	if(cfg.openToRead(g_buffer))
	{
		std::string data = cfg.fetchKey("Version");
		if(data.empty())
			return;
		#if !(CLIENT_OVVERIDE_VERSION > 0)
		else
			g_clientVersion = SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10));

		data = cfg.fetchKey("Host");
		m_clientHost.assign(data);
		data = cfg.fetchKey("Port");
		m_clientPort.assign(data);
		data = cfg.fetchKey("Proxy");
		m_clientProxy.assign(data);
		data = cfg.fetchKey("ProxyAuth");
		m_clientProxyAuth.assign(data);
		#endif

		data = cfg.fetchKey("Engine");
		m_engine = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		data = cfg.fetchKey("BasicBrightness");
		m_lightAmbient = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_lightAmbient = (m_lightAmbient > 100 ? 100 : m_lightAmbient);
		data = cfg.fetchKey("LevelSeparator");
		m_levelSeparator = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_levelSeparator = (m_levelSeparator > 100 ? 100 : m_levelSeparator);
		data = cfg.fetchKey("LightMode");
		m_lightMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_lightMode = (m_lightMode > CLIENT_LIGHT_MODE_NEW ? CLIENT_LIGHT_MODE_OLD : m_lightMode);

		data = cfg.fetchKey("WindowedMode");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());

			StringVector windowData = UTIL_explodeString(data, ",");
			if(windowData.size() == 5)
			{
				m_windowX = SDL_static_cast(Sint32, SDL_strtol(windowData[0].c_str(), NULL, 10));
				m_windowY = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
				m_windowW = SDL_static_cast(Sint32, SDL_strtol(windowData[2].c_str(), NULL, 10));
				m_windowH = SDL_static_cast(Sint32, SDL_strtol(windowData[3].c_str(), NULL, 10));
				m_windowCachedW = m_windowW;
				m_windowCachedH = m_windowH;
				m_maximized = (windowData[4] == "yes" ? true : false);
			}
		}
		data = cfg.fetchKey("Fullscreen");
		m_fullscreen = (data == "yes" ? true : false);
		data = cfg.fetchKey("FullscreenMode");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());

			StringVector windowData = UTIL_explodeString(data, ",");
			if(windowData.size() == 3)
			{
				m_fullScreenWidth = SDL_static_cast(Sint32, SDL_strtol(windowData[0].c_str(), NULL, 10));
				m_fullScreenHeight = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
				m_fullScreenBits = SDL_static_cast(Sint32, SDL_strtol(windowData[2].c_str(), NULL, 10));
			}
			else if(windowData.size() == 4)
			{
				m_fullScreenWidth = SDL_static_cast(Sint32, SDL_strtol(windowData[0].c_str(), NULL, 10));
				m_fullScreenHeight = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
				m_fullScreenBits = SDL_static_cast(Sint32, SDL_strtol(windowData[2].c_str(), NULL, 10));
				m_fullScreenHZ = SDL_static_cast(Sint32, SDL_strtol(windowData[3].c_str(), NULL, 10));
			}
		}

		data = cfg.fetchKey("ConfineFramerate");
		SDL_setFramerate(&g_fpsmanager, SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10)));
		data = cfg.fetchKey("NolimitFramerate");
		m_unlimitedFPS = (data == "yes" ? true : false);

		data = cfg.fetchKey("VerticalSync");
		m_vsync = (data == "yes" ? true : false);
		data = cfg.fetchKey("Sharpening");
		m_sharpening = (data == "yes" ? true : false);
		data = cfg.fetchKey("Antialiasing");
		m_antialiasing = (data == "yes" ? CLIENT_ANTIALIASING_NORMAL : data == "integer" ? CLIENT_ANTIALIASING_INTEGER : CLIENT_ANTIALIASING_NONE);
		m_antialiasing = (m_antialiasing > CLIENT_ANTIALIASING_INTEGER ? CLIENT_ANTIALIASING_NORMAL : m_antialiasing);

		data = cfg.fetchKey("ClassicControl");
		m_classicControl = (data == "yes" ? true : false);
		data = cfg.fetchKey("AutoChaseOff");
		m_autoChaseOff = (data == "yes" ? true : false);
		data = cfg.fetchKey("CreatureInfo");
		m_showNames = (SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10)) > 0 ? true : false);
		data = cfg.fetchKey("CreatureMarks");
		m_showMarks = (data == "yes" ? true : false);
		data = cfg.fetchKey("CreaturePvPFrames");
		m_showPvPFrames = (data == "yes" ? true : false);
		data = cfg.fetchKey("CreatureIcons");
		m_showIcons = (data == "yes" ? true : false);
		data = cfg.fetchKey("TextualEffects");
		m_showTextualEffects = (data == "yes" ? true : false);
		data = cfg.fetchKey("CooldownBar");
		m_showCooldown = (data == "yes" ? true : false);

		data = cfg.fetchKey("InfoMessages");
		m_showInfoMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("EventMessages");
		m_showEventMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("StatusMessages");
		m_showStatusMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("StatusMsgOfOthers");
		m_showStatusOthersMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("TimeStamps");
		m_showTimestamps = (data == "yes" ? true : false);
		data = cfg.fetchKey("Levels");
		m_showLevels = (data == "yes" ? true : false);
		data = cfg.fetchKey("PrivateMessages");
		m_showPrivateMessages = (data == "yes" ? true : false);

		data = cfg.fetchKey("AttackMode");
		m_attackMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10)) + 1;
		m_attackMode = (m_attackMode > ATTACKMODE_DEFENSE ? ATTACKMODE_BALANCED : m_attackMode);
		data = cfg.fetchKey("ChaseMode");
		m_chaseMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_chaseMode = (m_chaseMode > CHASEMODE_FOLLOW ? CHASEMODE_STAND : m_chaseMode);
		data = cfg.fetchKey("SecureMode");
		m_secureMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_secureMode = (m_secureMode > SECUREMODE_UNSECURE ? SECUREMODE_SECURE : m_secureMode);
		data = cfg.fetchKey("PvpMode");
		m_pvpMode = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_pvpMode = (m_pvpMode > PVPMODE_RED_FIST ? PVPMODE_DOVE : m_secureMode);

		data = cfg.fetchKey("LastMotD");
		m_motdNumber = SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10));
		data = cfg.fetchKey("AutomapZoom");
		g_game.minimapSetZoom(SDL_static_cast(Sint32, SDL_strtol(data.c_str(), NULL, 10)));
		data = cfg.fetchKey("ConsoleHeight");
		m_consoleHeight = SDL_static_cast(Sint32, SDL_strtol(data.c_str(), NULL, 10));

		data = cfg.fetchKey("ActivatedIgnoreList");
		m_activatedBlackList = (data == "yes" ? true : false);
		data = cfg.fetchKey("ActivatedWhiteList");
		m_activatedWhiteList = (data == "yes" ? true : false);
		data = cfg.fetchKey("IgnoreYelling");
		m_ignoreYellingMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("IgnorePrivateMessages");
		m_ignorePrivateMessages = (data == "yes" ? true : false);
		data = cfg.fetchKey("AllowVipMessages");
		m_allowVipMessages = (data == "yes" ? true : false);

		data = cfg.fetchKey("White");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());
			m_whiteList.clear();

			StringVector whiteListElements = UTIL_explodeString(data, ",");
			for(StringVector::iterator it = whiteListElements.begin(), end = whiteListElements.end(); it != end; ++it)
			{
				std::string& whiteListElement = (*it);
				if(whiteListElement.back() == '"')
					whiteListElement.pop_back();
				if(whiteListElement.front() == '"')
					whiteListElement.erase(whiteListElement.begin());

				m_whiteList[whiteListElement] = true;
			}
		}
		data = cfg.fetchKey("Ignore");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());
			m_blackList.clear();

			StringVector blackListElements = UTIL_explodeString(data, ",");
			for(StringVector::iterator it = blackListElements.begin(), end = blackListElements.end(); it != end; ++it)
			{
				std::string& blackListElement = (*it);
				if(blackListElement.back() == '"')
					blackListElement.pop_back();
				if(blackListElement.front() == '"')
					blackListElement.erase(blackListElement.begin());

				m_blackList[blackListElement] = true;
			}
		}
		//data = cfg.fetchKey("KnownTutorialHints");

		data = cfg.fetchKey("LevelBar");
		m_showLevelBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("StaminaBar");
		m_showStaminaBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("MagLevelBar");
		m_showMagLevelBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("OfflineTrainingBar");
		m_showTrainingBar = (data == "yes" ? true : false);
		data = cfg.fetchKey("SkillBars");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());
			for(Sint32 i = Skills_Fist; i < Skills_LastSkill; ++i)
				m_showSkillsBar[i] = false;

			StringVector skillsData = UTIL_explodeString(data, ",");
			for(StringVector::iterator it = skillsData.begin(), end = skillsData.end(); it != end; ++it)
			{
				Uint32 skillId = SDL_static_cast(Uint32, SDL_strtoul((*it).c_str(), NULL, 10));
				if(skillId < Skills_LastSkill)
					m_showSkillsBar[skillId] = true;
			}
		}

		StringVector vectorData = cfg.fetchKeys("ContentWindow");
		for(StringVector::iterator it = vectorData.begin(), end = vectorData.end(); it != end; ++it)
		{
			data = (*it);
			if(data.size() > 2)
			{
				data.pop_back();
				data.erase(data.begin());

				StringVector windowData = UTIL_explodeString(data, ",");
				if(windowData.size() == 2)
					m_contentWindows[SDL_static_cast(Uint32, SDL_strtoul(windowData[0].c_str(), NULL, 10))] = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
				else if(windowData.size() == 3)
				{
					Uint32 windowId = SDL_static_cast(Uint32, SDL_strtoul(windowData[0].c_str(), NULL, 10));
					m_contentWindows[windowId] = SDL_static_cast(Sint32, SDL_strtol(windowData[1].c_str(), NULL, 10));
					m_parentWindows[windowId] = SDL_static_cast(Sint32, SDL_strtol(windowData[2].c_str(), NULL, 10));
				}
			}
		}

		vectorData = cfg.fetchKeys("OpenDialogs");
		for(StringVector::iterator it = vectorData.begin(), end = vectorData.end(); it != end; ++it)
		{
			data = (*it);
			m_openDialogs.push_back(SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10)));
		}

		vectorData = cfg.fetchKeys("Buddy");
		for(StringVector::iterator it = vectorData.begin(), end = vectorData.end(); it != end; ++it)
		{
			data = (*it);
			if(data.size() > 2)
			{
				data.pop_back();
				data.erase(data.begin());

				StringVector windowData = UTIL_explodeString(data, ",");
				if(windowData.size() == 4)
				{
					Uint32 playerGUID = SDL_static_cast(Uint32, SDL_strtoul(windowData[0].c_str(), NULL, 10));
					Uint32 iconId = SDL_static_cast(Uint32, SDL_strtoul(windowData[1].c_str(), NULL, 10));
					if(windowData[2].back() == '"')
						windowData[2].pop_back();
					if(windowData[2].front() == '"')
						windowData[2].erase(windowData[2].begin());

					setVipData(playerGUID, windowData[2], iconId, (windowData[3] == "yes" ? true : false));
				}
			}
		}

		data = cfg.fetchKey("BattleSortmethod");
		m_battleSortMethod = SDL_static_cast(SortMethods, SDL_strtoul(data.c_str(), NULL, 10));
		m_battleSortMethod = (m_battleSortMethod > Sort_Descending_Name ? Sort_Ascending_Time : m_battleSortMethod);
		data = cfg.fetchKey("BuddySortmethods");
		if(data.size() > 2)
		{
			data.pop_back();
			data.erase(data.begin());

			StringVector sortData = UTIL_explodeString(data, ",");
			if(sortData.size() == 3)
				m_buddySortmethod = SDL_static_cast(VipSortMethods, SDL_strtoul(sortData[0].c_str(), NULL, 10));
		}
		data = cfg.fetchKey("BuddyHideOffline");
		m_buddyHideOffline = (data == "yes" ? true : false);
		data = cfg.fetchKey("BuddyHideGroups");
		m_buddyHideGroups = (data == "yes" ? true : false);

		data = cfg.fetchKey("SortOrderBuy");
		m_buySortMethod = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_buySortMethod = (m_buySortMethod > Shop_Sort_Weight ? Shop_Sort_Name : m_buySortMethod);
		data = cfg.fetchKey("SortOrderSell");
		m_sellSortMethod = SDL_static_cast(Uint8, SDL_strtoul(data.c_str(), NULL, 10));
		m_sellSortMethod = (m_sellSortMethod > Shop_Sort_Weight ? Shop_Sort_Name : m_sellSortMethod);
		data = cfg.fetchKey("BuyWithBackpacks");
		m_buyWithBackpacks = (data == "yes" ? true : false);
		data = cfg.fetchKey("IgnoreCapacity");
		m_ignoreCapacity = (data == "yes" ? true : false);
		data = cfg.fetchKey("SellEquipped");
		m_ignoreEquiped = (data == "yes" ? false : true);

		data = cfg.fetchKey("LeftSidebars");
		m_leftPanel = SDL_static_cast(Sint32, SDL_strtol(data.c_str(), NULL, 10));
		m_leftPanel = (m_leftPanel == 0 ? GUI_PANEL_RANDOM : m_leftPanel + GUI_PANEL_EXTRA_LEFT_START - 1);
		data = cfg.fetchKey("RightSidebars");
		m_rightPanel = SDL_static_cast(Sint32, SDL_strtol(data.c_str(), NULL, 10));
		m_rightPanel = (m_rightPanel == 0 ? GUI_PANEL_MAIN : m_rightPanel + GUI_PANEL_EXTRA_RIGHT_START - 1);
	}
	#if CLIENT_OVVERIDE_VERSION > 0
	g_clientVersion = CLIENT_OVERRIDE_PROTOCOL_VERSION;
	g_game.clientChangeVersion(CLIENT_OVERRIDE_PROTOCOL_VERSION, CLIENT_OVERRIDE_FILE_VERSION);
	#else
	g_game.clientChangeVersion(g_clientVersion, g_clientVersion);
	#endif
}

void Engine::saveCFG()
{
	Config cfg;
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%sconfig.cfg", g_prefPath.c_str());
	if(cfg.openToSave(g_buffer))
	{
		#if CLIENT_OVVERIDE_VERSION > 0
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", CLIENT_OVVERIDE_VERSION);
		#else
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", g_clientVersion);
		#endif
		cfg.insertKey("Version", std::string(g_buffer, SDL_static_cast(size_t, len)));

		#if !(CLIENT_OVVERIDE_VERSION > 0)
		cfg.insertKey("Host", m_clientHost);
		cfg.insertKey("Port", m_clientPort);
		cfg.insertKey("Proxy", m_clientProxy);
		cfg.insertKey("ProxyAuth", m_clientProxyAuth);
		#endif

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_engine));
		cfg.insertKey("Engine", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", "yes");
		cfg.insertKey("LightEffects", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_lightAmbient));
		cfg.insertKey("BasicBrightness", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_levelSeparator));
		cfg.insertKey("LevelSeparator", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_lightMode));
		cfg.insertKey("LightMode", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%d,%d,%d,%d,%s)", m_windowX, m_windowY, m_windowCachedW, m_windowCachedH, (m_maximized ? "yes" : "no"));
		cfg.insertKey("WindowedMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_fullscreen ? "yes" : "no"));
		cfg.insertKey("Fullscreen", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%d,%d,%d,%d)", m_fullScreenWidth, m_fullScreenHeight, m_fullScreenBits, m_fullScreenHZ);
		cfg.insertKey("FullscreenMode", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", g_fpsmanager.rate);
		cfg.insertKey("ConfineFramerate", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_unlimitedFPS ? "yes" : "no"));
		cfg.insertKey("NolimitFramerate", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_vsync ? "yes" : "no"));
		cfg.insertKey("VerticalSync", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_sharpening ? "yes" : "no"));
		cfg.insertKey("Sharpening", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_antialiasing == CLIENT_ANTIALIASING_NORMAL ? "yes" : m_antialiasing == CLIENT_ANTIALIASING_INTEGER ? "integer" : "no"));
		cfg.insertKey("Antialiasing", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_classicControl ? "yes" : "no"));
		cfg.insertKey("ClassicControl", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_autoChaseOff ? "yes" : "no"));
		cfg.insertKey("AutoChaseOff", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", (m_showNames ? 2 : 0));
		cfg.insertKey("CreatureInfo", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showMarks ? "yes" : "no"));
		cfg.insertKey("CreatureMarks", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showPvPFrames ? "yes" : "no"));
		cfg.insertKey("CreaturePvPFrames", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showIcons ? "yes" : "no"));
		cfg.insertKey("CreatureIcons", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showTextualEffects ? "yes" : "no"));
		cfg.insertKey("TextualEffects", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showCooldown ? "yes" : "no"));
		cfg.insertKey("CooldownBar", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showInfoMessages ? "yes" : "no"));
		cfg.insertKey("InfoMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showEventMessages ? "yes" : "no"));
		cfg.insertKey("EventMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showStatusMessages ? "yes" : "no"));
		cfg.insertKey("StatusMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showStatusOthersMessages ? "yes" : "no"));
		cfg.insertKey("StatusMsgOfOthers", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showTimestamps ? "yes" : "no"));
		cfg.insertKey("TimeStamps", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showLevels ? "yes" : "no"));
		cfg.insertKey("Levels", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showPrivateMessages ? "yes" : "no"));
		cfg.insertKey("PrivateMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_attackMode - 1));
		cfg.insertKey("AttackMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_chaseMode));
		cfg.insertKey("ChaseMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_secureMode));
		cfg.insertKey("SecureMode", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_pvpMode));
		cfg.insertKey("PvpMode", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", m_motdNumber);
		cfg.insertKey("LastMotD", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", g_game.minimapGetZoom());
		cfg.insertKey("AutomapZoom", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%d", m_consoleHeight);
		cfg.insertKey("ConsoleHeight", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_activatedBlackList ? "yes" : "no"));
		cfg.insertKey("ActivatedIgnoreList", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_activatedWhiteList ? "yes" : "no"));
		cfg.insertKey("ActivatedWhiteList", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_ignoreYellingMessages ? "yes" : "no"));
		cfg.insertKey("IgnoreYelling", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_ignorePrivateMessages ? "yes" : "no"));
		cfg.insertKey("IgnorePrivateMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_allowVipMessages ? "yes" : "no"));
		cfg.insertKey("AllowVipMessages", std::string(g_buffer, SDL_static_cast(size_t, len)));

		bool itFirst = true;
		std::string listString("{");
		for(std::unordered_map<std::string, bool>::iterator it = m_whiteList.begin(), end = m_whiteList.end(); it != end; ++it)
		{
			if(it->second)
			{
				if(itFirst)
					itFirst = false;
				else
					listString.append(1, ',');

				listString.append(1, '"');
				listString.append(it->first);
				listString.append(1, '"');
			}
		}
		listString.append(1, '}');
		cfg.insertKey("White", listString);
		itFirst = true;
		listString.assign("{");
		for(std::unordered_map<std::string, bool>::iterator it = m_blackList.begin(), end = m_blackList.end(); it != end; ++it)
		{
			if(it->second)
			{
				if(itFirst)
					itFirst = false;
				else
					listString.append(1, ',');

				listString.append(1, '"');
				listString.append(it->first);
				listString.append(1, '"');
			}
		}
		listString.append(1, '}');
		cfg.insertKey("Ignore", listString);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "{}");
		cfg.insertKey("KnownTutorialHints", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showLevelBar ? "yes" : "no"));
		cfg.insertKey("LevelBar", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showStaminaBar ? "yes" : "no"));
		cfg.insertKey("StaminaBar", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showMagLevelBar ? "yes" : "no"));
		cfg.insertKey("MagLevelBar", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_showTrainingBar ? "yes" : "no"));
		cfg.insertKey("OfflineTrainingBar", std::string(g_buffer, SDL_static_cast(size_t, len)));

		itFirst = true;
		listString.assign("{");
		for(Sint32 i = Skills_Fist; i < Skills_LastSkill; ++i)
		{
			if(m_showSkillsBar[i])
			{
				if(itFirst)
					itFirst = false;
				else
					listString.append(1, ',');

				listString.append(std::to_string(i));
			}
		}
		listString.append(1, '}');
		cfg.insertKey("SkillBars", listString);
		for(std::map<Uint32, Sint32>::iterator it = m_contentWindows.begin(), end = m_contentWindows.end(); it != end; ++it)
		{
			std::map<Uint32, Sint32>::iterator pit = m_parentWindows.find(it->first);
			if(pit != m_parentWindows.end())
				len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%u,%d,%d)", it->first, it->second, pit->second);
			else
				len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%u,%d)", it->first, it->second);

			cfg.insertKey("ContentWindow", std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		for(std::vector<Uint32>::iterator it = m_openDialogs.begin(), end = m_openDialogs.end(); it != end; ++it)
		{
			len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", (*it));
			cfg.insertKey("OpenDialogs", std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		for(std::map<Uint32, VipData>::iterator it = m_vipData.begin(), end = m_vipData.end(); it != end; ++it)
		{
			VipData& vip = it->second;
			len = SDL_snprintf(g_buffer, sizeof(g_buffer), "(%u,%u,\"%s\",%s)", it->first, vip.iconId, vip.description.c_str(), (vip.notifyLogin ? "yes" : "no"));
			cfg.insertKey("Buddy", std::string(g_buffer, SDL_static_cast(size_t, len)));
		}

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_battleSortMethod));
		cfg.insertKey("BattleSortmethod", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_buddySortmethod == 0 ? "(0,1,2)" : m_buddySortmethod == 1 ? "(1,2,0)" : "(2,0,1)"));
		cfg.insertKey("BuddySortmethods", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_buddyHideOffline ? "yes" : "no"));
		cfg.insertKey("BuddyHideOffline", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", (m_buddyHideGroups ? "yes" : "no"));
		cfg.insertKey("BuddyHideGroups", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_buySortMethod));
		cfg.insertKey("SortOrderBuy", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, m_sellSortMethod));
		cfg.insertKey("SortOrderSell", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), (m_buyWithBackpacks ? "yes" : "no"));
		cfg.insertKey("BuyWithBackpacks", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), (m_ignoreCapacity ? "yes" : "no"));
		cfg.insertKey("IgnoreCapacity", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), (m_ignoreEquiped ? "no" : "yes"));
		cfg.insertKey("SellEquipped", std::string(g_buffer, SDL_static_cast(size_t, len)));

		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%d", (m_leftPanel == GUI_PANEL_RANDOM ? 0 : m_leftPanel - GUI_PANEL_EXTRA_LEFT_START + 1));
		cfg.insertKey("LeftSidebars", std::string(g_buffer, SDL_static_cast(size_t, len)));
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%d", (m_rightPanel == GUI_PANEL_MAIN ? 0 : m_rightPanel - GUI_PANEL_EXTRA_RIGHT_START + 1));
		cfg.insertKey("RightSidebars", std::string(g_buffer, SDL_static_cast(size_t, len)));
	}
}

void Engine::attachFullScreenInfo()
{
	SDL_DisplayMode displayMode;
	Sint32 displayIndex = SDL_GetWindowDisplayIndex(m_window);
	Sint32 displayModes = SDL_GetNumDisplayModes(displayIndex);
	for(Sint32 i = 0; i < displayModes; ++i)
	{
		Sint32 reportBits;
		SDL_GetDisplayMode(displayIndex, i, &displayMode);
		if(m_fullScreenWidth == displayMode.w && m_fullScreenHeight == displayMode.h && m_fullScreenHZ == displayMode.refresh_rate)
		{
			if(displayMode.format == SDL_PIXELFORMAT_UNKNOWN && m_fullScreenBits == 32)
			{
				SDL_SetWindowDisplayMode(m_window, &displayMode);
				return;
			}
			else
			{
				reportBits = SDL_BITSPERPIXEL(displayMode.format);
				if(reportBits == 24)
					reportBits = SDL_BYTESPERPIXEL(displayMode.format) * 8;
				if(m_fullScreenBits == reportBits)
				{
					SDL_SetWindowDisplayMode(m_window, &displayMode);
					return;
				}
			}
		}
	}
}

void Engine::run()
{
	SDL_DisplayMode displayMode;
	if(SDL_GetDesktopDisplayMode(0, &displayMode) == 0)
	{
		m_fullScreenWidth = displayMode.w;
		m_fullScreenHeight = displayMode.h;
		m_fullScreenHZ = displayMode.refresh_rate;
		if(displayMode.format == SDL_PIXELFORMAT_UNKNOWN)
			m_fullScreenBits = 32;
		else
		{
			m_fullScreenBits = SDL_BITSPERPIXEL(displayMode.format);
			if(m_fullScreenBits == 24)
				m_fullScreenBits = SDL_BYTESPERPIXEL(displayMode.format) * 8;
		}
	}
	loadCFG();

	Uint32 windowflags = (SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	if(m_fullscreen)
	{
		windowflags |= SDL_WINDOW_FULLSCREEN;
		m_windowW = m_fullScreenWidth;
		m_windowH = m_fullScreenHeight;
	}
	else if(m_maximized)
	{
		windowflags |= SDL_WINDOW_MAXIMIZED;
		m_windowW = m_windowCachedW;
		m_windowH = m_windowCachedH;
	}

	m_window = SDL_CreateWindow(PRODUCT_NAME, m_windowX, m_windowY, m_windowW, m_windowH, windowflags);
	if(!m_window)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Couldn't create SDL Window: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		g_running = false;
		return;
	}

	if(m_fullscreen)
	{
		attachFullScreenInfo();
		SDL_GetWindowDisplayMode(m_window, &displayMode);
		m_windowW = displayMode.w;
		m_windowH = displayMode.h;
	}
	else
	{
		SDL_GetWindowSize(m_window, &m_windowW, &m_windowH);
		SDL_SetWindowMinimumSize(m_window, GAME_MINIMUM_WIDTH, GAME_MINIMUM_HEIGHT);
	}
	m_windowId = SDL_GetWindowID(m_window);
	m_controlFPS = (!m_unlimitedFPS && !m_vsync);

	//createwindow don't generate resize event so send one ourselves
	UTIL_ResizeEvent(m_windowId, m_windowW, m_windowH);

	initFont(CLIENT_FONT_NONOUTLINED, 256, 128, 32, 7, 8, 16, 14);
	initFont(CLIENT_FONT_OUTLINED, 512, 128, 32, 7, 16, 16, 16);
	initFont(CLIENT_FONT_SMALL, 256, 64, 32, 7, 8, 8, 8);

	UTIL_createMainWindow();
	resetToDefaultHotkeys(false);
}

void Engine::terminate()
{
	if(m_window && !m_fullscreen)
	{
		//Some disgusting "hack" to save original window parameters
		//TOOD: do it the proper way
		if(m_maximized)
			SDL_RestoreWindow(m_window);

		SDL_GetWindowPosition(m_window, &m_windowX, &m_windowY);
		SDL_GetWindowSize(m_window, &m_windowW, &m_windowH);
		m_windowCachedW = m_windowW;
		m_windowCachedH = m_windowH;
	}
	clearWindows();
	clearPanels();
	checkReleaseQueue();
	m_surface.reset();

	if(m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = NULL;
	}
	saveCFG();
}

void Engine::parseCommands(int argc, char* argv[])
{
	//Does argv[0] on all platforms is the path to our application?
	for(int i = 1; i < argc; ++i)
	{
		if(SDL_strcasecmp(argv[i], "-force-d3d11") == 0)
			m_engine = CLIENT_ENGINE_DIRECT3D11;
		else if(SDL_strcasecmp(argv[i], "-force-d3d9") == 0)
			m_engine = CLIENT_ENGINE_DIRECT3D;
		else if(SDL_strcasecmp(argv[i], "-force-d3d7") == 0)
			m_engine = CLIENT_ENGINE_DIRECT3D7;
		else if(SDL_strcasecmp(argv[i], "-force-vulkan") == 0)
			m_engine = CLIENT_ENGINE_VULKAN;
		else if(SDL_strcasecmp(argv[i], "-force-opengl-core") == 0)
			m_engine = CLIENT_ENGINE_OPENGLCORE;
		else if(SDL_strcasecmp(argv[i], "-force-opengl") == 0 || SDL_strcasecmp(argv[i], "-force-opengl-legacy") == 0)
			m_engine = CLIENT_ENGINE_OPENGL;
		else if(SDL_strcasecmp(argv[i], "-force-opengles") == 0)
			m_engine = CLIENT_ENGINE_OPENGLES;
		else if(SDL_strcasecmp(argv[i], "-force-opengles2") == 0)
			m_engine = CLIENT_ENGINE_OPENGLES2;
		else if(SDL_strcasecmp(argv[i], "-force-software") == 0)
			m_engine = CLIENT_ENGINE_SOFTWARE;
	}
}

bool Engine::RecreateWindow(bool vulkan, bool opengl)
{
	if(m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = NULL;
	}

	Uint32 windowflags = (SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	if(m_fullscreen)
	{
		windowflags |= SDL_WINDOW_FULLSCREEN;
		m_windowW = m_fullScreenWidth;
		m_windowH = m_fullScreenHeight;
	}
	else if(m_maximized)
	{
		windowflags |= SDL_WINDOW_MAXIMIZED;
		m_windowW = m_windowCachedW;
		m_windowH = m_windowCachedH;
	}

	if(vulkan)
		windowflags |= SDL_WINDOW_VULKAN;
	else if(opengl)
		windowflags |= SDL_WINDOW_OPENGL;

	m_window = SDL_CreateWindow(PRODUCT_NAME, m_windowX, m_windowY, m_windowW, m_windowH, windowflags);
	if(!m_window)
	{
		if(!vulkan && !opengl)
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "Couldn't recreate SDL Window: %s", SDL_GetError());
			UTIL_MessageBox(true, g_buffer);
			exit(-1);
		}
		return false;
	}

	if(m_fullscreen)
	{
		attachFullScreenInfo();

		SDL_DisplayMode displayMode;
		SDL_GetWindowDisplayMode(m_window, &displayMode);
		m_windowW = displayMode.w;
		m_windowH = displayMode.h;
	}
	else
	{
		SDL_GetWindowSize(m_window, &m_windowW, &m_windowH);
		SDL_SetWindowMinimumSize(m_window, GAME_MINIMUM_WIDTH, GAME_MINIMUM_HEIGHT);
	}
	m_windowId = SDL_GetWindowID(m_window);

	//createwindow don't generate resize event so send one ourselves
	UTIL_ResizeEvent(m_windowId, m_windowW, m_windowH);
	return true;
}

bool Engine::init()
{
	m_surface.reset();
	if(m_engine == CLIENT_ENGINE_SOFTWARE)
		m_surface = std::move(std::make_unique<SurfaceSoftware>());
	#if defined(SDL_VIDEO_VULKAN) && !defined(__linux__)
	else if(m_engine == CLIENT_ENGINE_VULKAN)
		m_surface = std::move(std::make_unique<SurfaceVulkan>());
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL)
	else if(m_engine == CLIENT_ENGINE_OPENGLCORE)
		m_surface = std::move(std::make_unique<SurfaceOpenglCore>());
	else if(m_engine == CLIENT_ENGINE_OPENGL)
		m_surface = std::move(std::make_unique<SurfaceOpengl>());
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES)
	else if(m_engine == CLIENT_ENGINE_OPENGLES)
		m_surface = std::move(std::make_unique<SurfaceOpenglES>());
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES2)
	else if(m_engine == CLIENT_ENGINE_OPENGLES2)
		m_surface = std::move(std::make_unique<SurfaceOpenglES2>());
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D)
	else if(m_engine == CLIENT_ENGINE_DIRECT3D)
		m_surface = std::move(std::make_unique<SurfaceDirect3D9>());
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D11)
	else if(m_engine == CLIENT_ENGINE_DIRECT3D11)
		m_surface = std::move(std::make_unique<SurfaceDirect3D11>());
	#endif
	#if defined(SDL_VIDEO_RENDER_DDRAW)
	else if(m_engine == CLIENT_ENGINE_DIRECT3D7)
		m_surface = std::move(std::make_unique<SurfaceDirectDraw>());
	#endif
	else
	{
		m_engine = m_engines.back();
		return false;
	}

	if(!m_surface->isSupported())
	{
		for(std::vector<Uint8>::iterator it = m_engines.begin(), end = m_engines.end(); it != end; ++it)
		{
			if((*it) == m_engine)
			{
				m_engines.erase(it);
				break;
			}
		}
		if(m_engines.empty())
		{
			UTIL_MessageBox(true, "The engine couldn't find any sufficient graphic engine.");
			g_running = false;
			return false;
		}
		m_engine = m_engines.back();
		return false;
	}
	m_surface->doResize(m_windowW, m_windowH);
	m_surface->init();
	m_surface->spriteManagerReset();
	return true;
}

void Engine::initFont(Uint8 font, Sint32 width, Sint32 height, Sint16 hchars, Sint16 vchars, Sint16 maxchw, Sint16 maxchh, Sint16 spaceh)
{
	Uint16 picture = 0;
	switch(font)
	{
		case CLIENT_FONT_NONOUTLINED:
			picture = GUI_FONT_NONOUTLINED_IMAGE;
			break;
		case CLIENT_FONT_OUTLINED:
			picture = GUI_FONT_OUTLINED_IMAGE;
			break;
		case CLIENT_FONT_SMALL:
			picture = GUI_FONT_SMALL_IMAGE;
			break;
		default: break;
	}
	if(!picture)
		return;

	Sint32 w, h;
	unsigned char* pixels = LoadPicture(picture, true, w, h);
	if(!pixels || width != w || height != h)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Cannot read file '%s'.\n\nPlease re-install the program.", g_picPath.c_str());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
		return;
	}

	Uint32 protectionSize = (w * h * 4) - 4;
	m_charPicture[font] = picture;
	for(Sint32 i = 1; i < 32; ++i)
	{
		m_charx[font][i] = 0; m_chary[font][i] = 0;
		m_charw[font][i] = 0; m_charh[font][i] = 0;
	}

	for(Sint16 j = 0; j < vchars; ++j)
	{
		for(Sint16 k = 0; k < hchars; ++k)
		{
			Sint32 ch = 32 + (j * hchars) + k;
			if(ch > 255)
				continue;

			m_charx[font][ch] = k * maxchw;
			m_chary[font][ch] = j * maxchh;

			Sint16 chWidth = 0, chHeight = 0;
			for(Sint16 xPos = m_charx[font][ch]; xPos < m_charx[font][ch] + maxchw; ++xPos)
			{
				for(Sint16 yPos = m_chary[font][ch]; yPos < m_chary[font][ch] + maxchh; ++yPos)
				{
					Uint32 offset = (yPos * width + xPos) * 4;
					if(offset <= protectionSize && pixels[offset + 3] == 0xFF)
					{
						if(chWidth < xPos - m_charx[font][ch])
							chWidth = xPos - m_charx[font][ch];
						if(chHeight < yPos - m_chary[font][ch])
							chHeight = yPos - m_chary[font][ch];
					}
				}
			}

			m_charw[font][ch] = chWidth + 1;
			m_charh[font][ch] = chHeight + 1;
		}
	}

	SDL_free(pixels);
	m_charw[font][0] = maxchw;
	m_charh[font][0] = spaceh;
	switch(font)
	{
		case CLIENT_FONT_NONOUTLINED:
			m_charx[font][0] = 1; m_chary[font][0] = 0;
			m_charw[font][32] = m_charw[font][160] = 2;
			break;

		case CLIENT_FONT_OUTLINED:
			m_charx[font][0] = -1; m_chary[font][0] = 1;
			m_charw[font][32] = m_charw[font][160] = 4;
			break;

		case CLIENT_FONT_SMALL:
			m_charx[font][0] = m_chary[font][0] = 0;
			m_charw[font][32] = m_charw[font][160] = 2;
			break;

		default: break;
	}
}

Uint32 Engine::calculateFontWidth(Uint8 fontId, const std::string& text, size_t pos, size_t len)
{
	len += pos;
	if(len > text.length())
		len = text.length();

	Sint32 xSpace = m_charx[fontId][0];
	Sint32 ySpace = m_chary[fontId][0];
	Uint32 calculatedWidth = 0;
	Uint8 character;
	for(size_t i = pos; i < len; ++i)
	{
		character = SDL_static_cast(Uint8, text[i]);
		switch(character)
		{
			case '\n':
			case '\r'://return here?
				return calculatedWidth + ySpace;
			case 0x0E://Special case - change rendering color
			{
				if(i + 4 < len)//First check if we have the color bytes
					i += 3;
				else
					i = len;
			}
			break;
			case 0x0F://Special case - change back standard color
				break;
			default:
				calculatedWidth += m_charw[fontId][character] + xSpace;
				break;
		}
	}
	return calculatedWidth + ySpace;
}

Uint32 Engine::calculateFontWidth(Uint8 fontId, const std::string& text)
{
	Sint32 xSpace = m_charx[fontId][0];
	Sint32 ySpace = m_chary[fontId][0];
	Uint32 calculatedWidth = 0;
	Uint8 character;
	for(size_t i = 0, len = text.length(); i < len; ++i)
	{
		character = SDL_static_cast(Uint8, text[i]);
		switch(character)
		{
			case '\n':
			case '\r':
				return calculatedWidth + ySpace;
			case 0x0E://Special case - change rendering color
			{
				if(i + 4 < len)//First check if we have the color bytes
					i += 3;
				else
					i = len;
			}
			break;
			case 0x0F://Special case - change back standard color
				break;
			default:
				calculatedWidth += m_charw[fontId][character] + xSpace;
				break;
		}
	}
	return calculatedWidth + ySpace;
}

void Engine::exitGame()
{
	if(m_ingame)
		UTIL_exitWarning();
	else
	{
		g_inited = false;
		g_running = false;
	}
}

void Engine::checkReleaseQueue()
{
	for(std::vector<GUI_Window*>::iterator it = m_toReleaseWindows.begin(), end = m_toReleaseWindows.end(); it != end; ++it)
		delete (*it);

	m_toReleaseWindows.clear();
	for(std::vector<GUI_Panel*>::iterator it = m_toReleasePanels.begin(), end = m_toReleasePanels.end(); it != end; ++it)
		delete (*it);

	m_toReleasePanels.clear();
}

void Engine::updateThink()
{
	//Event that get called by every 5 seconds
	checkReleaseQueue();
	g_game.updatePlayerExperienceTable();
	g_game.sendPing();
}

void Engine::clearWindows()
{
	if(m_actWindow)
	{
		m_toReleaseWindows.push_back(m_actWindow);
		m_actWindow = NULL;
	}
	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
		m_toReleaseWindows.push_back((*it));

	m_windows.clear();
}

void Engine::addWindow(GUI_Window* pWindow, bool topMost)
{
	pWindow->onReshape(m_windowW, m_windowH);
	if(topMost)
	{
		if(m_actWindow)
			m_windows.insert(m_windows.begin(), m_actWindow);
		m_actWindow = pWindow;
	}
	else if(!m_actWindow)
		m_actWindow = pWindow;
	else
		m_windows.push_back(pWindow);
}

void Engine::removeWindow(GUI_Window* pWindow)
{
	m_toReleaseWindows.push_back(pWindow);
	if(pWindow == m_actWindow)
	{
		m_actWindow = NULL;
		if(!m_windows.empty())
		{
			m_actWindow = m_windows.front();
			m_windows.erase(m_windows.begin());
		}
		return;
	}

	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
	{
		if((*it) == pWindow)
		{
			m_windows.erase(it);
			return;
		}
	}
}

GUI_Window* Engine::getWindow(Uint32 internalID)
{
	if(m_actWindow && m_actWindow->getInternalID() == internalID)
		return m_actWindow;
	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
	{
		if((*it)->getInternalID() == internalID)
			return (*it);
	}
	return NULL;
}

void Engine::onKeyDown(SDL_Event& event)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	HotkeyUsage* hotkey = getHotkey(event.key.keysym.sym, event.key.keysym.mod);
	if(hotkey)
	{
		switch(hotkey->hotkey)
		{
			case CLIENT_HOTKEY_DIALOGS_OPENTERMINAL:
			{
				if(event.key.repeat == 0)
					m_showLogger = !m_showLogger;
			}
			return;
			case CLIENT_HOTKEY_UI_TOGGLEFPSINDICATOR:
			{
				if(event.key.repeat == 0)
					m_showPerformance = !m_showPerformance;
			}
			return;
			case CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN:
			{
				if(event.key.repeat == 0)
				{
					m_windowW = m_windowCachedW;
					m_windowH = m_windowCachedH;
					m_fullscreen = !m_fullscreen;
					g_inited = false;
				}
			}
			return;
			default: break;
		}
	}

	if(m_showLogger)
	{
		g_logger.onKeyDown(event);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onKeyDown(event);
		return;
	}

	if(m_ingame)
	{
		if(event.key.keysym.mod == KMOD_NONE)
		{
			if(event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
				g_chat.sendMessage();
			else if(event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_HOME || event.key.keysym.sym == SDLK_END)
				g_chat.onKeyDown(event);
		}
		else if(event.key.keysym.mod == KMOD_SHIFT)
		{
			if(event.key.keysym.sym == SDLK_UP)
				g_chat.navigateHistory(-1);
			else if(event.key.keysym.sym == SDLK_DOWN)
				g_chat.navigateHistory(1);
			else if(event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_HOME || event.key.keysym.sym == SDLK_END)
				g_chat.onKeyDown(event);
		}
		else if(event.key.keysym.mod == KMOD_CTRL)
		{
			if(event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_x || event.key.keysym.sym == SDLK_c || event.key.keysym.sym == SDLK_v)
				g_chat.onKeyDown(event);
		}

		if(hotkey)
		{
			switch(hotkey->hotkey)
			{
				case CLIENT_HOTKEY_MOVEMENT_GOEAST: g_game.checkMovement(DIRECTION_EAST); break;
				case CLIENT_HOTKEY_MOVEMENT_GONORTH: g_game.checkMovement(DIRECTION_NORTH); break;
				case CLIENT_HOTKEY_MOVEMENT_GOWEST: g_game.checkMovement(DIRECTION_WEST); break;
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTH: g_game.checkMovement(DIRECTION_SOUTH); break;
				case CLIENT_HOTKEY_MOVEMENT_GONORTHWEST: g_game.checkMovement(DIRECTION_NORTHWEST); break;
				case CLIENT_HOTKEY_MOVEMENT_GONORTHEAST: g_game.checkMovement(DIRECTION_NORTHEAST); break;
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHWEST: g_game.checkMovement(DIRECTION_SOUTHWEST); break;
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHEAST: g_game.checkMovement(DIRECTION_SOUTHEAST); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNEAST: g_game.sendTurn(DIRECTION_EAST); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNNORTH: g_game.sendTurn(DIRECTION_NORTH); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNWEST: g_game.sendTurn(DIRECTION_WEST); break;
				case CLIENT_HOTKEY_MOVEMENT_TURNSOUTH: g_game.sendTurn(DIRECTION_SOUTH); break;
				case CLIENT_HOTKEY_MOVEMENT_MOUNT:
				{
					if(event.key.repeat == 0 && g_game.hasGameFeature(GAME_FEATURE_MOUNTS))
					{
						Creature* localCreature = g_map.getLocalCreature();
						if(localCreature)
							g_game.sendMount(!localCreature->getMountType());
					}
				}
				break;
				case CLIENT_HOTKEY_MOVEMENT_STOPACTIONS:
				{
					if(event.key.repeat == 0)
						g_game.stopActions();
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENBUGREPORTS:
				{
					//Open bug reports
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENIGNORELIST:
				{
					//Open ignore list
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENOPTIONS:
				{
					if(event.key.repeat == 0)
						UTIL_options();
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENHOTKEYS:
				{
					if(event.key.repeat == 0)
						UTIL_hotkeyOptions();
				}
				break;
				case CLIENT_HOTKEY_DIALOGS_OPENQUESTLOG:
				{
					if(event.key.repeat == 0)
						g_game.sendOpenQuestLog();
				}
				break;
				case CLIENT_HOTKEY_WINDOWS_OPENVIPWINDOW:
				{
					if(event.key.repeat == 0)
						UTIL_toggleVipWindow();
				}
				break;
				case CLIENT_HOTKEY_WINDOWS_OPENBATTLEWINDOW:
				{
					if(event.key.repeat == 0)
						UTIL_toggleBattleWindow();
				}
				break;
				case CLIENT_HOTKEY_WINDOWS_OPENSKILLSWINDOW:
				{
					if(event.key.repeat == 0)
						UTIL_toggleSkillsWindow();
				}
				break;
				case CLIENT_HOTKEY_CHAT_CLOSECHANNEL:
				{
					if(event.key.repeat == 0)
						g_game.closeCurrentChannel();
				}
				break;
				case CLIENT_HOTKEY_CHAT_NEXTCHANNEL: g_game.switchToNextChannel(); break;
				case CLIENT_HOTKEY_CHAT_PREVIOUSCHANNEL: g_game.switchToPreviousChannel(); break;
				case CLIENT_HOTKEY_CHAT_OPENCHANNELLIST:
				{
					if(event.key.repeat == 0)
						g_game.sendRequestChannels();
				}
				break;
				case CLIENT_HOTKEY_CHAT_OPENHELPCHANNEL:
				{
					if(event.key.repeat == 0)
						g_game.openHelpChannel();
				}
				break;
				case CLIENT_HOTKEY_CHAT_OPENNPCCHANNEL:
				{
					if(event.key.repeat == 0)
						g_game.openNPCChannel();
				}
				break;
				case CLIENT_HOTKEY_CHAT_DEFAULTCHANNEL:
				{
					if(event.key.repeat == 0)
						g_game.switchToDefault();
				}
				break;
				case CLIENT_HOTKEY_CHAT_TOGGLECHAT:
				{
					//Toggle chat
				}
				break;
				case CLIENT_HOTKEY_MINIMAP_CENTER: g_game.minimapCenter(); break;
				case CLIENT_HOTKEY_MINIMAP_FLOORDOWN: g_game.minimapFloorDown(); break;
				case CLIENT_HOTKEY_MINIMAP_FLOORUP: g_game.minimapFloorUp(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLEAST: g_game.minimapScrollEast(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLNORTH: g_game.minimapScrollNorth(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLSOUTH: g_game.minimapScrollSouth(); break;
				case CLIENT_HOTKEY_MINIMAP_SCROLLWEST: g_game.minimapScrollWest(); break;
				case CLIENT_HOTKEY_MINIMAP_ZOOMIN: g_game.minimapZoomIn(); break;
				case CLIENT_HOTKEY_MINIMAP_ZOOMOUT: g_game.minimapZoomOut(); break;
				case CLIENT_HOTKEY_UI_TOGGLECREATUREINFO:
				{
					//Toggle creature info
				}
				break;
				case CLIENT_HOTKEY_COMBAT_SETOFFENSIVE:
				{
					if(event.key.repeat == 0)
					{
						setAttackMode(ATTACKMODE_ATTACK);
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_COMBAT_SETBALANCED:
				{
					if(event.key.repeat == 0)
					{
						setAttackMode(ATTACKMODE_BALANCED);
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_COMBAT_SETDEFENSIVE:
				{
					if(event.key.repeat == 0)
					{
						setAttackMode(ATTACKMODE_DEFENSE);
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_COMBAT_TOGGLECHASEMODE:
				{
					if(event.key.repeat == 0)
					{
						setChaseMode((getChaseMode() == CHASEMODE_STAND ? CHASEMODE_FOLLOW : CHASEMODE_STAND));
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_COMBAT_TOGGLESECUREMODE:
				{
					if(event.key.repeat == 0)
					{
						setSecureMode((getSecureMode() == SECUREMODE_SECURE ? SECUREMODE_UNSECURE : SECUREMODE_SECURE));
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETDOVE:
				{
					if(event.key.repeat == 0)
					{
						setPvpMode(PVPMODE_DOVE);
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETREDFIST:
				{
					if(event.key.repeat == 0)
					{
						setPvpMode(PVPMODE_RED_FIST);
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETWHITEHAND:
				{
					if(event.key.repeat == 0)
					{
						setPvpMode(PVPMODE_WHITE_HAND);
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_PVPMODE_SETYELLOWHAND:
				{
					if(event.key.repeat == 0)
					{
						setPvpMode(PVPMODE_YELLOW_HAND);
						g_game.sendAttackModes();
					}
				}
				break;
				case CLIENT_HOTKEY_MISC_LENSHELP:
				{
					if(event.key.repeat == 0)
						setAction(CLIENT_ACTION_LENSHELP);
				}
				break;
				case CLIENT_HOTKEY_MISC_CHANGECHARACTER:
				{
					if(event.key.repeat == 0)
						UTIL_createCharacterList();
				}
				break;
				case CLIENT_HOTKEY_MISC_CHANGEOUTFIT:
				{
					if(event.key.repeat == 0)
						g_game.sendRequestOutfit();
				}
				break;
				case CLIENT_HOTKEY_MISC_LOGOUT:
				{
					if(event.key.repeat == 0)
						g_game.sendLogout();
				}
				break;
				case CLIENT_HOTKEY_MISC_TAKESCREENSHOT:
				{
					//Screenshot
				}
				break;
				case CLIENT_HOTKEY_MISC_NEXTPRESET:
				{
					//Switch to next preset
				}
				break;
				case CLIENT_HOTKEY_MISC_PREVIOUSPRESET:
				{
					//Switch to previous preset
				}
				break;
				case CLIENT_HOTKEY_ACTION:
				{
					//Implement actions
				}
				break;
				default: break;
			}
			return;
		}
	}
}

void Engine::onKeyUp(SDL_Event& event)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onKeyUp(event);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onKeyUp(event);
		return;
	}

	if(m_ingame)
	{
		if(event.key.keysym.mod == KMOD_NONE)
		{
			if(event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_HOME || event.key.keysym.sym == SDLK_END)
				g_chat.onKeyUp(event);
		}
		else if(event.key.keysym.mod == KMOD_SHIFT)
		{
			if(event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_HOME || event.key.keysym.sym == SDLK_END)
				g_chat.onKeyUp(event);
		}
		else if(event.key.keysym.mod == KMOD_CTRL)
		{
			if(event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_x || event.key.keysym.sym == SDLK_c || event.key.keysym.sym == SDLK_v)
				g_chat.onKeyUp(event);
		}

		HotkeyUsage* hotkey = getHotkey(event.key.keysym.sym, event.key.keysym.mod);
		if(hotkey)
		{
			switch(hotkey->hotkey)
			{
				case CLIENT_HOTKEY_MOVEMENT_GOEAST:
				case CLIENT_HOTKEY_MOVEMENT_GONORTH:
				case CLIENT_HOTKEY_MOVEMENT_GOWEST:
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTH:
				case CLIENT_HOTKEY_MOVEMENT_GONORTHWEST:
				case CLIENT_HOTKEY_MOVEMENT_GONORTHEAST:
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHWEST:
				case CLIENT_HOTKEY_MOVEMENT_GOSOUTHEAST:
					g_game.releaseMovement();
					break;
				default: break;
			}
			return;
		}
	}
}

void Engine::onMouseMove(Sint32 x, Sint32 y)
{
	if(m_contextMenu)
	{
		m_contextMenu->onMouseMove(x, y, m_contextMenu->isInsideRect(x, y));
		return;
	}

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onMouseMove(x, y, true);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onMouseMove(x, y, m_actWindow->isInsideRect(x, y));
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->onMouseMove(x, y, (*it)->isInsideRect(x, y));

		{
			bool inside = m_leftPanelAddRect.isPointInside(x, y);
			if(m_leftAddPanel > 0)
			{
				if(m_leftAddPanel == 1 && !inside)
					m_leftAddPanel = 2;
				else if(m_leftAddPanel == 2 && inside)
					m_leftAddPanel = 1;
			}
			if(inside)
				showDescription(x, y, (m_canAddLeftPanel ? "Open new sidebar" : "Enlarge client or reduce game window to make room for further sidebar"));

			inside = m_leftPanelRemRect.isPointInside(x, y);
			if(m_leftRemPanel > 0)
			{
				if(m_leftRemPanel == 1 && !inside)
					m_leftRemPanel = 2;
				else if(m_leftRemPanel == 2 && inside)
					m_leftRemPanel = 1;
			}
			if(inside)
				showDescription(x, y, (m_haveExtraLeftPanel ? "Close this sidebar" : "Last sidebar cannot be closed"));
		}
		{
			bool inside = m_rightPanelAddRect.isPointInside(x, y);
			if(m_rightAddPanel > 0)
			{
				if(m_rightAddPanel == 1 && !inside)
					m_rightAddPanel = 2;
				else if(m_rightAddPanel == 2 && inside)
					m_rightAddPanel = 1;
			}
			if(inside)
				showDescription(x, y, (m_canAddRightPanel ? "Open new sidebar" : "Enlarge client or reduce game window to make room for further sidebar"));

			inside = m_rightPanelRemRect.isPointInside(x, y);
			if(m_rightRemPanel > 0)
			{
				if(m_rightRemPanel == 1 && !inside)
					m_rightRemPanel = 2;
				else if(m_rightRemPanel == 2 && inside)
					m_rightRemPanel = 1;
			}
			if(inside)
				showDescription(x, y, (m_haveExtraRightPanel ? "Close this sidebar" : "Last sidebar cannot be closed"));
		}

		g_chat.onMouseMove(m_chatWindowRect, x, y);
		if(m_actionData == CLIENT_ACTION_MOVEITEM || m_actionData == CLIENT_ACTION_USEWITH || m_actionData == CLIENT_ACTION_TRADE || m_actionData == CLIENT_ACTION_SEARCHHOTKEY)
			g_actualCursor = CLIENT_CURSOR_CROSSHAIR;
		else if(m_actionData == CLIENT_ACTION_LENSHELP)
			g_actualCursor = CLIENT_CURSOR_LENSHELP;
		else if(m_actionData == CLIENT_ACTION_LEFTMOUSE && m_moveItemX != SDL_MIN_SINT32 && m_moveItemY != SDL_MIN_SINT32)
		{
			Sint32 distanceX = std::abs(m_moveItemX - x);
			Sint32 distanceY = std::abs(m_moveItemY - y);
			if(distanceX >= 5 || distanceY >= 5)
			{
				m_actionData = CLIENT_ACTION_MOVEITEM;
				g_actualCursor = CLIENT_CURSOR_CROSSHAIR;

				//Disable move item actions
				m_moveItemX = SDL_MIN_SINT32;
				m_moveItemY = SDL_MIN_SINT32;
			}
		}
	}
	else
		g_mainWindow->onMouseMove(x, y, g_mainWindow->isInsideRect(x, y));
}

void Engine::onLMouseDown(Sint32 x, Sint32 y)
{
	if(m_actionData != CLIENT_ACTION_NONE)
	{
		if(m_classicControl && m_actionData == CLIENT_ACTION_RIGHTMOUSE)
			setAction(CLIENT_ACTION_EXTRAMOUSE);

		return;
	}

	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onLMouseDown(x, y);
		return;
	}

	if(m_actWindow)
	{
		if(m_actWindow->isInsideRect(x, y))
			m_actWindow->onLMouseDown(x, y);
		return;
	}

	if(m_ingame)
	{
		GUI_Panel* gPanel = NULL;
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		{
			if((*it)->isInsideRect(x, y))
			{
				gPanel = (*it);
				break;
			}
		}
		if(gPanel)
		{
			gPanel->onLMouseDown(x, y);
			return;
		}
		if(m_gameWindowRect.isPointInside(x, y))
		{
			Creature* topCreature;
			Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, true);
			if(tile)
			{
				Thing* thing = tile->getTopLookThing();
				if(thing)
				{
					Position& position = tile->getPosition();
					setActionData(CLIENT_ACTION_FIRST, (topCreature ? topCreature->getId() : 0), (thing->isItem() ? thing->getItem()->getID() : 0x62), position.x, position.y, position.z, SDL_static_cast(Uint8, tile->getThingStackPos(thing)));
					setActionData(CLIENT_ACTION_SECOND, 0, (thing->isItem() ? thing->getItem()->getItemCount() : 1), 0, 0, 0, 0);
					enableMoveItem(x, y);
				}
			}

			setAction(CLIENT_ACTION_LEFTMOUSE);
			return;
		}
		else
			g_chat.onLMouseDown(m_chatWindowRect, x, y);

		{
			if(m_leftPanelAddRect.isPointInside(x, y))
				m_leftAddPanel = 1;
			else if(m_leftPanelRemRect.isPointInside(x, y))
				m_leftRemPanel = 1;
		}
		{
			if(m_rightPanelAddRect.isPointInside(x, y))
				m_rightAddPanel = 1;
			else if(m_rightPanelRemRect.isPointInside(x, y))
				m_rightRemPanel = 1;
		}
	}
	else
	{
		if(g_mainWindow)
		{
			if(g_mainWindow->isInsideRect(x, y))
				g_mainWindow->onLMouseDown(x, y);
		}
	}
}

void Engine::onLMouseUp(Sint32 x, Sint32 y)
{
	if(m_actionData != CLIENT_ACTION_NONE)
	{
		if(!m_ingame || m_contextMenu || m_description || m_showLogger || m_actWindow || m_actionData == CLIENT_ACTION_LENSHELP)
		{
			setAction(CLIENT_ACTION_NONE);
			return;
		}

		Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
		if(m_actionData == CLIENT_ACTION_MOVEITEM)
		{
			GUI_Panel* gPanel = NULL;
			for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			{
				if((*it)->isInsideRect(x, y))
				{
					gPanel = (*it);
					break;
				}
			}
			if(gPanel)
				gPanel->onLMouseUp(x, y);
			else if(m_gameWindowRect.isPointInside(x, y))
			{
				Creature* topCreature;
				Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, true);
				if(tile)
				{
					Position& position = tile->getPosition();
					initMove(position.x, position.y, position.z);
				}
			}

			setAction(CLIENT_ACTION_NONE);
		}
		else if(m_actionData == CLIENT_ACTION_USEWITH)
		{
			Creature* topCreature = NULL;
			ItemUI* itemui = NULL;
			Thing* useThing = NULL;

			GUI_Panel* gPanel = NULL;
			for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			{
				if((*it)->isInsideRect(x, y))
				{
					gPanel = (*it);
					break;
				}
			}
			if(gPanel)
			{
				itemui = SDL_reinterpret_cast(ItemUI*, gPanel->onAction(x, y));
				if(!itemui && g_game.getSelectID() != 0)
					topCreature = g_map.getCreatureById(g_game.getSelectID());
			}
			else if(m_gameWindowRect.isPointInside(x, y))
			{
				Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, true);
				if(tile)
					useThing = tile->getTopUseThing();
			}

			ClientActionData& actionData = g_engine.m_actionDataStructure[CLIENT_ACTION_FIRST];
			if(topCreature)
				g_game.sendUseOnCreature(Position(actionData.posX, actionData.posY, actionData.posZ), actionData.itemId, actionData.posStack, topCreature->getId());
			else if(useThing)
			{
				if(useThing->isItem())
				{
					Item* item = useThing->getItem();
					Position& position = item->getCurrentPosition();
					Tile* itemTile = g_map.getTile(position);
					if(itemTile)
						g_game.sendUseItemEx(Position(actionData.posX, actionData.posY, actionData.posZ), actionData.itemId, actionData.posStack, position, item->getID(), SDL_static_cast(Uint8, itemTile->getThingStackPos(item)));
				}
			}
			else if(itemui)
			{
				Position& position = itemui->getCurrentPosition();
				g_game.sendUseItemEx(Position(actionData.posX, actionData.posY, actionData.posZ), actionData.itemId, actionData.posStack, position, itemui->getID(), 0);
			}

			setAction(CLIENT_ACTION_NONE);
		}
		else if(m_actionData == CLIENT_ACTION_TRADE)
		{
			Creature* topCreature = NULL;

			GUI_Panel* gPanel = NULL;
			for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			{
				if((*it)->isInsideRect(x, y))
				{
					gPanel = (*it);
					break;
				}
			}
			if(gPanel)
			{
				if(g_game.getSelectID() != 0)
					topCreature = g_map.getCreatureById(g_game.getSelectID());
			}
			else if(m_gameWindowRect.isPointInside(x, y))
				g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, true);

			ClientActionData& actionData = g_engine.m_actionDataStructure[CLIENT_ACTION_FIRST];
			if(topCreature && topCreature->isPlayer())
				g_game.sendRequestTrade(Position(actionData.posX, actionData.posY, actionData.posZ), actionData.itemId, actionData.posStack, topCreature->getId());
			else
				g_game.processTextMessage(MessageFailure, "Select a player to trade with.");

			setAction(CLIENT_ACTION_NONE);
		}
		else if(m_actionData == CLIENT_ACTION_SEARCHHOTKEY)
		{
			//Make hotkey
			setAction(CLIENT_ACTION_NONE);
		}
		else if((!(mouseState & SDL_BUTTON_RMASK) && m_actionData == CLIENT_ACTION_EXTRAMOUSE) || m_actionData == CLIENT_ACTION_LEFTMOUSE)
		{
			Creature* topCreature = NULL;
			ItemUI* itemui = NULL;
			Thing* lookThing = NULL;
			Thing* useThing = NULL;

			GUI_Panel* gPanel = NULL;
			for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			{
				if((*it)->isInsideRect(x, y))
				{
					gPanel = (*it);
					break;
				}
			}
			if(gPanel)
				itemui = SDL_reinterpret_cast(ItemUI*, gPanel->onAction(x, y));
			else if(m_gameWindowRect.isPointInside(x, y))
			{
				Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, true);
				if(tile)
				{
					lookThing = tile->getTopLookThing();
					useThing = tile->getTopUseThing();
				}
			}

			if(topCreature || itemui || lookThing || useThing)
			{
				Uint16 keyMods = UTIL_parseModifiers(SDL_static_cast(Uint16, SDL_GetModState()));
				if(m_actionData == CLIENT_ACTION_EXTRAMOUSE || keyMods == KMOD_SHIFT)
				{
					if(topCreature)
					{
						if(g_game.hasGameFeature(GAME_FEATURE_LOOKATCREATURE))
							g_game.sendLookInBattle(topCreature->getId());
						else
						{
							Position& position = topCreature->getCurrentPosition();
							Tile* creatureTile = g_map.getTile(position);
							if(creatureTile)
								g_game.sendLookAt(position, 0x62, SDL_static_cast(Uint8, creatureTile->getThingStackPos(topCreature)));
						}
					}
					else if(lookThing)
					{
						if(lookThing->isItem())
						{
							Item* item = lookThing->getItem();
							Position& position = item->getCurrentPosition();
							Tile* itemTile = g_map.getTile(position);
							if(itemTile)
								g_game.sendLookAt(position, item->getID(), SDL_static_cast(Uint8, itemTile->getThingStackPos(item)));
						}
					}
					else if(itemui)
					{
						Position& position = itemui->getCurrentPosition();
						g_game.sendLookAt(position, itemui->getID(), 0);
					}
				}
				else if(m_actionData == CLIENT_ACTION_LEFTMOUSE)
				{
					if(!m_classicControl && keyMods == KMOD_ALT)
					{
						if(topCreature && g_game.getPlayerID() != topCreature->getId())
							g_game.sendAttack((g_game.getAttackID() == topCreature->getId()) ? NULL : topCreature);
						else if(itemui)
						{
							GUI_ContextMenu* newMenu = createThingContextMenu(NULL, itemui, NULL);
							newMenu->setEventCallback(&Engine::standardThingEvent);
							showContextMenu(newMenu, x, y);
						}
					}
					else if(!m_classicControl && keyMods == KMOD_CTRL)
					{
						if(useThing)
						{
							if(useThing->isItem())
							{
								Item* item = useThing->getItem();
								Position& position = item->getCurrentPosition();
								Tile* itemTile = g_map.getTile(position);
								if(itemTile)
								{
									if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_MultiUse))
									{
										setActionData(CLIENT_ACTION_FIRST, 0, item->getID(), position.x, position.y, position.z, SDL_static_cast(Uint8, itemTile->getUseStackPos(item)));
										setAction(CLIENT_ACTION_USEWITH);
										return;
									}
									else
										g_game.sendUseItem(position, item->getID(), SDL_static_cast(Uint8, itemTile->getUseStackPos(item)), g_game.findEmptyContainerId());
								}
							}
						}
						else if(itemui)
						{
							Position& position = itemui->getCurrentPosition();
							if(itemui->getThingType() && itemui->getThingType()->hasFlag(ThingAttribute_MultiUse))
							{
								setActionData(CLIENT_ACTION_FIRST, 0, itemui->getID(), position.x, position.y, position.z, 0);
								setAction(CLIENT_ACTION_USEWITH);
								return;
							}
							else
							{
								if(position.y & 0x40)
									g_game.sendUseItem(position, itemui->getID(), 0, (position.y & 0x0F));
								else
									g_game.sendUseItem(position, itemui->getID(), 0, g_game.findEmptyContainerId());
							}
						}
					}
					else if(m_classicControl && keyMods == KMOD_ALT)
					{
						if(topCreature && g_game.getPlayerID() != topCreature->getId())
							g_game.sendAttack((g_game.getAttackID() == topCreature->getId()) ? NULL : topCreature);
						else if(useThing)
						{
							if(useThing->isItem())
							{
								Item* item = useThing->getItem();
								Position& position = item->getCurrentPosition();
								Tile* itemTile = g_map.getTile(position);
								if(itemTile)
								{
									if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_MultiUse))
									{
										setActionData(CLIENT_ACTION_FIRST, 0, item->getID(), position.x, position.y, position.z, SDL_static_cast(Uint8, itemTile->getUseStackPos(item)));
										setAction(CLIENT_ACTION_USEWITH);
										return;
									}
									else
										g_game.sendUseItem(position, item->getID(), SDL_static_cast(Uint8, itemTile->getUseStackPos(item)), g_game.findEmptyContainerId());
								}
							}
						}
						else if(itemui)
						{
							Position& position = itemui->getCurrentPosition();
							if(itemui->getThingType() && itemui->getThingType()->hasFlag(ThingAttribute_MultiUse))
							{
								setActionData(CLIENT_ACTION_FIRST, 0, itemui->getID(), position.x, position.y, position.z, 0);
								setAction(CLIENT_ACTION_USEWITH);
								return;
							}
							else
							{
								if(position.y & 0x40)
									g_game.sendUseItem(position, itemui->getID(), 0, (position.y & 0x0F));
								else
									g_game.sendUseItem(position, itemui->getID(), 0, g_game.findEmptyContainerId());
							}
						}
					}
					else if(m_classicControl && keyMods == KMOD_CTRL)
					{
						GUI_ContextMenu* newMenu = createThingContextMenu(topCreature, itemui, (useThing && useThing->isItem() ? useThing->getItem() : NULL));
						newMenu->setEventCallback(&Engine::standardThingEvent);
						showContextMenu(newMenu, x, y);
					}
					else if(keyMods == KMOD_NONE && m_gameWindowRect.isPointInside(x, y))
					{
						Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, false);
						if(tile)
						{
							Position& autoWalkPosition = tile->getPosition();
							g_game.startAutoWalk(autoWalkPosition);
						}
					}
				}
			}
			else
			{
				Uint16 keyMods = UTIL_parseModifiers(SDL_static_cast(Uint16, SDL_GetModState()));
				if(keyMods == KMOD_NONE && m_gameWindowRect.isPointInside(x, y))
				{
					Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, false);
					if(tile)
					{
						Position& autoWalkPosition = tile->getPosition();
						g_game.startAutoWalk(autoWalkPosition);
					}
				}
			}

			setAction(CLIENT_ACTION_NONE);
		}
		return;
	}

	if(m_contextMenu)
	{
		m_contextMenu->onLMouseUp(x, y);
		delete m_contextMenu;
		m_contextMenu = NULL;
		return;
	}

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onLMouseUp(x, y);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onLMouseUp(x, y);
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->onLMouseUp(x, y);

		g_chat.onLMouseUp(m_chatWindowRect, x, y);

		{
			if(m_leftAddPanel == 1)
			{
				m_panels.push_back(new GUI_Panel(iRect(0, 0, 0, 0), (m_leftPanel == GUI_PANEL_RANDOM ? GUI_PANEL_EXTRA_LEFT_START : ++m_leftPanel)));
				recalculateGameWindow();
			}
			else if(m_leftRemPanel == 1)
			{
				if(m_leftPanel != GUI_PANEL_RANDOM)
				{
					for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
					{
						if((*it)->getInternalID() == m_leftPanel)
						{
							delete (*it);
							m_panels.erase(it);
							break;
						}
					}
					recalculateGameWindow();
				}
			}

			m_leftAddPanel = 0;
			m_leftRemPanel = 0;
		}
		{
			if(m_rightAddPanel == 1)
			{
				m_panels.push_back(new GUI_Panel(iRect(0, 0, 0, 0), (m_rightPanel == GUI_PANEL_MAIN ? GUI_PANEL_EXTRA_RIGHT_START : ++m_rightPanel)));
				recalculateGameWindow();
			}
			else if(m_rightRemPanel == 1)
			{
				if(m_rightPanel != GUI_PANEL_MAIN)
				{
					for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
					{
						if((*it)->getInternalID() == m_rightPanel)
						{
							delete (*it);
							m_panels.erase(it);
							break;
						}
					}
					recalculateGameWindow();
				}
			}

			m_rightAddPanel = 0;
			m_rightRemPanel = 0;
		}
	}
	else
	{
		if(g_mainWindow)
			g_mainWindow->onLMouseUp(x, y);
	}
}

void Engine::onRMouseDown(Sint32 x, Sint32 y)
{
	if(m_actionData != CLIENT_ACTION_NONE)
	{
		if(m_classicControl && m_actionData == CLIENT_ACTION_LEFTMOUSE)
			setAction(CLIENT_ACTION_EXTRAMOUSE);

		return;
	}

	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onRMouseDown(x, y);
		return;
	}

	if(m_actWindow)
	{
		if(m_actWindow->isInsideRect(x, y))
			m_actWindow->onRMouseDown(x, y);
		return;
	}

	if(m_ingame)
	{
		GUI_Panel* gPanel = NULL;
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		{
			if((*it)->isInsideRect(x, y))
			{
				gPanel = (*it);
				break;
			}
		}
		if(gPanel)
		{
			gPanel->onRMouseDown(x, y);
			return;
		}
		if(m_gameWindowRect.isPointInside(x, y))
			setAction(CLIENT_ACTION_RIGHTMOUSE);
		else
			g_chat.onRMouseDown(m_chatWindowRect, x, y);
	}
	else
	{
		if(g_mainWindow)
		{
			if(g_mainWindow->isInsideRect(x, y))
				g_mainWindow->onRMouseDown(x, y);
		}
	}
}

void Engine::onRMouseUp(Sint32 x, Sint32 y)
{
	if(m_actionData != CLIENT_ACTION_NONE)
	{
		if(!m_ingame || m_contextMenu || m_description || m_showLogger || m_actWindow || m_actionData == CLIENT_ACTION_USEWITH || m_actionData == CLIENT_ACTION_TRADE || m_actionData == CLIENT_ACTION_LENSHELP || m_actionData == CLIENT_ACTION_SEARCHHOTKEY)
		{
			setAction(CLIENT_ACTION_NONE);
			return;
		}

		Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
		if((!(mouseState & SDL_BUTTON_LMASK) && m_actionData == CLIENT_ACTION_EXTRAMOUSE) || m_actionData == CLIENT_ACTION_RIGHTMOUSE)
		{
			Creature* topCreature = NULL;
			ItemUI* itemui = NULL;
			Thing* lookThing = NULL;
			Thing* useThing = NULL;

			GUI_Panel* gPanel = NULL;
			for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			{
				if((*it)->isInsideRect(x, y))
				{
					gPanel = (*it);
					break;
				}
			}
			if(gPanel)
				itemui = SDL_reinterpret_cast(ItemUI*, gPanel->onAction(x, y));
			else if(m_gameWindowRect.isPointInside(x, y))
			{
				Tile* tile = g_map.findTile(x, y, m_gameWindowRect, m_scaledSize, m_scale, topCreature, true);
				if(tile)
				{
					lookThing = tile->getTopLookThing();
					useThing = tile->getTopUseThing();
				}
			}

			if(topCreature || itemui || lookThing || useThing)
			{
				Uint16 keyMods = UTIL_parseModifiers(SDL_static_cast(Uint16, SDL_GetModState()));
				if(m_actionData == CLIENT_ACTION_EXTRAMOUSE || keyMods == KMOD_SHIFT)
				{
					if(topCreature)
					{
						if(g_game.hasGameFeature(GAME_FEATURE_LOOKATCREATURE))
							g_game.sendLookInBattle(topCreature->getId());
						else
						{
							Position& position = topCreature->getCurrentPosition();
							Tile* creatureTile = g_map.getTile(position);
							if(creatureTile)
								g_game.sendLookAt(position, 0x62, SDL_static_cast(Uint8, creatureTile->getThingStackPos(topCreature)));
						}
					}
					else if(lookThing)
					{
						if(lookThing->isItem())
						{
							Item* item = lookThing->getItem();
							Position& position = item->getCurrentPosition();
							Tile* itemTile = g_map.getTile(position);
							if(itemTile)
								g_game.sendLookAt(position, item->getID(), SDL_static_cast(Uint8, itemTile->getThingStackPos(item)));
						}
					}
					else if(itemui)
					{
						Position& position = itemui->getCurrentPosition();
						g_game.sendLookAt(position, itemui->getID(), 0);
					}
				}
				else if(m_actionData == CLIENT_ACTION_RIGHTMOUSE)
				{
					if(!m_classicControl && keyMods == KMOD_ALT)
					{
						if(topCreature && g_game.getPlayerID() != topCreature->getId())
							g_game.sendAttack((g_game.getAttackID() == topCreature->getId()) ? NULL : topCreature);
						else if(itemui)
						{
							GUI_ContextMenu* newMenu = createThingContextMenu(NULL, itemui, NULL);
							newMenu->setEventCallback(&Engine::standardThingEvent);
							showContextMenu(newMenu, x, y);
						}
					}
					else if(!m_classicControl && keyMods == KMOD_CTRL)
					{
						if(useThing)
						{
							if(useThing->isItem())
							{
								Item* item = useThing->getItem();
								Position& position = item->getCurrentPosition();
								Tile* itemTile = g_map.getTile(position);
								if(itemTile)
								{
									if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_MultiUse))
									{
										setActionData(CLIENT_ACTION_FIRST, 0, item->getID(), position.x, position.y, position.z, SDL_static_cast(Uint8, itemTile->getUseStackPos(item)));
										setAction(CLIENT_ACTION_USEWITH);
										return;
									}
									else
										g_game.sendUseItem(position, item->getID(), SDL_static_cast(Uint8, itemTile->getUseStackPos(item)), g_game.findEmptyContainerId());
								}
							}
						}
						else if(itemui)
						{
							Position& position = itemui->getCurrentPosition();
							if(itemui->getThingType() && itemui->getThingType()->hasFlag(ThingAttribute_MultiUse))
							{
								setActionData(CLIENT_ACTION_FIRST, 0, itemui->getID(), position.x, position.y, position.z, 0);
								setAction(CLIENT_ACTION_USEWITH);
								return;
							}
							else
							{
								if(position.y & 0x40)
									g_game.sendUseItem(position, itemui->getID(), 0, (position.y & 0x0F));
								else
									g_game.sendUseItem(position, itemui->getID(), 0, g_game.findEmptyContainerId());
							}
						}
					}
					else if(m_classicControl && (keyMods == KMOD_NONE || keyMods == KMOD_ALT))
					{
						if(topCreature && g_game.getPlayerID() != topCreature->getId())
							g_game.sendAttack((g_game.getAttackID() == topCreature->getId()) ? NULL : topCreature);
						else if(useThing)
						{
							if(useThing->isItem())
							{
								Item* item = useThing->getItem();
								Position& position = item->getCurrentPosition();
								Tile* itemTile = g_map.getTile(position);
								if(itemTile)
								{
									if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_MultiUse))
									{
										setActionData(CLIENT_ACTION_FIRST, 0, item->getID(), position.x, position.y, position.z, SDL_static_cast(Uint8, itemTile->getUseStackPos(item)));
										setAction(CLIENT_ACTION_USEWITH);
										return;
									}
									else
										g_game.sendUseItem(position, item->getID(), SDL_static_cast(Uint8, itemTile->getUseStackPos(item)), g_game.findEmptyContainerId());
								}
							}
						}
						else if(itemui)
						{
							Position& position = itemui->getCurrentPosition();
							if(itemui->getThingType() && itemui->getThingType()->hasFlag(ThingAttribute_MultiUse))
							{
								setActionData(CLIENT_ACTION_FIRST, 0, itemui->getID(), position.x, position.y, position.z, 0);
								setAction(CLIENT_ACTION_USEWITH);
								return;
							}
							else
							{
								if(position.y & 0x40)
									g_game.sendUseItem(position, itemui->getID(), 0, (position.y & 0x0F));
								else
									g_game.sendUseItem(position, itemui->getID(), 0, g_game.findEmptyContainerId());
							}
						}
					}
					else if((m_classicControl && keyMods == KMOD_CTRL) || (!m_classicControl && keyMods == KMOD_NONE))
					{
						GUI_ContextMenu* newMenu = createThingContextMenu(topCreature, itemui, (useThing && useThing->isItem() ? useThing->getItem() : NULL));
						newMenu->setEventCallback(&Engine::standardThingEvent);
						showContextMenu(newMenu, x, y);
					}
				}
			}

			setAction(CLIENT_ACTION_NONE);
		}
		return;
	}

	if(m_contextMenu)
	{
		delete m_contextMenu;
		m_contextMenu = NULL;
		return;
	}

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onRMouseUp(x, y);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onRMouseUp(x, y);
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->onRMouseUp(x, y);

		g_chat.onRMouseUp(m_chatWindowRect, x, y);
	}
	else
	{
		if(g_mainWindow)
			g_mainWindow->onRMouseUp(x, y);
	}
}

void Engine::onWheel(Sint32 x, Sint32 y, bool wheelUP)
{
	if(m_contextMenu)
		return;

	if(m_description)
	{
		delete m_description;
		m_description = NULL;
	}

	if(m_showLogger)
	{
		g_logger.onWheel(x, y, wheelUP);
		return;
	}

	if(m_actWindow)
	{
		if(m_actWindow->isInsideRect(x, y))
			m_actWindow->onWheel(x, y, wheelUP);
		return;
	}

	if(m_ingame)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		{
			if((*it)->isInsideRect(x, y))
			{
				(*it)->onWheel(x, y, wheelUP);
				return;
			}
		}
		g_chat.onWheel(m_chatWindowRect, x, y, wheelUP);
	}
	else
	{
		if(g_mainWindow && g_mainWindow->isInsideRect(x, y))
			g_mainWindow->onWheel(x, y, wheelUP);
	}
}

void Engine::onTextInput(const char* textInput)
{
	if(m_showLogger)
	{
		g_logger.onTextInput(textInput);
		return;
	}

	if(m_actWindow)
	{
		m_actWindow->onTextInput(textInput);
		return;
	}

	if(m_ingame)
		g_chat.onTextInput(textInput);
	else
	{
		if(g_mainWindow)
			g_mainWindow->onTextInput(textInput);
	}
}

void Engine::windowResize(Sint32 width, Sint32 height)
{
	m_windowW = width;
	m_windowH = height;
	if(!m_maximized && !m_fullscreen)
	{
		m_windowCachedW = m_windowW;
		m_windowCachedH = m_windowH;
	}
	m_surface->doResize(m_windowW, m_windowH);
	for(std::vector<GUI_Window*>::iterator it = m_windows.begin(), end = m_windows.end(); it != end; ++it)
		(*it)->onReshape(m_windowW, m_windowH);

	if(m_actWindow)
		m_actWindow->onReshape(m_windowW, m_windowH);

	if(g_mainWindow)
		g_mainWindow->onReshape(m_windowW, m_windowH);

	if(m_ingame)
		recalculateGameWindow();
}

void Engine::windowMoved(Sint32 x, Sint32 y)
{
	if(!m_maximized && !m_fullscreen)
	{
		m_windowX = x;
		m_windowY = y;
	}
}

void Engine::takeScreenshot(void* data1, void* data2)
{
	//TODO: add other popular extensions like png
	char* fileName = SDL_reinterpret_cast(char*, data1);
	Uint32 flags = SDL_static_cast(Uint32, SDL_reinterpret_cast(size_t, data2));
	if(!fileName || !m_surface)
		return;

	Sint32 screenWidth;
	Sint32 screenHeight;
	bool pixelsBGRA;
	unsigned char* pixels = m_surface->getScreenPixels(screenWidth, screenHeight, pixelsBGRA);
	if(!pixels)
	{
		//We don't have pixels this frame so we can't do much
		SDL_free(fileName);
		return;
	}
	if(flags == SCREENSHOT_FLAG_SAVEASBMP)
	{
		Uint32 Rmask, Gmask, Bmask;
		if(pixelsBGRA)
		{
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Rmask = 0x0000FF00;
			Gmask = 0x00FF0000;
			Bmask = 0xFF000000;
			#else
			Rmask = 0x00FF0000;
			Gmask = 0x0000FF00;
			Bmask = 0x000000FF;
			#endif
		}
		else
		{
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Rmask = 0xFF000000;
			Gmask = 0x00FF0000;
			Bmask = 0x0000FF00;
			#else
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			#endif
		}
		SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, screenWidth, screenHeight, 32, 4 * screenWidth, Rmask, Gmask, Bmask, 0x00000000);
		SDL_SaveBMP(s, fileName);
		SDL_FreeSurface(s);
	}
	SDL_free(fileName);
	SDL_free(pixels);
}

HotkeyUsage* Engine::getHotkey(SDL_Keycode key, Uint16 mods)
{
	std::map<Uint16, std::map<SDL_Keycode, size_t>>::iterator mit = m_hotkeyFastAccess.find(mods);
	if(mit != m_hotkeyFastAccess.end())
	{
		std::map<SDL_Keycode, size_t>::iterator it = mit->second.find(key);
		if(it != mit->second.end())
			return &m_hotkeys[it->second];
		else
			return NULL;
	}
	return NULL;
}

void Engine::bindHotkey(ClientHotkeyKeys hotKey, SDL_Keycode key, Uint16 mods, ClientHotkeys hotkeyType)
{
	std::map<Uint16, std::map<SDL_Keycode, size_t>>::iterator mit = m_hotkeyFastAccess.find(mods);
	if(mit != m_hotkeyFastAccess.end())
	{
		std::map<SDL_Keycode, size_t>::iterator it = mit->second.find(key);
		if(it != mit->second.end())
			mit->second.erase(it);
	}

	HotkeyUsage newHotkey;
	newHotkey.action.type = CLIENT_HOTKEY_ACTION_NONE;
	newHotkey.keycode = key;
	newHotkey.hotkey = hotkeyType;
	newHotkey.modifers = mods;
	newHotkey.keyid = SDL_static_cast(Uint8, hotKey);
	m_hotkeyFastAccess[mods][key] = m_hotkeys.size();
	m_hotkeys.push_back(newHotkey);
}

void Engine::resetToDefaultHotkeys(bool wasd)
{
	if(wasd)
	{
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_w, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNNORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_a, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_s, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_d, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNEAST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_w, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_a, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_s, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_d, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOEAST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RETURN, KMOD_NONE, CLIENT_HOTKEY_CHAT_TOGGLECHAT);
		bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_ENTER, KMOD_NONE, CLIENT_HOTKEY_CHAT_TOGGLECHAT);
	}
	else
	{
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_UP, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNNORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_LEFT, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_DOWN, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RIGHT, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNEAST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_UP, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_LEFT, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOWEST);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_DOWN, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTH);
		bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RIGHT, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOEAST);
	}
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_8, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNNORTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_4, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNWEST);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_2, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNSOUTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_6, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_TURNEAST);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_8, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_4, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOWEST);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_2, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTH);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_KP_6, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_7, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTHWEST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_9, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GONORTHEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_1, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTHWEST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_KP_3, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_GOSOUTHEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_t, (KMOD_CTRL + KMOD_ALT), CLIENT_HOTKEY_DIALOGS_OPENTERMINAL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_r, KMOD_CTRL, CLIENT_HOTKEY_MOVEMENT_MOUNT);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_z, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENBUGREPORTS);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_i, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENIGNORELIST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_k, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENHOTKEYS);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_u, KMOD_CTRL, CLIENT_HOTKEY_DIALOGS_OPENQUESTLOG);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_p, KMOD_CTRL, CLIENT_HOTKEY_WINDOWS_OPENVIPWINDOW);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_b, KMOD_CTRL, CLIENT_HOTKEY_WINDOWS_OPENBATTLEWINDOW);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_s, KMOD_CTRL, CLIENT_HOTKEY_WINDOWS_OPENSKILLSWINDOW);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_e, KMOD_CTRL, CLIENT_HOTKEY_CHAT_CLOSECHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_o, KMOD_CTRL, CLIENT_HOTKEY_CHAT_OPENCHANNELLIST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_t, KMOD_CTRL, CLIENT_HOTKEY_CHAT_OPENHELPCHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_n, KMOD_CTRL, CLIENT_HOTKEY_UI_TOGGLECREATUREINFO);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_f, KMOD_CTRL, CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_h, KMOD_CTRL, CLIENT_HOTKEY_MISC_LENSHELP);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_g, KMOD_CTRL, CLIENT_HOTKEY_MISC_CHANGECHARACTER);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_l, KMOD_CTRL, CLIENT_HOTKEY_MISC_LOGOUT);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_q, KMOD_CTRL, CLIENT_HOTKEY_MISC_LOGOUT);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_TAB, KMOD_SHIFT, CLIENT_HOTKEY_CHAT_PREVIOUSCHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_d, KMOD_ALT, CLIENT_HOTKEY_CHAT_DEFAULTCHANNEL);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_PAGEDOWN, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_FLOORDOWN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_PAGEUP, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_FLOORUP);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_RIGHT, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLEAST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_UP, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLNORTH);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_DOWN, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLSOUTH);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_LEFT, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_SCROLLWEST);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_END, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_ZOOMIN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_HOME, KMOD_ALT, CLIENT_HOTKEY_MINIMAP_ZOOMOUT);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_F8, KMOD_ALT, CLIENT_HOTKEY_UI_TOGGLEFPSINDICATOR);
	bindHotkey(CLIENT_HOTKEY_SECOND_KEY, SDLK_RETURN, KMOD_ALT, CLIENT_HOTKEY_UI_TOGGLEFULLSCREEN);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_ESCAPE, KMOD_NONE, CLIENT_HOTKEY_MOVEMENT_STOPACTIONS);
	bindHotkey(CLIENT_HOTKEY_FIRST_KEY, SDLK_TAB, KMOD_NONE, CLIENT_HOTKEY_CHAT_NEXTCHANNEL);
}

Sint32 Engine::calculateMainHeight()
{
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it)->getInternalID() == GUI_PANEL_MAIN)
			return (*it)->getRect().y2 - (*it)->getFreeHeight();
	}
	return 0;
}

void Engine::recalculateGameWindow()
{
	m_leftPanel = GUI_PANEL_RANDOM;
	m_rightPanel = GUI_PANEL_MAIN;
	m_haveExtraLeftPanel = false;
	m_haveExtraRightPanel = false;

	Sint32 minWidth = 248;
	Sint32 minHeight = 184;

	Sint32 x = 0;
	Sint32 y = 0;
	Sint32 width = m_windowW;
	Sint32 height = UTIL_max<Sint32>(minHeight, UTIL_min<Sint32>(m_windowH - 111, m_windowH - m_consoleHeight));
	Sint32 mainHeight = calculateMainHeight();
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(); it != m_panels.end(); ++it)
	{
		Sint32 panelId = (*it)->getInternalID();
		switch(panelId)
		{
			case GUI_PANEL_MAIN:
			{
				iRect panelRect = (*it)->getRect();
				panelRect.x1 = m_windowW - GAME_PANEL_FIXED_WIDTH;
				panelRect.x2 = GAME_PANEL_FIXED_WIDTH;
				panelRect.y1 = y;
				panelRect.y2 = mainHeight;
				(*it)->setRect(panelRect);
			}
			break;
			case GUI_PANEL_RIGHT:
			{
				width -= GAME_PANEL_FIXED_WIDTH;

				iRect panelRect = (*it)->getRect();
				panelRect.x1 = m_windowW - GAME_PANEL_FIXED_WIDTH;
				panelRect.x2 = GAME_PANEL_FIXED_WIDTH;
				panelRect.y1 = y + mainHeight;
				panelRect.y2 = m_windowH - mainHeight;
				(*it)->setRect(panelRect);
				(*it)->tryFreeHeight(0);
			}
			break;
			default:
			{
				if(width - GAME_PANEL_FIXED_WIDTH < minWidth)
				{
					delete (*it);
					it = m_panels.erase(it);
					--it;//We can do this assuming main panel should be first in vector
					break;
				}

				if(panelId >= GUI_PANEL_EXTRA_RIGHT_START && panelId <= GUI_PANEL_EXTRA_RIGHT_END)
				{
					width -= GAME_PANEL_FIXED_WIDTH;
					if(m_rightPanel == GUI_PANEL_MAIN || panelId > m_rightPanel)
						m_rightPanel = panelId;

					iRect panelRect = (*it)->getRect();
					panelRect.x1 = m_windowW - ((panelId - GUI_PANEL_EXTRA_RIGHT_START + 2) * GAME_PANEL_FIXED_WIDTH);
					panelRect.x2 = GAME_PANEL_FIXED_WIDTH;
					panelRect.y1 = y;
					panelRect.y2 = m_windowH;
					(*it)->setRect(panelRect);
					(*it)->tryFreeHeight(0);
					m_haveExtraRightPanel = true;
				}
				else if(panelId >= GUI_PANEL_EXTRA_LEFT_START && panelId <= GUI_PANEL_EXTRA_LEFT_END)
				{
					x += GAME_PANEL_FIXED_WIDTH;
					width -= GAME_PANEL_FIXED_WIDTH;
					if(m_leftPanel == GUI_PANEL_RANDOM || panelId > m_leftPanel)
						m_leftPanel = panelId;

					iRect panelRect = (*it)->getRect();
					panelRect.x1 = (panelId - GUI_PANEL_EXTRA_LEFT_START) * GAME_PANEL_FIXED_WIDTH;
					panelRect.x2 = GAME_PANEL_FIXED_WIDTH;
					panelRect.y1 = y;
					panelRect.y2 = m_windowH;
					(*it)->setRect(panelRect);
					(*it)->tryFreeHeight(0);
					m_haveExtraLeftPanel = true;
				}
			}
			break;
		}
	}

	m_scaledSize = UTIL_min<Sint32>((width - 4) / (GAME_MAP_WIDTH - 3), (height - 4) / (GAME_MAP_HEIGHT - 3));
	m_scale = m_scaledSize * 0.03125f;

	Sint32 windowWidth = m_scaledSize * (GAME_MAP_WIDTH - 3);
	Sint32 windowHeight = m_scaledSize * (GAME_MAP_HEIGHT - 3);
	height = UTIL_min<Sint32>(height, windowHeight + 4);
	m_consoleHeight = m_windowH - height;
	Sint32 windowX = UTIL_max<Sint32>(x + (width / 2 - windowWidth / 2), x + 2);
	Sint32 windowY = UTIL_max<Sint32>(y + (height / 2 - windowHeight / 2), y + 2);
	m_gameBackgroundRect = iRect(x, y, width, height);
	m_gameWindowRect = iRect(windowX, windowY, windowWidth, windowHeight);
	m_chatWindowRect = iRect(x, height, width, m_consoleHeight);

	bool condition = (width - windowWidth - 4 >= GAME_PANEL_FIXED_WIDTH);
	m_canAddLeftPanel = (condition ? true : false);
	m_canAddRightPanel = (condition ? true : false);
}

void Engine::setConsoleHeight(Sint32 height)
{
	m_consoleHeight = height;
	recalculateGameWindow();
}

void Engine::update()
{
	if(m_ingame)
		g_map.update();
}

void Engine::redraw()
{
	m_surface->beginScene();
	if(m_ingame)
	{
		g_map.render();
		m_surface->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, m_gameBackgroundRect.x1, m_gameBackgroundRect.y1, m_gameBackgroundRect.x2, m_gameBackgroundRect.y2);
		m_surface->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_gameWindowRect.x1 - 1, m_gameWindowRect.y1 - 1, m_gameWindowRect.x2 + 2, 1);
		m_surface->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_gameWindowRect.x1 - 1, m_gameWindowRect.y1, 1, m_gameWindowRect.y2 + 1);
		m_surface->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_gameWindowRect.x1, m_gameWindowRect.y1+ m_gameWindowRect.y2, m_gameWindowRect.x2 + 1, 1);
		m_surface->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_gameWindowRect.x1 + m_gameWindowRect.x2, m_gameWindowRect.y1, 1, m_gameWindowRect.y2);
		m_surface->drawGameScene(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, m_gameWindowRect.x1, m_gameWindowRect.y1, m_gameWindowRect.x2, m_gameWindowRect.y2);
		m_surface->setClipRect(m_gameWindowRect.x1, m_gameWindowRect.y1, m_gameWindowRect.x2, m_gameWindowRect.y2);
		g_map.renderInformations(m_gameWindowRect.x1, m_gameWindowRect.y1, m_gameWindowRect.x2, m_gameWindowRect.y2, m_scale, m_scaledSize);
		m_surface->disableClipRect();
		g_chat.render(m_chatWindowRect);
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->render();

		{
			m_leftPanelAddRect = iRect(m_gameBackgroundRect.x1, m_gameBackgroundRect.y1, GUI_UI_SIDEBAR_LEFT_ADD_UP_W, GUI_UI_SIDEBAR_LEFT_ADD_UP_H);
			if(m_leftPanel != GUI_PANEL_RANDOM)
				m_leftPanelAddRect.x1 = m_gameBackgroundRect.x1 - 2;

			if(m_canAddLeftPanel)
			{
				if(m_leftAddPanel == 1)
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_LEFT_ADD_DOWN_X, GUI_UI_SIDEBAR_LEFT_ADD_DOWN_Y, m_leftPanelAddRect.x1, m_leftPanelAddRect.y1, m_leftPanelAddRect.x2, m_leftPanelAddRect.y2);
				else
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_LEFT_ADD_UP_X, GUI_UI_SIDEBAR_LEFT_ADD_UP_Y, m_leftPanelAddRect.x1, m_leftPanelAddRect.y1, m_leftPanelAddRect.x2, m_leftPanelAddRect.y2);
			}
			else
				m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_LEFT_ADD_DISABLED_X, GUI_UI_SIDEBAR_LEFT_ADD_DISABLED_Y, m_leftPanelAddRect.x1, m_leftPanelAddRect.y1, m_leftPanelAddRect.x2, m_leftPanelAddRect.y2);

			m_leftPanelRemRect = iRect(m_leftPanelAddRect.x1, m_leftPanelAddRect.y1 + m_leftPanelAddRect.y2, GUI_UI_SIDEBAR_LEFT_REMOVE_UP_W, GUI_UI_SIDEBAR_LEFT_REMOVE_UP_H);
			if(m_haveExtraLeftPanel)
			{
				if(m_leftRemPanel == 1)
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_LEFT_REMOVE_DOWN_X, GUI_UI_SIDEBAR_LEFT_REMOVE_DOWN_Y, m_leftPanelRemRect.x1, m_leftPanelRemRect.y1, m_leftPanelRemRect.x2, m_leftPanelRemRect.y2);
				else
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_LEFT_REMOVE_UP_X, GUI_UI_SIDEBAR_LEFT_REMOVE_UP_Y, m_leftPanelRemRect.x1, m_leftPanelRemRect.y1, m_leftPanelRemRect.x2, m_leftPanelRemRect.y2);
			}
			else
				m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_LEFT_REMOVE_DISABLED_X, GUI_UI_SIDEBAR_LEFT_REMOVE_DISABLED_Y, m_leftPanelRemRect.x1, m_leftPanelRemRect.y1, m_leftPanelRemRect.x2, m_leftPanelRemRect.y2);
		}
		{
			m_rightPanelAddRect = iRect(m_gameBackgroundRect.x1 + m_gameBackgroundRect.x2 - 7, m_gameBackgroundRect.y1, GUI_UI_SIDEBAR_RIGHT_ADD_UP_W, GUI_UI_SIDEBAR_RIGHT_ADD_UP_H);
			if(m_canAddRightPanel)
			{
				if(m_rightAddPanel == 1)
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_RIGHT_ADD_DOWN_X, GUI_UI_SIDEBAR_RIGHT_ADD_DOWN_Y, m_rightPanelAddRect.x1, m_rightPanelAddRect.y1, m_rightPanelAddRect.x2, m_rightPanelAddRect.y2);
				else
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_RIGHT_ADD_UP_X, GUI_UI_SIDEBAR_RIGHT_ADD_UP_Y, m_rightPanelAddRect.x1, m_rightPanelAddRect.y1, m_rightPanelAddRect.x2, m_rightPanelAddRect.y2);
			}
			else
				m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_RIGHT_ADD_DISABLED_X, GUI_UI_SIDEBAR_RIGHT_ADD_DISABLED_Y, m_rightPanelAddRect.x1, m_rightPanelAddRect.y1, m_rightPanelAddRect.x2, m_rightPanelAddRect.y2);

			m_rightPanelRemRect = iRect(m_rightPanelAddRect.x1, m_rightPanelAddRect.y1 + m_rightPanelAddRect.y2, GUI_UI_SIDEBAR_RIGHT_REMOVE_UP_W, GUI_UI_SIDEBAR_RIGHT_REMOVE_UP_H);
			if(m_haveExtraRightPanel)
			{
				if(m_rightRemPanel == 1)
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_RIGHT_REMOVE_DOWN_X, GUI_UI_SIDEBAR_RIGHT_REMOVE_DOWN_Y, m_rightPanelRemRect.x1, m_rightPanelRemRect.y1, m_rightPanelRemRect.x2, m_rightPanelRemRect.y2);
				else
					m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_RIGHT_REMOVE_UP_X, GUI_UI_SIDEBAR_RIGHT_REMOVE_UP_Y, m_rightPanelRemRect.x1, m_rightPanelRemRect.y1, m_rightPanelRemRect.x2, m_rightPanelRemRect.y2);
			}
			else
				m_surface->drawPicture(GUI_UI_IMAGE, GUI_UI_SIDEBAR_RIGHT_REMOVE_DISABLED_X, GUI_UI_SIDEBAR_RIGHT_REMOVE_DISABLED_Y, m_rightPanelRemRect.x1, m_rightPanelRemRect.y1, m_rightPanelRemRect.x2, m_rightPanelRemRect.y2);
		}

		if(m_topPanel)
			m_topPanel->render();
	}
	else
	{
		m_surface->drawBackground(GUI_BACKGROUND_IMAGE, GUI_BACKGROUND_X, GUI_BACKGROUND_Y, GUI_BACKGROUND_W, GUI_BACKGROUND_H, 0, 0, m_windowW, m_windowH);
		if(g_mainWindow)
			g_mainWindow->render();
	}

	if(m_showPerformance)
	{
		extern Uint32 g_frameDiff;
		extern Uint16 g_lastFrames;

		Sint32 posX;
		if(m_ingame)
			posX = m_gameBackgroundRect.x1 + m_gameBackgroundRect.x2 - 5;
		else
			posX = m_windowW - 5;

		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u FPS", g_lastFrames);
		drawFont(CLIENT_FONT_OUTLINED, posX, 5, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u ms", g_frameDiff);
		drawFont(CLIENT_FONT_OUTLINED, posX, 19, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Ping: %u ms", g_ping);
		drawFont(CLIENT_FONT_OUTLINED, posX, 33, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Software: %s", m_surface->getSoftware());
		drawFont(CLIENT_FONT_OUTLINED, posX, 47, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Hardware: %s", m_surface->getHardware());
		drawFont(CLIENT_FONT_OUTLINED, posX, 61, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
		len = SDL_snprintf(g_buffer, sizeof(g_buffer), "VRAM: %u MB", m_surface->getVRAM());
		drawFont(CLIENT_FONT_OUTLINED, posX, 75, std::string(g_buffer, SDL_static_cast(size_t, len)), 255, 255, 255, CLIENT_FONT_ALIGN_RIGHT);
	}

	if(m_actWindow)
		m_actWindow->render();

	if(m_showLogger)
		g_logger.render(0, 0, m_windowW, m_windowH);

	if(m_description)
		m_description->render();

	if(m_contextMenu)
		m_contextMenu->render();

	m_surface->endScene();
}

void Engine::drawFont(Uint8 fontId, Sint32 x, Sint32 y, const std::string& text, Uint8 r, Uint8 g, Uint8 b, Sint32 align, size_t pos, size_t len)
{
	if(align != CLIENT_FONT_ALIGN_LEFT)
	{
		drawFont(fontId, x, y, text.substr(pos, len), r, g, b, align);
		return;
	}

	len += pos;
	if(len > text.length())
		len = text.length();

	m_surface->drawFont(m_charPicture[fontId], x, y, text, pos, len, r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
}

void Engine::drawFont(Uint8 fontId, Sint32 x, Sint32 y, const std::string& text, Uint8 r, Uint8 g, Uint8 b, Sint32 align)
{
	if(align != CLIENT_FONT_ALIGN_LEFT)
	{
		Sint32 rx = x, ry = y;
		size_t pos = 0, start = 0;
		while((pos = text.find('\n', pos)) != std::string::npos)
		{
			Uint32 calculatedWidth = calculateFontWidth(fontId, text, start, pos);
			switch(align)
			{
				case CLIENT_FONT_ALIGN_RIGHT:
					rx -= calculatedWidth;
					break;
				case CLIENT_FONT_ALIGN_CENTER:
					rx -= calculatedWidth / 2;
					break;
				default: break;
			}
			m_surface->drawFont(m_charPicture[fontId], rx, ry, text, start, pos, r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
			++pos;
			start = pos;
			rx = x; ry += m_charh[fontId][0];
		}

		pos = text.length();
		if(start >= pos)
			return;

		Uint32 calculatedWidth = calculateFontWidth(fontId, text, start, pos);
		switch(align)
		{
			case CLIENT_FONT_ALIGN_RIGHT:
				rx -= calculatedWidth;
				break;
			case CLIENT_FONT_ALIGN_CENTER:
				rx -= calculatedWidth / 2;
				break;
			default: break;
		}
		m_surface->drawFont(m_charPicture[fontId], rx, ry, text, start, pos, r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
		return;
	}
	m_surface->drawFont(m_charPicture[fontId], x, y, text, 0, text.length(), r, g, b, m_charx[fontId], m_chary[fontId], m_charw[fontId], m_charh[fontId]);
}

void Engine::drawItem(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern, Uint8 animation)
{
	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Default].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Default].m_height;
	Uint8 fl = thing->m_frameGroup[ThingFrameGroup_Default].m_layers;
	if(fx == 1 && fy == 1)
	{
		for(Uint8 l = 0; l < fl; ++l)
		{
			Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, 0, 0, l, xPattern, yPattern, zPattern, animation);
			if(sprite != 0)
				m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
		}
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Default].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs) + 31) / 32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled / (frs * 0.03125f));
		for(Uint8 l = 0; l < fl; ++l)
		{
			Sint32 posYc = y + scaled - scale;
			for(Uint8 cy = 0; cy < fy; ++cy)
			{
				Sint32 posXc = x + scaled - scale;
				for(Uint8 cx = 0; cx < fx; ++cx)
				{
					Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, cx, cy, l, xPattern, yPattern, zPattern, animation);
					if(sprite != 0)
					{
						Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
						Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
						if(dx < x)
						{
							Sint32 diff = x - dx;
							dx += diff;
							dw -= diff;
							sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
							sw -= sx;
						}
						if(dy < y)
						{
							Sint32 diff = y - dy;
							dy += diff;
							dh -= diff;
							sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
							sh -= sy;
						}
						m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
					}
					posXc -= scale;
				}
				posYc -= scale;
			}
		}
	}
}

void Engine::drawOutfit(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern, Uint8 animation, Uint32 outfitColor)
{
	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Idle].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Idle].m_height;
	Uint8 fl = thing->m_frameGroup[ThingFrameGroup_Idle].m_layers;
	if(fx == 1 && fy == 1)
	{
		if(fl > 1)
		{
			Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, 0, 0, 0, xPattern, yPattern, zPattern, animation);
			Uint32 spriteMask = thing->getSprite(ThingFrameGroup_Idle, 0, 0, 1, xPattern, yPattern, zPattern, animation);
			if(sprite != 0)
			{
				if(spriteMask != 0)
					m_surface->drawSpriteMask(sprite, spriteMask, x, y, scaled, scaled, 0, 0, 32, 32, outfitColor);
				else
					m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
			}
		}
		else
		{
			Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, 0, 0, 0, xPattern, yPattern, zPattern, animation);
			if(sprite != 0)
				m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
		}
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Idle].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs) + 31) / 32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled / (frs * 0.03125f));
		if(fl > 1)
		{
			Sint32 posYc = y + scaled - scale;
			for(Uint8 cy = 0; cy < fy; ++cy)
			{
				Sint32 posXc = x + scaled - scale;
				for(Uint8 cx = 0; cx < fx; ++cx)
				{
					Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, cx, cy, 0, xPattern, yPattern, zPattern, animation);
					Uint32 spriteMask = thing->getSprite(ThingFrameGroup_Idle, cx, cy, 1, xPattern, yPattern, zPattern, animation);
					if(sprite != 0)
					{
						Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
						Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
						if(dx < x)
						{
							Sint32 diff = x - dx;
							dx += diff;
							dw -= diff;
							sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
							sw -= sx;
						}
						if(dy < y)
						{
							Sint32 diff = y - dy;
							dy += diff;
							dh -= diff;
							sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
							sh -= sy;
						}
						if(spriteMask != 0)
							m_surface->drawSpriteMask(sprite, spriteMask, dx, dy, dw, dh, sx, sy, sw, sh, outfitColor);
						else
							m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
					}
					posXc -= scale;
				}
				posYc -= scale;
			}
		}
		else
		{
			Sint32 posYc = y + scaled - scale;
			for(Uint8 cy = 0; cy < fy; ++cy)
			{
				Sint32 posXc = x + scaled - scale;
				for(Uint8 cx = 0; cx < fx; ++cx)
				{
					Uint32 sprite = thing->getSprite(ThingFrameGroup_Idle, cx, cy, 0, xPattern, yPattern, zPattern, animation);
					if(sprite != 0)
					{
						Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
						Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
						if(dx < x)
						{
							Sint32 diff = x - dx;
							dx += diff;
							dw -= diff;
							sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
							sw -= sx;
						}
						if(dy < y)
						{
							Sint32 diff = y - dy;
							dy += diff;
							dh -= diff;
							sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
							sh -= sy;
						}
						m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
					}
					posXc -= scale;
				}
				posYc -= scale;
			}
		}
	}
}

void Engine::drawEffect(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern, Uint8 animation)
{
	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Default].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Default].m_height;
	if(fx == 1 && fy == 1)
	{
		Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, 0, 0, 0, xPattern, yPattern, zPattern, animation);
		if(sprite != 0)
			m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Default].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs) + 31) / 32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled / (frs * 0.03125f));
		Sint32 posYc = y + scaled - scale;
		for(Uint8 cy = 0; cy < fy; ++cy)
		{
			Sint32 posXc = x + scaled - scale;
			for(Uint8 cx = 0; cx < fx; ++cx)
			{
				Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, cx, cy, 0, xPattern, yPattern, zPattern, animation);
				if(sprite != 0)
				{
					Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
					Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
					if(dx < x)
					{
						Sint32 diff = x - dx;
						dx += diff;
						dw -= diff;
						sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
						sw -= sx;
					}
					if(dy < y)
					{
						Sint32 diff = y - dy;
						dy += diff;
						dh -= diff;
						sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
						sh -= sy;
					}
					m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
				}
				posXc -= scale;
			}
			posYc -= scale;
		}
	}
}

void Engine::drawDistanceEffect(ThingType* thing, Sint32 x, Sint32 y, Sint32 scaled, Uint8 xPattern, Uint8 yPattern, Uint8 zPattern, Uint8 animation)
{
	Uint8 fx = thing->m_frameGroup[ThingFrameGroup_Default].m_width;
	Uint8 fy = thing->m_frameGroup[ThingFrameGroup_Default].m_height;
	if(fx == 1 && fy == 1)
	{
		Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, 0, 0, 0, xPattern, yPattern, zPattern, animation);
		if(sprite != 0)
			m_surface->drawSprite(sprite, x, y, scaled, scaled, 0, 0, 32, 32);
	}
	else
	{
		Uint8 frs = thing->m_frameGroup[ThingFrameGroup_Default].m_realSize;
		Uint8 fmax = SDL_static_cast(Uint8, (SDL_static_cast(Uint32, frs) + 31) / 32);
		fx = (fx > fmax ? fmax : fx);
		fy = (fy > fmax ? fmax : fy);
		Sint32 scale = SDL_static_cast(Sint32, scaled / (frs * 0.03125f));
		Sint32 posYc = y + scaled - scale;
		for(Uint8 cy = 0; cy < fy; ++cy)
		{
			Sint32 posXc = x + scaled - scale;
			for(Uint8 cx = 0; cx < fx; ++cx)
			{
				Uint32 sprite = thing->getSprite(ThingFrameGroup_Default, cx, cy, 0, xPattern, yPattern, zPattern, animation);
				if(sprite != 0)
				{
					Sint32 dx = posXc, dy = posYc, dw = scale, dh = scale;
					Sint32 sx = 0, sy = 0, sw = 32, sh = 32;
					if(dx < x)
					{
						Sint32 diff = x - dx;
						dx += diff;
						dw -= diff;
						sx = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
						sw -= sx;
					}
					if(dy < y)
					{
						Sint32 diff = y - dy;
						dy += diff;
						dh -= diff;
						sy = SDL_static_cast(Sint32, SDL_static_cast(float, diff) / SDL_static_cast(float, scale) * 32.f);
						sh -= sy;
					}
					m_surface->drawSprite(sprite, dx, dy, dw, dh, sx, sy, sw, sh);
				}
				posXc -= scale;
			}
			posYc -= scale;
		}
	}
}

unsigned char* Engine::LoadSprite(Uint32 spriteId, bool bgra)
{
	if(spriteId == 0 || spriteId > g_spriteCounts)
		return NULL;

	if(bgra)
		return g_spriteManager.LoadSprite_BGRA(spriteId);
	return g_spriteManager.LoadSprite_RGBA(spriteId);
}

unsigned char* Engine::LoadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, bool bgra)
{
	unsigned char* pixels = LoadSprite(spriteId, bgra);
	if(!pixels)
		return NULL;

	unsigned char* tempPixels = LoadSprite(maskSpriteId, bgra);
	if(!tempPixels)
	{
		SDL_free(pixels);
		return NULL;
	}

	float hR, hG, hB, bR, bG, bB, lR, lG, lB, fR, fG, fB;
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor), hR, hG, hB);
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor >> 8), bR, bG, bB);
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor >> 16), lR, lG, lB);
	getOutfitColorFloat(SDL_static_cast(Uint8, outfitColor >> 24), fR, fG, fB);
	if(bgra)
	{
		for(Uint16 i = 0; i <= 4092; i += 4)
		{
			if(tempPixels[i + 3] == SDL_ALPHA_OPAQUE)
			{
				Uint32 U32pixel = *SDL_reinterpret_cast(Uint32*, &tempPixels[i]);
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Head
				if(U32pixel == 0xFFFFFF00)
				#else
				if(U32pixel == 0x00FFFFFF)
				#endif
				{
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * hR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * hG);
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * hB);
				}
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Body
				else if(U32pixel == 0xFFFF0000)
				#else
				else if(U32pixel == 0x0000FFFF)
				#endif
				{
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * bR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * bG);
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * bB);
				}
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Legs
				else if(U32pixel == 0xFF00FF00)
				#else
				else if(U32pixel == 0x00FF00FF)
				#endif
				{
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * lR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * lG);
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * lB);
				}
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Feet
				else if(U32pixel == 0xFF0000FF)
				#else
				else if(U32pixel == 0xFF0000FF)
				#endif
				{
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * fR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * fG);
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * fB);
				}
			}
		}
	}
	else
	{
		for(Uint16 i = 0; i <= 4092; i += 4)
		{
			if(tempPixels[i + 3] == SDL_ALPHA_OPAQUE)
			{
				Uint32 U32pixel = *SDL_reinterpret_cast(Uint32*, &tempPixels[i]);
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Head
				if(U32pixel == 0xFF00FFFF)
				#else
				if(U32pixel == 0xFFFF00FF)
				#endif
				{
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * hR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * hG);
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * hB);
				}
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Body
				else if(U32pixel == 0xFF0000FF)
				#else
				else if(U32pixel == 0xFF0000FF)
				#endif
				{
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * bR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * bG);
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * bB);
				}
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Legs
				else if(U32pixel == 0xFF00FF00)
				#else
				else if(U32pixel == 0x00FF00FF)
				#endif
				{
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * lR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * lG);
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * lB);
				}
				#if SDL_BYTEORDER == SDL_LIL_ENDIAN //Feet
				else if(U32pixel == 0xFFFF0000)
				#else
				else if(U32pixel == 0x0000FFFF)
				#endif
				{
					pixels[i] = SDL_static_cast(Uint8, pixels[i] * fR);
					pixels[i + 1] = SDL_static_cast(Uint8, pixels[i + 1] * fG);
					pixels[i + 2] = SDL_static_cast(Uint8, pixels[i + 2] * fB);
				}
			}
		}
	}
	SDL_free(tempPixels);
	return pixels;
}

unsigned char* Engine::LoadPicture(Uint16 pictureId, bool bgra, Sint32& width, Sint32& height)
{
	if(pictureId >= g_pictureCounts)
		return NULL;

	SDL_RWops* pictures = SDL_RWFromFile(g_picPath.c_str(), "rb");
	if(!pictures)
		return NULL;

	SDL_RWseek(pictures, 6, RW_SEEK_SET);
	for(Uint16 i = 0; i <= pictureId; ++i)
	{
		Uint8 w = SDL_ReadU8(pictures);
		Uint8 h = SDL_ReadU8(pictures);
		SDL_RWseek(pictures, 3, RW_SEEK_CUR);//ignore colorkey
		if(i == pictureId)
		{
			width = SDL_static_cast(Sint32, w * 32);
			height = SDL_static_cast(Sint32, h * 32);

			Uint32 protectionSize = width * height * 4;
			unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_calloc(protectionSize, sizeof(unsigned char)));
			if(!pixels)
			{
				SDL_RWclose(pictures);
				return NULL;
			}

			Uint32 chunkLoc, oldLoc;
			protectionSize -= 4;
			for(Uint8 y = 0; y < h; ++y)
			{
				for(Uint8 x = 0; x < w; ++x)
				{
					Uint16 pixelSize, chunkSize;
					Uint16 readData = 0, writeData = 0;

					chunkLoc = SDL_ReadLE32(pictures);
					oldLoc = SDL_static_cast(Uint32, SDL_RWtell(pictures));
					SDL_RWseek(pictures, chunkLoc, RW_SEEK_SET);

					pixelSize = SDL_ReadLE16(pictures);
					while(readData < pixelSize)
					{
						chunkSize = SDL_ReadLE16(pictures);
						writeData += chunkSize;
						chunkSize = SDL_ReadLE16(pictures);
						readData += 4;
						for(Uint16 j = 0; j < chunkSize; ++j)
						{
							Uint16 xPos = x * 32 + (writeData + j) % 32;
							Uint16 yPos = y * 32 + (writeData + j) / 32;
							Uint32 offset = (yPos * width + xPos) * 4;
							if(offset <= protectionSize)
							{
								if(bgra)
								{
									pixels[offset + 2] = SDL_ReadU8(pictures);
									pixels[offset + 1] = SDL_ReadU8(pictures);
									pixels[offset] = SDL_ReadU8(pictures);
									pixels[offset + 3] = SDL_ReadU8(pictures);
								}
								else
								{
									pixels[offset] = SDL_ReadU8(pictures);
									pixels[offset + 1] = SDL_ReadU8(pictures);
									pixels[offset + 2] = SDL_ReadU8(pictures);
									pixels[offset + 3] = SDL_ReadU8(pictures);
								}
								readData += 4;
							}
						}
						writeData += chunkSize;
					}
					SDL_RWseek(pictures, oldLoc, RW_SEEK_SET);
				}
			}
			SDL_RWclose(pictures);
			return pixels;
		}
		else
			SDL_RWseek(pictures, w * h * 4, RW_SEEK_CUR);
	}
	SDL_RWclose(pictures);
	return NULL;
}

void Engine::issueNewConnection(bool protocolGame)
{
	releaseConnection();

	Protocol* protocol;
	if(protocolGame)
	{
		protocol = new ProtocolGame();

		CharacterDetail& character = m_characters[SDL_static_cast(size_t, m_characterSelectId)];
		#if CLIENT_OVVERIDE_VERSION == 0
		g_connection = new Connection(character.worldIp.c_str(), character.worldPort, m_clientProxy.c_str(), m_clientProxyAuth.c_str(), protocol);
		#else
		g_connection = new Connection(character.worldIp.c_str(), character.worldPort, "", "", protocol);
		#endif
	}
	else
	{
		protocol = new ProtocolLogin();
		#if CLIENT_OVVERIDE_VERSION == 0
		g_connection = new Connection(m_clientHost.c_str(), SDL_static_cast(Uint16, SDL_strtoul(m_clientPort.c_str(), NULL, 10)), m_clientProxy.c_str(), m_clientProxyAuth.c_str(), protocol);
		#else
		g_connection = new Connection(CLIENT_OVERRIDE_LOGIN_HOST, CLIENT_OVERRIDE_LOGIN_PORT, "", "", protocol);
		#endif
	}
}

void Engine::releaseConnection()
{
	if(g_connection)
	{
		delete g_connection;
		g_connection = NULL;
	}
}

void Engine::initMove(Uint16 posX, Uint16 posY, Uint8 posZ)
{
	ClientActionData& actionData = g_engine.m_actionDataStructure[CLIENT_ACTION_FIRST];
	Position fromPos = Position(actionData.posX, actionData.posY, actionData.posZ);
	Position toPos = Position(posX, posY, posZ);
	Uint16 itemCount = m_actionDataStructure[CLIENT_ACTION_SECOND].itemId;
	if(itemCount <= 1)
		g_game.sendMove(fromPos, actionData.itemId, actionData.posStack, toPos, 1);
	else
	{
		Uint16 keyMods = UTIL_parseModifiers(SDL_static_cast(Uint16, SDL_GetModState()));
		if(keyMods == KMOD_CTRL)
			g_game.sendMove(fromPos, actionData.itemId, actionData.posStack, toPos, itemCount);
		else if(keyMods == KMOD_SHIFT)
			g_game.sendMove(fromPos, actionData.itemId, actionData.posStack, toPos, 1);
		else
		{
			setActionData(CLIENT_ACTION_SECOND, 0, itemCount, posX, posY, posZ, 0);
			UTIL_createItemMove();
		}
	}
}

void Engine::standardThingEvent(Uint32 event, Sint32)
{
	ClientActionData& actionData = g_engine.m_actionDataStructure[CLIENT_ACTION_FIRST];
	switch(event)
	{
		case THING_EVENT_LOOK:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x == 0xFFFF)
				g_game.sendLookAt(pos, actionData.itemId, actionData.posStack);
			else
			{
				Uint32 creatureId = actionData.creatureId;
				Creature* creature = g_map.getCreatureById(creatureId);
				if(creature)
				{
					if(g_game.hasGameFeature(GAME_FEATURE_LOOKATCREATURE))
					{
						g_game.sendLookInBattle(creatureId);
						return;
					}
					else
					{
						Position& position = creature->getCurrentPosition();
						Tile* creatureTile = g_map.getTile(position);
						if(creatureTile)
						{
							g_game.sendLookAt(position, 0x62, SDL_static_cast(Uint8, creatureTile->getThingStackPos(creature)));
							return;
						}
					}
				}

				Tile* tile = g_map.getTile(pos);
				if(tile)
				{
					Thing* lookThing = tile->getTopLookThing();
					if(lookThing)
						g_game.sendLookAt(pos, (lookThing->isItem() ? lookThing->getItem()->getID() : 0x62), SDL_static_cast(Uint8, tile->getThingStackPos(lookThing)));
				}
			}
		}
		break;
		case THING_EVENT_OPEN:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x == 0xFFFF)
			{
				if(pos.y & 0x40)
					g_game.sendUseItem(pos, actionData.itemId, actionData.posStack, (pos.y & 0x0F));
				else
					g_game.sendUseItem(pos, actionData.itemId, actionData.posStack, g_game.findEmptyContainerId());
			}
			else
			{
				Tile* tile = g_map.getTile(pos);
				if(tile)
				{
					Thing* useThing = tile->getTopUseThing();
					if(useThing)
						g_game.sendUseItem(pos, (useThing->isItem() ? useThing->getItem()->getID() : 0x62), SDL_static_cast(Uint8, tile->getUseStackPos(useThing)), g_game.findEmptyContainerId());
				}
			}
		}
		break;
		case THING_EVENT_USE:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x == 0xFFFF)
				g_game.sendUseItem(pos, actionData.itemId, actionData.posStack, g_game.findEmptyContainerId());
			else
			{
				Tile* tile = g_map.getTile(pos);
				if(tile)
				{
					Thing* useThing = tile->getTopUseThing();
					if(useThing)
						g_game.sendUseItem(pos, (useThing->isItem() ? useThing->getItem()->getID() : 0x62), SDL_static_cast(Uint8, tile->getUseStackPos(useThing)), g_game.findEmptyContainerId());
				}
			}
		}
		break;
		case THING_EVENT_USEWITH:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x == 0xFFFF)
				g_engine.setAction(CLIENT_ACTION_USEWITH);
			else
			{
				Tile* tile = g_map.getTile(pos);
				if(tile)
				{
					Thing* useThing = tile->getTopUseThing();
					if(useThing && useThing->isItem())
					{
						Item* item = useThing->getItem();
						if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_MultiUse))
						{
							g_engine.setActionData(CLIENT_ACTION_FIRST, 0, item->getID(), pos.x, pos.y, pos.z, SDL_static_cast(Uint8, tile->getUseStackPos(useThing)));
							g_engine.setAction(CLIENT_ACTION_USEWITH);
						}
					}
				}
			}
		}
		break;
		case THING_EVENT_ROTATE:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x != 0xFFFF)
			{
				Tile* tile = g_map.getTile(pos);
				if(tile)
				{
					Thing* useThing = tile->getTopUseThing();
					if(useThing)
						g_game.sendRotateItem(pos, (useThing->isItem() ? useThing->getItem()->getID() : 0x62), SDL_static_cast(Uint8, tile->getUseStackPos(useThing)));
				}
			}
		}
		break;
		case THING_EVENT_TRADE:
		{
			//Trade
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x == 0xFFFF)
				g_engine.setAction(CLIENT_ACTION_TRADE);
			else
			{
				Tile* tile = g_map.getTile(pos);
				if(tile)
				{
					Thing* useThing = tile->getTopUseThing();
					if(useThing && useThing->isItem())
					{
						Item* item = useThing->getItem();
						g_engine.setActionData(CLIENT_ACTION_FIRST, 0, item->getID(), pos.x, pos.y, pos.z, SDL_static_cast(Uint8, tile->getUseStackPos(useThing)));
						g_engine.setAction(CLIENT_ACTION_TRADE);
					}
				}
			}
		}
		break;
		case THING_EVENT_WRAP:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x != 0xFFFF)
			{
				Tile* tile = g_map.getTile(pos);
				if(tile)
				{
					Thing* useThing = tile->getTopUseThing();
					if(useThing)
						g_game.sendWrapState(pos, (useThing->isItem() ? useThing->getItem()->getID() : 0x62), SDL_static_cast(Uint8, tile->getUseStackPos(useThing)));
				}
			}
		}
		break;
		case THING_EVENT_MOVEUP:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x == 0xFFFF)
				g_game.sendMove(pos, actionData.itemId, 0, Position(pos.x, pos.y, 0xFE), g_engine.m_actionDataStructure[CLIENT_ACTION_SECOND].itemId);
		}
		break;
		case THING_EVENT_BROWSEFIELD:
		{
			Position pos = Position(actionData.posX, actionData.posY, actionData.posZ);
			if(pos.x != 0xFFFF)
				g_game.sendBrowseField(pos);
		}
		break;
		case THING_EVENT_REPORTCOORDS:
		{
			//Report Coordinate
		}
		break;
		case THING_EVENT_SETOUTFIT: g_game.sendRequestOutfit(); break;
		case THING_EVENT_MOUNT: g_game.sendMount(true); break;
		case THING_EVENT_DISMOUNT: g_game.sendMount(false); break;
		case THING_EVENT_ATTACK: g_game.sendAttack((g_game.getAttackID() == actionData.creatureId) ? NULL : g_map.getCreatureById(actionData.creatureId)); break;
		case THING_EVENT_FOLLOW: g_game.sendFollow((g_game.getFollowID() == actionData.creatureId) ? NULL : g_map.getCreatureById(actionData.creatureId)); break;
		case THING_EVENT_COPYNAME:
		{
			Creature* creature = g_map.getCreatureById(actionData.creatureId);
			if(creature)
				UTIL_SetClipboardTextLatin1(creature->getName().c_str());
		}
		break;
		case THING_EVENT_INVITETOPARTY: g_game.sendInviteToParty(actionData.creatureId); break;
		case THING_EVENT_JOINTOPARTY: g_game.sendJoinToParty(actionData.creatureId); break;
		case THING_EVENT_REVOKEINVITATION: g_game.sendRevokePartyInvitation(actionData.creatureId); break;
		case THING_EVENT_PASSLEADERSHIP: g_game.sendPassPartyLeadership(actionData.creatureId); break;
		case THING_EVENT_LEAVEPARTY: g_game.sendLeaveParty(); break;
		case THING_EVENT_ENABLESHAREDEXPERIENCE: g_game.sendEnableSharedPartyExperience(true); break;
		case THING_EVENT_DISABLESHAREDEXPERIENCE: g_game.sendEnableSharedPartyExperience(false); break;
		case THING_EVENT_JOINAGGRESSION: g_game.sendJoinAggression(actionData.creatureId); break;
		case THING_EVENT_MESSAGETO:
		{
			Creature* creature = g_map.getCreatureById(actionData.creatureId);
			if(creature)
				g_game.sendOpenPrivateChannel(creature->getName());
		}
		break;
		case THING_EVENT_INVITETOPRIVATECHAT:
		{
			Creature* creature = g_map.getCreatureById(actionData.creatureId);
			if(creature)
				g_game.sendChannelInvite(creature->getName(), SDL_static_cast(Uint16, g_chat.getOwnPrivateChannel()));
		}
		break;
		case THING_EVENT_EXCLUDEFROMPRIVATECHAT:
		{
			Creature* creature = g_map.getCreatureById(actionData.creatureId);
			if(creature)
				g_game.sendChannelExclude(creature->getName(), SDL_static_cast(Uint16, g_chat.getOwnPrivateChannel()));
		}
		break;
		case THING_EVENT_ADDTOVIPLIST:
		{
			Creature* creature = g_map.getCreatureById(actionData.creatureId);
			if(creature)
				g_game.sendAddVip(creature->getName());
		}
		break;
		case THING_EVENT_IGNORE:
		{
			Creature* creature = g_map.getCreatureById(actionData.creatureId);
			if(creature)
				UTIL_toggleIgnore(creature->getName());
		}
		break;
		case THING_EVENT_RULEVIOLATION:
		{
			//Rule Violation
		}
		break;
		case THING_EVENT_REPORTNAME:
		{
			//Report Name
		}
		break;
		case THING_EVENT_REPORTBOTMACRO:
		{
			//Report Bot/Macro
		}
		break;
		default: break;
	}
}

GUI_ContextMenu* Engine::createThingContextMenu(Creature* creature, ItemUI* itemui, Item* item)
{
	GUI_ContextMenu* newMenu = new GUI_ContextMenu();
	if(item)
	{
		Position& position = item->getCurrentPosition();
		newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_LOOK, "Look", (m_classicControl ? "" : "(Shift)"));

		ThingType* itemType = item->getThingType();
		if(itemType)
		{
			if(itemType->hasFlag(ThingAttribute_Container))
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_OPEN, "Open", (m_classicControl ? "" : "(Ctrl)"));
			else if(itemType->hasFlag(ThingAttribute_MultiUse))
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_USEWITH, "Use with ...", (m_classicControl ? "" : "(Ctrl)"));
			else
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_USE, "Use", (m_classicControl ? "" : "(Ctrl)"));

			if(itemType->hasFlag(ThingAttribute_Wrapable))
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_WRAP, "Wrap", "");
			else if(itemType->hasFlag(ThingAttribute_Unwrapable))
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_WRAP, "Unwrap", "");

			if(itemType->hasFlag(ThingAttribute_Rotateable))
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_ROTATE, "Rotate", "");

			if(g_game.hasGameFeature(GAME_FEATURE_BROWSEFIELD))
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_BROWSEFIELD, "Browse Field", "");

			/*
			if(g_game.canReportBugs())
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_REPORTCOORDS, "Report Coordinate", "");
			*/

			if(itemType->hasFlag(ThingAttribute_Pickupable))
			{
				newMenu->addSeparator();
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_TRADE, "Trade with ...", "");
			}
		}
		else
			newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_USE, "Use", (m_classicControl ? "" : "(Ctrl)"));

		ClientActionData& actionData = m_actionDataStructure[CLIENT_ACTION_FIRST];
		actionData.itemId = item->getID();
		actionData.posX = position.x;
		actionData.posY = position.y;
		actionData.posZ = position.z;
		actionData.posStack = 0;
	}
	else if(itemui)
	{
		Position& position = itemui->getCurrentPosition();
		newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_LOOK, "Look", (m_classicControl ? "" : "(Shift)"));

		ThingType* itemType = itemui->getThingType();
		if(itemType)
		{
			if(itemType->hasFlag(ThingAttribute_Container))
			{
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_OPEN, "Open", (m_classicControl ? "" : "(Ctrl)"));
				if(position.y & 0x40)
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_USE, "Open in new window", "");
			}
			else if(itemType->hasFlag(ThingAttribute_MultiUse))
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_USEWITH, "Use with ...", (m_classicControl ? "" : "(Ctrl)"));
			else
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_USE, "Use", (m_classicControl ? "" : "(Ctrl)"));

			if(itemType->hasFlag(ThingAttribute_Pickupable))
			{
				newMenu->addSeparator();
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_TRADE, "Trade with ...", "");
			}

			if((position.y & 0x40) && g_game.containerHasParent(position.y & 0x0F))
			{
				m_actionDataStructure[CLIENT_ACTION_SECOND].itemId = itemui->getItemCount();
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_MOVEUP, "Move up", "");
			}
		}
		else
			newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_USE, "Use", (m_classicControl ? "" : "(Ctrl)"));

		ClientActionData& actionData = m_actionDataStructure[CLIENT_ACTION_FIRST];
		actionData.itemId = itemui->getID();
		actionData.posX = position.x;
		actionData.posY = position.y;
		actionData.posZ = position.z;
		actionData.posStack = 0;
	}
	else
	{
		ClientActionData& actionData = m_actionDataStructure[CLIENT_ACTION_FIRST];
		actionData.itemId = 0;
		actionData.posX = 0xFFFF;
		actionData.posY = 0;
		actionData.posZ = 0;
		actionData.posStack = 0;
	}

	if(creature)
	{
		newMenu->addSeparator();
		if(g_game.getPlayerID() == creature->getId())
		{
			newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_SETOUTFIT, "Set Outfit", "");
			if(g_game.hasGameFeature(GAME_FEATURE_MOUNTS))
			{
				if(!creature->getMountType())
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_MOUNT, "Mount", "");
				else
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_DISMOUNT, "Dismount", "");
			}

			if(!(g_game.getIcons() & ICON_SWORDS))
			{
				Uint8 playerShield = creature->getShield();
				if(UTIL_isPartyMember(playerShield))
				{
					if(UTIL_isPartyLeader(playerShield))
					{
						if(UTIL_isPartySharedEnabled(playerShield))
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_DISABLESHAREDEXPERIENCE, "Disable Shared Experience", "");
						else
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_ENABLESHAREDEXPERIENCE, "Enable Shared Experience", "");
					}

					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_LEAVEPARTY, "Leave Party", "");
				}
			}
		}
		else
		{
			newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_ATTACK, (g_game.getAttackID() == creature->getId() ? "Stop Attack" : "Attack"), (m_classicControl ? "" : "(Alt)"));
			newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_FOLLOW, (g_game.getFollowID() == creature->getId() ? "Stop Follow" : "Follow"), "");
			if(creature->isPlayer())
			{
				std::string& creatureName = creature->getName();
				newMenu->addSeparator();

				SDL_snprintf(g_buffer, sizeof(g_buffer), "Message to %s", creatureName.c_str());
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_MESSAGETO, g_buffer, "");
				if(g_chat.getOwnPrivateChannel() != SDL_static_cast(Uint32, -1))
				{
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_INVITETOPRIVATECHAT, "Invite to private chat", "");
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_EXCLUDEFROMPRIVATECHAT, "Exclude from private chat", "");
				}
				if(!UTIL_haveVipPlayer(creatureName))
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_ADDTOVIPLIST, "Add to VIP list", "");

				if(!UTIL_onWhiteList(creatureName))
				{
					SDL_snprintf(g_buffer, sizeof(g_buffer), (UTIL_onBlackList(creatureName) ? "Unignore %s" : "Ignore %s"), creatureName.c_str());
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_IGNORE, g_buffer, "");
				}

				Uint8 targetShield = creature->getShield();
				Uint8 playerShield = (g_map.getLocalCreature() ? g_map.getLocalCreature()->getShield() : SHIELD_NONE);
				switch(playerShield)
				{
					case SHIELD_NONE:
					case SHIELD_WHITEBLUE:
					{
						if(targetShield == SHIELD_WHITEYELLOW)
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Join %s's Party", creatureName.c_str());
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_JOINTOPARTY, g_buffer, "");
						}
						else if(targetShield != SHIELD_GRAY)
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_INVITETOPARTY, "Invite to Party", "");
					}
					break;
					case SHIELD_WHITEYELLOW:
					{
						if(targetShield == SHIELD_WHITEBLUE)
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Revoke %s's Invitation", creatureName.c_str());
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_REVOKEINVITATION, g_buffer, "");
						}
					}
					break;
					case SHIELD_BLUE:
					case SHIELD_BLUE_SHAREDEXP:
					case SHIELD_BLUE_NOSHAREDEXP_BLINK:
					case SHIELD_BLUE_NOSHAREDEXP:
					{
						if(targetShield > SHIELD_WHITEBLUE)
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_JOINAGGRESSION, "Join Aggression", "");
					}
					break;
					case SHIELD_YELLOW:
					case SHIELD_YELLOW_SHAREDEXP:
					case SHIELD_YELLOW_NOSHAREDEXP_BLINK:
					case SHIELD_YELLOW_NOSHAREDEXP:
					{
						if(targetShield == SHIELD_WHITEBLUE)
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Revoke %s's Invitation", creatureName.c_str());
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_REVOKEINVITATION, g_buffer, "");
						}
						else if(targetShield == SHIELD_BLUE || targetShield == SHIELD_BLUE_SHAREDEXP || targetShield == SHIELD_BLUE_NOSHAREDEXP_BLINK || targetShield == SHIELD_BLUE_NOSHAREDEXP)
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Pass Leadership to %s", creatureName.c_str());
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_PASSLEADERSHIP, g_buffer, "");
						}
						else if(targetShield == SHIELD_GRAY)
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_JOINAGGRESSION, "Join Aggression", "");
						else
							newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_INVITETOPARTY, "Invite to Party", "");
					}
					break;
					default: break;
				}
			}
		}

		/*
		if(hasRuleViolation())
		{
			newMenu->addSeparator();
			newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_RULEVIOLATION, "Rule Violation", "");
		}
		//Report Name
		//Report Bot/Macro
		*/

		newMenu->addSeparator();
		newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, THING_EVENT_COPYNAME, "Copy Name", "");
		m_actionDataStructure[CLIENT_ACTION_FIRST].creatureId = creature->getId();
	}
	else
		m_actionDataStructure[CLIENT_ACTION_FIRST].creatureId = 0;

	return newMenu;
}

void Engine::enableMoveItem(Sint32 x, Sint32 y)
{
	m_moveItemX = x;
	m_moveItemY = y;
}

void Engine::setActionData(ClientActions data, Uint32 creatureId, Uint16 itemId, Uint16 posX, Uint16 posY, Uint8 posZ, Uint8 posStack)
{
	ClientActionData& actionData = m_actionDataStructure[data];
	actionData.creatureId = creatureId;
	actionData.itemId = itemId;
	actionData.posX = posX;
	actionData.posY = posY;
	actionData.posZ = posZ;
	actionData.posStack = posStack;
}

void Engine::setAction(ClientActions action)
{
	m_actionData = action;
	if(m_actionData == CLIENT_ACTION_MOVEITEM || m_actionData == CLIENT_ACTION_USEWITH || m_actionData == CLIENT_ACTION_TRADE || m_actionData == CLIENT_ACTION_SEARCHHOTKEY)
		setCursor(CLIENT_CURSOR_CROSSHAIR);
	else if(m_actionData == CLIENT_ACTION_LENSHELP)
		setCursor(CLIENT_CURSOR_LENSHELP);
	else
		setCursor(CLIENT_CURSOR_ARROW);
}

void Engine::showContextMenu(GUI_ContextMenu* menu, Sint32 mouseX, Sint32 mouseY)
{
	if(m_contextMenu)
		delete m_contextMenu;
	m_contextMenu = menu;
	m_contextMenu->setDisplay(mouseX, mouseY);
}

void Engine::showDescription(Sint32 mouseX, Sint32 mouseY, const std::string& description, Uint32 delay)
{
	if(!m_description)
		m_description = new GUI_Description();
	m_description->setDisplay(mouseX, mouseY, description, delay);
}

bool Engine::addToPanel(GUI_PanelWindow* pPanel, Sint32 preferredPanel)
{
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if(preferredPanel != GUI_PANEL_RANDOM)
		{
			if((*it)->getInternalID() == preferredPanel)
			{
				if((*it)->getFreeHeight() >= pPanel->getRect().y2 || (*it)->tryFreeHeight(pPanel))
				{
					if(pPanel->getParent())
						pPanel->getParent()->removePanel(pPanel, false);

					(*it)->addPanel(pPanel);
					pPanel->setParent((*it));
					return true;
				}
				return false;
			}
		}
		else
		{
			if((*it)->getFreeHeight() >= pPanel->getRect().y2)
			{
				if(pPanel->getParent())
					pPanel->getParent()->removePanel(pPanel, false);

				(*it)->addPanel(pPanel);
				pPanel->setParent((*it));
				return true;
			}
		}
	}
	if(preferredPanel == GUI_PANEL_RANDOM)
	{
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		{
			if((*it)->tryFreeHeight(pPanel))
			{
				if(pPanel->getParent())
					pPanel->getParent()->removePanel(pPanel, false);

				(*it)->addPanel(pPanel);
				pPanel->setParent((*it));
				return true;
			}
		}
	}
	return false;
}

void Engine::removePanelWindow(GUI_PanelWindow* pPanel)
{
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		(*it)->removePanel(pPanel, false);

	delete pPanel;
}

GUI_PanelWindow* Engine::getPanel(Uint32 internalID)
{
	GUI_PanelWindow* panel = NULL;
	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		panel = (*it)->getPanel(internalID);
		if(panel)
			return panel;
	}
	return panel;
}

void Engine::clearPanels()
{
	if(!m_panels.empty())
	{
		m_openDialogs.clear();
		for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		{
			std::vector<GUI_PanelWindow*>& windows = (*it)->getPanelWindows();
			for(std::vector<GUI_PanelWindow*>::iterator wit = windows.begin(), wend = windows.end(); wit != wend; ++wit)
			{
				Uint32 windowId = (*wit)->getInternalID();
				switch(windowId)
				{
					//Save only windows that needed saving
					case GUI_PANEL_WINDOW_INVENTORY:
					case GUI_PANEL_WINDOW_INVENTORY_MINIMIZED:
					case GUI_PANEL_WINDOW_MINIMAP:
					case GUI_PANEL_WINDOW_HEALTH:
					case GUI_PANEL_WINDOW_BUTTONS:
					case GUI_PANEL_WINDOW_SKILLS:
					case GUI_PANEL_WINDOW_BATTLE:
					case GUI_PANEL_WINDOW_VIP:
					case GUI_PANEL_WINDOW_SPELL_LIST:
					case GUI_PANEL_WINDOW_UNJUSTIFIED_POINTS:
					case GUI_PANEL_WINDOW_PREY_WIDGET:
					case GUI_PANEL_WINDOW_PARTY:
					case GUI_PANEL_WINDOW_TOURNAMENT_WIDGET:
					case GUI_PANEL_WINDOW_ANALYTICS_SELECTOR:
					case GUI_PANEL_WINDOW_ANALYTICS_HUNTING:
					case GUI_PANEL_WINDOW_ANALYTICS_LOOT:
					case GUI_PANEL_WINDOW_ANALYTICS_SUPPLY:
					case GUI_PANEL_WINDOW_ANALYTICS_IMPACT:
					case GUI_PANEL_WINDOW_ANALYTICS_XP:
					case GUI_PANEL_WINDOW_ANALYTICS_DROP:
					case GUI_PANEL_WINDOW_ANALYTICS_QUEST:
					{
						m_parentWindows[windowId] = (*it)->getInternalID();
						m_openDialogs.push_back(windowId);
					}
					break;
					default: break;
				}
			}
		}
	}

	for(std::vector<GUI_Panel*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		m_toReleasePanels.push_back((*it));
	m_panels.clear();
}

void Engine::checkPanelWindows(GUI_PanelWindow* pPanel, Sint32 x, Sint32 y)
{
	GUI_Panel* gPanel = NULL;
	iRect currentRect = pPanel->getRect();
	GUI_Panel* parent = pPanel->getParent();
	std::vector<GUI_Panel*>::iterator it = m_panels.begin();
	for(std::vector<GUI_Panel*>::iterator end = m_panels.end(); it != end; ++it)
	{
		if((*it)->isInsideRect(x, y))
		{
			if((*it) == parent)
			{
				parent->checkPanel(pPanel, x, y);
				pPanel->setRect(currentRect);
			}
			else if(pPanel->isParentChangeable())
			{
				if((*it)->getFreeHeight() >= currentRect.y2)
				{
					gPanel = (*it);
					break;
				}
			}
			return;
		}
	}
	if(gPanel)
	{
		if(parent)
			parent->removePanel(pPanel, false);

		gPanel->addPanel(pPanel);
		gPanel->checkPanel(pPanel, x, y);
		pPanel->setRect(currentRect);
		pPanel->setParent(gPanel);
	}
}

void Engine::resizePanel(GUI_PanelWindow* pPanel, Sint32 x, Sint32 y)
{
	GUI_Panel* parent = pPanel->getParent();
	if(parent)
		parent->resizePanel(pPanel, x, y);
}

void Engine::processGameStart()
{
	m_ingame = true;
	g_ping = 0;

	g_chat.gameStart();
	g_game.reset();
	g_game.resetPlayerExperienceTable();
	clearWindows();
	clearPanels();
	checkReleaseQueue();

	g_game.sendPing();
	g_game.sendPingBack();
	g_game.sendAttackModes();

	bool haveMinimap = false, haveHealth = false, haveInventory = false, haveButtons = false;
	m_panels.push_back(new GUI_Panel(iRect(0, 0, GAME_PANEL_FIXED_WIDTH, 1000), GUI_PANEL_MAIN));
	for(std::vector<Uint32>::iterator it = m_openDialogs.begin(), end = m_openDialogs.end(); it != end; ++it)
	{
		Uint32 windowId = (*it);
		if(!haveMinimap && windowId == GUI_PANEL_WINDOW_MINIMAP)
		{
			UTIL_createMinimapPanel();
			haveMinimap = true;
		}
		else if(!haveHealth && windowId == GUI_PANEL_WINDOW_HEALTH)
		{
			UTIL_createHealthPanel();
			haveHealth = true;
		}
		else if(!haveInventory && windowId == GUI_PANEL_WINDOW_INVENTORY)
		{
			UTIL_createInventoryPanel(false);
			haveInventory = true;
		}
		else if(!haveInventory && windowId == GUI_PANEL_WINDOW_INVENTORY_MINIMIZED)
		{
			UTIL_createInventoryPanel(true);
			haveInventory = true;
		}
		else if(!haveButtons && windowId == GUI_PANEL_WINDOW_BUTTONS)
		{
			UTIL_createButtonsPanel();
			haveButtons = true;
		}
	}
	if(!haveMinimap) UTIL_createMinimapPanel();
	if(!haveHealth) UTIL_createHealthPanel();
	if(!haveInventory) UTIL_createInventoryPanel();
	if(!haveButtons) UTIL_createButtonsPanel();

	m_panels.push_back(new GUI_Panel(iRect(0, 0, GAME_PANEL_FIXED_WIDTH, m_windowH - calculateMainHeight()), GUI_PANEL_RIGHT));
	if(m_rightPanel != GUI_PANEL_MAIN)
	{
		Sint32 panels = m_rightPanel - GUI_PANEL_EXTRA_RIGHT_START;
		for(Sint32 i = 0; i <= panels; ++i)
			m_panels.push_back(new GUI_Panel(iRect(0, 0, GAME_PANEL_FIXED_WIDTH, m_windowH), GUI_PANEL_EXTRA_RIGHT_START + i));
	}
	if(m_leftPanel != GUI_PANEL_RANDOM)
	{
		Sint32 panels = m_leftPanel - GUI_PANEL_EXTRA_LEFT_START;
		for(Sint32 i = 0; i <= panels; ++i)
			m_panels.push_back(new GUI_Panel(iRect(0, 0, GAME_PANEL_FIXED_WIDTH, m_windowH), GUI_PANEL_EXTRA_LEFT_START + i));
	}
	for(std::vector<Uint32>::iterator it = m_openDialogs.begin(), end = m_openDialogs.end(); it != end; ++it)
	{
		Uint32 windowId = (*it);
		switch(windowId)
		{
			case GUI_PANEL_WINDOW_SKILLS: UTIL_toggleSkillsWindow(); break;
			case GUI_PANEL_WINDOW_BATTLE: UTIL_toggleBattleWindow(); break;
			case GUI_PANEL_WINDOW_VIP: UTIL_toggleVipWindow(); break;
			case GUI_PANEL_WINDOW_PARTY: UTIL_togglePartyWindow(); break;
			default: break;
		}
	}

	recalculateGameWindow();
	m_parentWindows.clear();
}

void Engine::processGameEnd()
{
	m_ingame = false;
	g_ping = 0;

	//Reset map and creatures
	g_map.changeMap(DIRECTION_INVALID);
	g_map.resetCreatures();

	clearWindows();
	clearPanels();
	checkReleaseQueue();
}

void Engine::setVipData(Uint32 playerGUID, const std::string& description, Uint32 iconId, bool notifyLogin)
{
	VipData vip;
	vip.iconId = iconId;
	vip.description = description;
	vip.notifyLogin = notifyLogin;
	m_vipData[playerGUID] = vip;
}

VipData* Engine::getVipData(Uint32 playerGUID)
{
	std::map<Uint32, VipData>::iterator it = m_vipData.find(playerGUID);
	if(it != m_vipData.end())
		return &it->second;

	return NULL;
}
