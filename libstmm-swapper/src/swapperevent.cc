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
 * File:   swapperevent.cc
 */

#include "swapperevent.h"

#include <stmm-games/level.h>
#include <stmm-games/block.h>
#include <stmm-games/named.h>
#include <stmm-games/appconfig.h>
#include <stmm-games/keyactionevent.h>
#include <stmm-games/utile/tilecoords.h>
#include <stmm-games/util/util.h>
#include <stmm-games/util/namedindex.h>

#include <stmm-input/xyevent.h>
#include <stmm-input-ev/pointerevent.h>

#include <tuple>
#include <string>

namespace stmg
{

static const std::string s_sTileAniRemoving = "TILEANI:REMOVING";

static const std::string s_sSwapperLeftCharName ="SWAPPER:LEFT";
static const std::string s_sSwapperRightCharName ="SWAPPER:RIGHT";
static constexpr const int32_t s_nZObjectZSwapper = 3000;

const std::string SwapperEvent::s_sKeyActionLeft = "SwapperEvent:Left";
const std::string SwapperEvent::s_sKeyActionRight = "SwapperEvent:Right";
const std::string SwapperEvent::s_sKeyActionUp = "SwapperEvent:Up";
const std::string SwapperEvent::s_sKeyActionDown = "SwapperEvent:Down";
const std::string SwapperEvent::s_sKeyActionSwap = "SwapperEvent:Swap";
const std::string SwapperEvent::s_sKeyActionPushRow = "SwapperEvent:PushRow";
const std::string SwapperEvent::s_sKeyActionNext = "SwapperEvent:Next";

const int32_t SwapperEvent::s_nAreaMinW = 2;
const int32_t SwapperEvent::s_nAreaMinH = 2;

const int32_t SwapperEvent::s_nDeaultBlockW = 2;
const int32_t SwapperEvent::s_nDeaultBlockH = 1;

static constexpr const int32_t s_nKeysBufferSize = 10;

SwapperEvent::SwapperEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, LevelBlock(false)
, m_oData(std::move(oInit))
, m_oKeys(s_nKeysBufferSize)
{
	commonInit();
}
void SwapperEvent::reInit(Init&& oInit) noexcept
{
//std::cout << "SwapperEvent(" << getId() << ")::trigger" << '\n';
//std::cout << "  nInitPosX=" << nInitPosX << "  nInitPosY=" << nInitPosY << '\n';
//std::cout << "  nAreaX=" << nAreaX << "  nAreaY=" << nAreaY << "  nAreaW=" << nAreaW << "  nAreaH=" << nAreaH << '\n';
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
void SwapperEvent::commonInit() noexcept
{
	assert((m_oData.m_nLevelTeam == -1) || (m_oData.m_nLevelTeam >= 0));

	Level& oLevel = level();
	const AppConfig& oAppConfig = *oLevel.prefs().getAppConfig();
	NamedIndex& oTileAnisIndex = oLevel.getNamed().tileAnis();

	m_bSubshowMode = oLevel.subshowMode();

	m_nKeyActionLeft = oAppConfig.getKeyActionId(s_sKeyActionLeft);
	m_nKeyActionRight = oAppConfig.getKeyActionId(s_sKeyActionRight);
	m_nKeyActionUp = oAppConfig.getKeyActionId(s_sKeyActionUp);
	m_nKeyActionDown = oAppConfig.getKeyActionId(s_sKeyActionDown);
	m_nKeyActionSwap = oAppConfig.getKeyActionId(s_sKeyActionSwap);
	m_nKeyActionPushRow = oAppConfig.getKeyActionId(s_sKeyActionPushRow);
	m_nKeyActionNext = oAppConfig.getKeyActionId(s_sKeyActionNext);

	if (!m_oData.m_refImmovable) {
		m_oData.m_refImmovable = std::make_unique<TileSelector>();
	}

	m_nTileAniRemovingIndex = oTileAnisIndex.getIndex(s_sTileAniRemoving);
	if (m_nTileAniRemovingIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! ScrollerEvent: tile animation '" << s_sTileAniRemoving << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniRemovingIndex = oTileAnisIndex.addName(s_sTileAniRemoving);
	}

	initBlock();

	m_eState = SWAPPER_EVENT_STATE_ACTIVATE;
	m_nTickStarted = -1;
	m_bPaused = false;
	m_nLastLevelPlayer = -1;
}
void SwapperEvent::initBlock() noexcept
{
	if (m_oData.m_oBlock.isEmpty()) {
		NamedIndex& oCharsIndex = level().getNamed().chars();
		int32_t nSwapperLeftCharIndex = oCharsIndex.getIndex(s_sSwapperLeftCharName);
		if (nSwapperLeftCharIndex < 0) {
			#ifndef NDEBUG
			std::cout << "Warning! SwapperEvent: char '" << s_sSwapperLeftCharName << "' not defined!" << '\n';
			#endif //NDEBUG
			nSwapperLeftCharIndex = oCharsIndex.addName(s_sSwapperLeftCharName);
		}
		int32_t nSwapperRightCharIndex = oCharsIndex.getIndex(s_sSwapperRightCharName);
		if (nSwapperRightCharIndex < 0) {
			#ifndef NDEBUG
			std::cout << "Warning! SwapperEvent: char '" << s_sSwapperRightCharName << "' not defined!" << '\n';
			#endif //NDEBUG
			nSwapperRightCharIndex = oCharsIndex.addName(s_sSwapperRightCharName);
		}
		std::vector<Tile> aBrick(2, Tile());
		aBrick[0].getTileChar().setCharIndex(nSwapperLeftCharIndex);
		aBrick[1].getTileChar().setCharIndex(nSwapperRightCharIndex);
		std::vector< std::tuple<bool, int32_t, int32_t> > aBrickPos(2);
		aBrickPos[0] = std::make_tuple(true, 0, 0);
		aBrickPos[1] = std::make_tuple(true, 1, 0);
		m_oData.m_oBlock = Block(2, aBrick, 1, {aBrickPos});
	} else {
		// check no gaps in shape ids.
		#ifndef NDEBUG
		int32_t nShapeIdx = 0;
		int32_t nCurShapeId = m_oData.m_oBlock.shapeFirst();
		while (nCurShapeId >= 0) {
			assert(nShapeIdx == nCurShapeId);
			assert(m_oData.m_oBlock.shapeTotVisibleBricks(nCurShapeId) >= 2);
			nCurShapeId = m_oData.m_oBlock.shapeNext(nCurShapeId);
			++nShapeIdx;
		};
		#endif //NDEBUG
	}
	//
	#ifndef NDEBUG
	const int32_t nMinW = std::max(m_oData.m_oBlock.shapeWidth(m_oData.m_oBlock.widestShapeId()), s_nAreaMinW);
	const int32_t nMinH = std::max(m_oData.m_oBlock.shapeHeight(m_oData.m_oBlock.highestShapeId()), s_nAreaMinH);
	assert((m_oData.m_oArea.m_nX >= 0) && (m_oData.m_oArea.m_nW >= nMinW) && (m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW <= level().boardWidth()));
	assert((m_oData.m_oArea.m_nY >= 0) && (m_oData.m_oArea.m_nH >= nMinH) && (m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH <= level().boardHeight()));
	#endif //NDEBUG

	const int32_t nInitialShape = 0;
	int32_t nDeltaX;
	int32_t nDeltaY;
	if (canPlaceShapeInArea(nInitialShape, nDeltaX, nDeltaY)) {
		m_oData.m_oInitPos.m_nX += nDeltaX;
		m_oData.m_oInitPos.m_nY += nDeltaY;
		m_bNoSwap = false;
	} else {
		m_bNoSwap = true;
	}

	LevelBlock::blockInitialSet(m_oData.m_oBlock, nInitialShape, m_oData.m_oInitPos, true, m_oData.m_nLevelTeam);
}

void SwapperEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
//std::cout << "SwapperEvent(" << getId() << ")::trigger" << '\n';
	//TODO
	// ACTIVATE activate event
	// INIT     init block
	// FALL     just live
	Level& oLevel = level();
	const int32_t nTimer = oLevel.game().gameElapsed();
//std::cout << "SwapperEvent(" << getId() << ")::trigger timer=" << nTimer << " state=" << (int32_t)m_eState << '\n';
	switch (m_eState)
	{
		case SWAPPER_EVENT_STATE_ACTIVATE:
		{
			m_eState = SWAPPER_EVENT_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nTimer);
				break;
			}
		} //fallthrough
		case SWAPPER_EVENT_STATE_INIT:
		{
			if (p0TriggeringEvent != nullptr) {
				return; //------------------------------------------------------
			}
			const NPoint oPos = LevelBlock::blockPos();
			blockMove(m_oData.m_oInitPos.m_nX - oPos.m_nX, m_oData.m_oInitPos.m_nY - oPos.m_nY);
			m_oKeys.clear();
			oLevel.blockAdd(this, LevelBlock::MGMT_TYPE_AUTO_SCROLL);
			m_eState = SWAPPER_EVENT_STATE_WAIT;
			m_nTickStarted = nTimer;
			if (m_oData.m_nTicksToLive > 0) {
				oLevel.activateEvent(this, nTimer + m_oData.m_nTicksToLive);
			}
			break;
		}
		case SWAPPER_EVENT_STATE_WAIT:
		{
//std::cout << "SwapperEvent(" << getId() << ")::trigger WAIT" << '\n';
			if (p0TriggeringEvent != nullptr) {
				const Block& oBlock = LevelBlock::blockGet();
				const int32_t nCurShapeId = LevelBlock::blockGetShapeId();
				if ((nMsg == MESSAGE_PAUSE) || (nMsg == MESSAGE_RESUME)) {
					if (nMsg == MESSAGE_PAUSE) {
						m_bPaused = true;
					} else {
						m_bPaused = false;
					}
				} else {
					if ((nMsg == MESSAGE_NEXT_SHAPE) || (nMsg == MESSAGE_PREV_SHAPE)) {
						nValue = nCurShapeId;
						if (nMsg ==	MESSAGE_PREV_SHAPE) {
							--nValue;
							if (nValue < 0) {
								nValue = oBlock.totShapes() - 1;
							}
						} else {
							++nValue;
							if (nValue >= oBlock.totShapes()) {
								nValue = 0;
							}
						}
						if (nValue != nCurShapeId) {
							nMsg = MESSAGE_SET_SHAPE;
						}
					}
					if (nMsg ==	MESSAGE_SET_SHAPE) {
						const int32_t nNewShapeId = nValue;
						if ((nNewShapeId != nCurShapeId) && oBlock.isShapeId(nNewShapeId)) {
							int32_t nDeltaX;
							int32_t nDeltaY;
							if (canPlaceShapeInArea(nNewShapeId, nDeltaX, nDeltaY)) {
								blockMoveRotate(nNewShapeId, nDeltaX, nDeltaY);
							}
						}
					}
				}
//std::cout << "SwapperEvent(" << getId() << ")::trigger m_nTicksToLive=" << m_nTicksToLive << '\n';
				if (m_oData.m_nTicksToLive > 0) {
					const int32_t nRestToLive = m_nTickStarted + m_oData.m_nTicksToLive - nTimer;
					oLevel.activateEvent(this, std::min<int32_t>(0, nRestToLive));
					break;
				}
				return; //------------------------------------------------------
			}
//std::cout << "SwapperEvent(" << getId() << ")::trigger DEAD" << '\n';
			m_eState = SWAPPER_EVENT_STATE_DEAD;
			#ifndef NDEBUG
			const bool bRemoved =
			#endif //NDEBUG
			remove();
			assert(bRemoved);
		}
		break;
		case SWAPPER_EVENT_STATE_DEAD:
		{
		}
		break;
	}
}
bool SwapperEvent::canPlaceShapeInArea(int32_t nShapeId, int32_t& nDeltaX, int32_t& nDeltaY) const noexcept
{
	nDeltaX = 0;
	nDeltaY = 0;
	const Block& oBlock = LevelBlock::blockGet();
	const NPoint oPos = LevelBlock::blockPos();
	int32_t nPosX = oPos.m_nX;
	int32_t nPosY = oPos.m_nY;
	const int32_t nOldPosX = nPosX;
	const int32_t nOldPosY = nPosY;

	const int32_t nLargestShapeWidth = oBlock.maxWidth();
	const int32_t nMaxHorizMove = std::max<int32_t>(nLargestShapeWidth - 1, static_cast<int32_t>(1));

	const int32_t nHighestShapeHeight = oBlock.maxHeight();
	const int32_t nMaxVertMove = std::max<int32_t>(nHighestShapeHeight - 1, static_cast<int32_t>(1));

	const int32_t nMinX = oBlock.shapeMinX(nShapeId);
	const int32_t nMinY = oBlock.shapeMinY(nShapeId);
	const int32_t nMaxX = oBlock.shapeMaxX(nShapeId);
	const int32_t nMaxY = oBlock.shapeMaxY(nShapeId);
	if (nPosX + nMinX < m_oData.m_oArea.m_nX) {
		nPosX = m_oData.m_oArea.m_nX - nMinX;
		nDeltaX = nPosX - nOldPosX;
		if (nDeltaX > nMaxHorizMove) {
			return false; //----------------------------------------------------
		}
	}
	if (nPosX + nMaxX >= m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW) {
		if (nDeltaX != 0) {
			// The shape doesn't fit in the area
			return false; //----------------------------------------------------
		}
		nPosX = m_oData.m_oArea.m_nX + m_oData.m_oArea.m_nW - (nMaxX + 1);
		nDeltaX = nPosX - nOldPosX;
		if ((- nDeltaX) > nMaxHorizMove) {
			return false; //----------------------------------------------------
		}
	}
	if (nPosY + nMinY < m_oData.m_oArea.m_nY) {
		nPosY = m_oData.m_oArea.m_nY - nMinY;
		nDeltaY = nPosY - nOldPosY;
		if (nDeltaY > nMaxVertMove) {
			return false; //----------------------------------------------------
		}
	}
	if (nPosY + nMaxY >= m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH) {
		if (nDeltaY != 0) {
			// The shape doesn't fit in the area
			return false; //----------------------------------------------------
		}
		nPosY = m_oData.m_oArea.m_nY + m_oData.m_oArea.m_nH - (nMaxY + 1);
		nDeltaY = nPosY - nOldPosY;
		if ((- nDeltaY) > nMaxVertMove) {
			return false; //----------------------------------------------------
		}
	}
	return true;
}
void SwapperEvent::onScrolled(Direction::VALUE eDir) noexcept
{
//std::cout << "SwapperEvent(" << blockGetId() << ")::onScrolled blockPosX=" << blockPosX() << "   blockPosY=" << blockPosY() << '\n';
	if (!level().blockMoveIsWithinArea(*this, 0, 0, m_oData.m_oArea.m_nX, m_oData.m_oArea.m_nY, m_oData.m_oArea.m_nW, m_oData.m_oArea.m_nH)) {
//std::cout << "               ::onScrolled  outside area, move back" << '\n';
		// was scrolled outside area
		const Direction::VALUE eOppDir = Direction::opposite(eDir);
		const int32_t nDeltaX = Direction::deltaX(eOppDir);
		const int32_t nDeltaY = Direction::deltaY(eOppDir);
		blockMove(nDeltaX, nDeltaY);
	}
}

void SwapperEvent::handleKeyActionInput(const shared_ptr<KeyActionEvent>& refEvent) noexcept
{
	KeyActionEvent* p0AEv = refEvent.get();
	const stmi::Event::AS_KEY_INPUT_TYPE eType = p0AEv->getType();
	if (eType != stmi::Event::AS_KEY_PRESS) {
//std::cout << "SwapperEvent::handleKeyActionInput not press" << '\n';
		return; //--------------------------------------------------------------
	}
	if (m_bPaused) {
		return; //--------------------------------------------------------------
	}
	const int32_t nKeyAction = p0AEv->getKeyAction();
	if (nKeyAction == m_nKeyActionNext) {
		level().blockCycleControl(this);
		return; //--------------------------------------------------------------
	}
	if (m_oKeys.isFull()) {
//std::cout << "SwapperEvent::handleKeyActionInput m_oKeys.isFull()" << '\n';
		return; //--------------------------------------------------------------
	}
	m_oKeys.write(nKeyAction);
}
void SwapperEvent::handleInput(const shared_ptr<stmi::Event>& refEvent) noexcept
{
	const stmi::Event::Class& oC = refEvent->getEventClass();
	if (! oC.isXYEvent()) {
		return; //--------------------------------------------------------------
	}
	const auto p0XYEvent = static_cast<stmi::XYEvent*>(refEvent.get());
	if (p0XYEvent->getXYGrabType() != stmi::XYEvent::XY_GRAB) {
		return; //--------------------------------------------------------------
	}
	if (oC == stmi::PointerEvent::getClass()) {
		const auto p0PointerEvent = static_cast<stmi::PointerEvent*>(refEvent.get());
		if (p0PointerEvent->getButton() != stmi::PointerEvent::s_nFirstButton) {
			informListeners(LISTENER_GROUP_PUSH_ROW, 1);
			return; //----------------------------------------------------------
		}
	}
	// Shape can be anything, the click cell is where the
	// top left of the bounding box should be placed on the board
	// X..     ..X.
	// ..X     X...
	//         ...X
	const bool bMoved = moveToXY(p0XYEvent->getX(), p0XYEvent->getY());
	if (bMoved && ! m_bNoSwap) {
		swapBlock();
	}

//std::cout << "SwapperEvent::handleInput pos=(" << p0XYEvent->getX() << "," << p0XYEvent->getY() << ")" << '\n';
}
void SwapperEvent::handleTimer() noexcept
{
//std::cout << "SwapperEvent(" << getId() << ")::handleTimer(" << level().game().gameElapsed() << ")"<< '\n';
	const FPoint oShowPos = level().showGet().getPos();
	//const int32_t nShowX = oShowPos.m_fX;
	const int32_t nShowY = static_cast<int32_t>(oShowPos.m_fY);
	//const double fDisplX = oShowPos.m_fX - nShowX;
	const double fDisplY = oShowPos.m_fY - nShowY;
	const NPoint oPos = LevelBlock::blockPos();
	const NPoint oBricksMinPos = LevelBlock::blockBricksMinPos();
	const int32_t nBlockMinY = oPos.m_nY + oBricksMinPos.m_nY;
	if ((nBlockMinY == 0) && (fDisplY > 0)) {
		// This class works with no scrolling or a scroller that just sets
		// the ShowY to values >= 0.0 and < 1.0
		// When a scroller is active the top line of the board should be inhabited
		// by this swapper only if it is completely visible (fDisplY == 0)
		move(Direction::DOWN);
	}

	if (m_oKeys.isEmpty()) {
		return; //--------------------------------------------------------------
	}
	const int32_t nKeyAction = m_oKeys.read();

//std::cout << "---nBlockPosY        = " << nBlockPosY << '\n';
//std::cout << "---blockBricksMinY() = " << blockBricksMinY() << '\n';
//std::cout << "---nBlockMinY        = " << nBlockMinY << '\n';
	if (nKeyAction == m_nKeyActionLeft)	{
		move(Direction::LEFT);
	} else if (nKeyAction == m_nKeyActionRight) {
		move(Direction::RIGHT);
	} else if (nKeyAction == m_nKeyActionUp) {
		move(Direction::UP);
	} else if (nKeyAction == m_nKeyActionDown) {
		move(Direction::DOWN);
	} else if (nKeyAction == m_nKeyActionSwap) {
		if (! m_bNoSwap) {
			swapBlock();
		}
	} else if (nKeyAction == m_nKeyActionPushRow) {
		informListeners(LISTENER_GROUP_PUSH_ROW, 1);
	}
}
bool SwapperEvent::swapBlock() noexcept
{
//std::cout << "swapBlock() = " << nBlockPosY << '\n';
	Level& oLevel = level();
	const NPoint oPos = LevelBlock::blockPos();
	const int32_t& nBlockX = oPos.m_nX;
	const int32_t& nBlockY = oPos.m_nY;
	m_oReciclerTileCoords.create(m_refTileCoords);
	Tile oOldTile;
	int32_t nFirstVisibleBrickId = -1;
	const std::vector<int32_t>& aBrickIds = blockBrickIds();
	for (const int32_t nBrickId : aBrickIds) {
		if (!blockBrickVisible(nBrickId)) {
			continue; // for---
		}
		const NPoint oBrickPos = LevelBlock::blockBrickPos(nBrickId);
		const int32_t nX = nBlockX + oBrickPos.m_nX;
		const int32_t nY = nBlockY + oBrickPos.m_nY;
		if ( (oLevel.boardGetOwner(nX, nY) != nullptr)
					|| (oLevel.boardGetTileAnimator(nX, nY, m_nTileAniRemovingIndex) != nullptr) ) {
			return false; //----------------------------------------------------
		}
		const Tile& oTile = oLevel.boardGetTile(nX, nY);
		if ((!oTile.isEmpty()) && m_oData.m_refImmovable->select(oTile)) {
			// immovable involved: can't swap
			informListeners(LISTENER_GROUP_IMMOVABLE, Util::packPointToInt32(NPoint{nX, nY}));
			return false; //----------------------------------------------------
		}
		if (nFirstVisibleBrickId < 0) {
			nFirstVisibleBrickId = nBrickId;
		} else {
			m_refTileCoords->add(nX, nY, oOldTile);
		}
		oOldTile = oTile;
	}
	assert(m_refTileCoords->size() >= 1); // one was skipped
	{
		const NPoint oBrickPos = LevelBlock::blockBrickPos(nFirstVisibleBrickId);
		const int32_t nX = nBlockX + oBrickPos.m_nX;
		const int32_t nY = nBlockY + oBrickPos.m_nY;
		m_refTileCoords->add(nX, nY, oOldTile);
		oLevel.boardModify(*m_refTileCoords);
		informListeners(LISTENER_GROUP_SWAPPED, Util::packPointToInt32(NPoint{nX, nY}));
	}
	const int32_t nTotSelectors = static_cast<int32_t>(m_oData.m_aSwappedSelectors.size());
	if (nTotSelectors > 0) {
		for (auto itTiles = m_refTileCoords->begin(); itTiles != m_refTileCoords->end(); itTiles.next()) {
			const Tile& oTile = itTiles.getTile();
			const int32_t nX = itTiles.x();
			const int32_t nY = itTiles.y();
			if (! oTile.isEmpty()) {
				for (int32_t nSelectorIdx = 0; nSelectorIdx < nTotSelectors; ++nSelectorIdx) {
					auto& refSelector = m_oData.m_aSwappedSelectors[nSelectorIdx];
					if (refSelector->select(oTile)) {
						informListeners(LISTENER_GROUP_SWAPPED_0 + nSelectorIdx, Util::packPointToInt32(NPoint{nX, nY}));
					}
				}
			}
		}
	}
	return true;
}
bool SwapperEvent::move(Direction::VALUE eDir) noexcept
{
	assert((static_cast<int32_t>(eDir) >= 0) && (static_cast<int32_t>(eDir) < 4));
	const NPoint oPos = LevelBlock::blockPos();
	const int32_t nPosX = oPos.m_nX;
	const int32_t nPosY = oPos.m_nY;

	const int32_t nDx = Direction::deltaX(eDir);
	const int32_t nDy = Direction::deltaY(eDir);

//std::cout << "SwapperEvent(" << blockGetId() << ")::move(nDx=" << nDx << ", nDy=" << nDy << ")  nPosX=" << nPosX << " nPosY=" << nPosY << '\n';

	const std::vector< Block::Contact >& aContacts = LevelBlock::blockContacts(eDir);
	for (const Block::Contact& oContactBrickPos : aContacts) {
		const int32_t nBoardX = nPosX + oContactBrickPos.m_nRelX;
		const int32_t nBoardY = nPosY + oContactBrickPos.m_nRelY;
		if (! m_oData.m_oArea.containsPoint(NPoint{nBoardX, nBoardY})) {
			// cannot move
//std::cout << "        ::move()  cannot move Contact=(x=" << nBoardX << ",y=" << nBoardY << ")  m_oArea.m_nY=" << m_oArea.m_nY << " m_oArea.m_nH=" << m_oArea.m_nH << '\n';
			return false; //----------------------------------------------------
		}
	}

	blockMove(nDx, nDy);
	return true;
}
bool SwapperEvent::moveToXY(double fNewX, double fNewY) noexcept
{
	if (m_bSubshowMode) {
		if (m_nLastLevelPlayer < 0) {
			return false; //----------------------------------------------------
		}
	}
	Level& oLevel = level();
	LevelShow& oLevelShow = [&]() -> LevelShow&
	{
		if (m_bSubshowMode) {
			return oLevel.subshowGet(m_nLastLevelPlayer);
		} else {
			return oLevel.showGet();
		}
	}();
	const FPoint oBoardNew = oLevelShow.getBoardPos(fNewX, fNewY);
	int32_t nNewX = oBoardNew.m_fX;
	int32_t nNewY = oBoardNew.m_fY;

	// Example:
	//  o...            o: origin
	//  ..X.            X: oMinPos = (2,1) relative to origin
	//  ....
	//  ...Y
	// (nNewX, nNewY) is where X should be placed
	// => the new origin position is (nNewX - oMinPos.m_nX, nNewY - oMinPos.m_nY)
	const Block& oBlock = blockGet();
	const int32_t nShapeId = blockGetShapeId();
	NPoint oMinPos = oBlock.shapeMinPos(nShapeId);
	nNewX -= oMinPos.m_nX;
	nNewY -= oMinPos.m_nY;
	const NPoint oMaxPos = oBlock.shapeMaxPos(nShapeId);
	const int32_t nMaxBoardX = nNewX + oMaxPos.m_nX;
	const int32_t nMaxBoardY = nNewY + oMaxPos.m_nY;
	if (! m_oData.m_oArea.containsPoint(NPoint{nMaxBoardX, nMaxBoardY})) {
		return false; //--------------------------------------------------------
	}
	const int32_t nMinBoardX = nNewX + oMinPos.m_nX;
	const int32_t nMinBoardY = nNewY + oMinPos.m_nY;
	if (! m_oData.m_oArea.containsPoint(NPoint{nMinBoardX, nMinBoardY})) {
		return false; //--------------------------------------------------------
	}
	const NPoint oPos = LevelBlock::blockPos();
	const int32_t nDx = nNewX - oPos.m_nX;
	const int32_t nDy = nNewY - oPos.m_nY;
	blockMove(nDx, nDy);
	return true;
}
void SwapperEvent::onPlayerChanged() noexcept
{
	const int32_t nLevelPlayer = getPlayer();
	if (nLevelPlayer < 0) {
		// not controlled
		return; //--------------------------------------------------------------
	}
	if (m_nLastLevelPlayer == nLevelPlayer) {
		return; //--------------------------------------------------------------
	}
	m_nLastLevelPlayer = nLevelPlayer;
}

int32_t SwapperEvent::blockPosZ() const noexcept
{
	return s_nZObjectZSwapper;
}

void SwapperEvent::fall() noexcept
{
//std::cout << "SwapperEvent(" << getId() << ")::fall" << '\n';
}
bool SwapperEvent::fuseTo(LevelBlock& /*oLevelBlock*/) noexcept
{
	return false;
}
bool SwapperEvent::remove() noexcept
{
	const bool bRemoved = LevelBlock::remove();
	if (bRemoved) {
		privateOnRemove();
	}
	return bRemoved;
}
void SwapperEvent::privateOnRemove() noexcept
{
	informListeners(LISTENER_GROUP_FINISHED, 0);
}
bool SwapperEvent::destroy() noexcept
{
	return false;
}
bool SwapperEvent::freeze() noexcept
{
	return false;
}
bool SwapperEvent::canFuseWith(LevelBlock& /*oLevelBlock*/) const noexcept
{
	return false;
}
bool SwapperEvent::removeBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}
bool SwapperEvent::destroyBrick(int32_t /*nBrickId*/) noexcept
{
	return false;
}
LevelBlock::QUERY_ATTACK_TYPE SwapperEvent::queryAttack(LevelBlock& /*oAttacker*/, int32_t /*nBoardX3*/
														, int32_t /*nBoardY*/
														, const Tile& /*oTile*/) const noexcept
{
	return LevelBlock::QUERY_ATTACK_TYPE_NOTHING;
}
bool SwapperEvent::attack(LevelBlock& /*oAttacker*/, int32_t /*nBoardX*/, int32_t /*nBoardY*/
						, const Tile& /*oTile*/) noexcept
{
	return false;
}

} // namespace stmg
