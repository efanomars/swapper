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
 * File:   gravityevent.cc
 */

#include "gravityevent.h"

#include <stmm-games/named.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/coords.h>
#include <stmm-games/utile/tilerect.h>

#include <cassert>
#include <iostream>
#include <string>


namespace stmg
{

const int32_t GravityEvent::s_nMagicBlockFinishedMsg = 67643702;

static std::string s_sTileAniRemoving = "TILEANI:REMOVING";
static std::string s_sTileAniMaterializing = "TILEANI:MATERIALIZING";

GravityEvent::GravityEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oData(std::move(oInit))
{
	commonInit();
}

void GravityEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
void GravityEvent::commonInit() noexcept
{
	assert((m_oData.m_nRepeat > 0) || (m_oData.m_nRepeat == -1));
	assert(m_oData.m_nStep > 0);

	Level& oLevel = level();
	m_nTileAniRemovingIndex = oLevel.getNamed().tileAnis().getIndex(s_sTileAniRemoving);
	if (m_nTileAniRemovingIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! GravityEvent: tile animation '" << s_sTileAniRemoving << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniRemovingIndex = oLevel.getNamed().tileAnis().addName(s_sTileAniRemoving);
	}
	m_nTileAniMaterializingIndex = oLevel.getNamed().tileAnis().getIndex(s_sTileAniMaterializing);
	if (m_nTileAniMaterializingIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! GravityEvent: tile animation '" << s_sTileAniMaterializing << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniMaterializingIndex = oLevel.getNamed().tileAnis().addName(s_sTileAniMaterializing);
	}

	m_nBoardW = oLevel.boardWidth();
	m_nBoardH = oLevel.boardHeight();
	//
	runtimeInit();
}
void GravityEvent::runtimeInit() noexcept
{
	// run-time
	m_eState = GRAVITY_STATE_ACTIVATE;
	m_nCounter = 0;
	m_aColDirty.clear();
	m_aColDirty.resize(m_nBoardW, true);
	m_aFallingCols.clear();
}

void GravityEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
//std::cout << "GravityEvent::trigger m_oData.m_nStep=" << m_oData.m_nStep << '\n';
	//TODO
	// ACTIVATE activate event
	// INIT init and register
	// RUN  create gravityblocks if there are gaps underneath
	// last RUN disactivate, unregister, state to INIT
	//          send messages to finished-group listeners
	Level& oLevel = level();
	const int32_t nTimer = oLevel.game().gameElapsed();
	switch (m_eState)
	{
		case GRAVITY_STATE_ACTIVATE:
		{
			m_eState = GRAVITY_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nTimer);
				break;
			}
		} //fallthrough
		case GRAVITY_STATE_INIT:
		{
			m_eState = GRAVITY_STATE_RUN;
			m_nCounter = 0;
			for (int32_t nX = 0; nX < m_nBoardW; ++nX) {
				m_aColDirty[nX] = true;
			}
			oLevel.boardAddListener(this);
			m_nNextCheckGameTick = nTimer;
		} //fallthrough
		case GRAVITY_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				if ((p0TriggeringEvent == this) && (nMsg == s_nMagicBlockFinishedMsg)) {
					freeGravityBlock(nValue);
				}
				oLevel.activateEvent(this, m_nNextCheckGameTick);
				break; //switch
			}
			const bool bTerminate = (m_oData.m_nRepeat != -1) && (m_nCounter >= m_oData.m_nRepeat);
			if (bTerminate && m_aFallingCols.empty()) {
				m_eState = GRAVITY_STATE_INIT;
				oLevel.boardRemoveListener(this);
				//
				runtimeInit();
				//
				informListeners(LISTENER_GROUP_FINISHED, 0);
			} else {
				++m_nCounter;
				if (!bTerminate) {
					checkForHoles();
				}
				m_nNextCheckGameTick = nTimer + m_oData.m_nStep;
				oLevel.activateEvent(this, m_nNextCheckGameTick);
			}
		}
		break;
	}
}
void GravityEvent::checkForHoles() noexcept
{
//std::cout << "GravityEvent::checkForHoles" << '\n';
	for (int32_t nX = 0; nX < m_nBoardW; ++nX) {
		if (m_aColDirty[nX]) {
			// might set dirty again!
			const bool bStillDirty = checkForHolesCol(nX);
			if (!bStillDirty) {
				m_aColDirty[nX] = bStillDirty;
			}
		}
	}
}
bool GravityEvent::checkForHolesCol(int32_t nX) noexcept
{
//std::cout << "GravityEvent::checkForHolesCol(" << nX << ")" << '\n';
	Level& oLevel = level();
	bool bStillDirty = false;
	// find first empty
	int32_t nCurY = m_nBoardH - 1;
	while ((nCurY > 0) && !oLevel.boardGetTile(nX, nCurY).isEmpty()) {
		--nCurY;
	}
	// loop while not reached top
	while (nCurY > 0) {
		// nCurY is empty
		// find next not empty
		do {
			--nCurY;
		} while ((nCurY >= 0) && oLevel.boardGetTile(nX, nCurY).isEmpty());
		if (nCurY < 0) {
			// no tiles to unfreeze
			break; //while
		}
		int32_t nFirstNotEmpty = nCurY;
		// find next empty
		do {
			--nCurY;
		} while ((nCurY >= 0) && (!oLevel.boardGetTile(nX, nCurY).isEmpty()));

		int32_t nFloatingY = nCurY + 1;
		int32_t nFloatingH = nFirstNotEmpty - nCurY;
		bool bNotBusy = true;
		for (int32_t nCheckY = nFloatingY; nCheckY < nFloatingY + nFloatingH; ++nCheckY) {
			if ((oLevel.boardGetTileAnimator(nX, nCheckY, m_nTileAniRemovingIndex) != nullptr)
					|| (oLevel.boardGetTileAnimator(nX, nCheckY, m_nTileAniMaterializingIndex) != nullptr)) {
				bNotBusy = false;
				bStillDirty = true;
			}
		}
		if (bNotBusy) {
			createGravityBlock(nX, nFloatingY, nFloatingH);
		}
	}
	return bStillDirty;
}
LevelBlock* GravityEvent::create() noexcept
{
	static int32_t s_nUniqueRuntimeId = 0;
	++s_nUniqueRuntimeId;

	m_aFallingCols.push_back({});
	Level* p0Level = &level();
	m_oGravityBlockRecycler.create(m_aFallingCols.back(), p0Level, this, s_nMagicBlockFinishedMsg, s_nUniqueRuntimeId);

	return m_aFallingCols.back().get();
}
void GravityEvent::createGravityBlock(int32_t nX, int32_t nY, int32_t nH) noexcept
{
//std::cout << "GravityEvent::createGravityBlock(" << nX << "," << nY << "," << nH << ")" << '\n';
	Coords oCoords;
	oCoords.addRect(nX, nY, 1, nH);
	level().boabloUnfreeze(oCoords, *this, LevelBlock::MGMT_TYPE_AUTO_STRICT_OWNER);
}
void GravityEvent::freeGravityBlock(int32_t nRuntimeId) noexcept
{
	const int32_t nIdx = findGravityBlock(nRuntimeId);
	assert(nIdx >= 0);
	std::swap(m_aFallingCols[nIdx], m_aFallingCols.back());
	m_aFallingCols.pop_back();
}
int32_t GravityEvent::findGravityBlock(int32_t nRuntimeId) noexcept
{
	int32_t nIdx = 0;
	for (const auto& refGravityBlock : m_aFallingCols) {
		if (refGravityBlock->getRuntimeId() == nRuntimeId) {
			return nIdx; //-----------------------------------------------------
		}
		++nIdx;
	}
	return -1;
}
void GravityEvent::boabloPreFreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void GravityEvent::boabloPostFreeze(const Coords& oCoords) noexcept
{
	boardPostDestroy(oCoords); //TODO make separate method
}
void GravityEvent::boabloPreUnfreeze(const Coords& oCoords) noexcept
{
	boardPostDestroy(oCoords); //TODO make separate method
}
void GravityEvent::boabloPostUnfreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void GravityEvent::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept
{
	NRect oRect;
	oRect.m_nW = m_nBoardW;
	oRect.m_nH = m_nBoardH;
	boardPreInsert(eDir, oRect, refTiles);
}
void GravityEvent::boardPostScroll(Direction::VALUE eDir) noexcept
{
	NRect oRect;
	oRect.m_nW = m_nBoardW;
	oRect.m_nH = m_nBoardH;
	boardPostInsert(eDir, oRect);
}
void GravityEvent::boardPreInsert(Direction::VALUE eDir, NRect /*oRect*/, const shared_ptr<TileRect>& refTiles) noexcept
{
	if (eDir == Direction::DOWN) {
		if (refTiles && !TileRect::isAllEmptyTiles(*refTiles)) {
			level().gameStatusTechnical(std::vector<std::string>{"GravityEvent::boardPreInsert()","DOWN only supports empty tiles insertion"});
			return;
		}
	}
}
void GravityEvent::boardPostInsert(Direction::VALUE eDir, NRect oRect) noexcept
{
	if (!((eDir == Direction::UP) || (eDir == Direction::DOWN))) {
		level().gameStatusTechnical(std::vector<std::string>{"GravityEvent::boardPostInsert()","Only DOWN and UP supported"});
		return;
	}
	if (oRect.m_nY != 0) {
		level().gameStatusTechnical(std::vector<std::string>{"GravityEvent::boardPostInsert()","Only nY=0 supported"});
		return;
	}
	if (eDir == Direction::DOWN) {
		boardPostDeleteDown(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW);
	} else{
		boardPostInsertUp(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW);
	}
}
void GravityEvent::boardPostDeleteDown(int32_t
										#ifndef NDEBUG
										nDelY
										#endif //NDEBUG
										, int32_t nDelX, int32_t nDelW) noexcept
{
//std::cout << "GravityEvent::cellsDeleteDown() nDelY=" << nDelY << '\n';
	assert((nDelX >= 0) && (nDelX < m_nBoardW));
	assert((nDelY >= 0) && (nDelY < m_nBoardH));
	assert((nDelW > 0) && (nDelX + nDelW <= m_nBoardW));

	for (int32_t nCurX = nDelX; nCurX < nDelX + nDelW; ++nCurX) {
		m_aColDirty[nCurX] = true;
	}
}
void GravityEvent::boardPostInsertUp(int32_t
									#ifndef NDEBUG
									nInsY
									#endif //NDEBUG
									, int32_t nInsX, int32_t nInsW) noexcept
{
	assert((nInsX >= 0) && (nInsX < m_nBoardW));
	assert((nInsY >= 0) && (nInsY < m_nBoardH));
	assert((nInsW > 0) && (nInsX + nInsW <= m_nBoardW));
	for (int32_t nCurX = nInsX; nCurX < nInsX + nInsW; ++nCurX) {
		m_aColDirty[nCurX] = true;
	}
}
void GravityEvent::boardPreDestroy(const Coords& /*oCoords*/) noexcept
{
}
void GravityEvent::boardPostDestroy(const Coords& oCoords) noexcept
{
	//TODO very inefficient
	boardPostModify(oCoords);
}
void GravityEvent::boardPreModify(const TileCoords& /*oTileCoords*/) noexcept
{
}
void GravityEvent::boardPostModify(const Coords& oCoords) noexcept
{
	const NRect oRect = oCoords.getMinMax();
	assert((oRect.m_nX >= 0) && (oRect.m_nX < m_nBoardW));
	assert((oRect.m_nY >= 0) && (oRect.m_nY < m_nBoardH));
	assert((oRect.m_nW > 0) && (oRect.m_nX + oRect.m_nW <= m_nBoardW));
	assert((oRect.m_nH > 0) && (oRect.m_nY + oRect.m_nH <= m_nBoardH));
	for (int32_t nCurX = oRect.m_nX; nCurX < oRect.m_nX + oRect.m_nW; ++nCurX) {
//std::cout << "boardPostModify m_aColDirty[" << nCurX << "] is dirty" << '\n';
		m_aColDirty[nCurX] = true;
	}
}

} // namespace stmg
