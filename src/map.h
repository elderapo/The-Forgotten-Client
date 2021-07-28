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

#ifndef __FILE_MAP_h_
#define __FILE_MAP_h_

#include "position.h"
#include "screenText.h"

#include <queue>

#define MAP_WIDTH_OFFSET (GAME_MAP_WIDTH / 2)
#define MAP_HEIGHT_OFFSET (GAME_MAP_HEIGHT / 2)

class Tile;
class Effect;
class DistanceEffect;
class AnimatedText;
class StaticText;
class Creature;

typedef std::unordered_map<Uint32, Creature*> knownCreatures;

struct AStarNode
{
	AStarNode* parent;
	Sint32 f, g;
	Uint16 x, y;
	bool closedNode;
};

static const Sint32 MAX_NODES_GRIDSIZE = 256 * 256;
static const Sint32 MAX_NODES_COMPLEXITY = 50000;

static const Sint32 MAP_NORMALWALKFACTOR = 1;
static const Sint32 MAP_DIAGONALWALKFACTOR = 2;

struct CompareNode : std::binary_function<std::pair<Sint32, Sint32>, std::pair<Sint32, Sint32>, bool> {
	bool operator()(std::pair<Sint32, Sint32> a, std::pair<Sint32, Sint32> b) const {
		return b.first < a.first;
	}
};

class PQueue_Nodes : public std::priority_queue<std::pair<Sint32, Sint32>, std::vector<std::pair<Sint32, Sint32>>, CompareNode>
{
	public: PQueue_Nodes(size_t reserve_size) {this->c.reserve(reserve_size);}
};

class AStarNodes
{
	public:
		AStarNodes(const Position& startPos);
		~AStarNodes();

		// non-copyable
		AStarNodes(const AStarNodes&) = delete;
		AStarNodes& operator=(const AStarNodes&) = delete;

		// non-moveable
		AStarNodes(AStarNodes&&) = delete;
		AStarNodes& operator=(AStarNodes&&) = delete;

		bool createOpenNode(AStarNode* parent, Sint32 f, Uint32 xy, Sint32 Sx, Sint32 Sy, const Position& targetPos, const Position& pos);
		AStarNode* getBestNode();
		SDL_INLINE void openNode(AStarNode* node) {node->closedNode = false; openNodes.emplace((node->f + node->g), SDL_static_cast(Sint32, node - nodes));}
		SDL_INLINE Sint32 getNodeSize() const {return curNode;}
		AStarNode* getNodeByPosition(Uint32 xy);

	private:
		PQueue_Nodes openNodes;
		std::vector<Sint32> nodesTable;
		AStarNode* nodes;
		Sint32 curNode;
};

class Map
{
	public:
		Map();
		~Map();

		// non-copyable
		Map(const Map&) = delete;
		Map& operator=(const Map&) = delete;

		// non-moveable
		Map(Map&&) = delete;
		Map& operator=(Map&&) = delete;

		Tile* getTile(const Position& position);
		Tile* getTileOrCreate(const Position& position);
		Tile* resetTile(const Position& position, Sint32 offset);
		void cleanTile(const Position& position, Sint32 offset);

		void update();
		void render();
		void renderInformations(Sint32 px, Sint32 py, Sint32 pw, Sint32 ph, float scale, Sint32 scaledSize);

		void addOnscreenText(OnscreenMessages position, MessageMode mode, const std::string& text);
		void addAnimatedText(const Position& position, Uint8 color, const std::string& text);
		void addStaticText(const Position& position, const std::string& name, MessageMode mode, const std::string& text);
		void changeMap(Direction direction);

		void checkMagicEffects();
		void checkDistanceEffects();
		void addDistanceEffect(DistanceEffect* distanceEffect, Uint8 posZ);
		void removeMagicEffects(Uint8 posZ);
		void removeMagicEffects(const Position& position, Uint16 effectId);

		PathFind findPath(std::vector<Direction>& directions, const Position& startPos, const Position& endPos);
		Tile* findTile(Sint32 x, Sint32 y, iRect& gameWindow, Sint32 scaledSize, float scale, Creature*& topCreature, bool multifloor);

		Creature* getLocalCreature() {return m_localCreature;}
		void setLocalCreature(Creature* creature) {m_localCreature = creature;}

		void resetCreatures();
		void addCreatureById(Uint32 creatureId, Creature* creature);
		void removeCreatureById(Uint32 creatureId);
		Creature* getCreatureById(Uint32 creatureId);

		void setCentralPosition(Position pos);
		SDL_INLINE Position& getCentralPosition() {return m_centerPosition;}

		SDL_INLINE void needUpdateCache() {m_needUpdateCache = true;}
		void updateCacheMap();

	protected:
		knownCreatures m_knownCreatures;
		std::vector<DistanceEffect*> m_distanceEffects[GAME_MAP_FLOORS + 1];
		std::vector<AnimatedText*> m_animatedTexts;
		std::vector<StaticText*> m_staticTexts;
		Tile* m_tiles[GAME_MAP_FLOORS + 1][GAME_MAP_HEIGHT][GAME_MAP_WIDTH] = {};

		Creature* m_localCreature = NULL;
		Position m_centerPosition;
		// ScreenText m_onscreenMessages[ONSCREEN_MESSAGE_LAST] = {ONSCREEN_MESSAGE_BOTTOM, ONSCREEN_MESSAGE_CENTER_LOW, ONSCREEN_MESSAGE_CENTER_HIGH, ONSCREEN_MESSAGE_TOP};
		ScreenText *m_onscreenMessages[OnscreenMessages::ONSCREEN_MESSAGE_LAST];

		Uint32 m_magicEffectsTime = 0;
		Uint32 m_distanceEffectsTime = 0;

		Sint32 m_cachedFirstVisibleGround[GAME_MAP_HEIGHT][GAME_MAP_WIDTH] = {};
		Sint32 m_cachedFirstGrounds[GAME_MAP_HEIGHT][GAME_MAP_WIDTH] = {};
		Sint32 m_cachedFirstFullGrounds[GAME_MAP_HEIGHT][GAME_MAP_WIDTH] = {};
		Sint32 m_cachedLastVisibleFloor = 7;
		Sint32 m_cachedFirstVisibleFloor = 0;

		bool m_needUpdateCache = true;
};

#endif /* __FILE_MAP_h_ */
