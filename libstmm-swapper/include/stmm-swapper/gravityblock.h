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
 * File:   gravityblock.h
 */

#ifndef STMG_GRAVITY_BLOCK_H_
#define STMG_GRAVITY_BLOCK_H_

#include <stmm-games/level.h>
#include <stmm-games/levellisteners.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/util/direction.h>

#include <memory>

#include <stdint.h>

namespace stmg { class Event; }
namespace stmg { class TileBuffer; }
namespace stmg { class TileRect; }
namespace stmg { class Coords; }
namespace stmg { class Tile; }
namespace stmg { class TileCoords; }
namespace stmg { struct NRect; }

namespace stmg
{

using std::shared_ptr;

class GravityBlock : public LevelBlock, public BoardListener
{
public:
	GravityBlock(Level* p0Level, Event* p0Parent, int32_t nFinishedMsg, int32_t nRuntimeId) noexcept;

	void reInit(Level* p0Level, Event* p0Parent, int32_t nFinishedMsg, int32_t nRuntimeId) noexcept;

	int32_t getRuntimeId() const noexcept { return m_nRuntimeId; }
	FPoint blockVTPos(int32_t nViewTick, int32_t nTotViewTicks) const noexcept override;
	int32_t blockPosZ() const noexcept override { return s_nZObjectZGravityBlock; }

	void handleTimer() noexcept override;
	void fall() noexcept override;

	bool remove() noexcept override;
	bool destroy() noexcept override;
	bool freeze() noexcept override;
	bool fuseTo(LevelBlock& oLevelBlock) noexcept override;
	bool removeBrick(int32_t nBrickId) noexcept override;
	bool destroyBrick(int32_t nBrickId) noexcept override;

	LevelBlock::QUERY_ATTACK_TYPE queryAttack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY, const Tile& oTile) const noexcept override;
	bool attack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY, const Tile& oTile) noexcept override;
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
	void privateOnFreeze() noexcept;
	void boardPreDeleteDown(int32_t nY, int32_t nX, int32_t nW) noexcept;
	void boardPreInsertUp(int32_t nY, int32_t nX, int32_t nW, const shared_ptr<TileBuffer>& refBuffer) noexcept;
	bool canMove(Direction::VALUE eDir, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH) noexcept;
	bool canMove(Direction::VALUE eDir) noexcept;
	bool move(Direction::VALUE eDir) noexcept;
	bool move(Direction::VALUE eDir, bool bDoMove
					, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH) noexcept;

	const Level& level() const noexcept { return *m_p0Level; }
	Level& level() noexcept { return *m_p0Level; }

private:
	Level* m_p0Level;
	int32_t m_nBoardWidth;
	int32_t m_nBoardHeight;

	Event* m_p0Parent;
	int32_t m_nTriggerFinishedMsg;
	int32_t m_nRuntimeId;

	bool m_bLastFallTick;
	static const int32_t s_nFallingBlocksFactor;
	static const int32_t s_nZObjectZGravityBlock;
private:
	GravityBlock() = delete;
	GravityBlock(const GravityBlock& oSource) = delete;
	GravityBlock& operator=(const GravityBlock& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_GRAVITY_BLOCK_H_ */

