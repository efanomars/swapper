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
 * File:   gravityevent.h
 */

#ifndef STMG_GRAVITY_EVENT_H_
#define STMG_GRAVITY_EVENT_H_

#include "gravityblock.h"

#include <stmm-games/level.h>
#include <stmm-games/event.h>
#include <stmm-games/levellisteners.h>
#include <stmm-games/util/recycler.h>
#include <stmm-games/util/direction.h>

#include <vector>
#include <memory>

#include <stdint.h>

namespace stmg { class TileRect; }
namespace stmg { class Coords; }
namespace stmg { class TileCoords; }
namespace stmg { class LevelBlock; }
namespace stmg { struct NRect; }

namespace stmg
{

using std::shared_ptr;

class GravityEvent : public Event, public BoardListener, private Level::LevelBlockCreator
{
public:
	struct LocalInit
	{
		int32_t m_nRepeat = -1; /**< How many times the board has to be checked for gravity blocks. -1 means infinite. Default is -1. */
		int32_t m_nStep = 1; /**< The interval between checks in game ticks. Must be &gt;= 1. Default is 1. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit Initialization data.
	 */
	explicit GravityEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit Initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;
	// BoardListener
	void boabloPreFreeze(LevelBlock& oBlock) noexcept override;
	void boabloPostFreeze(const Coords& oCoords) noexcept override;
	void boabloPreUnfreeze(const Coords& oCoords) noexcept override;
	void boabloPostUnfreeze(LevelBlock& oBlock) noexcept override;
	void boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostScroll(Direction::VALUE eDir) noexcept override;
	void boardPreInsert(Direction::VALUE eDir, NRect oRect, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostInsert(Direction::VALUE eDir, NRect oRect) noexcept override;
	void boardPreDestroy(const Coords& oCoords) noexcept override;
	void boardPostDestroy(const Coords& oCoords) noexcept override;
	void boardPreModify(const TileCoords& oTileCoords) noexcept override;
	void boardPostModify(const Coords& oCoords) noexcept override;

private:
	void commonInit() noexcept;
	void runtimeInit() noexcept;

	void checkForHoles() noexcept;
	bool checkForHolesCol(int32_t nX) noexcept;
	LevelBlock* create() noexcept override;
	void createGravityBlock(int32_t nX, int32_t nY, int32_t nH) noexcept;
	void freeGravityBlock(int32_t nRuntimeId) noexcept;
	int32_t findGravityBlock(int32_t nRuntimeId) noexcept;

	void boardPostDeleteDown(int32_t nY, int32_t nX, int32_t nW) noexcept;
	void boardPostInsertUp(int32_t nY, int32_t nX, int32_t nW) noexcept;
private:
	LocalInit m_oData;
	int32_t m_nTileAniRemovingIndex;
	int32_t m_nTileAniMaterializingIndex;

	int32_t m_nBoardW;
	int32_t m_nBoardH;

	enum GRAVITY_STATE
	{
		GRAVITY_STATE_ACTIVATE = 0,
		GRAVITY_STATE_INIT = 1,
		GRAVITY_STATE_RUN = 2
	};
	GRAVITY_STATE m_eState;

	int32_t m_nNextCheckGameTick;

	std::vector<bool> m_aColDirty; // size: m_nBoardW

	std::vector< shared_ptr<GravityBlock> > m_aFallingCols;
	Recycler<GravityBlock> m_oGravityBlockRecycler;

	static const int32_t s_nMagicBlockFinishedMsg;

	int32_t m_nCounter;
private:
	GravityEvent() = delete;
	GravityEvent(const GravityEvent& oSource) = delete;
	GravityEvent& operator=(const GravityEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_GRAVITY_EVENT_H_ */

