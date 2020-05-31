/*
 * Copyright © 2019  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   tileremoverevent.h
 */

#ifndef STMG_TILE_REMOVER_EVENT_H_
#define STMG_TILE_REMOVER_EVENT_H_

//#include <stmm-games/level.h>
#include <stmm-games/event.h>
#include <stmm-games/levellisteners.h>
#include <stmm-games/tileanimator.h>
#include <stmm-games/utile/querytileremoval.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/direction.h>
#include <stmm-games/util/recycler.h>

#include <vector>
#include <memory>
#include <list>

#include <stdint.h>

namespace stmg { class ExtendedBoard; }
namespace stmg { class LeelBlock; }
namespace stmg { class TileCoords; }
namespace stmg { class Coords; }

namespace stmg
{

using std::shared_ptr;

class TileRemoverEvent : public Event, public BoardListener, public TileAnimator, public QueryTileRemoval
{
public:
	struct LocalInit
	{
		int32_t m_nRepeat = -1; /**< How many times the board has to be checked for tile removal. The default is -1 (means infinite). */
		int32_t m_nStep = 1; /**< The interval between checks in game ticks. Must be &gt;= 1. The default is 1. */
		NRect m_oArea; /**< The area to be checked. Must be completely within board. */
		int32_t m_nDeleteAfter = 0; /**< The number of game ticks after detection the to be removed tiles are animated.
									 * Default is 0. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit The initialization data.
	 */
	explicit TileRemoverEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	/** Tells whether tiles would be removed in an extended board.
	 * The extended area doesn't need to even intersect with the initialization area.
	 * @param oBoard The extended board.
	 * @param oExtArea The extended area that could also be partly outside the level's board. Size must be positive.
	 * @return The number of tiles that would be removed.
	 */
	int32_t wouldRemoveTiles(const ExtendedBoard& oBoard, const NRect& oExtArea) noexcept override;
	/** The area passed to the constructor.
	 * @return The area within the board.
	 */
	const NRect& getArea() const noexcept { return m_oData.m_oArea; }

	// Event
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;
	// BoardListener
	void boabloPreFreeze(LevelBlock& oBlock) noexcept override;
	void boabloPostFreeze(const Coords& oCoords) noexcept override;
	void boabloPreUnfreeze(const Coords& oCoords) noexcept override;
	void boabloPostUnfreeze(LevelBlock& oBlock) noexcept override;
	void boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostScroll(Direction::VALUE eDir) noexcept override;
	void boardPreInsert(Direction::VALUE eDir, NRect oArea, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostInsert(Direction::VALUE eDir, NRect oArea) noexcept override;
	void boardPreDestroy(const Coords& oCoords) noexcept override;
	void boardPostDestroy(const Coords& oCoords) noexcept override;
	void boardPreModify(const TileCoords& oTileCoords) noexcept override;
	void boardPostModify(const Coords& oCoords) noexcept override;

	// TileAnimator
	double getElapsed01(int32_t nHash, int32_t nX, int32_t nY, int32_t nAni, int32_t nViewTick
						, int32_t nTotViewTicks) const noexcept override;
	double getElapsed01(int32_t nHash, const LevelBlock& oLevelBlock, int32_t nBrickIdx
						, int32_t nAni, int32_t nViewTick, int32_t nTotViewTicks) const noexcept override;

	enum {
		LISTENER_GROUP_REMOVED = 10 /**< The number of tiles removed is passed as the message nValue
									 * (see trigger() function). */
	};

	static constexpr int32_t s_nMaxDeleteAfter = 10000;

protected:
	/** Function that identifies the tiles to remove.
	 * To avoid rescanning the whole board the instance keeps track of the
	 * tiles modified (detection through BoardListener interface) within the
	 * delimiting area from the last check.
	 * For each of them this function is called.
	 *
	 * Must be implemented by subclasses.
	 *
	 * Implementations should not return coordinates of cells that currently
	 * have an active tile animator with index getTileAniRemovingIndex().
	 * @param oFromXY The point from which to check. Must be within oArea.
	 * @param oArea The area in which to check. Cannot be empty.
	 * @param aToRemove The to be removed tile coords.
	 * @return Whether to remove.
	 */
	virtual bool getCoordsToRemove(NPoint oFromXY, const NRect& oArea, Coords& aToRemove) noexcept = 0;
	/** Function that identifies the tiles to remove from extended board.
	 * To avoid rescanning the whole board the instance keeps track of the
	 * tiles modified (detection through BoardListener interface) within the
	 * delimiting area from the last check.
	 * For each of them this function is called.
	 *
	 * Must be implemented by subclasses.
	 *
	 * Implementations should not return coordinates of cells that currently
	 * have an active tile animator with index getTileAniRemovingIndex().
	 * @param oFromXY The point from which to check. Must be within oArea.
	 * @param oArea The area in which to check. Cannot be empty.
	 * @param oBoard The extended board from which get tile status.
	 * @param aToRemove The to be removed tile coords.
	 * @return Whether to remove.
	 */
	virtual bool getCoordsToRemove(NPoint oFromXY, const NRect& oArea, const ExtendedBoard& oBoard, Coords& aToRemove) noexcept = 0;

	/** The tile animator index of TILEANI:REMOVING.
	 * This is set by constructor and reInit.
	 * @return The tile ani index into level().getNamed().tileAnis().
	 */
	inline int32_t getTileAniRemovingIndex() const noexcept { return m_nTileAniRemovingIndex; }

private:
	struct DownCounter
	{
		int32_t m_nAllIdx; // index in m_aAllDownCounters
		bool m_bLocked;
		int32_t m_nTotSteps;
		int32_t m_nCountdown;
		bool m_bInitAni;
		shared_ptr<Coords> m_refCoords;
	};

	void commonInit() noexcept;
	void initRuntime() noexcept;

	void checkForTilesToRemove() noexcept;
	void remove() noexcept;

	void boardPostDeleteDown(int32_t nY, int32_t nX, int32_t nW) noexcept;
	void boardPostInsertUp(int32_t nY, int32_t nX, int32_t nW) noexcept;

	void recyclePushBackDownCounter(shared_ptr<DownCounter>& refDownCounter) noexcept;
	shared_ptr<DownCounter> recyclePopDownCounter() noexcept;

	enum TILE_REMOVER_STATE
	{
		TILE_REMOVER_STATE_ACTIVATE = 0,
		TILE_REMOVER_STATE_INIT = 1,
		TILE_REMOVER_STATE_RUN = 2
	};
private:
	LocalInit m_oData;

	int32_t m_nBoardW;
	int32_t m_nBoardH;

	int32_t m_nTileAniRemovingIndex;

	shared_ptr<Coords> m_refDirty;
	shared_ptr<Coords> m_refWorker;
	shared_ptr<Coords> m_refToRemove;
	typedef std::list< shared_ptr<DownCounter> > DownCounterList;
	DownCounterList m_oStages;

	DownCounterList m_oDownCounters;
	DownCounterList m_oInactiveDownCounters;
	std::vector< shared_ptr<DownCounter> > m_aAllDownCounters;
	Recycler<Coords> m_oCoordsRecycler;

	TILE_REMOVER_STATE m_eState;
	int32_t m_nCounter;
private:
	TileRemoverEvent() = delete;
	TileRemoverEvent(const TileRemoverEvent& oSource) = delete;
	TileRemoverEvent& operator=(const TileRemoverEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_TILE_REMOVER_EVENT_H_ */

