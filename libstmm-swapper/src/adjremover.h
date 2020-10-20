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
 * File:   adjremover.h
 */

#ifndef STMG_ADJ_REMOVER_H_
#define STMG_ADJ_REMOVER_H_

#include <stmm-games/tile.h>

#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/coords.h>

#include <vector>
#include <unordered_set>

#include <stdint.h>

namespace stmg
{

namespace AdjRemover
{

template<class TTiles, class TTileBusy, class TTilesSame>
void getCoordsToRemove(NPoint oFromXY, const NRect& oArea
						, Coords& oToRemove
						, const TTiles& oTiles, const TTileBusy& oTileBusy
						, const TTilesSame& oTilesSame) noexcept
{
	const int32_t& nFromX = oFromXY.m_nX;
	const int32_t& nFromY = oFromXY.m_nY;
//std::cout << "AdjRemover::getCoordsToRemove nFromX=" << nFromX << " nFromY=" << nFromY << '\n';
	const Tile& oTile1 = oTiles(nFromX, nFromY);
	std::unordered_set< NPoint > oToEval;
	oToEval.insert(oFromXY);

	while (!oToEval.empty()) {
		const NPoint oXY = *oToEval.begin();
		oToEval.erase(oToEval.begin());
		const int32_t nX = oXY.m_nX;
		const int32_t nY = oXY.m_nY;
//std::cout << "AdjRemover::getCoordsToRemove eval nX=" << nX << " nY=" << nY << '\n';
		if ((! oTileBusy(nX, nY)) && oTilesSame(oTile1, oTiles(nX, nY))) {
//std::cout << "AdjRemover::getCoordsToRemove same" << '\n';
			oToRemove.add(nX, nY);
			if (nX > oArea.m_nX) {
				if (! oToRemove.contains(nX - 1, nY)) {
					oToEval.insert(NPoint{nX - 1, nY});
				}
			}
			if (nX < oArea.m_nX + oArea.m_nW - 1) {
				if (! oToRemove.contains(nX + 1, nY)) {
//std::cout << "AdjRemover::getCoordsToRemove add +1,0" << '\n';
					oToEval.insert(NPoint{nX + 1, nY});
				}
			}
			if (nY > oArea.m_nY) {
				if (! oToRemove.contains(nX, nY - 1)) {
					oToEval.insert(NPoint{nX, nY - 1});
				}
			}
			if (nY < oArea.m_nY + oArea.m_nH - 1) {
				if (! oToRemove.contains(nX, nY + 1)) {
					oToEval.insert(NPoint{nX, nY + 1});
				}
			}
		}
	}
}

template<class TTiles, class TTileBusy, class TTilesSame>
void getCoordsToRemoveHorizVert(NPoint oFromXY, int32_t nMinLen, const NRect& oArea
								, Coords& oToRemove
								, const TTiles& oTiles, const TTileBusy& oTileBusy
								, const TTilesSame& oTilesSame) noexcept
{
	assert(nMinLen >= 2);
	const int32_t& nFromX = oFromXY.m_nX;
	const int32_t& nFromY = oFromXY.m_nY;
	const Tile& oTile1 = oTiles(nFromX, nFromY);
	//TODO recycle and vector instead of unordered_set
	std::unordered_set< NPoint > oToEvalHoriz;
	std::unordered_set< NPoint > oToEvalVert;
	oToEvalHoriz.insert(oFromXY);
	oToEvalVert.insert(oFromXY);
	std::unordered_set< NPoint > oDoneHoriz;
	std::unordered_set< NPoint > oDoneVert;

	while (true) {
		const bool bHorizEmpty = oToEvalHoriz.empty();
		if (bHorizEmpty && oToEvalVert.empty()) {
			break;
		}
		if (! bHorizEmpty) {
			const NPoint oXY = *oToEvalHoriz.begin();
			oToEvalHoriz.erase(oToEvalHoriz.begin());
			const int32_t nY = oXY.m_nY;
			int32_t nEndX = oXY.m_nX + 1;
			for (; nEndX < oArea.m_nX + oArea.m_nW; ++nEndX) {
				if ((oTileBusy(nEndX, nY)) || ! oTilesSame(oTile1, oTiles(nEndX, nY))) {
					break;
				}
			}
			int32_t nStartX = oXY.m_nX - 1;
			for (; nStartX >= oArea.m_nX; --nStartX) {
				if ((oTileBusy(nStartX, nY)) || ! oTilesSame(oTile1, oTiles(nStartX, nY))) {
					break;
				}
			}
	//std::cout << "AdjRemover::getCoordsToRemove nStartX = " << nStartX << "  nEndX = " << nEndX << '\n';
			if (nEndX - 1 - nStartX >= nMinLen) {
				for (int32_t nCurX = nStartX + 1; nCurX < nEndX; ++nCurX) {
					oToRemove.add(nCurX, nY);
					if (oDoneVert.find(NPoint{nCurX, nY}) == oDoneVert.end()) {
						oToEvalVert.insert(NPoint{nCurX, nY});
					}
				}
			}
			for (int32_t nCurX = nStartX + 1; nCurX < nEndX; ++nCurX) {
				oDoneHoriz.insert(NPoint{nCurX, nY});
			}
		} else {
			assert(! oToEvalVert.empty());
			NPoint oXY = *oToEvalVert.begin();
			oToEvalVert.erase(oToEvalVert.begin());
			const int32_t nX = oXY.m_nX;
			int32_t nEndY = oXY.m_nY + 1;
			for (; nEndY < oArea.m_nY + oArea.m_nH; ++nEndY) {
				if ((oTileBusy(nX, nEndY)) || ! oTilesSame(oTile1, oTiles(nX, nEndY))) {
					break;
				}
			}
			int32_t nStartY = oXY.m_nY - 1;
			for (; nStartY >= oArea.m_nY; --nStartY) {
				if ((oTileBusy(nX, nStartY)) || ! oTilesSame(oTile1, oTiles(nX, nStartY))) {
					break;
				}
			}
			if (nEndY - 1 - nStartY >= nMinLen) {
				for (int32_t nCurY = nStartY + 1; nCurY < nEndY; ++nCurY) {
					oToRemove.add(nX, nCurY);
					if (oDoneHoriz.find(NPoint{nX, nCurY}) == oDoneHoriz.end()) {
						oToEvalHoriz.insert(NPoint{nX, nCurY});
					}
				}
			}
			for (int32_t nCurY = nStartY + 1; nCurY < nEndY; ++nCurY) {
				oDoneVert.insert(NPoint{nX, nCurY});
			}
		}
	}
}

} // namespace AdjRemover

} // namespace stmg

#endif	/* STMG_ADJ_REMOVER_H_ */
