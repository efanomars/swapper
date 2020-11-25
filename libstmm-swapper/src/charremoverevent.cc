/*
 * Copyright © 2020  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   charremoverevent.cc
 */

#include "charremoverevent.h"

#include "adjremover.h"

#include <stmm-games/level.h>
#include <stmm-games/utile/extendedboard.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/coords.h>

#include <cassert>
//#include <iostream>

namespace stmg
{

CharRemoverEvent::CharRemoverEvent(Init&& oInit) noexcept
: TileRemoverEvent(std::move(oInit))
, m_oData(std::move(oInit))
, m_bHasIrremovable(m_oData.m_refIrremovable.operator bool())
{
	assert(m_oData.m_nMinAdj > 1);
}

void CharRemoverEvent::reInit(Init&& oInit) noexcept
{
	TileRemoverEvent::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	assert(m_oData.m_nMinAdj > 1);
	m_bHasIrremovable = m_oData.m_refIrremovable.operator bool();
}

bool CharRemoverEvent::getCoordsToRemove(NPoint oFromXY, const NRect& oArea, Coords& aToRemove) noexcept
{
	Level& oLevel = level();
	const int32_t nTileAniRemovingIndex = getTileAniRemovingIndex();
	const int32_t nMinAdj = (m_oData.m_bHorizVert ? m_oData.m_nMinAdj : 0);
	if (m_bHasIrremovable) {
		AdjRemover::getCoordsToRemove(nMinAdj, oFromXY, oArea, aToRemove
			, [&](int32_t nX, int32_t nY) -> const Tile&
			{
				return oLevel.boardGetTile(nX, nY);
			}, [&](int32_t nX, int32_t nY) -> bool
			{
				return (oLevel.boardGetTileAnimator(nX, nY, nTileAniRemovingIndex) != nullptr);
			}, [&](const Tile& oTile) -> bool
			{
				return oTile.isEmpty() || m_oData.m_refIrremovable->select(oTile);
			}, [&](const Tile& oTile1, const Tile& oTile2) -> bool
			{
				return (oTile1.getTileChar() == oTile2.getTileChar());
			});
	} else {
		AdjRemover::getCoordsToRemove(nMinAdj, oFromXY, oArea, aToRemove
			, [&](int32_t nX, int32_t nY) -> const Tile&
			{
				return oLevel.boardGetTile(nX, nY);
			}, [&](int32_t nX, int32_t nY) -> bool
			{
				return (oLevel.boardGetTileAnimator(nX, nY, nTileAniRemovingIndex) != nullptr);
			}, [&](const Tile& oTile) -> bool
			{
				return oTile.isEmpty();
			}, [&](const Tile& oTile1, const Tile& oTile2) -> bool
			{
				return (oTile1.getTileChar() == oTile2.getTileChar());
			});
	}
	if (nMinAdj > 0) {
		return (aToRemove.size() > 0); //---------------------------------------
	} else {
		return (static_cast<int32_t>(aToRemove.size()) >= m_oData.m_nMinAdj);
	}
}
bool CharRemoverEvent::getCoordsToRemove(NPoint oFromXY, const NRect& oArea, const ExtendedBoard& oBoard, Coords& aToRemove) noexcept
{
	const int32_t nTileAniRemovingIndex = getTileAniRemovingIndex();
	const int32_t nMinAdj = (m_oData.m_bHorizVert ? m_oData.m_nMinAdj : 0);
	if (m_bHasIrremovable) {
		AdjRemover::getCoordsToRemove(nMinAdj, oFromXY, oArea, aToRemove
			, [&](int32_t nX, int32_t nY) -> const Tile&
			{
				return oBoard.boardGetTile(nX, nY);
			}, [&](int32_t nX, int32_t nY) -> bool
			{
				return (oBoard.boardGetTileAnimator(nX, nY, nTileAniRemovingIndex) != nullptr);
			}, [&](const Tile& oTile) -> bool
			{
				return oTile.isEmpty() || m_oData.m_refIrremovable->select(oTile);
			}, [&](const Tile& oTile1, const Tile& oTile2) -> bool
			{
				return (oTile1.getTileChar() == oTile2.getTileChar());
			});
	} else {
		AdjRemover::getCoordsToRemove(nMinAdj, oFromXY, oArea, aToRemove
			, [&](int32_t nX, int32_t nY) -> const Tile&
			{
				return oBoard.boardGetTile(nX, nY);
			}, [&](int32_t nX, int32_t nY) -> bool
			{
				return (oBoard.boardGetTileAnimator(nX, nY, nTileAniRemovingIndex) != nullptr);
			}, [&](const Tile& oTile) -> bool
			{
				return oTile.isEmpty();
			}, [&](const Tile& oTile1, const Tile& oTile2) -> bool
			{
				return (oTile1.getTileChar() == oTile2.getTileChar());
			});
	}
	if (nMinAdj > 0) {
		return (aToRemove.size() > 0); //---------------------------------------
	} else {
		return (static_cast<int32_t>(aToRemove.size()) >= m_oData.m_nMinAdj);
	}
}

} // namespace stmg
