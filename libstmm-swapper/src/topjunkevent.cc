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
 * File:   topjunkevent.cc
 */

#include "topjunkevent.h"

#include <stmm-games/level.h>
#include <stmm-games/named.h>
#include <stmm-games/gameproxy.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/tile.h>
#include <stmm-games/util/helpers.h>
#include <stmm-games/util/namedindex.h>

#include <limits>
#include <string>
#include <cassert>
//#include <iostream>

namespace stmg
{

static std::string s_sTileAniRemoving = "TILEANI:REMOVING";
static std::string s_sTileAniMaterializing = "TILEANI:MATERIALIZING";

constexpr static const int32_t s_nNextDropInfinity = std::numeric_limits<int32_t>::max() - 10;

TopJunkEvent::TopJunkEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
{
	commonInit(std::move(oInit));
}

void TopJunkEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	commonInit(std::move(oInit));
}
void TopJunkEvent::commonInit(LocalInit&& oInit) noexcept
{
	Level& oLevel = level();
	m_nMaterializingTicksFrom = oInit.m_nMaterializingTicksFrom;
	m_nMaterializingTicksTo = oInit.m_nMaterializingTicksTo;
	m_nMaterializingMillisecFrom = oInit.m_nMaterializingMillisecFrom;
	m_nMaterializingMillisecTo = oInit.m_nMaterializingMillisecTo;
	m_nWaitTicksFrom = oInit.m_nWaitTicksFrom;
	m_nWaitTicksTo = oInit.m_nWaitTicksTo;
	m_nWaitMillisecFrom = oInit.m_nWaitMillisecFrom;
	m_nWaitMillisecTo = oInit.m_nWaitMillisecTo;
	m_nMaxJunkPerDrop = oInit.m_nMaxJunkPerDrop;
	m_nTopY = oInit.m_nTopY;

	m_nBoardW = oLevel.boardWidth();
	m_nBoardH = oLevel.boardHeight();
	m_oMaterializingCounters.resize(m_nBoardW);
	// checks
	assert(! oInit.m_aProbTileGens.empty());
	assert(m_nMaterializingTicksFrom >= 0);
	assert(m_nMaterializingTicksTo >= m_nMaterializingTicksFrom);
	assert(m_nMaterializingMillisecFrom >= 0);
	assert(m_nMaterializingMillisecTo >= m_nMaterializingMillisecFrom);
	assert(m_nWaitTicksFrom >= 0);
	assert(m_nWaitTicksTo >= m_nWaitTicksFrom);
	assert(m_nWaitMillisecFrom >= 0);
	assert(m_nWaitMillisecTo >= m_nWaitMillisecFrom);
	assert(m_nMaxJunkPerDrop >= 1);
	assert((m_nTopY >= 0) && (m_nTopY < m_nBoardH));
	//
	m_eState = TOP_JUNK_STATE_ACTIVATE;
	//
	NamedIndex& oTileAnisIndex = oLevel.getNamed().tileAnis();
	m_nTileAniRemovingIndex = oTileAnisIndex.getIndex(s_sTileAniRemoving);
	if (m_nTileAniRemovingIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! TopJunkEvent: tile animation '" << s_sTileAniRemoving << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniRemovingIndex = oTileAnisIndex.addName(s_sTileAniRemoving);
	}
	m_nTileAniMaterializeIndex = oTileAnisIndex.getIndex(s_sTileAniMaterializing);
	if (m_nTileAniMaterializeIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! TopJunkEvent: tile animation '" << s_sTileAniMaterializing << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniMaterializeIndex = oTileAnisIndex.addName(s_sTileAniMaterializing);
	}

	const auto nTotProbTileGens = static_cast<int32_t>(oInit.m_aProbTileGens.size());
	assert(nTotProbTileGens > 0);
	m_aRandomTiles.resize(nTotProbTileGens);
	for (int32_t nGenIdx = 0; nGenIdx < nTotProbTileGens; ++nGenIdx) {
		assert(oInit.m_aProbTileGens[nGenIdx].m_aProbTraitSets.size() > 0);
		m_aRandomTiles[nGenIdx] = std::make_unique<RandomTiles>(oLevel.game().getRandomSource()
																, std::move(oInit.m_aProbTileGens[nGenIdx]));
	}
	resetRuntime();
}
void TopJunkEvent::resetRuntime() noexcept
{
	m_nCurRandomGen = 0;
	m_nJunkTilesToDrop = 0;
	m_nMaterializingAnis = 0;
	m_nNextDropTick = -1;
	m_nLeftOver = 0;
	m_nLastRunTick = -1;
	m_bPaused = false;
	m_oTileCoords.reInit();
}
int32_t TopJunkEvent::calcNextDrop(GameProxy& oGame, int32_t nCurGameTick, int32_t nCurInterval) noexcept
{
	const int32_t nWaitMillisec = oGame.random(m_nWaitMillisecFrom, m_nWaitMillisecTo);
	const int32_t nWaitTicks = oGame.random(m_nWaitTicksFrom, m_nWaitTicksTo) + nWaitMillisec / nCurInterval;
	// next drop is always at least one tick away
	return nCurGameTick + ((nWaitTicks <= 0) ? 1 : nWaitTicks);
}
void TopJunkEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
	//TODO
	// ACTIVATE activate event
	// INIT init
	// RUN  scroll up if possible
	// last RUN deactivate, state to INIT
	//          send messages to finished-group listeners
	Level& oLevel = level();
	auto& oGame = oLevel.game();
	const int32_t nCurInterval = oGame.gameInterval();

	const int32_t nTimer = oGame.gameElapsed();
//std::cout << "TopJunkEvent::trigger nMsg=" << nMsg << "  nValue=" << nValue << "  nTimer=" << nTimer << '\n';
	//TODO every time a push row is executed also a scroll is (which is not good)
	switch (m_eState)
	{
		case TOP_JUNK_STATE_ACTIVATE:
		{
			m_eState = TOP_JUNK_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nTimer);
				return; //------------------------------------------------------
			}
		} //fallthrough
		case TOP_JUNK_STATE_INIT:
		{
			oLevel.boardAddListener(this);
		} //fallthrough
		case TOP_JUNK_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				switch (nMsg)
				{
					case MESSAGE_DROP_JUNK:
					{
						m_nJunkTilesToDrop += std::max(1, nValue);
					}
					break;
					case MESSAGE_SET_TILE_GEN:
					{
						m_nCurRandomGen = std::min(std::max(0, nValue), static_cast<int32_t>(m_aRandomTiles.size()) - 1);
					}
					break;
					case MESSAGE_NEXT_TILE_GEN:
					{
						if (m_nCurRandomGen < static_cast<int32_t>(m_aRandomTiles.size()) - 1) {
							++m_nCurRandomGen;
						}
					}
					break;
					case MESSAGE_PREV_TILE_GEN:
					{
						if (m_nCurRandomGen > 0) {
							--m_nCurRandomGen;
						}
					}
					break;
					case MESSAGE_PAUSE:
					{
						m_bPaused = true;
					}
					break;
					case MESSAGE_RESUME:
					{
						m_bPaused = false;
					}
					break;
				}
				//
				const int32_t nNextDropTick = (m_bPaused ? s_nNextDropInfinity : m_nNextDropTick);
				if ((m_nMaterializingAnis > 0) || (nNextDropTick <= nTimer) || (m_nLeftOver > 0)) {
					if ((m_nJunkTilesToDrop > m_nLeftOver) && (m_nNextDropTick < 0)) {
						// there are tiles on top of the left overs
						// next drop planned for immediately (theoretically since might be paused)
						// after the left overs (if any) are handled
						m_nNextDropTick = nTimer;
//std::cout << "TopJunkEvent::trigger 1 -- m_nNextDropTick=" << m_nNextDropTick << '\n';
					}
					const bool bReactivateNow = (m_nLastRunTick < nTimer);
					activateAndInform(oLevel, -1, nTimer + (bReactivateNow ? 0 : 1));
					//
				} else if (m_nJunkTilesToDrop > 0) {
					if (m_nNextDropTick < 0) {
						// paused,  but next drop planned for now
						m_nNextDropTick = nTimer;
					}
					activateAndInform(oLevel, -1, nNextDropTick);
				}
				return; //------------------------------------------------------
			}
			m_nLastRunTick = nTimer;
//std::cout << "TopJunkEvent::trigger m_nLastRunTick=" << m_nLastRunTick << '\n';
			if (m_nMaterializingAnis > 0) {
				// continue or finish animations
				continueOrFinishTileAnis(oLevel);
			}
			bool bStillMaterializing = (m_nMaterializingAnis > 0);
			if (m_nJunkTilesToDrop == 0) {
				assert(m_nLeftOver == 0);
				assert(m_nNextDropTick < 0);
				if (bStillMaterializing) {
					activateAndInform(oLevel, -1, nTimer + 1);
				} else {
					// don't set the timer, MESSAGE_DROP_JUNK (or MESSAGE_RESUME if paused) will wake us up
				}
				return; //------------------------------------------------------
			}
			const bool bLeftOver = (m_nLeftOver > 0);
			if (! bLeftOver) {
				if (m_bPaused) {
					if (bStillMaterializing) {
						activateAndInform(oLevel, -1, nTimer + 1);
					}
					return; //--------------------------------------------------
					//
				} else if (nTimer < m_nNextDropTick) {
					// Wait a little longer for next main drop
					const int32_t nActivateTick = (bStillMaterializing ? nTimer + 1 : m_nNextDropTick);
//std::cout << "TopJunkEvent::trigger C -- m_nNextDropTick=" << m_nNextDropTick << " bStillMaterializing=" << bStillMaterializing << '\n';
					activateAndInform(oLevel, -1, nActivateTick);
					return; //--------------------------------------------------
				}
			}
			// handle left overs if necessary
			const int32_t nTilesToAdd = (bLeftOver ? m_nLeftOver : std::min(m_nMaxJunkPerDrop, m_nJunkTilesToDrop));
//std::cout << "TopJunkEvent::trigger nTimer=" << nTimer << " m_nLeftOver=" << m_nLeftOver << "  nTilesToAdd=" << nTilesToAdd << '\n';
			const int32_t nAddedTiles = fillEmptyTilesOfTopLine(oLevel, nCurInterval, nTilesToAdd);
			m_nJunkTilesToDrop -= nAddedTiles;
			m_nLeftOver = nTilesToAdd - nAddedTiles;
			bStillMaterializing = (m_nMaterializingAnis > 0);
//std::cout << "TopJunkEvent::trigger  m_nJunkTilesToDrop=" << m_nJunkTilesToDrop << " m_nLeftOver=" << m_nLeftOver << "  nAddedTiles=" << nAddedTiles << '\n';
//std::cout << "TopJunkEvent::trigger  m_bPaused=" << m_bPaused << '\n';
			if (m_nLeftOver > 0) {
				if (m_nNextDropTick >= 0) {
					// procrastinate next drop
					++m_nNextDropTick;
				} else {
					//
					if (m_nJunkTilesToDrop > m_nLeftOver) {
						m_nNextDropTick = calcNextDrop(oGame, nTimer, nCurInterval);
//std::cout << "TopJunkEvent::trigger 3 -- m_nNextDropTick=" << m_nNextDropTick << " bStillMaterializing=" << bStillMaterializing << '\n';
					}
				}
				// left overs are handled next tick
				activateAndInform(oLevel, nAddedTiles, nTimer + 1);
				return; //------------------------------------------------------
			}
			if (bLeftOver) {
				// Just put on board left overs (successfully)
				assert((m_nNextDropTick >= 0) == (m_nJunkTilesToDrop > 0));
			}
			if (m_nJunkTilesToDrop > 0) {
				if (m_nNextDropTick <= nTimer) {
					m_nNextDropTick = calcNextDrop(oGame, nTimer, nCurInterval);
				}
				const int32_t nActivateTick = (bStillMaterializing ? nTimer + 1 : m_nNextDropTick);
				activateAndInform(oLevel, nAddedTiles, nActivateTick);
				return; //------------------------------------------------------
			}
			m_nNextDropTick = -1;
//std::cout << "TopJunkEvent::trigger 5 -- m_nNextDropTick=" << m_nNextDropTick << " bStillMaterializing=" << bStillMaterializing << '\n';
			const int32_t nActivateTick = (bStillMaterializing ? nTimer + 1 : m_nNextDropTick);
			activateAndInform(oLevel, nAddedTiles, nActivateTick);
		}
		break;
	}
}
void TopJunkEvent::activateAndInform(Level& oLevel, int32_t nAddedTiles, int32_t nActivateTick) noexcept
{
	if (nActivateTick >= 0) {
		oLevel.activateEvent(this, nActivateTick);
	}
	if (nAddedTiles > 0) {
		informListeners(LISTENER_GROUP_PLACED_JUNK, nAddedTiles);
	}
}
int32_t TopJunkEvent::fillEmptyTilesOfTopLine(Level& oLevel, int32_t nCurInterval, int32_t nTilesToAdd) noexcept
{
//std::cout << "TopJunkEvent::fillEmptyTilesOfTopLine  nTilesToAdd=" << nTilesToAdd << "  nCurInterval=" << nCurInterval << '\n';
	//const FPoint oShowPos = oLevel.showGet().getPos();
	//const int32_t nShowY = static_cast<int32_t>(oShowPos.m_fY);
	//const double fDisplY = oShowPos.m_fY - nShowY;
	//const int32_t = ((fDisplY > 0) ? 1 : 0);
	const int32_t nTopY = m_nTopY;
	//
	auto& oGame = oLevel.game();
	//
	int32_t nAddedTiles = 0;
	m_oTileCoords.reInit();
	while (nTilesToAdd > 0) {
//std::cout << "TopJunkEvent::fillEmptyTilesOfTopLine  m_nJunkTilesToDrop=" << m_nJunkTilesToDrop << '\n';
		const int32_t nColX = findEmptiestColumn(oLevel, nTopY);
		if (nColX < 0) {
			break;
		}
		TileDownCounter& oTDC = m_oMaterializingCounters[nColX];
		assert(oTDC.m_nCountdown < 0);
		const int32_t nMaterializingMillisec = oGame.random(m_nMaterializingMillisecFrom, m_nMaterializingMillisecTo);
		const int32_t nMaterializingTicks = std::max(1, oGame.random(m_nMaterializingTicksFrom, m_nMaterializingTicksTo) + nMaterializingMillisec / nCurInterval);
//std::cout << "TopJunkEvent::fillEmptyTilesOfTopLine nMaterializingTicks=" << nMaterializingTicks << "  nColX=" << nColX << '\n';
		oTDC.m_nCountdown = nMaterializingTicks - 1;
		oTDC.m_nGameTicksDuration = nMaterializingTicks;
		oTDC.m_nY = nTopY;
		++m_nMaterializingAnis;
		oLevel.boardSetTileAnimator(nColX, nTopY, m_nTileAniMaterializeIndex, this, 0);
//std::cout << "TopJunkEvent::fillEmptyTilesOfTopLine nCurX=" << nColX << "  oTDC.m_nY=" << oTDC.m_nY << "  oTDC.m_nCountdown=" << oTDC.m_nCountdown << '\n';
		oLevel.boardAnimateTile({nColX, nTopY});
		Tile oTile = m_aRandomTiles[m_nCurRandomGen]->createTile();
		m_oTileCoords.add(nColX, nTopY, oTile);
		assert(! oTile.isEmpty());
		//
		++nAddedTiles;
		--nTilesToAdd;
	}
	if (nAddedTiles > 0) {
		oLevel.boardModify(m_oTileCoords);
	}
	return nAddedTiles;
}
int32_t TopJunkEvent::findEmptiestColumn(Level& oLevel, int32_t nTopY) noexcept
{
	int32_t nEmptiestCol = -1;
	int32_t nEmptiestTot = 0;
	const int32_t nMiddleX = m_nBoardW / 2;
	int32_t nCount = 0;
	for (; nCount < m_nBoardW; ++nCount) {
		bool bRight = ((nCount % 2) == 0);
		const int32_t nDispl = (nCount + 1) / 2;
		int32_t nCurX = nMiddleX + (bRight ? +nDispl : -nDispl);
		TileDownCounter& oTDC = m_oMaterializingCounters[nCurX];
//std::cout << "TopJunkEvent::findEmptiestColumn oTDC.m_nCountdown=" << oTDC.m_nCountdown << "  nCurX=" << nCurX << "  nTopY=" << nTopY << '\n';
		if (oTDC.m_nCountdown >= 0) {
			continue; // for nCurX
		}
		int32_t nCurY = nTopY;
		for (; nCurY < m_nBoardH; ++nCurY) {
			if (! oLevel.boardGetTile(nCurX, nCurY).isEmpty()) {
				break; // for nCurY
			}
			if (oLevel.boardGetOwner(nCurX, nCurY) != nullptr) {
				break; // for nCurY
			}
			if (oLevel.boardGetTileAnimator(nCurX, nCurY, m_nTileAniRemovingIndex) != nullptr) {
				break; // for nCurY
			}
			if (oLevel.boardGetTileAnimator(nCurX, nCurY, m_nTileAniMaterializeIndex) != nullptr) {
				break; // for nCurY
			}
			if (m_oTileCoords.contains(nCurX, nCurY)) {
				break; // for nCurY
			}
		}
		if ((nCurY - nTopY) > nEmptiestTot) {
			nEmptiestTot = (nCurY - nTopY);
			nEmptiestCol = nCurX;
//std::cout << "TopJunkEvent::findEmptiestColumn nEmptiestTot=" << nEmptiestTot << "  nEmptiestCol=" << nEmptiestCol << '\n';
		}
	}
	return nEmptiestCol;
}
void TopJunkEvent::continueOrFinishTileAnis(Level& oLevel) noexcept
{
//std::cout << "TopJunkEvent::continueOrFinishTileAnis" << '\n';
	for (int32_t nColX = 0; nColX < m_nBoardW; ++nColX) {
		TileDownCounter& oTDC = m_oMaterializingCounters[nColX];
		if (oTDC.m_nCountdown >= 0) {
			--oTDC.m_nCountdown;
			if (oTDC.m_nCountdown < 0) {
				//oTDC.m_nCountdown = -1;
				--m_nMaterializingAnis;
				oLevel.boardSetTileAnimator(nColX, oTDC.m_nY, m_nTileAniMaterializeIndex, nullptr, 0);
//std::cout << "TopJunkEvent::continueOrFinishTileAnis nColX=" << nColX << "  oTDC.m_nY=" << oTDC.m_nY << '\n';
			}
			oLevel.boardAnimateTile({nColX, oTDC.m_nY});
		}
	}
}
void TopJunkEvent::removeAllTileAnis(const NRect& oArea) noexcept
{
	Level& oLevel = level();
	for (int32_t nCurX = 0; nCurX < m_nBoardW; ++nCurX) {
		TileDownCounter& oTDC = m_oMaterializingCounters[nCurX];
		if (oTDC.m_nCountdown >= 0) {
			if (oArea.containsPoint(NPoint{nCurX, oTDC.m_nY})) {
				oTDC.m_nCountdown = -1;
				--m_nMaterializingAnis;
				oLevel.boardSetTileAnimator(nCurX, oTDC.m_nY, m_nTileAniMaterializeIndex, nullptr, 0);
				oLevel.boardAnimateTile({nCurX, oTDC.m_nY});
//std::cout << "TopJunkEvent::removeAllTileAnis nCurX=" << nCurX << "  oTDC.m_nY=" << oTDC.m_nY << '\n';
			}
		}
	}
}
void TopJunkEvent::removeCoordsTileAnis(const Coords& oCoords) noexcept
{
	Level& oLevel = level();
	for (int32_t nCurX = 0; nCurX < m_nBoardW; ++nCurX) {
		TileDownCounter& oTDC = m_oMaterializingCounters[nCurX];
		if (oTDC.m_nCountdown >= 0) {
			if (oCoords.contains(nCurX, oTDC.m_nY)) {
				oTDC.m_nCountdown = -1;
				--m_nMaterializingAnis;
				oLevel.boardSetTileAnimator(nCurX, oTDC.m_nY, m_nTileAniMaterializeIndex, nullptr, 0);
				oLevel.boardAnimateTile({nCurX, oTDC.m_nY});
			}
		}
	}
}
void TopJunkEvent::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
	if ((eDir == Direction::LEFT) || (eDir == Direction::RIGHT)) {
		removeAllTileAnis(NRect{0, 0, m_nBoardW, m_nBoardH});
		return; //--------------------------------------------------------------
	}
	const NSize oBoardSize = NSize{m_nBoardW, m_nBoardH};
	const NRect oRemove = Helpers::boardScrollRemove(oBoardSize, eDir);
	removeAllTileAnis(oRemove);
}
void TopJunkEvent::boardPostScroll(Direction::VALUE eDir) noexcept
{
	if ((eDir == Direction::LEFT) || (eDir == Direction::RIGHT)) {
		return; //--------------------------------------------------------------
	}
	const NSize oBoardSize = NSize{m_nBoardW, m_nBoardH};
	Level& oLevel = level();
	// tile animations within move area have been moved (up or down)
	const int32_t nDeltaY = Direction::deltaY(eDir);
	NRect oMoveArea = Helpers::boardScrollMovingPre(oBoardSize, eDir);
	for (int32_t nCurX = 0; nCurX < m_nBoardW; ++nCurX) {
		TileDownCounter& oTDC = m_oMaterializingCounters[nCurX];
		if (oTDC.m_nCountdown >= 0) {
			if (oMoveArea.containsPoint(NPoint{nCurX, oTDC.m_nY})) {
//std::cout << "TopJunkEvent::boardPostScroll nCurX=" << nCurX << "  oTDC.m_nY=" << oTDC.m_nY << "  oTDC.m_nCountdown=" << oTDC.m_nCountdown << '\n';
				oTDC.m_nY += nDeltaY;
				assert(this == oLevel.boardGetTileAnimator(nCurX, oTDC.m_nY, m_nTileAniMaterializeIndex));
				oLevel.boardAnimateTile({nCurX, oTDC.m_nY});
			} else {
				// do nothing
			}
		}
	}
}
void TopJunkEvent::boabloPreFreeze(LevelBlock& oBlock) noexcept
{
	removeCoordsTileAnis(LevelBlock::getCoords(oBlock));
}
void TopJunkEvent::boabloPostFreeze(const Coords& /*oCoords*/) noexcept
{
}
void TopJunkEvent::boabloPreUnfreeze(const Coords& oCoords) noexcept
{
	removeCoordsTileAnis(oCoords);
}
void TopJunkEvent::boabloPostUnfreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void TopJunkEvent::boardPreInsert(Direction::VALUE /*eDir*/, NRect oArea, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
	//TODO should only remove if within modified area
	removeAllTileAnis(oArea);
}
void TopJunkEvent::boardPostInsert(Direction::VALUE /*eDir*/, NRect /*oArea*/) noexcept
{
}
void TopJunkEvent::boardPreDestroy(const Coords& oCoords) noexcept
{
	removeCoordsTileAnis(oCoords);
}
void TopJunkEvent::boardPostDestroy(const Coords& /*oCoords*/) noexcept
{
}
void TopJunkEvent::boardPreModify(const TileCoords& oTileCoords) noexcept
{
	if ((&oTileCoords) != (&m_oTileCoords)) {
		removeCoordsTileAnis(oTileCoords);
	}
}
void TopJunkEvent::boardPostModify(const Coords& /*oCoords*/) noexcept
{
}

double TopJunkEvent::getElapsed01(int32_t /*nHash*/, int32_t nX, int32_t nY, int32_t nAni
								, int32_t nViewTick, int32_t nTotViewTicks) const noexcept
{
//std::cout << "TopJunkEvent::getElapsed01() nViewTick=" << nViewTick << " nTotViewTicks=" << nTotViewTicks << '\n';
	assert(nTotViewTicks > 0);
	assert((nViewTick >= 0) && (nViewTick < nTotViewTicks));
	if (nAni != m_nTileAniMaterializeIndex) {
		assert(false);
		return 1.0; //----------------------------------------------------------
	}

//std::cout << "TopJunkEvent::getElapsed01() nX=" << nX << " nY=" << nY << '\n';
	assert((nX >= 0) && (nX < m_nBoardW));
	const TileDownCounter& oTDC = m_oMaterializingCounters[nX];
	assert(oTDC.m_nCountdown >= 0);
	assert(oTDC.m_nY == nY);

	const int32_t nTotSteps = oTDC.m_nGameTicksDuration + 0 * nY;
	const int32_t nCountdown = oTDC.m_nCountdown + 1;
//std::cout << "TopJunkEvent::getElapsed01() nTotSteps=" << nTotSteps << " nCountdown=" << nCountdown << '\n';
	assert(nTotSteps > 0);
	assert((nCountdown > 0) && (nCountdown <= nTotSteps));
	//TODO make an example to explain this scary formula
	const double fElapsed = 1.0 + (- nCountdown + (nViewTick + 0.5) / nTotViewTicks) / nTotSteps;
	assert((fElapsed >= 0.0) && (fElapsed <= 1.0));
	return fElapsed;
}
double TopJunkEvent::getElapsed01(int32_t /*nHash*/, const LevelBlock& /*oLevelBlock*/, int32_t /*nBrickIdx*/, int32_t /*nAni*/
								, int32_t /*nViewTick*/, int32_t /*nTotViewTicks*/) const noexcept
{
	assert(false);
	return 1.0;
}

} // namespace stmg
