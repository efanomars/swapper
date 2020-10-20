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
 * File:   gravityblock.cc
 */

#include "gravityblock.h"

#include <stmm-games/block.h>
#include <stmm-games/utile/tilerect.h>
#include <stmm-games/util/basictypes.h>

#include <vector>
#include <cassert>
//#include <iostream>

namespace stmg { class Coords; }
namespace stmg { class Event; }
namespace stmg { class Level; }
namespace stmg { class Tile; }
namespace stmg { class TileBuffer; }
namespace stmg { class TileCoords; }

namespace stmg
{

const int32_t GravityBlock::s_nFallingBlocksFactor = 3;
const int32_t GravityBlock::s_nZObjectZGravityBlock = 10;

GravityBlock::GravityBlock(Level* p0Level, Event* p0Parent, int32_t nFinishedMsg, int32_t nRuntimeId) noexcept
: LevelBlock(true)
, m_p0Level(p0Level)
, m_p0Parent(p0Parent)
, m_nTriggerFinishedMsg(nFinishedMsg)
, m_nRuntimeId(nRuntimeId)
{
	commonInit();
}

void GravityBlock::reInit(Level* p0Level, Event* p0Parent, int32_t nFinishedMsg, int32_t nRuntimeId) noexcept
{
	m_p0Level = p0Level;
	m_p0Parent = p0Parent;
	m_nTriggerFinishedMsg = nFinishedMsg;
	m_nRuntimeId = nRuntimeId;
	commonInit();
}
void GravityBlock::commonInit() noexcept
{
	assert(m_p0Level != nullptr);
	assert(m_p0Parent != nullptr);
	m_nBoardWidth = m_p0Level->boardWidth();
	m_nBoardHeight = m_p0Level->boardHeight();
	m_bLastFallTick = false;
	level().boardAddListener(this);
}
FPoint GravityBlock::blockVTPos(int32_t nViewTick, int32_t nTotViewTicks) const noexcept
{
	const NPoint oPos = LevelBlock::blockPos();
	const double fPosX = static_cast<double>(oPos.m_nX);
	double fPosY = static_cast<double>(oPos.m_nY);
	if (m_bLastFallTick) {
		fPosY += 1.0 * (-1.0 + ((1.0 + nViewTick) / nTotViewTicks)) * Direction::deltaY(Direction::DOWN);
//std::cout << "GravityBlock(" << getId() << ")::handleTimer(" << level().game().gameElapsed() << ")"<< '\n';
//std::cout << "GravityBlock::getPosY m_nPosY=" << m_nPosY << "  fPosY=" << fPosY << '\n';
	}
	return FPoint{fPosX, fPosY};
}
void GravityBlock::handleTimer() noexcept
{
//std::cout << "GravityBlock(" << getId() << ")::handleTimer(" << level().game().gameElapsed() << ")"<< '\n';
	Level& oLevel = level();
	const int32_t nTimer = oLevel.game().gameElapsed();
	const int32_t nFactor = std::max<int32_t>(1, oLevel.getFallEachTicks() / s_nFallingBlocksFactor);
	m_bLastFallTick = false;
	if (nTimer % nFactor == 0) {
		if (!move(Direction::DOWN)) {
			#ifndef NDEBUG
			const bool bFroze =
			#endif //NDEBUG
			freeze();
			assert(bFroze);
		} else {
			m_bLastFallTick = true;
		}
	}
}
bool GravityBlock::canMove(Direction::VALUE eDir) noexcept
{
	return move(eDir, false, 0,0,0,0);
}
bool GravityBlock::canMove(Direction::VALUE eDir, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH) noexcept
{
	return move(eDir, false, nClipX, nClipY, nClipW, nClipH);
}
bool GravityBlock::move(Direction::VALUE eDir) noexcept
{
	return move(eDir, true, 0,0,0,0);
}
bool GravityBlock::move(Direction::VALUE eDir, bool bDoMove
		, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH) noexcept
{
	assert((static_cast<int32_t>(eDir) >= 0) && (static_cast<int32_t>(eDir) < 4));
	Level& oLevel = level();
	const NPoint oPos = LevelBlock::blockPos();
	const int32_t nPosX = oPos.m_nX;
	const int32_t nPosY = oPos.m_nY;
	const std::vector< Block::Contact >& aContacts = blockContacts(eDir);
	for (auto& oContactBrickPos : aContacts) {
		const int32_t nBoardX = nPosX + oContactBrickPos.m_nRelX;
		const int32_t nBoardY = nPosY + oContactBrickPos.m_nRelY;
		if ((nClipW == 0) || (nClipH == 0)
				|| ((nBoardX >= nClipX) && (nBoardX < nClipX + nClipW)
					&& (nBoardY >= nClipY) && (nBoardY < nClipY + nClipH)))
		{
			if ((nBoardX < 0) || (nBoardX >= m_nBoardWidth)
					|| (nBoardY >= m_nBoardHeight) || (nBoardY < 0)) {
				// cannot move
				return false;
			}
			const Tile& oTile = oLevel.boardGetTile(nBoardX, nBoardY);
			if (!oTile.isEmpty()) {
				// solid tile: cannot move
				return false;
			} else {
				LevelBlock* p0LevelBlock = oLevel.boardGetOwner(nBoardX, nBoardY);
				if (p0LevelBlock != nullptr) {
					assert(p0LevelBlock != this);
					return false;
				}
			}
		}
	}
	if (bDoMove) {
		const int32_t nDx = Direction::deltaX(eDir);
		const int32_t nDy = Direction::deltaY(eDir);
		blockMove(nDx, nDy);
	}
	return true;
}

void GravityBlock::fall() noexcept
{
}

void GravityBlock::privateOnFreeze() noexcept
{
	level().boardRemoveListener(this);
	if (m_p0Parent != nullptr) {
		m_p0Level->triggerEvent(m_p0Parent, m_nTriggerFinishedMsg, m_nRuntimeId, m_p0Parent);
	}
}
bool GravityBlock::remove() noexcept
{
	return freeze();
}
bool GravityBlock::destroy() noexcept
{
	return freeze();
}
bool GravityBlock::freeze() noexcept
{
	#ifndef NDEBUG
	const bool bFroze =
	#endif //NDEBUG
	LevelBlock::freeze();
	assert(bFroze);
	privateOnFreeze();
	return true;
}
bool GravityBlock::fuseTo(LevelBlock& /*oLevelBlock*/) noexcept
{
	return false;
}
bool GravityBlock::removeBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}
bool GravityBlock::destroyBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}

LevelBlock::QUERY_ATTACK_TYPE GravityBlock::queryAttack(LevelBlock& /*oAttacker*/
														, int32_t /*nBoardX*/, int32_t /*nBoardY*/, const Tile& /*oTile*/) const noexcept
{
	return LevelBlock::QUERY_ATTACK_TYPE_FREEZE_ATTACKED;
}
bool GravityBlock::attack(LevelBlock& /*oAttacker*/, int32_t /*nBoardX*/, int32_t /*nBoardY*/, const Tile& /*oTile*/) noexcept
{
	#ifndef NDEBUG
	const bool bFroze =
	#endif //NDEBUG
	freeze();
	assert(bFroze);
	return true;
}

void GravityBlock::boabloPreFreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void GravityBlock::boabloPostFreeze(const Coords& /*oCoords*/) noexcept
{
}
void GravityBlock::boabloPreUnfreeze(const Coords& /*oCoords*/) noexcept
{
}
void GravityBlock::boabloPostUnfreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void GravityBlock::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
	assert(blockIsAutoOwner());
	const int32_t nDx = Direction::deltaX(eDir);
	const int32_t nDy = Direction::deltaY(eDir);
	if (!level().blockMoveIsWithinArea(*this, nDx, nDy, 0, 0, m_nBoardWidth, m_nBoardHeight)) {
		// block is scrolled outside board, freeze first
		#ifndef NDEBUG
		const bool bFroze =
		#endif //NDEBUG
		freeze();
		assert(bFroze);
	}
}
void GravityBlock::boardPostScroll(Direction::VALUE /*eDir*/) noexcept
{
}
void GravityBlock::boardPreInsert(Direction::VALUE eDir, NRect oRect, const shared_ptr<TileRect>& refTiles) noexcept
{
	if (eDir == Direction::DOWN) {
		if (refTiles && !TileRect::isAllEmptyTiles(*refTiles)) {
			level().gameStatusTechnical(std::vector<std::string>{"GravityBlock::boardPreInsert()","DOWN only supports empty tiles insertion"});
			return;
		}
	}
	if (!((eDir == Direction::UP) || (eDir == Direction::DOWN))) {
		level().gameStatusTechnical(std::vector<std::string>{"GravityBlock::boardPreInsert()","Only DOWN and UP supported"});
		return;
	}
	if (oRect.m_nY != 0) {
		level().gameStatusTechnical(std::vector<std::string>{"GravityBlock::boardPreInsert()","Only nY=0 supported"});
		return;
	}
	if (eDir == Direction::DOWN) {
		boardPreDeleteDown(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW);
	} else{
		boardPreInsertUp(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW, shared_ptr<TileBuffer>());
	}
}
void GravityBlock::boardPostInsert(Direction::VALUE /*eDir*/, NRect /*oRect*/) noexcept
{
}
void GravityBlock::boardPreDeleteDown(int32_t nDelY, int32_t nX, int32_t nW) noexcept
{
//std::cout << "GravityBlock(" << getId() << ")::boardPreDeleteDown" << '\n';
	if ((nDelY > 0) && !canMove(Direction::UP, nX, 0, nW, nDelY)) {
		// it is pushed down by some board cell
		#ifndef NDEBUG
		const bool bFroze =
		#endif //NDEBUG
		freeze();
		assert(bFroze);
	}
}
void GravityBlock::boardPreInsertUp(int32_t nY, int32_t nX, int32_t nW, const shared_ptr<TileBuffer>& /*refBuffer*/) noexcept
{
//std::cout << "GravityBlock(" << getId() << ")::boardPreInsertUp" << '\n';
	if ((nY > 0) && !canMove(Direction::DOWN, nX, 1, nW, nY)) {
		// pushed up by some board cell
		#ifndef NDEBUG
		const bool bFroze =
		#endif //NDEBUG
		freeze();
		assert(bFroze);
		return;
	}
	if (level().blockIntersectsArea(*this, nX, nY, nW, 1)) {
		#ifndef NDEBUG
		const bool bFroze =
		#endif //NDEBUG
		freeze();
		assert(bFroze);
		return;
	}
}
void GravityBlock::boardPreDestroy(const Coords& /*oCoords*/) noexcept
{
}
void GravityBlock::boardPostDestroy(const Coords& /*oCoords*/) noexcept
{
}
void GravityBlock::boardPreModify(const TileCoords& /*oTileCoords*/) noexcept
{
}
void GravityBlock::boardPostModify(const Coords& /*oCoords*/) noexcept
{
}

} // namespace stmg
