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
 * File:   xmlswapperevent.cc
 */

#include "xmlswapperevent.h"

#include <stmm-swapper/swapperevent.h>

#include <stmm-games-xml-base/xmlcommonerrors.h>
#include <stmm-games-xml-base/xmlutil//xmlbasicparser.h>
#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmltraitsparser.h>

#include <stmm-games/game.h>
#include <stmm-games/block.h>
#include <stmm-games/util/util.h>
#include <stmm-games/util/basictypes.h>

//#include <cassert>
//#include <iostream>
#include <utility>

namespace stmg
{

const std::string XmlSwapperEventParser::s_sEventSwapperNodeName = "SwapperEvent";

static const std::string s_sEventSwapperBlockAttr = "block";
static const std::string s_sEventSwapperTeamAttr = "team";
static const std::string s_sEventSwapperInitXAttr = "posX";
static const std::string s_sEventSwapperInitYAttr = "posY";
static const std::string s_sEventSwapperAreaXAttr = "areaX";
static const std::string s_sEventSwapperAreaYAttr = "areaY";
static const std::string s_sEventSwapperAreaWAttr = "areaW";
static const std::string s_sEventSwapperAreaHAttr = "areaH";
static const std::string s_sEventSwapperTicksToLiveAttr = "ticksToLive";
static const std::string s_sEventSwapperImmovableNodeName = "Immovable";
static const std::string s_sEventSwapperSwappedNodeName = "Swapped";

static const std::string s_sEventSwapperSwappedListenerGroupStart = "SWAPPED_";

XmlSwapperEventParser::XmlSwapperEventParser()
: XmlEventParser(s_sEventSwapperNodeName)
{
}

Event* XmlSwapperEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventSwapper(oCtx, p0Element);
}

Event* XmlSwapperEventParser::parseEventSwapper(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "XmlSwapperEventParser::parseEventSwapper" << '\n';
	oCtx.addChecker(p0Element);
	SwapperEvent::Init oInit;
	parseEventBase(oCtx, p0Element, oInit);
	;
	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	oInit.m_oArea = oXmlBasicParser.parseNRect(oCtx, p0Element
												, s_sEventSwapperAreaXAttr, s_sEventSwapperAreaYAttr, s_sEventSwapperAreaWAttr, s_sEventSwapperAreaHAttr
												, true, NRect{0, 0, oCtx.level().boardWidth(), oCtx.level().boardHeight()}
												, NSize{SwapperEvent::s_nAreaMinW, SwapperEvent::s_nAreaMinH});
	//
	int32_t nBlockW = SwapperEvent::s_nDeaultBlockW;
	int32_t nBlockH = SwapperEvent::s_nDeaultBlockH;
	int32_t nBlockX = 0;
	int32_t nBlockY = 0;
	//
	const auto oPairBlock = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSwapperBlockAttr);
	if (oPairBlock.first) {
		const std::string& sBlock = oPairBlock.second;
		const bool bFound = getBlock(oCtx, sBlock, oInit.m_oBlock);
		if (!bFound) {
			throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSwapperBlockAttr
										, Util::stringCompose("Attribute '%1': Block '%2' not defined", s_sEventSwapperBlockAttr, sBlock));
		}
		const int32_t nTotRemovedShapes = oInit.m_oBlock.shapeRemoveAllInvisible();
		if (nTotRemovedShapes > 0) {
			throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSwapperBlockAttr
										, Util::stringCompose("Attribute '%1': Block '%2' cannot contain empty shapes", s_sEventSwapperBlockAttr, sBlock));
		}
		if (oInit.m_oBlock.isEmpty()) {
			throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSwapperBlockAttr
										, Util::stringCompose("Attribute '%1': Block cannot be empty", s_sEventSwapperBlockAttr));
		}
		int32_t nShapeIdx = 0;
		int32_t nCurShapeId = oInit.m_oBlock.shapeFirst();
		while (nCurShapeId >= 0) {
			if (nShapeIdx != nCurShapeId) {
				throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSwapperBlockAttr
											, Util::stringCompose("Attribute '%1': Block must have incremental shapes", s_sEventSwapperBlockAttr));
			}
			if (oInit.m_oBlock.shapeTotVisibleBricks(nCurShapeId) < 2) {
				throw XmlCommonErrors::error(oCtx, p0Element, s_sEventSwapperBlockAttr
											, Util::stringCompose("Attribute '%1': Block must have \nat least 2 visible bricks for each shape", s_sEventSwapperBlockAttr));
			}
			nCurShapeId = oInit.m_oBlock.shapeNext(nCurShapeId);
			++nShapeIdx;
		}
		const int32_t nInitialShape = 0;
		nBlockW = oInit.m_oBlock.shapeWidth(nInitialShape);
		nBlockH = oInit.m_oBlock.shapeHeight(nInitialShape);
		nBlockX = oInit.m_oBlock.shapeMinX(nInitialShape);
		nBlockY = oInit.m_oBlock.shapeMinY(nInitialShape);
	}
	//
	oInit.m_oInitPos.m_nX = oInit.m_oArea.m_nX + (oInit.m_oArea.m_nW - nBlockW) / 2 - nBlockX;
	const auto oPairInitPosX = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSwapperInitXAttr);
	if (oPairInitPosX.first) {
		const std::string& sInitPosX = oPairInitPosX.second;
		oInit.m_oInitPos.m_nX = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSwapperInitXAttr, sInitPosX, false
															, true, oInit.m_oArea.m_nX - nBlockX, true, oInit.m_oArea.m_nX + oInit.m_oArea.m_nW  - nBlockX - nBlockW);
	}
	oInit.m_oInitPos.m_nY = oInit.m_oArea.m_nY + (oInit.m_oArea.m_nH - nBlockH) / 2 - nBlockY;
	const auto oPairInitPosY = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSwapperInitYAttr);
	if (oPairInitPosY.first) {
		const std::string& sInitPosY = oPairInitPosY.second;
		oInit.m_oInitPos.m_nY = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSwapperInitYAttr, sInitPosY, false
															, true, oInit.m_oArea.m_nY - nBlockY, true, oInit.m_oArea.m_nY + oInit.m_oArea.m_nH  - nBlockY - nBlockH);
	}
	;
	const auto oPairTicksToLive = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventSwapperTicksToLiveAttr);
	if (oPairTicksToLive.first) {
		const std::string& sTicksToLive = oPairTicksToLive.second;
		oInit.m_nTicksToLive = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventSwapperTicksToLiveAttr, sTicksToLive, false
																	, true, -1, false, -1);
	}
	;
	const xmlpp::Element* p0ImmovableElement = getXmlConditionalParser().parseUniqueElement(oCtx, p0Element, s_sEventSwapperImmovableNodeName, false);
	if (p0ImmovableElement != nullptr) {
		oInit.m_refImmovable = getXmlTraitsParser().parseTileSelectorAnd(oCtx, p0ImmovableElement);
		//refTileSwapped->dump();
	}
	;
	getXmlConditionalParser().visitNamedElementChildren(oCtx, p0Element, s_sEventSwapperSwappedNodeName, [&](const xmlpp::Element* p0SwappedElement)
	{
		oInit.m_aSwappedSelectors.push_back(getXmlTraitsParser().parseTileSelectorAnd(oCtx, p0SwappedElement));
	});
	;
	const auto oTupleTeam = getXmlConditionalParser().parseOwnerExists(oCtx, p0Element);
	const bool bExists = std::get<0>(oTupleTeam);
	// Note: ignores mate even if defined because blocks can currently only
	//       be restricted to a team not a player
	// Note 2: in OneTeamPerLevel mode it doesn't matter if a block can only
	//         be controlled by players of a certain team since there is only
	//         one team in the level anyway
	const int32_t nTeam = (bExists ? std::get<1>(oTupleTeam) : -1);
	if (oCtx.game().isAllTeamsInOneLevel()) {
		oInit.m_nLevelTeam = nTeam;
	} else {
		oInit.m_nLevelTeam = 0;
	}
	oCtx.removeChecker(p0Element, true);
	auto refSwapperEvent = std::make_unique<SwapperEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refSwapperEvent), p0Element);
}
int32_t XmlSwapperEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "SET_SHAPE") {
		nMsg = SwapperEvent::MESSAGE_SET_SHAPE;
	} else if (sMsgName == "NEXT_SHAPE") {
		nMsg = SwapperEvent::MESSAGE_NEXT_SHAPE;
	} else if (sMsgName == "PREV_SHAPE") {
		nMsg = SwapperEvent::MESSAGE_PREV_SHAPE;
	} else if (sMsgName == "PAUSE") {
		nMsg = SwapperEvent::MESSAGE_PAUSE;
	} else if (sMsgName == "RESUME") {
		nMsg = SwapperEvent::MESSAGE_RESUME;
	} else {
		nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlSwapperEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "PUSH_ROW") {
		nListenerGroup = SwapperEvent::LISTENER_GROUP_PUSH_ROW;
	} else if (sListenerGroupName == "SWAPPED") {
		nListenerGroup = SwapperEvent::LISTENER_GROUP_SWAPPED;
	} else if (sListenerGroupName == "IMMOVABLE") {
		nListenerGroup = SwapperEvent::LISTENER_GROUP_IMMOVABLE;
	} else {
		const auto nSwappedStartSize = s_sEventSwapperSwappedListenerGroupStart.size();
		if (sListenerGroupName.substr(0, nSwappedStartSize) == s_sEventSwapperSwappedListenerGroupStart) {
			int32_t nTotSwappeds = 0;
			getXmlConditionalParser().visitNamedElementChildren(oCtx, p0Element->get_parent(), s_sEventSwapperSwappedNodeName, [&](const xmlpp::Element* /*p0SwappedElement*/)
			{
				++nTotSwappeds;
			});
			if (nTotSwappeds == 0) {
				throw XmlCommonErrors::errorElementExpected(oCtx, p0Element->get_parent(), s_sEventSwapperSwappedNodeName);
			}
			std::string sNr = sListenerGroupName.substr(nSwappedStartSize);
			const int32_t nNr = XmlUtil::strToNumber(oCtx, p0Element, sAttr, sNr, false, true, 0, true, nTotSwappeds - 1);
			nListenerGroup = SwapperEvent::LISTENER_GROUP_SWAPPED_0 + nNr;
		} else {
			nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
		}
	}
	return nListenerGroup;
}

} // namespace stmg
