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
 * File:   swapperevent.h
 */

#ifndef STMG_SWAPPER_EVENT_H_
#define STMG_SWAPPER_EVENT_H_

#include <stmm-games/keyactionevent.h>
#include <stmm-games/event.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/block.h>
#include <stmm-games/utile/tileselector.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/circularbuffer.h>
#include <stmm-games/util/recycler.h>

#include <memory>
#include <vector>
#include <string>

#include <stdint.h>

namespace stmg { class TileCoords; }

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

class SwapperEvent : public Event, public LevelBlock
{
public:
	struct LocalInit
	{
		Block m_oBlock; /**< The block. If empty default is used, otherwise every shape must have at least 2 visible bricks.*/
		NPoint m_oInitPos; /**< The initial position within the board. Should place block within m_oArea.*/
		NRect m_oArea; /**< Size must be at least s_nAreaMinW x s_nAreaMinH. */
		int32_t m_nTicksToLive = -1; /**< Game ticks after which the swapper (LevelBlock) is removed. Default is -1 (means never). */
		unique_ptr<TileSelector> m_refImmovable; /**< The tiles that cannot be swapped. */
		std::vector< unique_ptr<TileSelector> > m_aSwappedSelectors; /**< The tile selectors that trigger LISTENER_GROUP_SWAPPED_N when swapped. */
		int32_t m_nLevelTeam = -1; /**< The level team that can take control of the swapper or -1 if any. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * If not empty, the block's shapes must have incremental ids from 0 to m_oBlock.totShapes().
	 * The initial shape of the block is 0.
	 *
	 * The default block has width s_nDeaultBlockW and height s_nDeaultBlockH.
	 * It's bricks have charName "SWAPPER:LEFT" and "SWAPPER:RIGHT".
	 *
	 * If the block initial position doesn't place all its visible bricks within m_oArea,
	 * the block cannot swap.
	 * @param oInit The initialization data.
	 */
	explicit SwapperEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * See constructor.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	//Event
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;
	//LevelBlock
	void handleKeyActionInput(const shared_ptr<KeyActionEvent>& refEvent) noexcept override;
	void handleTimer() noexcept override;
	int32_t blockPosZ() const noexcept override;
	void fall() noexcept override;
	bool remove() noexcept override;
	bool destroy() noexcept override;
	bool freeze() noexcept override;
	bool fuseTo(LevelBlock& oLevelBlock) noexcept override;
	bool canFuseWith(LevelBlock& oLevelBlock) const noexcept override;
	bool removeBrick(int32_t nBrickId) noexcept override;
	bool destroyBrick(int32_t nBrickId) noexcept override;

	LevelBlock::QUERY_ATTACK_TYPE queryAttack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY
											, const Tile& oTile) const noexcept override;
	bool attack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY, const Tile& oTile) noexcept override;

	// input
	enum {
		MESSAGE_SET_SHAPE = 100 /**< Sets the shape (the index, not the id) of the swapper defined by nValue. */
		, MESSAGE_NEXT_SHAPE = 101 /**< Sets the next shape of the block. Cyclic. */
		, MESSAGE_PREV_SHAPE = 102 /**< Sets the previous shape of the block. Cyclic. */
		, MESSAGE_PAUSE = 110 /**< Pauses the swapper. All key input is lost. */
		, MESSAGE_RESUME = 111 /**< Resumes the swapper. */
	};
	// Outputs
	enum {
		LISTENER_GROUP_PUSH_ROW = 10 /**< The player pressed the associated to s_sKeyActionPushRow. */
		, LISTENER_GROUP_SWAPPED = 20 /**< The player swapped (s_sKeyActionSwap) two or more tiles.
										 * nValue: packed tile coordinates using Util::packPointToInt32(X,Y) of first tile of swapper. */
		, LISTENER_GROUP_IMMOVABLE = 30 /**< The player tried to swap at least one immovable tile.
										 * nValue: packed immovable tile coordinates using Util::packPointToInt32(X,Y) */
		, LISTENER_GROUP_SWAPPED_0 = 50 /**< The player swapped a tile selected by LocalInit::m_aSwappedSelectors[0].
										 * nValue: packed tile coordinates using Util::packPointToInt32(X,Y) */
		//, LISTENER_GROUP_SWAPPED_1 = 51 // The player swapped a tile selected by LocalInit::m_aSwappedSelectors[1].
		//, LISTENER_GROUP_SWAPPED_2 = 52 // The player swapped a tile selected by LocalInit::m_aSwappedSelectors[1].
		//, LISTENER_GROUP_SWAPPED_N = 50 + N
	};

	static const std::string s_sKeyActionLeft;
	static const std::string s_sKeyActionRight;
	static const std::string s_sKeyActionUp;
	static const std::string s_sKeyActionDown;
	static const std::string s_sKeyActionSwap;
	static const std::string s_sKeyActionPushRow;
	static const std::string s_sKeyActionNext;

	static const int32_t s_nDeaultBlockW;
	static const int32_t s_nDeaultBlockH;

	static const int32_t s_nAreaMinW;
	static const int32_t s_nAreaMinH;

protected:
	void onScrolled(Direction::VALUE eDir) noexcept override;
private:
	void commonInit() noexcept;
	void initBlock() noexcept;
	void privateOnRemove() noexcept;
	bool move(Direction::VALUE eDir) noexcept;
	bool canPlaceShapeInArea(int32_t nShapeId, int32_t& nDeltaX, int32_t& nDeltaY) const noexcept;
	bool swapBlock(int32_t nBlockPosY) noexcept;

	enum SWAPPER_EVENT_STATE
	{
		SWAPPER_EVENT_STATE_ACTIVATE = 0,
		SWAPPER_EVENT_STATE_INIT = 1,
		SWAPPER_EVENT_STATE_WAIT = 2,
		SWAPPER_EVENT_STATE_DEAD = 3
	};
private:
	LocalInit m_oData;

	// Set to true if initial position of first shape of block doesn't fit into area.
	// In that case wapping is disabled
	bool m_bNoSwap;
	int32_t m_nTileAniRemovingIndex;

	SWAPPER_EVENT_STATE m_eState;
	int32_t m_nTickStarted;
	bool m_bPaused;

	CircularBuffer<int32_t> m_oKeys; // Value: nKeyActionId

	Recycler<TileCoords> m_oReciclerTileCoords;
	shared_ptr<TileCoords> m_refTileCoords;

	int32_t m_nKeyActionLeft;
	int32_t m_nKeyActionRight;
	int32_t m_nKeyActionUp;
	int32_t m_nKeyActionDown;
	int32_t m_nKeyActionSwap;
	int32_t m_nKeyActionPushRow;
	int32_t m_nKeyActionNext;

private:
	SwapperEvent() = delete;
	SwapperEvent(const SwapperEvent& oSource) = delete;
	SwapperEvent& operator=(const SwapperEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_SWAPPER_EVENT_H_ */

