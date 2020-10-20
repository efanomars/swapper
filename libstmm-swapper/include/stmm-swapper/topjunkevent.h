/*
 * Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   topjunkevent.h
 */

#ifndef STMG_TOP_JUNK_EVENT_H_
#define STMG_TOP_JUNK_EVENT_H_

#include <stmm-games/levellisteners.h>
#include <stmm-games/event.h>
#include <stmm-games/tileanimator.h>
#include <stmm-games/utile/randomtiles.h>
#include <stmm-games/utile/tilecoords.h>
#include <stmm-games/util/direction.h>

#include <memory>
#include <vector>

#include <stdint.h>

namespace stmg { class LeelBlock; }
namespace stmg { class TileCoords; }
namespace stmg { class Coords; }
namespace stmg { class Level; }
namespace stmg { class GameProxy; }

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

// Puts tiles according to a random tile generator at a certain row but only if no
// TILEANI:REMOVING animation is  going on.
// the junk tiles appear in the least filled columns

// Defines a tile ani TILEANI:MATERIALIZE that vanishes as soon as a cell is
// removed

// Gravity event (or block?) has to not activate if TILEANI:MATERIALIZE is defined

// the row is filled with the number of tiles defined by a cumulated input
// if not all can be put on board they are saved for later (next game tick)

// The TILEANI:MATERIALIZE tile animation is removed if cell modified (swapped)

// The TILEANI:MATERIALIZE tile animations can be scrolled from position y = 1
// to y = 0. In any other dir they are removed (the animation not the tile itself).

class TopJunkEvent : public Event, public BoardListener, public TileAnimator
{
public:
	struct LocalInit
	{
		int32_t m_nMaterializingTicksFrom = 0; /**< The number of game ticks the materializing animation
												 * should at least last. Adds up to m_nMaterializingMillisecFrom.
												 * Must be &gt;= 0. The default is 0. */
		int32_t m_nMaterializingTicksTo = 0; /**< The number of game ticks the materializing animation
												 * should at most last. Adds up to m_nMaterializingMillisecTo.
												 * Must be &gt;= m_nMaterializingTicksFrom. The default is 0. */
		int32_t m_nMaterializingMillisecFrom = 0; /**< The number of milliseconds the materializing animation
												 * should at least last. Adds up to m_nMaterializingTicksFrom.
												 * Must be &gt;= 0. The default is 0. */
		int32_t m_nMaterializingMillisecTo = 0; /**< The number of milliseconds the materializing animation
												 * should at most last. Adds up to m_nMaterializingTicksTo.
												 * Must be &gt;= m_nMaterializingMillisecFrom. The default is 0. */
		int32_t m_nWaitTicksFrom = 1; /**< The number of game ticks the interval between drops
										 * should at least last. Adds up to m_nWaitMillisecFrom.
										 * Must be &gt;= 0. The default is 1. */
		int32_t m_nWaitTicksTo = 1; /**< The number of game ticks the interval between drops
										 * should at most last. Adds up to m_nWaitMillisecTo.
										 * Must be &gt;= m_nWaitTicksFrom. The default is 1. */
		int32_t m_nWaitMillisecFrom = 1; /**< The number of milliseconds the interval between drops
										 * should at least last. Adds up to m_nWaitTicksFrom.
										 * Must be &gt;= 0. The default is 1. */
		int32_t m_nWaitMillisecTo = 1; /**< The number of milliseconds the interval between drops
										 * should at most last. Adds up to m_nWaitTicksTo.
										 * Must be &gt;= m_nWaitMillisecFrom. The default is 1. */
		int32_t m_nMaxJunkPerDrop = 1; /**< The maximal number of junk tiles per drop.
										 * When the wait between drops is 0 this value is ignored. */
		int32_t m_nTopY = 1; /**< The row in which the new tiles will materialize. Must be within the board. Default is 1. */
		std::vector< RandomTiles::ProbTileGen > m_aProbTileGens; /**< The available new junk generators. Cannot be empty. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * The wait betwwen drops is always converted in ticks (if milliseconds are defined)
	 * using the value of the current tick interval. The wait is always at least
	 * one game tick.
	 * @param oInit The initialization data.
	 */
	explicit TopJunkEvent(Init&& oInit) noexcept;

protected:
	/** Reinitialization.
	 * See constuctor.
	 */
	void reInit(Init&& oInit) noexcept;
public:

	//Event
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;
	// BoardListener
	void boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostScroll(Direction::VALUE eDir) noexcept override;
	void boabloPreFreeze(LevelBlock& oBlock) noexcept override;
	void boabloPostFreeze(const Coords& oCoords) noexcept override;
	void boabloPreUnfreeze(const Coords& oCoords) noexcept override;
	void boabloPostUnfreeze(LevelBlock& oBlock) noexcept override;
	void boardPreInsert(Direction::VALUE eDir, NRect oArea, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostInsert(Direction::VALUE eDir, NRect oArea) noexcept override;
	void boardPreDestroy(const Coords& oCoords) noexcept override;
	void boardPostDestroy(const Coords& oCoords) noexcept override;
	void boardPreModify(const TileCoords& oTileCoords) noexcept override;
	void boardPostModify(const Coords& oCoords) noexcept override;
	// TileAnimator
	double getElapsed01(int32_t nHash, int32_t nX, int32_t nY, int32_t nAni, int32_t nViewTick, int32_t nTotViewTicks) const noexcept override;
	double getElapsed01(int32_t nHash, const LevelBlock& oLevelBlock, int32_t nBrickIdx, int32_t nAni, int32_t nViewTick, int32_t nTotViewTicks) const noexcept override;

	// input
	enum {
		MESSAGE_DROP_JUNK = 100 /**< The number of junk tiles to put on the board is in nValue. If nValue is &lt;=0 ignored. */
		, MESSAGE_SET_TILE_GEN = 120 /**< nValue is index into LocalInit::m_aProbTileGens. If negative sets to 0.
										 * If bigger than vector size sets last. */
		, MESSAGE_NEXT_TILE_GEN = 121 /**< Switches to next row generator. If last stays on last. */
		, MESSAGE_PREV_TILE_GEN = 122 /**< Switches to previous row generator. If first stays on first. */
		, MESSAGE_PAUSE = 150 /**< Pause dropping tiles. */
		, MESSAGE_RESUME = 151 /**< Resme dropping tiles. */
	};
	// output
	enum {
		LISTENER_GROUP_PLACED_JUNK = 10 /**< Placed junk. The number of junk tiles put on board is in nValue. */
	};

private:
	void commonInit(LocalInit&& oInit) noexcept;
	void resetRuntime() noexcept;

	// Returns the number of newly added junk tiles
	int32_t fillEmptyTilesOfTopLine(Level& oLevel, int32_t nCurInterval, int32_t nTilesToAdd) noexcept;
	// The x of the column with more empty tiles on top
	// or -1 if all full
	int32_t findEmptiestColumn(Level& oLevel, int32_t nTopY) noexcept;

	void continueOrFinishTileAnis(Level& oLevel) noexcept;
	void removeAllTileAnis(const NRect& oArea) noexcept;
	void removeCoordsTileAnis(const Coords& oCoords) noexcept;

	int32_t calcNextDrop(GameProxy& oGame, int32_t nCurGameTick, int32_t nCurInterval) noexcept;
	void activateAndInform(Level& oLevel, int32_t nAddedTiles, int32_t nActivateTick) noexcept;

private:
	int32_t m_nTileAniRemovingIndex;
	int32_t m_nTileAniMaterializeIndex;

	std::vector< unique_ptr<RandomTiles> > m_aRandomTiles;

	int32_t m_nMaterializingTicksFrom;
	int32_t m_nMaterializingTicksTo;
	int32_t m_nMaterializingMillisecFrom;
	int32_t m_nMaterializingMillisecTo;
	int32_t m_nWaitTicksFrom;
	int32_t m_nWaitTicksTo;
	int32_t m_nWaitMillisecFrom;
	int32_t m_nWaitMillisecTo;
	int32_t m_nMaxJunkPerDrop;
	int32_t m_nTopY;

	int32_t m_nBoardW;
	int32_t m_nBoardH;

	enum TOP_JUNK_STATE
	{
		TOP_JUNK_STATE_ACTIVATE = 0,
		TOP_JUNK_STATE_INIT = 1,
		TOP_JUNK_STATE_RUN = 2
	};
	TOP_JUNK_STATE m_eState;

	int32_t m_nCurRandomGen; // index into m_aRandomTiles

	// Whether the droppings should pause. If paused, m_nJunkTilesToDrop is still increased for later
	// and m_nLeftOver put on board (if possible) and materializing animations are handled
	bool m_bPaused;

	// The total number of junk tiles that still have to be put on board
	int32_t m_nJunkTilesToDrop;
	// Prevents an instance to activate itself to the same game tick multiple times
	int32_t m_nLastRunTick;
	// Since main drops are interleaved by a random number of ticks, this tells when the next is
	// to happen. If -1, no main drop planned. Left overs are handled in the next game tick
	// and procrastinate the next drop.
	// When paused this is not set to -1 since the pause might be very short.
	int32_t m_nNextDropTick;
	// Junk tiles from last drop that couldn't be placed because of m_nMaxJunkPerDrop
	// or because  there is no space. These are handled even if paused!
	int32_t m_nLeftOver;

	struct TileDownCounter
	{
		int32_t m_nCountdown = -1; // -1 if no animation
		int32_t m_nGameTicksDuration = 0; // Since it depends on m_nMaterializingMillisec, can vary during the game
		int32_t m_nY; // 0 if first row fully visible when created, 1 otherwise
	};
	// On each column of the board there can only be one materializing animation
	std::vector<TileDownCounter> m_oMaterializingCounters; // Size: m_nBoardWidth, Index: x within board
	int32_t m_nMaterializingAnis; // current number of active animations (m_oMaterializingCounters[nX].m_nCountdown >= 0)
	TileCoords m_oTileCoords; // Set when putting new tiles on top row
private:
	TopJunkEvent() = delete;
	TopJunkEvent(const TopJunkEvent& oSource) = delete;
	TopJunkEvent& operator=(const TopJunkEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_TOP_JUNK_EVENT_H_ */

