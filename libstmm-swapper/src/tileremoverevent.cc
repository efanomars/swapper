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
 * File:   tileremoverevent.cc
 */

#include "tileremoverevent.h"

#include <stmm-games/level.h>
#include <stmm-games/named.h>
#include <stmm-games/utile/tilerect.h>
#include <stmm-games/utile/tilecoords.h>
#include <stmm-games/util/coords.h>

#include <cassert>
//#include <iostream>

namespace stmg
{

static std::string s_sTileAniRemoving = "TILEANI:REMOVING";

TileRemoverEvent::TileRemoverEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oData(std::move(oInit))
{
	commonInit();
}

void TileRemoverEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
void TileRemoverEvent::commonInit() noexcept
{
	Level& oLevel = level();
	m_nBoardW = oLevel.boardWidth();
	m_nBoardH = oLevel.boardHeight();

	assert((m_oData.m_nRepeat > 0) || (m_oData.m_nRepeat == -1));
	assert(m_oData.m_nStep > 0);
	assert((m_oData.m_oArea.m_nX >= 0) && (m_oData.m_oArea.m_nW > 0) && (m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW <= m_nBoardW));
	assert((m_oData.m_oArea.m_nY >= 0) && (m_oData.m_oArea.m_nH > 0) && (m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH <= m_nBoardH));
	assert((m_oData.m_nDeleteAfter >= 0) && (m_oData.m_nDeleteAfter <= s_nMaxDeleteAfter));

	m_nTileAniRemovingIndex = oLevel.getNamed().tileAnis().getIndex(s_sTileAniRemoving);
	if (m_nTileAniRemovingIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! TileRemoverEvent: tile animation '" << s_sTileAniRemoving << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniRemovingIndex = oLevel.getNamed().tileAnis().addName(s_sTileAniRemoving);
	}

	initRuntime();
}
void TileRemoverEvent::initRuntime() noexcept
{
	m_eState = TILE_REMOVER_STATE_ACTIVATE;
	m_nCounter = 0;
	const int32_t nMinSize = m_nBoardW * m_nBoardH;
	if (!m_refDirty) {
		m_oCoordsRecycler.create(m_refDirty, nMinSize);
		m_oCoordsRecycler.create(m_refWorker, nMinSize);
		m_oCoordsRecycler.create(m_refToRemove, nMinSize);
	} else {
		m_refDirty->reInit(nMinSize);
		m_refWorker->reInit(nMinSize);
		m_refToRemove->reInit(nMinSize);
	}
	m_oStages.clear();
	m_oDownCounters.clear();
}
void TileRemoverEvent::trigger(int32_t /*nMsg*/, int32_t /*nValue*/, Event* p0TriggeringEvent) noexcept
{
//std::cout << "TileRemoverEvent::trigger" << '\n';
	//TODO
	// ACTIVATE activate event
	// INIT init and register
	// RUN
	// last RUN deactivate, unregister, state to INIT
	//          send messages to finished-group listeners
	Level& oLevel = level();
	const int32_t nTimer = oLevel.game().gameElapsed();
	switch (m_eState)
	{
		case TILE_REMOVER_STATE_ACTIVATE:
		{
			m_eState = TILE_REMOVER_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nTimer);
				break;
			}
		} //fallthrough
		case TILE_REMOVER_STATE_INIT:
		{
			m_eState = TILE_REMOVER_STATE_RUN;
			m_nCounter = 0;
			m_refDirty->reInit();
			for (int32_t nX = m_oData.m_oArea.m_nX; nX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW; ++nX) {
				for (int32_t nY = m_oData.m_oArea.m_nY; nY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH; ++nY) {
					if ( (!oLevel.boardGetTile(nX, nY).isEmpty())
							&& (oLevel.boardGetTileAnimator(nX, nY, m_nTileAniRemovingIndex) == nullptr)
							) {
						m_refDirty->add(nX, nY);
					}
				}
			}
			oLevel.boardAddListener(this);
		} //fallthrough
		case TILE_REMOVER_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				break; //switch
			}
			const bool bTerminate = (m_oData.m_nRepeat != -1) && (m_nCounter >= m_oData.m_nRepeat);
			if (bTerminate && m_oStages.empty()) {
				m_eState = TILE_REMOVER_STATE_INIT; //TODO TILE_REMOVER_STATE_DEAD
				oLevel.boardRemoveListener(this);
				initRuntime();
				informListeners(LISTENER_GROUP_FINISHED, 0);
			} else {
				++m_nCounter;
				if (!bTerminate) {
					checkForTilesToRemove();
				}
				remove();
				oLevel.activateEvent(this, nTimer + m_oData.m_nStep);
			}
		}
		break;
	}
}
void TileRemoverEvent::checkForTilesToRemove() noexcept
{
//std::cout << "TileRemoverEvent::checkForTilesToRemove" << '\n';
	Level& oLevel = level();
	assert(m_refWorker.use_count() == 2);
	m_refWorker->reInit();
	m_refWorker.swap(m_refDirty);
	while (m_refWorker->size() > 0) {
		const Coords::const_iterator itW = m_refWorker->begin();
		const int32_t nX = itW.x();
		const int32_t nY = itW.y();
//std::cout << "TileRemoverEvent::checkForTilesToRemove nX=" << nX << " nY=" << nY << '\n';
		m_refToRemove->reInit();
		assert(m_refToRemove.use_count() == 2);
		const bool bRemove = getCoordsToRemove(NPoint{nX, nY}, m_oData.m_oArea, *m_refToRemove);
		if (bRemove) {
			const int32_t nTotToRemove = m_refToRemove->size();
			m_refWorker->remove(*m_refToRemove);
			m_refWorker->remove(nX,nY);

			shared_ptr<DownCounter> refDownCounter = recyclePopDownCounter();
			refDownCounter->m_bLocked = false;
			refDownCounter->m_nTotSteps = m_oData.m_nDeleteAfter;
			refDownCounter->m_nCountdown = m_oData.m_nDeleteAfter;
			refDownCounter->m_bInitAni = true;
			shared_ptr<Coords>& refCoords = refDownCounter->m_refCoords;
			m_oCoordsRecycler.create(refCoords, m_nBoardW * m_nBoardH);
			refCoords.swap(m_refToRemove);
			assert(m_refToRemove->size() == 0);
			m_oStages.push_back(refDownCounter);

			for (Coords::const_iterator it = refCoords->begin(); it != refCoords->end(); it.next()) {
				const int32_t nCrackX = it.x();
				const int32_t nCrackY = it.y();
				//mark as active for listeners
				oLevel.boardSetTileAnimator(nCrackX, nCrackY, m_nTileAniRemovingIndex, this, refDownCounter->m_nAllIdx);
			}
			informListeners(LISTENER_GROUP_REMOVED, nTotToRemove);
		} else {
			m_refWorker->remove(nX,nY);
		}
	}
}
int32_t TileRemoverEvent::wouldRemoveTiles(const ExtendedBoard& oBoard, const NRect& oExtArea) noexcept
{
	assert((oExtArea.m_nW > 0) && (oExtArea.m_nH > 0));
	const NRect oArea = NRect::boundingRect(m_oData.m_oArea, oExtArea);
//std::cout << "TileRemoverEvent::wouldRemoveTiles oArea.m_nY=" << oArea.m_nY << " oArea.m_nH=" << oArea.m_nH << '\n';

	assert(m_refWorker.use_count() == 2);
	m_refWorker->reInit();
	m_refWorker->addRect(oExtArea);
	int32_t nTotToRemove = 0;
	while (m_refWorker->size() > 0) {
		const Coords::const_iterator itW = m_refWorker->begin();
		const int32_t nX = itW.x();
		const int32_t nY = itW.y();
//std::cout << "TileRemoverEvent::wouldRemoveTiles nX=" << nX << " nY=" << nY << '\n';
		m_refToRemove->reInit();
		assert(m_refToRemove.use_count() == 2);
		const bool bRemove = getCoordsToRemove(NPoint{nX, nY}, oArea, oBoard, *m_refToRemove);
		if (bRemove) {
			nTotToRemove += m_refToRemove->size();
			m_refWorker->remove(*m_refToRemove);
		}
		m_refWorker->remove(nX,nY);
//std::cout << "TileRemoverEvent::wouldRemoveTiles bRemove=" << bRemove << " m_refToRemove->size()=" << m_refToRemove->size() << '\n';
	}
//std::cout << "TileRemoverEvent::wouldRemoveTiles nTotToRemove=" << nTotToRemove << '\n';
	return nTotToRemove;
}

void TileRemoverEvent::remove() noexcept
{
//std::cout << "TileRemoverEvent::remove()" << '\n';
	Level& oLevel = level();
	DownCounterList::iterator itCounters = m_oStages.begin();
	while (itCounters != m_oStages.end()) {
		DownCounter& oDownCounter = *(*itCounters);
		oDownCounter.m_bLocked = true;
		const int32_t nDelCountdown = oDownCounter.m_nCountdown;
//std::cout << "TileRemoverEvent::remove() nDelCountdown=" << nDelCountdown << '\n';
		oDownCounter.m_nCountdown--;
		const bool bDelete = (nDelCountdown == 0);
		const bool bInitAni = oDownCounter.m_bInitAni;
		oDownCounter.m_bInitAni = false;
		shared_ptr<Coords>& refCoords = oDownCounter.m_refCoords;
		assert(refCoords.use_count() == 2);
		const Coords& oCoords = *(refCoords);
		for (Coords::const_iterator it = oCoords.begin(); it != oCoords.end(); it.next()) {
			const int32_t nCrackX = it.x();
			const int32_t nCrackY = it.y();
			if (bDelete) {
				oLevel.boardSetTileAnimator(nCrackX, nCrackY, m_nTileAniRemovingIndex, nullptr, 0);
			} else if (bInitAni) {
			} else {
			}
			if (!bDelete) {
				oLevel.boardAnimateTile(NPoint{nCrackX, nCrackY});
			}
		}
		if (bDelete) {
			oLevel.boardDestroy(*refCoords);
		}
//std::cout << " dec oDownCounter.m_nCountdown = " << oDownCounter.m_nCountdown << '\n';
		oDownCounter.m_bLocked = false;
		if (bDelete) {
			recyclePushBackDownCounter(*itCounters);
			itCounters = m_oStages.erase(itCounters);
		} else {
			 ++itCounters;
		}
	}
}
void TileRemoverEvent::recyclePushBackDownCounter(shared_ptr<DownCounter>& refDownCounter) noexcept
{
//std::cout << "TileRemoverEvent::recyclePushBackDownCounter()"  << '\n';
	assert(refDownCounter);
	refDownCounter->m_refCoords.reset();
	m_oInactiveDownCounters.push_front(refDownCounter);
}
shared_ptr<TileRemoverEvent::DownCounter> TileRemoverEvent::recyclePopDownCounter() noexcept
{
//std::cout << "TileRemoverEvent::recyclePopDownCounter()"  << '\n';
	if (!m_oInactiveDownCounters.empty()) {
		shared_ptr<DownCounter> refTemp = *(m_oInactiveDownCounters.begin());
		m_oInactiveDownCounters.erase(m_oInactiveDownCounters.begin());
		return refTemp;
	}
	shared_ptr<DownCounter> refTemp = shared_ptr<DownCounter>(new DownCounter());
	const int32_t nAllIdx = static_cast<int32_t>(m_aAllDownCounters.size());
	refTemp->m_nAllIdx = nAllIdx;
	m_aAllDownCounters.push_back(refTemp);
	return refTemp;
}

double TileRemoverEvent::getElapsed01(int32_t nHash, int32_t /*nX*/, int32_t /*nY*/, int32_t nAni
									, int32_t nViewTick, int32_t nTotViewTicks) const noexcept
{
	assert(nTotViewTicks > 0);
	assert((nViewTick >= 0) && (nViewTick < nTotViewTicks));
	if (nAni != m_nTileAniRemovingIndex) {
		assert(false);
		return 1.0;
	}

	if ((nHash < 0) || (nHash >= static_cast<int32_t>(m_aAllDownCounters.size()))) {
		assert(false);
		return 1.0;
	}
	const DownCounter& oDownCounter = *(m_aAllDownCounters[nHash]);
	assert(oDownCounter.m_nAllIdx == nHash);

	const int32_t nTotSteps = oDownCounter.m_nTotSteps;
	const int32_t nCountdown = oDownCounter.m_nCountdown + 1;
//std::cout << "TileRemoverEvent::getElapsed01() nTotSteps=" << nTotSteps << " nCountdown=" << nCountdown << '\n';
	assert(nTotSteps > 0);
	assert((nCountdown > 0) && (nCountdown <= nTotSteps));
	//TODO make an example to explain this scary formula
	const double fElapsed = 1.0 + (- nCountdown + (nViewTick + 0.5) / nTotViewTicks) / nTotSteps;
	assert((fElapsed >= 0.0) && (fElapsed <= 1.0));
	return fElapsed;
}
double TileRemoverEvent::getElapsed01(int32_t /*nHash*/, const LevelBlock& /*oLevelBlock*/, int32_t /*nBrickIdx*/, int32_t /*nAni*/
									, int32_t /*nViewTick*/, int32_t /*nTotViewTicks*/) const noexcept
{
	assert(false);
	return 1.0;
}

void TileRemoverEvent::boabloPreFreeze(LevelBlock& oLevelBlock) noexcept
{
	const Coords oCoords = LevelBlock::getCoords(oLevelBlock);
	m_refDirty->remove(oCoords);
}
void TileRemoverEvent::boabloPostFreeze(const Coords& oCoords) noexcept
{
	boardPostModify(oCoords);
}
void TileRemoverEvent::boabloPreUnfreeze(const Coords& oCoords) noexcept
{
	m_refDirty->remove(oCoords);
}
void TileRemoverEvent::boabloPostUnfreeze(LevelBlock& oLevelBlock) noexcept
{
	const Coords oCoords = LevelBlock::getCoords(oLevelBlock);
	boardPostModify(oCoords);
}
void TileRemoverEvent::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept
{
	NRect oRect;
	oRect.m_nW = m_nBoardW;
	oRect.m_nH = m_nBoardH;
	boardPreInsert(eDir, oRect, refTiles);
}
void TileRemoverEvent::boardPostScroll(Direction::VALUE eDir) noexcept
{
	NRect oRect;
	oRect.m_nW = m_nBoardW;
	oRect.m_nH = m_nBoardH;
	boardPostInsert(eDir, oRect);
}
void TileRemoverEvent::boardPreInsert(Direction::VALUE eDir, NRect /*oRect*/, const shared_ptr<TileRect>& refTiles) noexcept
{
	if (eDir == Direction::DOWN) {
		if (refTiles && !TileRect::isAllEmptyTiles(*refTiles)) {
			level().gameStatusTechnical(std::vector<std::string>{"TileRemoverEvent::boardPreInsert()","DOWN only supports empty tiles insertion"});
			return;
		}
	}
}
void TileRemoverEvent::boardPostInsert(Direction::VALUE eDir, NRect oRect) noexcept
{
	if (!((eDir == Direction::UP) || (eDir == Direction::DOWN))) {
		level().gameStatusTechnical(std::vector<std::string>{"TileRemoverEvent::boardPostInsert()","Only DOWN and UP supported"});
		return;
	}
	if (oRect.m_nY != 0) {
		level().gameStatusTechnical(std::vector<std::string>{"TileRemoverEvent::boardPostInsert()","Only nY=0 supported"});
		return;
	}
	if (eDir == Direction::DOWN) {
		boardPostDeleteDown(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW);
	} else {
		boardPostInsertUp(oRect.m_nY + oRect.m_nH - 1, oRect.m_nX, oRect.m_nW);
	}
}
void TileRemoverEvent::boardPostDeleteDown(int32_t nDelY, int32_t nDelX, int32_t nDelW) noexcept
{
//std::cout << "TileRemoverEvent::cellsDeleteDown() nDelY=" << nDelY << '\n';
	assert((nDelX >= 0) && (nDelX < m_nBoardW));
	assert((nDelY >= 0) && (nDelY < m_nBoardH));
	assert((nDelW > 0) && (nDelX + nDelW <= m_nBoardW));

	const int32_t nDelFarX = nDelX + nDelW - 1;
	if ( (nDelFarX < m_oData.m_oArea.m_nX) || (nDelX >= m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW) || (m_oData.m_oArea.m_nY > nDelY) ) {
		// no interaction (intersection)
		return; //--------------------------------------------------------------
	}

	Level& oLevel = level();
	assert(m_refWorker.use_count() == 2);
	m_refWorker->reInit();
	// Move dirties (x,y) down
	for (Coords::const_iterator it = m_refDirty->begin(); it != m_refDirty->end(); it.next()) {
		const int32_t nCurX = it.x();
		int32_t nCurY = it.y();
		if ((nCurX >= nDelX) && (nCurX <= nDelFarX) && (nCurY < nDelY))  {
			++nCurY;
			assert(nCurY >= m_oData.m_oArea.m_nY);
			if (nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH) {
				m_refWorker->add(nCurX, nCurY);
			}
		} else {
			m_refWorker->add(nCurX, nCurY);
		}
	}
	// Dirty top line
	const int32_t nTopY = m_oData.m_oArea.m_nY;
	int32_t nTopX;
	if (nDelX < m_oData.m_oArea.m_nX) {
		nTopX = m_oData.m_oArea.m_nX;
	} else {
		nTopX = nDelX;
	}
	int32_t nTopFarX;
	if (nDelFarX > m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW - 1) {
		nTopFarX = m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW - 1;
	} else {
		nTopFarX = nDelFarX;
	}
	for (int32_t nCurX = nTopX; nCurX <= nTopFarX; ++nCurX) {
		if (!oLevel.boardGetTile(nCurX, nTopY).isEmpty()) {
			assert((nCurX >= m_oData.m_oArea.m_nX) && (nCurX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW));
			m_refWorker->add(nCurX, nTopY);
		}
	}
	// Dirty left line
	if (nDelX > m_oData.m_oArea.m_nX) {
		assert((nDelX >= m_oData.m_oArea.m_nX) && (nDelX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW));
		for (int32_t nCurY = nTopY; nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH; ++nCurY) {
			if ( (!oLevel.boardGetTile(nDelX, nCurY).isEmpty())
					&& (oLevel.boardGetTileAnimator(nDelX, nCurY, m_nTileAniRemovingIndex) == nullptr)
					) {
				assert((nCurY >= m_oData.m_oArea.m_nY) && (nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH));
				m_refWorker->add(nDelX, nCurY);
			}
		}
	}
	// Dirty right line
	if (nDelFarX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW - 1) {
		assert((nDelFarX >= m_oData.m_oArea.m_nX) && (nDelFarX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW));
		for (int32_t nCurY = m_oData.m_oArea.m_nY; nCurY < nTopY; ++nCurY) {
			if ( (!oLevel.boardGetTile(nDelFarX, nCurY).isEmpty())
					&& (oLevel.boardGetTileAnimator(nDelFarX, nCurY, m_nTileAniRemovingIndex) == nullptr)
					) {
				assert((nCurY >= m_oData.m_oArea.m_nY) && (nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH));
				m_refWorker->add(nDelFarX, nCurY);
			}
		}
	}
	m_refDirty.swap(m_refWorker);

	for (DownCounterList::iterator itStage = m_oStages.begin()
			; itStage != m_oStages.end(); ++itStage) {
		DownCounter& oDownCounter = *(*itStage);
//std::cout << "TileRemoverEvent::cellsDeleteDown() oDownCounter.m_bLocked=" << oDownCounter.m_bLocked << '\n';
		if (!oDownCounter.m_bLocked) {
			m_refWorker->reInit();
			shared_ptr<Coords>& refCoords = oDownCounter.m_refCoords;
			for (Coords::const_iterator it = refCoords->begin(); it != refCoords->end(); it.next()) {
				int32_t nCrackX = it.x();
				int32_t nCrackY = it.y();
				if ((nCrackX >= nDelX) && (nCrackX <= nDelFarX)) {
					assert(nCrackY != nDelY);
					if (nCrackY < nDelY) {
						++nCrackY;
					}
				}
				m_refWorker->add(nCrackX, nCrackY);
			}
			refCoords.swap(m_refWorker);
		}
	}
}
void TileRemoverEvent::boardPostInsertUp(int32_t nInsY, int32_t nInsX, int32_t nInsW) noexcept
{
	assert((nInsX >= 0) && (nInsX < m_nBoardW));
	assert((nInsY >= 0) && (nInsY < m_nBoardH));
	assert((nInsW > 0) && (nInsX + nInsW <= m_nBoardW));

	const int32_t nInsFarX = nInsX + nInsW - 1;
	if ( (nInsFarX < m_oData.m_oArea.m_nX) || (nInsX >= m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW) || (m_oData.m_oArea.m_nY > nInsY) ) {
		// no interaction (intersection)
		return; //--------------------------------------------------------------
	}

	Level& oLevel = level();
	assert(m_refWorker.use_count() == 2);
	m_refWorker->reInit();
	// Move dirties (x,y) up
	for (Coords::const_iterator it = m_refDirty->begin(); it != m_refDirty->end(); it.next()) {
		const int32_t nCurX = it.x();
		int32_t nCurY = it.y();
		if ((nCurX >= nInsX) && (nCurX <= nInsFarX) && (nCurY <= nInsY))  {
			nCurY--;
			assert(nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH);
			if (nCurY >= m_oData.m_oArea.m_nY) {
				m_refWorker->add(nCurX, nCurY);
			}
		} else {
			m_refWorker->add(nCurX, nCurY);
		}
	}
	// Dirty bottom line
	int32_t nBottomY;
	if (nInsY > m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH - 1) {
		nBottomY = m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH - 1;
	} else {
		nBottomY = nInsY;
	}
	assert((nBottomY >= m_oData.m_oArea.m_nY) && (nBottomY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH));
	int32_t nBottomX;
	if (nInsX < m_oData.m_oArea.m_nX) {
		nBottomX = m_oData.m_oArea.m_nX;
	} else {
		nBottomX = nInsX;
	}
	int32_t nBottomFarX;
	if (nInsFarX > m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW - 1) {
		nBottomFarX = m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW - 1;
	} else {
		nBottomFarX = nInsFarX;
	}
	for (int32_t nCurX = nBottomX; nCurX <= nBottomFarX; ++nCurX) {
		if (!oLevel.boardGetTile(nCurX, nBottomY).isEmpty()) {
			assert((nCurX >= m_oData.m_oArea.m_nX) && (nCurX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW));
			m_refWorker->add(nCurX, nBottomY);
		}
	}
	// Dirty left line
	if (nInsX > m_oData.m_oArea.m_nX) {
		assert((nInsX >= m_oData.m_oArea.m_nX) && (nInsX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW));
		for (int32_t nCurY = m_oData.m_oArea.m_nY; nCurY < nBottomY; ++nCurY) {
			if ( (!oLevel.boardGetTile(nInsX, nCurY).isEmpty())
					&& (oLevel.boardGetTileAnimator(nInsX, nCurY, m_nTileAniRemovingIndex) == nullptr)
					) {
				assert((nCurY >= m_oData.m_oArea.m_nY) && (nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH));
				m_refWorker->add(nInsX, nCurY);
			}
		}
	}
	// Dirty right line
	if (nInsFarX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW - 1) {
		assert((nInsFarX >= m_oData.m_oArea.m_nX) && (nInsFarX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW));
		for (int32_t nCurY = m_oData.m_oArea.m_nY; nCurY < nBottomY; ++nCurY) {
			if ( (!oLevel.boardGetTile(nInsFarX, nCurY).isEmpty())
					&& (oLevel.boardGetTileAnimator(nInsFarX, nCurY, m_nTileAniRemovingIndex) == nullptr)
					) {
				assert((nCurY >= m_oData.m_oArea.m_nY) && (nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH));
				m_refWorker->add(nInsFarX, nCurY);
			}
		}
	}
	m_refDirty.swap(m_refWorker);

	for (DownCounterList::iterator itStage = m_oStages.begin()
			; itStage != m_oStages.end(); ++itStage) {
		DownCounter& oDownCounter = *(*itStage);
//std::cout << "TileRemoverEvent::cellsInsertUp() oDownCounter.m_bLocked=" << oDownCounter.m_bLocked << '\n';
		if (!oDownCounter.m_bLocked) {
			m_refWorker->reInit();
			shared_ptr<Coords>& refCoords = oDownCounter.m_refCoords;
			assert(refCoords.use_count() == 2);
			for (Coords::const_iterator it = refCoords->begin(); it != refCoords->end(); it.next()) {
				int32_t nCrackX = it.x();
				int32_t nCrackY = it.y();
				if ((nCrackX >= nInsX) && (nCrackX <= nInsFarX)) {
					assert(nCrackY != 0);
					if (nCrackY <= nInsY) {
						--nCrackY;
					}
				}
				m_refWorker->add(nCrackX, nCrackY);
			}
			refCoords.swap(m_refWorker);
		}
	}
}
void TileRemoverEvent::boardPreDestroy(const Coords& oCoords) noexcept
{
	m_refDirty->remove(oCoords);
}
void TileRemoverEvent::boardPostDestroy(const Coords& /*oCoords*/) noexcept
{
}
void TileRemoverEvent::boardPreModify(const TileCoords& oTileCoords) noexcept
{
	m_refDirty->remove(oTileCoords);
}
void TileRemoverEvent::boardPostModify(const Coords& oCoords) noexcept
{
	Level& oLevel = level();
	for (Coords::const_iterator it = oCoords.begin(); it != oCoords.end(); it.next()) {
		const int32_t nCurX = it.x();
		const int32_t nCurY = it.y();
		if ((nCurX >= m_oData.m_oArea.m_nX) && (nCurX < m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW)) {
			if ((nCurY >= m_oData.m_oArea.m_nY) && (nCurY < m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH)) {
				if ( (!oLevel.boardGetTile(nCurX, nCurY).isEmpty())
						&& (oLevel.boardGetTileAnimator(nCurX, nCurY, m_nTileAniRemovingIndex) == nullptr)
						) {
					m_refDirty->add(nCurX, nCurY);
				}
			}
		}
	}
}

} // namespace stmg
