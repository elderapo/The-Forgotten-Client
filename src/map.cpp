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

#include "map.h"
#include "automap.h"
#include "engine.h"
#include "tile.h"
#include "creature.h"
#include "effect.h"
#include "distanceEffect.h"
#include "animatedText.h"
#include "staticText.h"
#include "light.h"

#include <algorithm>

Map g_map;
extern Automap g_automap;
extern Engine g_engine;
extern LightSystem g_light;
extern Uint32 g_frameTime;

std::vector<Effect*> g_effects;

AStarNodes::AStarNodes(const Position& startPos) : openNodes(MAX_NODES_COMPLEXITY)
{
	nodesTable.resize(MAX_NODES_GRIDSIZE, -1);
	nodes = SDL_reinterpret_cast(AStarNode*, SDL_malloc(sizeof(AStarNode) * MAX_NODES_COMPLEXITY));

	curNode = 1;

	AStarNode& startNode = nodes[0];
	startNode.parent = NULL;
	startNode.x = startPos.x;
	startNode.y = startPos.y;
	startNode.f = 0;
	startNode.g = 0;
	startNode.closedNode = false;
	nodesTable[(127 * 256) + 127] = 0;
	openNodes.emplace(0, 0);
}

AStarNodes::~AStarNodes()
{
	SDL_free(nodes);
}

bool AStarNodes::createOpenNode(AStarNode* parent, Sint32 f, Uint32 xy, Sint32 Sx, Sint32 Sy, const Position& targetPos, const Position& pos)
{
	if(curNode >= MAX_NODES_COMPLEXITY)
		return false;

	Sint32 retNode = curNode++;
	const Sint32 Dx = Position::getDistanceX(targetPos, pos);
	const Sint32 Dy = Position::getDistanceY(targetPos, pos);

	AStarNode& node = nodes[retNode];
	node.parent = parent;
	node.x = pos.x;
	node.y = pos.y;
	node.f = f;
	node.g = ((Dx - Sx) << 3) + ((Dy - Sy) << 3) + (((Dx > Dy) ? Dx : Dy) << 3);
	node.closedNode = false;
	nodesTable[xy] = retNode;
	openNodes.emplace((f + node.g), retNode);
	return true;
}

AStarNode* AStarNodes::getBestNode()
{
	while(!openNodes.empty())
	{
		AStarNode* node = &nodes[openNodes.top().second];
		if(!node->closedNode)
		{
			node->closedNode = true;
			openNodes.pop();
			return node;
		}

		openNodes.pop();
	}
	return NULL;
}

AStarNode* AStarNodes::getNodeByPosition(Uint32 xy)
{
	Sint32 index = nodesTable[xy];
	return (index == -1 ? NULL : &nodes[index]);
}

Map::Map()
{
	g_effects.reserve(EFFECT_MAX_INGAME_EFFECTS);
  m_onscreenMessages[ONSCREEN_MESSAGE_BOTTOM]     = new ScreenText(ONSCREEN_MESSAGE_BOTTOM);
  m_onscreenMessages[ONSCREEN_MESSAGE_CENTER_LOW] = new ScreenText(ONSCREEN_MESSAGE_CENTER_LOW);
  m_onscreenMessages[ONSCREEN_MESSAGE_CENTER_HIGH]= new ScreenText(ONSCREEN_MESSAGE_CENTER_HIGH);
  m_onscreenMessages[ONSCREEN_MESSAGE_TOP]        = new ScreenText(ONSCREEN_MESSAGE_TOP);
  m_onscreenMessages[ONSCREEN_MESSAGE_LAST]       = new ScreenText(ONSCREEN_MESSAGE_LAST);

}

Map::~Map()
{
	Sint32 z = 0;
	do
	{
		std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
		for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(), end = distanceEffects.end(); it != end; ++it)
			delete (*it);

		distanceEffects.clear();
		Sint32 y = 0;
		do
		{
			Sint32 x = 0;
			do
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
					delete tile;
			} while(++x < GAME_MAP_WIDTH);
		} while(++y < GAME_MAP_HEIGHT);
	} while(++z <= GAME_MAP_FLOORS);

	for(knownCreatures::iterator it = m_knownCreatures.begin(), end = m_knownCreatures.end(); it != end; ++it)
		delete it->second;

	m_knownCreatures.clear();
	for(std::vector<AnimatedText*>::iterator it = m_animatedTexts.begin(), end = m_animatedTexts.end(); it != end; ++it)
		delete (*it);

	m_animatedTexts.clear();
	for(std::vector<StaticText*>::iterator it = m_staticTexts.begin(), end = m_staticTexts.end(); it != end; ++it)
		delete (*it);

	m_staticTexts.clear();

  if( m_onscreenMessages[ONSCREEN_MESSAGE_BOTTOM] != nullptr)
    delete m_onscreenMessages[ONSCREEN_MESSAGE_BOTTOM];
  if( m_onscreenMessages[ONSCREEN_MESSAGE_BOTTOM] != nullptr)
    delete m_onscreenMessages[ONSCREEN_MESSAGE_CENTER_LOW];
  if( m_onscreenMessages[ONSCREEN_MESSAGE_CENTER_HIGH] != nullptr)
    delete m_onscreenMessages[ONSCREEN_MESSAGE_CENTER_HIGH];
  if( m_onscreenMessages[ONSCREEN_MESSAGE_TOP] != nullptr)
    delete m_onscreenMessages[ONSCREEN_MESSAGE_TOP];
  if( m_onscreenMessages[ONSCREEN_MESSAGE_LAST] != nullptr)
    delete m_onscreenMessages[ONSCREEN_MESSAGE_LAST];
}

Tile* Map::getTile(const Position& position)
{
	Sint16 offset = Position::getOffsetZ(m_centerPosition, position);
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(SDL_static_cast(Uint32, posX) >= GAME_MAP_WIDTH || SDL_static_cast(Uint32, posY) >= GAME_MAP_HEIGHT)
		return NULL;

	return m_tiles[position.z][posY][posX];
}

Tile* Map::getTileOrCreate(const Position& position)
{
	Sint16 offset = Position::getOffsetZ(m_centerPosition, position);
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(SDL_static_cast(Uint32, posX) >= GAME_MAP_WIDTH || SDL_static_cast(Uint32, posY) >= GAME_MAP_HEIGHT)
		return NULL;

	Tile*& tile = m_tiles[position.z][posY][posX];
	if(!tile)
		tile = new Tile(position);

	return tile;
}

Tile* Map::resetTile(const Position& position, Sint32 offset)
{
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(SDL_static_cast(Uint32, posX) >= GAME_MAP_WIDTH || SDL_static_cast(Uint32, posY) >= GAME_MAP_HEIGHT)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Tile Pos: %d, %d, %d\nPlayer Pos: %d, %d, %d\nRows: %d, %d\nOffset: %d", position.x, position.y, position.z, m_centerPosition.x, m_centerPosition.y, m_centerPosition.z, posX, posY, offset);
		UTIL_MessageBox(true, g_buffer);
		return NULL;
	}

	Tile*& tile = m_tiles[position.z][posY][posX];
	if(tile)
		tile->reset();
	else
	{
		tile = new Tile(position);
		return tile;
	}

	return tile;
}

void Map::cleanTile(const Position& position, Sint32 offset)
{
	Sint32 posX = Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 1) - offset;
	Sint32 posY = Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 1) - offset;
	if(SDL_static_cast(Uint32, posX) >= GAME_MAP_WIDTH || SDL_static_cast(Uint32, posY) >= GAME_MAP_HEIGHT)
		return;

	Tile*& tile = m_tiles[position.z][posY][posX];
	if(tile)
	{
		delete tile;
		tile = NULL;
	}
}

void Map::update()
{
	Effect::update();
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return;

	m_localCreature->update();
}

void Map::render()
{
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return;

	g_effects.clear();
	if(m_needUpdateCache)
		updateCacheMap();

	Sint32 offsetX = -m_localCreature->getOffsetX(true);
	Sint32 offsetY = -m_localCreature->getOffsetY(true);
	if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE)
		g_light.initLightMap(offsetX, offsetY, getCentralPosition().z);

	auto& renderer = g_engine.getRender();
	renderer->beginGameScene();
	Sint32 z = m_cachedLastVisibleFloor;
	do
	{
		if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE && z != m_cachedLastVisibleFloor)
		{
			Sint32 y = 0;
			do
			{
				Sint32 x = 0;
				#if ((GAME_MAP_WIDTH % 2) == 0)//Check if we can unroll it by 2
				do
				{
					//Reset only the first visible ground tiles to minimalize cpu usage
					if(m_cachedFirstGrounds[y][x + 0] == z)
						g_light.resetLightSource(x + 0, y);

					if(m_cachedFirstGrounds[y][x + 1] == z)
						g_light.resetLightSource(x + 1, y);

					x += 2;
				} while(x < GAME_MAP_WIDTH);
				#else
				do
				{
					//Reset only the first visible ground tiles to minimalize cpu usage
					if(m_cachedFirstGrounds[y][x] == z)
						g_light.resetLightSource(x, y);
				} while(++x < GAME_MAP_WIDTH);
				#endif
			} while(++y < GAME_MAP_HEIGHT);
		}

		Sint32 posY = -32 + offsetY, y = 0;
		do
		{
			Sint32 posX = -32 + offsetX, x = 0;
			do
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
				{
					tile->render(posX, posY, (m_cachedFirstFullGrounds[y][x] >= z));
					if(tile->isCreatureLying())//Redraw tiles in <^ directions
					{
						Sint32 y2 = 1;
						do
						{
							Sint32 x2 = 1;
							do
							{
								if((x2 | y2) == 0)
									continue;

								Sint32 indexX = x - x2;
								Sint32 indexY = y - y2;
								if(indexX >= 0 && indexY >= 0)
								{
									tile = m_tiles[z][indexY][indexX];
									if(tile)
										tile->reRenderTile(posX - x2 * 32, posY - y2 * 32);
								}
							} while(--x2 >= 0);
						} while(--y2 >= 0);
					}
					if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE && z == GAME_PLAYER_FLOOR + 1)
					{
						//Check for translucent on sea floor so if needed we draw small light source
						Tile* seaTile = m_tiles[GAME_PLAYER_FLOOR][y][x];
						if(seaTile && seaTile->hasTranslucentLight())
						{
							Uint16 translucentLight[2];
							translucentLight[0] = 1;
							translucentLight[1] = 215;
							g_light.addLightSource(posX, posY, translucentLight);
						}
					}
				}
				posX += 32;
			} while(++x < GAME_MAP_WIDTH);
			posY += 32;
		} while(++y < GAME_MAP_HEIGHT);

		for(std::vector<Effect*>::iterator it = g_effects.begin(), end = g_effects.end(); it != end; ++it)
		{
			Effect* effect = (*it);
			effect->render(effect->getCachedX(), effect->getCachedY());
		}
		g_effects.clear();

		Sint32 offsetZ = (z - SDL_static_cast(Sint32, m_centerPosition.z));
		std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
		for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(); it != distanceEffects.end();)
		{
			DistanceEffect* distanceEffect = (*it);
			if(distanceEffect->isDelayed())
			{
				++it;
				continue;
			}

			float flightProgress = distanceEffect->getFlightProgress();
			if(flightProgress > 1.f)
			{
				delete (*it);
				it = distanceEffects.erase(it);
			}
			else
			{
				const Position& fromPos = distanceEffect->getFromPos();
				const Position& toPos = distanceEffect->getToPos();

				Sint32 screenxFrom = ((Position::getOffsetX(fromPos, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
				Sint32 screenyFrom = ((Position::getOffsetY(fromPos, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;

				Sint32 screenxTo = ((Position::getOffsetX(toPos, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
				Sint32 screenyTo = ((Position::getOffsetY(toPos, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;

				Sint32 screenx = screenxFrom + SDL_static_cast(Sint32, (screenxTo - screenxFrom) * flightProgress);
				Sint32 screeny = screenyFrom + SDL_static_cast(Sint32, (screenyTo - screenyFrom) * flightProgress);
				distanceEffect->render(screenx, screeny);
				++it;
			}
		}
	} while(--z >= m_cachedFirstVisibleFloor);
	if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE)
	{
		if(g_engine.getLevelSeparator() != 100)
		{
			float brightness = g_engine.getLevelSeparator() / 100.f;
			Sint32 y = 0;
			do
			{
				Sint32 x = 0;
				#if ((GAME_MAP_WIDTH % 2) == 0)//Check if we can unroll it by 2
				do
				{
					if(m_cachedFirstGrounds[y][x + 0] > SDL_static_cast(Sint32, getCentralPosition().z))
						g_light.setLightSource(x + 0, y, brightness);

					if(m_cachedFirstGrounds[y][x + 1] > SDL_static_cast(Sint32, getCentralPosition().z))
						g_light.setLightSource(x + 1, y, brightness);

					x += 2;
				} while(x < GAME_MAP_WIDTH);
				#else
				do
				{
					if(m_cachedFirstGrounds[y][x] > SDL_static_cast(Sint32, getCentralPosition().z))
						g_light.setLightSource(x, y, brightness);
				} while(++x < GAME_MAP_WIDTH);
				#endif
			} while(++y < GAME_MAP_HEIGHT);
		}
	}
	renderer->endGameScene();
}

void Map::renderInformations(Sint32 px, Sint32 py, Sint32 pw, Sint32 ph, float scale, Sint32 scaledSize)
{
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return;

	Sint32 offsetX;
	Sint32 offsetY;
	Sint32 tileOffsetX;
	Sint32 tileOffsetY;
	Sint32 startX;
	Sint32 startY;
	Sint32 endX;
	Sint32 endY;
	if(!m_localCreature->isWalking())
	{
		offsetX = 0;
		offsetY = 0;
		tileOffsetX = 0;
		tileOffsetY = 0;
		startX = 1;
		startY = 1;
		endX = (GAME_MAP_WIDTH - 2);
		endY = (GAME_MAP_HEIGHT - 2);
	}
	else if(m_localCreature->isPreWalking())
	{
		switch(m_localCreature->getWalkDirection())
		{
			case DIRECTION_EAST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = 0;
				tileOffsetX = 32;
				tileOffsetY = 0;
				startX = 2;
				startY = 1;
				endX = (GAME_MAP_WIDTH - 1);
				endY = (GAME_MAP_HEIGHT - 2);
			}
			break;
			case DIRECTION_SOUTH:
			{
				offsetX = 0;
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 0;
				tileOffsetY = 32;
				startX = 1;
				startY = 2;
				endX = (GAME_MAP_WIDTH - 2);
				endY = (GAME_MAP_HEIGHT - 1);
			}
			break;
			case DIRECTION_WEST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = 0;
				tileOffsetX = -32;
				tileOffsetY = 0;
				startX = 0;
				startY = 1;
				endX = (GAME_MAP_WIDTH - 3);
				endY = (GAME_MAP_HEIGHT - 2);
			}
			break;
			case DIRECTION_SOUTHWEST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = -32;
				tileOffsetY = 32;
				startX = 0;
				startY = 2;
				endX = (GAME_MAP_WIDTH - 3);
				endY = (GAME_MAP_HEIGHT - 1);
			}
			break;
			case DIRECTION_SOUTHEAST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 32;
				tileOffsetY = 32;
				startX = 2;
				startY = 2;
				endX = (GAME_MAP_WIDTH - 1);
				endY = (GAME_MAP_HEIGHT - 1);
			}
			break;
			case DIRECTION_NORTHWEST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = -32;
				tileOffsetY = -32;
				startX = 0;
				startY = 0;
				endX = (GAME_MAP_WIDTH - 3);
				endY = (GAME_MAP_HEIGHT - 3);
			}
			break;
			case DIRECTION_NORTHEAST:
			{
				offsetX = -m_localCreature->getOffsetX(true);
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 32;
				tileOffsetY = -32;
				startX = 2;
				startY = 0;
				endX = (GAME_MAP_WIDTH - 1);
				endY = (GAME_MAP_HEIGHT - 3);
			}
			break;
			default://DIRECTION_NORTH
			{
				offsetX = 0;
				offsetY = -m_localCreature->getOffsetY(true);
				tileOffsetX = 0;
				tileOffsetY = -32;
				startX = 1;
				startY = 0;
				endX = (GAME_MAP_WIDTH - 2);
				endY = (GAME_MAP_HEIGHT - 3);
			}
			break;
		}
	}
	else
	{
		offsetX = -m_localCreature->getOffsetX();
		offsetY = -m_localCreature->getOffsetY();
		tileOffsetX = 0;
		tileOffsetY = 0;
		startX = 1;
		startY = 1;
		endX = (GAME_MAP_WIDTH - 2);
		endY = (GAME_MAP_HEIGHT - 2);
	}

	Sint32 fixedSize = scaledSize / 2;
	Sint32 z = SDL_static_cast(Sint32, m_centerPosition.z);
	Sint32 posY = offsetY + tileOffsetY, y = startY;
	do
	{
		Sint32 posX = offsetX + tileOffsetX + 16, x = startX;
		#if ((GAME_MAP_WIDTH % 4) == 0)//Check if we can unroll it by 4
		do
		{
			Tile* tile = m_tiles[z][y][x + 0];
			if(tile)
				tile->renderInformations(px, py, posX + 0, posY, scale, (m_cachedFirstVisibleGround[y][x + 0] >= z));

			tile = m_tiles[z][y][x + 1];
			if(tile)
				tile->renderInformations(px, py, posX + 32, posY, scale, (m_cachedFirstVisibleGround[y][x + 1] >= z));

			tile = m_tiles[z][y][x + 2];
			if(tile)
				tile->renderInformations(px, py, posX + 64, posY, scale, (m_cachedFirstVisibleGround[y][x + 2] >= z));

			tile = m_tiles[z][y][x + 3];
			if(tile)
				tile->renderInformations(px, py, posX + 96, posY, scale, (m_cachedFirstVisibleGround[y][x + 3] >= z));

			posX += 128;
			x += 4;
		} while(x < endX);
		#elif ((GAME_MAP_WIDTH % 3) == 0)//Check if we can unroll it by 3
		do
		{
			Tile* tile = m_tiles[z][y][x + 0];
			if(tile)
				tile->renderInformations(px, py, posX + 0, posY, scale, (m_cachedFirstVisibleGround[y][x + 0] >= z));

			tile = m_tiles[z][y][x + 1];
			if(tile)
				tile->renderInformations(px, py, posX + 32, posY, scale, (m_cachedFirstVisibleGround[y][x + 1] >= z));

			tile = m_tiles[z][y][x + 2];
			if(tile)
				tile->renderInformations(px, py, posX + 64, posY, scale, (m_cachedFirstVisibleGround[y][x + 2] >= z));

			posX += 96;
			x += 3;
		} while(x < endX);
		#elif ((GAME_MAP_WIDTH % 2) == 0)//Check if we can unroll it by 2
		do
		{
			Tile* tile = m_tiles[z][y][x + 0];
			if(tile)
				tile->renderInformations(px, py, posX + 0, posY, scale, (m_cachedFirstVisibleGround[y][x + 0] >= z));

			tile = m_tiles[z][y][x + 1];
			if(tile)
				tile->renderInformations(px, py, posX + 32, posY, scale, (m_cachedFirstVisibleGround[y][x + 1] >= z));

			posX += 64;
			x += 3;
		} while(x < endX);
		#else
		do
		{
			Tile* tile = m_tiles[z][y][x];
			if(tile)
				tile->renderInformations(px, py, posX, posY, scale, (m_cachedFirstVisibleGround[y][x] >= z));

			posX += 32;
		} while(++x < endX);
		#endif
		posY += 32;
	} while(++y < endY);

	if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE)
	{
		if(g_engine.getLightMode() == CLIENT_LIGHT_MODE_OLD)
			g_engine.getRender()->drawLightMap_old(g_light.getLightMap(), px + SDL_static_cast(Sint32, offsetX * scale), py + SDL_static_cast(Sint32, offsetY * scale), scaledSize, GAME_MAP_WIDTH, GAME_MAP_HEIGHT);
		else
			g_engine.getRender()->drawLightMap_new(g_light.getLightMap(), px + SDL_static_cast(Sint32, offsetX * scale), py + SDL_static_cast(Sint32, offsetY * scale), scaledSize, GAME_MAP_WIDTH, GAME_MAP_HEIGHT);
	}

	pw += px;
	ph += py;
	for(std::vector<StaticText*>::iterator it = m_staticTexts.begin(); it != m_staticTexts.end();)
	{
		StaticText* staticText = (*it);
		const Position& position = staticText->getPosition();
		Sint32 offsetZ = (SDL_static_cast(Sint32, position.z) - z);
		Sint32 screenx = ((Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
		Sint32 screeny = ((Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;
		screenx = SDL_static_cast(Sint32, screenx * scale) + px + fixedSize;
		screeny = SDL_static_cast(Sint32, screeny * scale) + py;
		staticText->render(screenx, screeny, px, py, pw, ph);
		if(staticText->canBeDeleted())
		{
			delete staticText;
			it = m_staticTexts.erase(it);
		}
		else
			++it;
	}
	for(std::vector<AnimatedText*>::iterator it = m_animatedTexts.begin(); it != m_animatedTexts.end();)
	{
		AnimatedText* animatedText = (*it);
		if(animatedText->canBeDeleted())
		{
			delete animatedText;
			it = m_animatedTexts.erase(it);
			continue;
		}

		const Position& position = animatedText->getPosition();
		Sint32 offsetZ = (SDL_static_cast(Sint32, position.z) - z);
		Sint32 screenx = ((Position::getOffsetX(position, m_centerPosition) + (MAP_WIDTH_OFFSET - 2) + offsetZ) * 32) + offsetX;
		Sint32 screeny = ((Position::getOffsetY(position, m_centerPosition) + (MAP_HEIGHT_OFFSET - 2) + offsetZ) * 32) + offsetY;
		screenx = SDL_static_cast(Sint32, screenx * scale) + px + fixedSize;
		screeny = SDL_static_cast(Sint32, screeny * scale) + py;
		animatedText->render(screenx, screeny, px, py, pw, ph);
		++it;
	}
	m_onscreenMessages[ONSCREEN_MESSAGE_BOTTOM]->render(px, py, pw, ph);
	m_onscreenMessages[ONSCREEN_MESSAGE_CENTER_LOW]->render(px, py, pw, ph);
	m_onscreenMessages[ONSCREEN_MESSAGE_CENTER_HIGH]->render(px, py, pw, ph);
	m_onscreenMessages[ONSCREEN_MESSAGE_TOP]->render(px, py, pw, ph);

	UTIL_refreshBattleWindow();
}

void Map::addOnscreenText(OnscreenMessages position, MessageMode mode, const std::string& text)
{
	m_onscreenMessages[position]->addMessage(mode, text);
}

void Map::addAnimatedText(const Position& position, Uint8 color, const std::string& text)
{
	if(g_engine.hasShowTextualEffects())
	{
		Sint32 additionalOffset = 0;
		for(std::vector<AnimatedText*>::iterator it = m_animatedTexts.begin(), end = m_animatedTexts.end(); it != end; ++it)
		{
			AnimatedText* animatedText = (*it);
			if(animatedText->getPosition() == position)
			{
				if(animatedText->merge(color, text))
					return;
				else
				{
					if(animatedText->needAdditionalOffset())
						additionalOffset += 10;
				}
			}
		}

		AnimatedText* newAnimatedText = new AnimatedText(position, color, additionalOffset, text);
		m_animatedTexts.push_back(newAnimatedText);
	}
}

void Map::addStaticText(const Position& position, const std::string& name, MessageMode mode, const std::string& text)
{
	for(std::vector<StaticText*>::iterator it = m_staticTexts.begin(), end = m_staticTexts.end(); it != end; ++it)
	{
		StaticText* staticText = (*it);
		if(staticText->getMode() == mode && staticText->getPosition() == position && staticText->getName() == name)
		{
			staticText->addMessage(name, mode, text);
			return;
		}
	}

	StaticText* newStaticText = new StaticText(position);
	newStaticText->addMessage(name, mode, text);
	m_staticTexts.push_back(newStaticText);
}

void Map::changeMap(Direction direction)
{
	switch(direction)
	{
		case DIRECTION_NORTH:
		{
			Sint32 z = 0;
			do
			{
				Sint32 x = 0;
				do
				{
					Tile* tile = m_tiles[z][GAME_MAP_HEIGHT - 1][x];
					delete tile;
				} while(++x < GAME_MAP_WIDTH);

				Sint32 y = GAME_MAP_HEIGHT - 2;
				do
				{
					x = 0;
					do
					{
						m_tiles[z][y + 1][x] = m_tiles[z][y][x];
					} while(++x < GAME_MAP_WIDTH);
				} while(--y >= 0);

				x = 0;
				do
				{
					//Reset the tiles that we get later
					m_tiles[z][0][x] = NULL;
				} while(++x < GAME_MAP_WIDTH);
			} while(++z <= GAME_MAP_FLOORS);
		}
		break;
		case DIRECTION_EAST:
		{
			Sint32 z = 0;
			do
			{
				Sint32 y = 0;
				do
				{
					Tile* tile = m_tiles[z][y][0];
					delete tile;
				} while(++y < GAME_MAP_HEIGHT);

				y = 0;
				do
				{
					Sint32 x = 0;
					do
					{
						m_tiles[z][y][x] = m_tiles[z][y][x + 1];
					} while(++x < GAME_MAP_WIDTH - 1);
				} while(++y < GAME_MAP_HEIGHT);

				y = 0;
				do
				{
					//Reset the tiles that we get later
					m_tiles[z][y][GAME_MAP_WIDTH - 1] = NULL;
				} while(++y < GAME_MAP_HEIGHT);
			} while(++z <= GAME_MAP_FLOORS);
		}
		break;
		case DIRECTION_SOUTH:
		{
			Sint32 z = 0;
			do
			{
				Sint32 x = 0;
				do
				{
					Tile* tile = m_tiles[z][0][x];
					delete tile;
				} while(++x < GAME_MAP_WIDTH);

				Sint32 y = 0;
				do
				{
					x = 0;
					do
					{
						m_tiles[z][y][x] = m_tiles[z][y + 1][x];
					} while(++x < GAME_MAP_WIDTH);
				} while(++y < GAME_MAP_HEIGHT - 1);

				x = 0;
				do
				{
					//Reset the tiles that we get later
					m_tiles[z][GAME_MAP_HEIGHT - 1][x] = NULL;
				} while(++x < GAME_MAP_WIDTH);
			} while(++z <= GAME_MAP_FLOORS);
		}
		break;
		case DIRECTION_WEST:
		{
			Sint32 z = 0;
			do
			{
				Sint32 y = 0;
				do
				{
					Tile* tile = m_tiles[z][y][GAME_MAP_WIDTH - 1];
					delete tile;
				} while(++y < GAME_MAP_HEIGHT);

				y = 0;
				do
				{
					Sint32 x = GAME_MAP_WIDTH - 2;
					do
					{
						m_tiles[z][y][x + 1] = m_tiles[z][y][x];
					} while(--x >= 0);
				} while(++y < GAME_MAP_HEIGHT);

				y = 0;
				do
				{
					//Reset the tiles that we get later
					m_tiles[z][y][0] = NULL;
				} while(++y < GAME_MAP_HEIGHT);
			} while(++z <= GAME_MAP_FLOORS);
		}
		break;
		default:
		{
			Sint32 z = 0;
			do
			{
				std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
				for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(), end = distanceEffects.end(); it != end; ++it)
					delete (*it);

				distanceEffects.clear();
				Sint32 y = 0;
				do
				{
					Sint32 x = 0;
					do
					{
						Tile*& tile = m_tiles[z][y][x];
						delete tile; tile = NULL;
					} while(++x < GAME_MAP_WIDTH);
				} while(++y < GAME_MAP_HEIGHT);
			} while(++z <= GAME_MAP_FLOORS);
		}
		break;
	}
}

void Map::checkMagicEffects()
{
	if(g_frameTime == m_magicEffectsTime)
		return;

	m_magicEffectsTime = g_frameTime;
	std::vector<Effect*> removeEffects; removeEffects.reserve(EFFECT_MAX_INGAME_EFFECTS);
	for(std::vector<Effect*>::iterator it = Effect::effects.begin(), end = Effect::effects.end(); it != end; ++it)
	{
		Effect* effect = (*it);
		if(!effect->isDelayed() && effect->canBeDeleted())
			removeEffects.push_back(effect);
	}

	for(std::vector<Effect*>::iterator it = removeEffects.begin(), end = removeEffects.end(); it != end; ++it)
	{
		Effect* effect = (*it);
		Tile* tile = getTile(effect->getPos());
		if(tile)
			tile->removeMagicEffect(effect);
		else
			delete effect;
	}
}

void Map::checkDistanceEffects()
{
	if(g_frameTime == m_distanceEffectsTime)
		return;

	m_distanceEffectsTime = g_frameTime;
	Sint32 z = 0;
	do
	{
		std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[z];
		for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(); it != distanceEffects.end();)
		{
			DistanceEffect* distanceEffect = (*it);
			if(!distanceEffect->isDelayed() && distanceEffect->getFlightProgress() > 1.f)
			{
				delete distanceEffect;
				it = distanceEffects.erase(it);
			}
			else
				++it;
		}
	} while(++z <= GAME_MAP_FLOORS);
}

void Map::addDistanceEffect(DistanceEffect* distanceEffect, Uint8 posZ)
{
	if(posZ > GAME_MAP_FLOORS)
	{
		delete distanceEffect;
		return;
	}
	m_distanceEffects[posZ].push_back(distanceEffect);
}

void Map::removeMagicEffects(Uint8 posZ)
{
	std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[posZ];
	for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(), end = distanceEffects.end(); it != end; ++it)
		delete (*it);

	distanceEffects.clear();
}

void Map::removeMagicEffects(const Position& position, Uint16 effectId)
{
	//QT Client have a bug that it doesn't register properly first created magic effect so it is registered with id 0
	//that's why in some cases this function might produce different results but I don't want to reproduce this bug
	Tile* tile = getTile(position);
	if(tile)
		tile->removeMagicEffects(effectId);

	std::vector<DistanceEffect*>& distanceEffects = m_distanceEffects[position.z];
	for(std::vector<DistanceEffect*>::iterator it = distanceEffects.begin(); it != distanceEffects.end();)
	{
		if((*it)->getFromPos() == position && (*it)->getID() == effectId)
		{
			delete (*it);
			it = distanceEffects.erase(it);
		}
		else
			++it;
	}
}

PathFind Map::findPath(std::vector<Direction>& directions, const Position& startPos, const Position& endPos)
{
	directions.clear();
	if(startPos == endPos)
		return PathFind_ReturnSamePosition;

	if(startPos.z < endPos.z)
		return PathFind_ReturnFirstGoDownStairs;
	else if(startPos.z > endPos.z)
		return PathFind_ReturnFirstGoUpStairs;

	Tile* endTile = getTile(endPos);
	if(endTile)
	{
		if(!endTile->isWalkable())
			return PathFind_ReturnImpossible;
	}
	else
	{
		if(g_automap.getSpeed(endPos) == 0)
			return PathFind_ReturnImpossible;
	}

	static const Sint32 dirNeighbors[8][5][2] = {
		{{-1, 0},{0,  1},{ 1,  0},{ 1,  1},{-1,  1}},
		{{-1, 0},{0,  1},{ 0, -1},{-1, -1},{-1,  1}},
		{{-1, 0},{1,  0},{ 0, -1},{-1, -1},{ 1, -1}},
		{{ 0, 1},{1,  0},{ 0, -1},{ 1, -1},{ 1,  1}},
		{{ 1, 0},{0, -1},{-1, -1},{ 1, -1},{ 1,  1}},
		{{-1, 0},{0, -1},{-1, -1},{ 1, -1},{-1,  1}},
		{{ 0, 1},{1,  0},{ 1, -1},{ 1,  1},{-1,  1}},
		{{-1, 0},{0,  1},{-1, -1},{ 1,  1},{-1,  1}}
	};
	static const Sint32 allNeighbors[8][2] = {
		{-1, 0},{0, 1},{1, 0},{0, -1},{-1, -1},{1, -1},{1, 1},{-1, 1}
	};

	Position pos = startPos;

	const Sint32 Sx = Position::getDistanceX(endPos, startPos);
	const Sint32 Sy = Position::getDistanceY(endPos, startPos);
	if(Sx > 127 || Sy > 127)
		return PathFind_ReturnTooFar;

	AutomapArea* areas[4];
	areas[0] = g_automap.getArea(startPos.x - 127, startPos.y - 127, startPos.z);
	areas[1] = g_automap.getArea(startPos.x - 127, startPos.y + 127, startPos.z);
	areas[2] = g_automap.getArea(startPos.x + 127, startPos.y - 127, startPos.z);
	areas[3] = g_automap.getArea(startPos.x + 127, startPos.y + 127, startPos.z);

	Sint32 startX = startPos.x - 127;
	Sint32 startY = startPos.y - 127;
	Uint16 areaX = areas[0]->getBasePosition().x + 256;
	Uint16 areaY = areas[0]->getBasePosition().y + 256;

	AStarNodes nodes(startPos);
	AStarNode* found = NULL;
	do
	{
		AStarNode* n = nodes.getBestNode();
		if(!n)
		{
			if(found)
				break;
			return PathFind_ReturnNoWay;
		}

		const Sint32 x = n->x;
		const Sint32 y = n->y;
		if(((n->x ^ endPos.x) | (n->y ^ endPos.y)) == 0 && (!found || n->f < found->f))
			found = n;

		if(found && n->f >= found->f)
			break;

		Uint32 dirCount;
		const Sint32* neighbors;
		if(n->parent)
		{
			const Sint32 offset_x = n->parent->x - x;
			const Sint32 offset_y = n->parent->y - y;
			if(offset_y == 0)
			{
				if(offset_x == -1)
					neighbors = *dirNeighbors[DIRECTION_WEST];
				else
					neighbors = *dirNeighbors[DIRECTION_EAST];
			}
			else if(offset_x == 0)
			{
				if(offset_y == -1)
					neighbors = *dirNeighbors[DIRECTION_NORTH];
				else
					neighbors = *dirNeighbors[DIRECTION_SOUTH];
			}
			else if(offset_y == -1)
			{
				if(offset_x == -1)
					neighbors = *dirNeighbors[DIRECTION_NORTHWEST];
				else
					neighbors = *dirNeighbors[DIRECTION_NORTHEAST];
			}
			else if(offset_x == -1)
				neighbors = *dirNeighbors[DIRECTION_SOUTHWEST];
			else
				neighbors = *dirNeighbors[DIRECTION_SOUTHEAST];

			dirCount = 5;
		}
		else
		{
			dirCount = 8;
			neighbors = *allNeighbors;
		}

		const Sint32 f = n->f;
		Uint32 i = 0;
		do
		{
			Sint32 posX = x + *neighbors++;
			Sint32 posY = y + *neighbors++;
			if(SDL_static_cast(Uint32, posX - startX) > 254 || SDL_static_cast(Uint32, posY - startY) > 254)
				continue;

			pos.x = SDL_static_cast(Uint16, posX);
			pos.y = SDL_static_cast(Uint16, posY);

			Sint32 speed = 100;
			endTile = getTile(pos);
			if(endTile)
			{
				if(!endTile->isWalkable() || endTile->getTopCreature() || ((pos.x != endPos.x || pos.y != endPos.y) && !endTile->isPathable()))
					continue;

				speed = SDL_static_cast(Sint32, endTile->getGroundSpeed());
			}
			else
			{
				//Not seen tiles should have 250 speed - that's how it working on Tibia
				//speed = SDL_static_cast(Sint32, areas[(pos.x < areaX ? (pos.y < areaY ? 0 : 1) : (pos.y < areaY ? 2 : 3))]->getSpeed(pos.x, pos.y));
				speed = SDL_static_cast(Sint32, areas[(pos.x < areaX ? 0 : 2) + (pos.y < areaY ? 0 : 1)]->getSpeed(pos.x, pos.y));
				if(speed == 0)
					continue;
			}

			const Sint32 walkFactor = (((std::abs(n->x - pos.x) + std::abs(n->y - pos.y)) - 1) * MAP_DIAGONALWALKFACTOR) + MAP_NORMALWALKFACTOR;
			const Sint32 cost = f + (speed * walkFactor);

			const Uint32 posXY = (SDL_static_cast(Uint32, posX - startX) * 256) + SDL_static_cast(Uint32, posY - startY);
			AStarNode* neighborNode = nodes.getNodeByPosition(posXY);
			if(neighborNode)
			{
				if(neighborNode->f <= cost)
					continue;

				neighborNode->f = cost;
				neighborNode->parent = n;
				nodes.openNode(neighborNode);
			}
			else
			{
				if(!nodes.createOpenNode(n, cost, posXY, Sx, Sy, endPos, pos))
				{
					if(found)
						break;
					return PathFind_ReturnNoWay;
				}
			}
		} while(++i < dirCount);
	} while(nodes.getNodeSize() < MAX_NODES_COMPLEXITY);
	if(!found)
		return PathFind_ReturnNoWay;

	Sint32 prevx = endPos.x;
	Sint32 prevy = endPos.y;
	do
	{
		pos.x = found->x;
		pos.y = found->y;

		Sint32 dx = pos.x - prevx;
		Sint32 dy = pos.y - prevy;

		prevx = pos.x;
		prevy = pos.y;

		endTile = getTile(pos);
		if(endTile)
		{
			//We can only send the tiles we know
			if(dx == 1)
			{
				if(dy == 1)
					directions.emplace_back(DIRECTION_NORTHWEST);
				else if(dy == -1)
					directions.emplace_back(DIRECTION_SOUTHWEST);
				else
					directions.emplace_back(DIRECTION_WEST);
			}
			else if(dx == -1)
			{
				if(dy == 1)
					directions.emplace_back(DIRECTION_NORTHEAST);
				else if(dy == -1)
					directions.emplace_back(DIRECTION_SOUTHEAST);
				else
					directions.emplace_back(DIRECTION_EAST);
			}
			else if(dy == 1)
				directions.emplace_back(DIRECTION_NORTH);
			else if(dy == -1)
				directions.emplace_back(DIRECTION_SOUTH);
		}
		found = found->parent;
	} while(found);
	return PathFind_ReturnSuccessfull;
}

Tile* Map::findTile(Sint32 x, Sint32 y, iRect& gameWindow, Sint32 scaledSize, float scale, Creature*& topCreature, bool multifloor)
{
	Tile* bestPossibleTile = NULL;
	if(!m_localCreature) //If somehow we don't have localcreature avoid crashing
		return bestPossibleTile;

	Sint32 xOffset = SDL_static_cast(Sint32, m_localCreature->getOffsetX(true) * scale);
	Sint32 yOffset = SDL_static_cast(Sint32, m_localCreature->getOffsetY(true) * scale);

	Sint32 firstVisibleFloor;
	Sint32 lastVisibleFloor;
	if(multifloor)
	{
		firstVisibleFloor = m_cachedLastVisibleFloor;
		lastVisibleFloor = m_cachedFirstVisibleFloor;
	}
	else
	{
		firstVisibleFloor = SDL_static_cast(Sint32, m_centerPosition.z);
		lastVisibleFloor = SDL_static_cast(Sint32, m_centerPosition.z);
	}
	topCreature = NULL;

	Sint32 posX = ((x - gameWindow.x1) / scaledSize) + 1;
	Sint32 posY = ((y - gameWindow.y1) / scaledSize) + 1;
	Sint32 z = lastVisibleFloor;
	do
	{
		if(!topCreature)
		{
			for(Sint32 y2 = 1; y2 >= -1; --y2)
			{
				for(Sint32 x2 = 1; x2 >= -1; --x2)
				{
					Sint32 indexX = posX + x2;
					Sint32 indexY = posY + y2;
					if(SDL_static_cast(Uint32, indexX) < GAME_MAP_WIDTH && SDL_static_cast(Uint32, indexY) < GAME_MAP_HEIGHT)
					{
						Tile* tile = m_tiles[z][indexY][indexX];
						if(tile)
						{
							Sint32 elevationSize = SDL_static_cast(Sint32, tile->getTileElevation() * scale);
							iRect irect = iRect(gameWindow.x1 + ((indexX - 1) * scaledSize) - elevationSize - xOffset, gameWindow.y1 + ((indexY - 1) * scaledSize) - elevationSize - yOffset, scaledSize + elevationSize, scaledSize + elevationSize);
							topCreature = tile->getTopCreature(x, y, irect, scale);
							if(topCreature)
								goto Search_for_Tile;
						}
					}
				}
			}
		}

		Search_for_Tile:
		Sint32 indexX = posX;
		Sint32 indexY = posY;
		if(SDL_static_cast(Uint32, indexX) < GAME_MAP_WIDTH && SDL_static_cast(Uint32, indexY) < GAME_MAP_HEIGHT)
		{
			Tile* tile = m_tiles[z][indexY][indexX];
			if(tile)
			{
				iRect irect = iRect(gameWindow.x1 + ((indexX - 1) * scaledSize) - xOffset, gameWindow.y1 + ((indexY - 1) * scaledSize) - yOffset, scaledSize, scaledSize);
				if(irect.isPointInside(x, y))
				{
					if(tile->isLookingPossible())
					{
						//When we don't have ground let's see if we can get another tile
						bestPossibleTile = tile;
						if(tile->hasground())
							return bestPossibleTile;
					}
				}
			}
		}
	} while(++z <= firstVisibleFloor);
	return bestPossibleTile;
}

void Map::resetCreatures()
{
	for(knownCreatures::iterator it = m_knownCreatures.begin(), end = m_knownCreatures.end(); it != end; ++it)
		delete it->second;

	m_knownCreatures.clear();
	UTIL_resetBattleCreatures();
}

void Map::addCreatureById(Uint32 creatureId, Creature* creature)
{
	removeCreatureById(creatureId);//Make sure we don't get any memory leak
	m_knownCreatures[creatureId] = creature;
	UTIL_addBattleCreature(SDL_reinterpret_cast(void*, creature));
}

void Map::removeCreatureById(Uint32 creatureId)
{
	knownCreatures::iterator it = m_knownCreatures.find(creatureId);
	if(it != m_knownCreatures.end())
	{
		UTIL_removeBattleCreature(SDL_reinterpret_cast(void*, it->second));
		delete it->second;
		m_knownCreatures.erase(it);
	}
}

Creature* Map::getCreatureById(Uint32 creatureId)
{
	knownCreatures::iterator it = m_knownCreatures.find(creatureId);
	if(it != m_knownCreatures.end())
		return it->second;
	else
		return NULL;
}

void Map::setCentralPosition(Position pos)
{
	if(m_centerPosition == pos)
		return;

	m_centerPosition = pos;
}

void Map::updateCacheMap()
{
	Sint32 posZ = SDL_static_cast(Sint32, m_centerPosition.z);
	if(posZ > GAME_PLAYER_FLOOR)
		m_cachedLastVisibleFloor = posZ + GAME_MAP_AWARERANGE;
	else
		m_cachedLastVisibleFloor = GAME_PLAYER_FLOOR;

	Sint32 firstFloor = 0;
	if(posZ > GAME_PLAYER_FLOOR)
		firstFloor = UTIL_max<Sint32>(posZ - GAME_MAP_AWARERANGE, GAME_PLAYER_FLOOR + 1);

	for(Sint32 x = -1; x <= 1 && firstFloor < posZ; ++x)
	{
		for(Sint32 y = -1; y <= 1 && firstFloor < posZ; ++y)
		{
			Position pos(SDL_static_cast(Uint16, SDL_static_cast(Sint32, m_centerPosition.x) + x), SDL_static_cast(Uint16, SDL_static_cast(Sint32, m_centerPosition.y) + y), m_centerPosition.z);

			bool isLookPossible = false;
			Tile* tile = getTile(pos);
			if(tile && tile->isLookPossible())
				isLookPossible = true;

			if(((x | y) == 0) || ((std::abs(x) != std::abs(y)) && isLookPossible))
			{
				Position upperPos = pos;
				Position coveredPos = pos;
				while(coveredPos.coveredUp() && upperPos.up() && upperPos.z >= firstFloor)
				{
					//Check tile physically above
					tile = getTile(upperPos);
					if(tile && tile->limitsFloorsView(!isLookPossible))
					{
						firstFloor = upperPos.z + 1;
						break;
					}

					//Check tile geometrically above
					tile = getTile(coveredPos);
					if(tile && tile->limitsFloorsView(isLookPossible))
					{
						firstFloor = coveredPos.z + 1;
						break;
					}
				}
			}
		}
	}

	m_cachedLastVisibleFloor = UTIL_max<Sint32>(0, UTIL_min<Sint32>(GAME_MAP_FLOORS, m_cachedLastVisibleFloor));
	m_cachedFirstVisibleFloor = UTIL_max<Sint32>(0, UTIL_min<Sint32>(GAME_MAP_FLOORS, firstFloor));
	m_needUpdateCache = false;
	Sint32 y = 0;
	do
	{
		Sint32 x = 0;
		do
		{
			bool haveFirstVisibleGround = false;
			bool haveFirstGrounds = false;
			bool haveFirstFullGrounds = false;
			Sint32 z = m_cachedFirstVisibleFloor;
			do
			{
				Tile* tile = m_tiles[z][y][x];
				if(tile)
				{
					if(!haveFirstVisibleGround && tile->limitsFloorsView(false))
					{
						m_cachedFirstVisibleGround[y][x] = z;
						haveFirstVisibleGround = true;
					}
					if(!haveFirstGrounds && tile->hasground())
					{
						m_cachedFirstGrounds[y][x] = z;
						haveFirstGrounds = true;
					}
					if(!haveFirstFullGrounds && tile->isFullground())
					{
						m_cachedFirstFullGrounds[y][x] = z;
						haveFirstFullGrounds = true;
					}
				}
			} while(++z < m_cachedLastVisibleFloor);

			if(!haveFirstVisibleGround)
				m_cachedFirstVisibleGround[y][x] = m_cachedLastVisibleFloor;

			if(!haveFirstGrounds)
				m_cachedFirstGrounds[y][x] = m_cachedLastVisibleFloor;

			if(!haveFirstFullGrounds)
				m_cachedFirstFullGrounds[y][x] = m_cachedLastVisibleFloor;
		} while(++x < GAME_MAP_WIDTH);
	} while(++y < GAME_MAP_HEIGHT);
}
