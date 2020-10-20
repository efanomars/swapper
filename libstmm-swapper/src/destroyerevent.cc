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
 * File:   destroyerevent.cc
 */

#include "destroyerevent.h"

#include <stmm-games/level.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/utile/tilecoords.h>
#include <stmm-games/util/util.h>
#include <stmm-games/gameproxy.h>

#include <array>
#include <cassert>
//#include <iostream>


namespace stmg
{

static constexpr int32_t s_nMaxToExplode = 20;

DestroyerEvent::DestroyerEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oData(std::move(oInit))
, m_oToExplodes(s_nMaxToExplode)
{
	commonInit();
}
void DestroyerEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
void DestroyerEvent::commonInit() noexcept
{
	Level& oLevel = level();

	m_eState = DESTROYER_STATE_ACTIVATE;

	const int32_t nBoardW = oLevel.boardWidth();
	const int32_t nBoardH = oLevel.boardHeight();

	NRect& oArea = m_oData.m_oArea;
	assert((oArea.m_nX >= 0) && (oArea.m_nX < nBoardW));
	assert((oArea.m_nY >= 0) && (oArea.m_nY < nBoardH));
	if (oArea.m_nW <= 0) {
		oArea.m_nW = nBoardW - oArea.m_nX;
	}
	if (oArea.m_nH <= 0) {
		oArea.m_nH = nBoardH - oArea.m_nY;
	}
}


void DestroyerEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
	Level& oLevel = level();
	//TODO
	// ACTIVATE activate event
	// RUN  check for full lines
	//      send messages to listeners with the number of full lines
	//      delete full lines
	const int32_t nGameTick = oLevel.game().gameElapsed();
//std::cout << "DestroyerEvent::trigger nGameTick=" << nGameTick << '\n';
	switch (m_eState)
	{
		case DESTROYER_STATE_ACTIVATE:
		{
			m_eState = DESTROYER_STATE_RUN;
			if (p0TriggeringEvent != nullptr) {
				if (! m_oToExplodes.isFull()) {
					ToExplode oTE;
					oTE.m_nMsg = nMsg;
					oTE.m_oOrigin = Util::unpackPointFromInt32(nValue);
					m_oToExplodes.write(std::move(oTE));
				}
				oLevel.activateEvent(this, nGameTick);
				break; //--------
			} else {
				assert(! m_oToExplodes.isFull());
				ToExplode oTE;
				oTE.m_nMsg = nMsg;
				const int32_t nBoardW = oLevel.boardWidth();
				const int32_t nBoardH = oLevel.boardHeight();
				oTE.m_oOrigin = NPoint{nBoardW, nBoardH};
				m_oToExplodes.write(std::move(oTE));
			}
		} // fallthrough
		case DESTROYER_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				if (! m_oToExplodes.isFull()) {
					ToExplode oTE;
					oTE.m_nMsg = nMsg;
					oTE.m_oOrigin = Util::unpackPointFromInt32(nValue);
					m_oToExplodes.write(std::move(oTE));
				}
				oLevel.activateEvent(this, nGameTick);
				break;
			}
			assert(! m_oToExplodes.isEmpty());
			const ToExplode& oTE = m_oToExplodes.read();
			if (m_oData.m_bRemove) {
				createExplosionRemove(oTE);
			} else {
				createExplosionDestroy(oTE);
			}
			if (! m_oToExplodes.isEmpty()) {
				oLevel.activateEvent(this, nGameTick);
			}
		}
		break;
	}
}

void DestroyerEvent::createExplosionDestroy(const ToExplode& oTE) noexcept
{
//std::cout << "DestroyerEvent::createExplosion()" << '\n';
	Coords oCoords;
	if (createExplosion(oCoords, oTE)) {
		if (! oCoords.isEmpty()) {
			Level& oLevel = level();
			oLevel.boardDestroy(oCoords);
		}
	}
}
void DestroyerEvent::createExplosionRemove(const ToExplode& oTE) noexcept
{
	TileCoords oCoords;
	if (createExplosion(oCoords, oTE)) {
		if (! oCoords.isEmpty()) {
			Level& oLevel = level();
			oLevel.boardModify(oCoords);
		}
	}
}
bool DestroyerEvent::createExplosion(Coords& oCoords, const ToExplode& oTE) const noexcept
{
//std::cout << "DestroyerEvent::createExplosion  nX=" << oTE.m_oOrigin.m_nX << "  nY=" << oTE.m_oOrigin.m_nY << '\n';
	const int32_t& nMsg = oTE.m_nMsg;
	if (nMsg == DestroyerEvent::MESSAGE_DESTROY_ROW) {
		addRow(oCoords, oTE.m_oOrigin.m_nY);
	} else if (nMsg == DestroyerEvent::MESSAGE_DESTROY_COLUMN) {
		addColumn(oCoords, oTE.m_oOrigin.m_nX);
	} else if (nMsg == DestroyerEvent::MESSAGE_DESTROY_ROW_COLUMN) {
		addRow(oCoords, oTE.m_oOrigin.m_nY);
		addColumn(oCoords, oTE.m_oOrigin.m_nX);
	} else if ((nMsg >= MESSAGE_DESTROY_R0) && (nMsg <= MESSAGE_DESTROY_RMAX)) {
		const int32_t nRadius = nMsg - MESSAGE_DESTROY_R0;
		addCircle(oCoords, oTE.m_oOrigin.m_nX, oTE.m_oOrigin.m_nY, nRadius);
	} else {
		return false; //--------------------------------------------------------
	}
	return true;
}
void DestroyerEvent::addRow(Coords& oCoords, int32_t nY) const noexcept
{
	const NRect& oArea = m_oData.m_oArea;
	if (! ((nY >= oArea.m_nY) && (nY < oArea.m_nY + oArea.m_nH))) {
		return;
	}
	for (int32_t nX = oArea.m_nX; nX < oArea.m_nX + oArea.m_nW; ++nX) {
//std::cout << "DestroyerEvent::addRow  nX=" << nX << "  nY=" << nY << '\n';
		oCoords.add(nX, nY);
	}
}
void DestroyerEvent::addColumn(Coords& oCoords, int32_t nX) const noexcept
{
	const NRect& oArea = m_oData.m_oArea;
	if (! ((nX >= oArea.m_nX) && (nX < oArea.m_nX + oArea.m_nW))) {
		return;
	}
	for (int32_t nY = oArea.m_nY; nY < oArea.m_nY + oArea.m_nH; ++nY) {
//std::cout << "DestroyerEvent::addColumn  nX=" << nX << "  nY=" << nY << '\n';
		oCoords.add(nX, nY);
	}
}
void DestroyerEvent::addCircle(Coords& oCoords, int32_t nX, int32_t nY, int32_t nRadius) const noexcept
{
//std::cout << "DestroyerEvent::addCircle  nX=" << nX << "  nY=" << nY << "  nRadius=" << nRadius << '\n';
	static constexpr int32_t nMaxRadius = MESSAGE_DESTROY_RMAX - MESSAGE_DESTROY_R0;
	static_assert(nMaxRadius == (6 - 1), "");
	static constexpr std::array< std::array<int32_t, 3>, 6 > s_aNotExplo{{{0,0,0},{0,0,0},{1,0,0},{2,1,0},{3,2,1},{4,2,1}}};
	assert((nRadius >= 0) && (nRadius <= nMaxRadius));
	const std::array<int32_t, 3>& aNotExplo = s_aNotExplo[nRadius];
	const NRect& oArea = m_oData.m_oArea;
//std::cout << "DestroyerEvent::addCircle  (" << oArea.m_nX << ", " << oArea.m_nY << ", " << oArea.m_nW << ", " << oArea.m_nH << ")" << '\n';
	for (int32_t nCurY = nY - nRadius; nCurY <= nY + nRadius; ++nCurY) {
		for (int32_t nCurX = nX - nRadius; nCurX <= nX + nRadius; ++nCurX) {
//std::cout << "DestroyerEvent::addCircle  nCurX=" << nCurX << "  nCurY=" << nCurY << '\n';
			if (! oArea.containsPoint(NPoint{nCurX, nCurY})) {
//std::cout << "DestroyerEvent::addCircle  outside of area" << '\n';
				continue;
			}
			const int32_t nDiffX0 = nCurX - (nX - nRadius);
			const int32_t nDiffX1 = (nX + nRadius) - nCurX;
			const int32_t nDiffY0 = nCurY - (nY - nRadius);
			const int32_t nDiffY1 = (nY + nRadius) - nCurY;
			if (nDiffX0 < 3) {
				const int32_t nExclude = aNotExplo[nDiffX0];
				if (nDiffY0 < nExclude) {
					continue;
				} else if (nDiffY1 < nExclude) {
					continue;
				}
			} else if (nDiffX1 < 3) {
				const int32_t nExclude = aNotExplo[nDiffX1];
				if (nDiffY0 < nExclude) {
					continue;
				} else if (nDiffY1 < nExclude) {
					continue;
				}
			}
//std::cout << "DestroyerEvent::addCircle  nCurX=" << nCurX << "  nCurY=" << nCurY << '\n';
			oCoords.add(nCurX, nCurY);
		}
	}
}

} // namespace stmg
