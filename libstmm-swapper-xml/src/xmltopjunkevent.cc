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
 * File:   xmltopjunkevent.cc
 */

#include "xmltopjunkevent.h"

#include <stmm-swapper/topjunkevent.h>

#include <stmm-games-xml-base/xmlcommonerrors.h>
#include <stmm-games-xml-game/xmlutile/xmlprobtilegenparser.h>
#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmltraitsparser.h>

#include <stmm-games/level.h>
#include <stmm-games/util/util.h>

//#include <cassert>
//#include <iostream>

#include <string>

namespace stmg
{

const std::string XmlTopJunkEventParser::s_sEventTopJunkNodeName = "TopJunkEvent";
static const std::string s_sEventTopJunkMaterializingTicksAttr = "materializingTicks";
static const std::string s_sEventTopJunkMaterializingTicksFromAttr = "materializingTicksFrom";
static const std::string s_sEventTopJunkMaterializingTicksToAttr = "materializingTicksTo";
static const std::string s_sEventTopJunkMaterializingMillisecAttr = "materializingMillisec";
static const std::string s_sEventTopJunkMaterializingMillisecFromAttr = "materializingMillisecFrom";
static const std::string s_sEventTopJunkMaterializingMillisecToAttr = "materializingMillisecTo";
static const std::string s_sEventTopJunkWaitTicksAttr = "waitTicks";
static const std::string s_sEventTopJunkWaitTicksFromAttr = "waitTicksFrom";
static const std::string s_sEventTopJunkWaitTicksToAttr = "waitTicksTo";
static const std::string s_sEventTopJunkWaitMillisecAttr = "waitMillisec";
static const std::string s_sEventTopJunkWaitMillisecFromAttr = "waitMillisecFrom";
static const std::string s_sEventTopJunkWaitMillisecToAttr = "waitMillisecTo";
static const std::string s_sEventTopJunkMaxJunkPerDropAttr = "maxJunkPerDrop";
static const std::string s_sEventTopJunkTopYAttr = "topY";
static const std::string s_sEventTopJunkProbTileGenNodeName = "TileGen";

XmlTopJunkEventParser::XmlTopJunkEventParser()
: XmlEventParser(s_sEventTopJunkNodeName)
{
}

Event* XmlTopJunkEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventTopJunk(oCtx, p0Element);
}

Event* XmlTopJunkEventParser::parseEventTopJunk(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventTopJunk" << '\n';
	oCtx.addChecker(p0Element);
	TopJunkEvent::Init oGInit;
	parseEventBase(oCtx, p0Element, oGInit);

	getXmlConditionalParser().parseAttributeFromTo(oCtx, p0Element, s_sEventTopJunkMaterializingTicksAttr
								, s_sEventTopJunkMaterializingTicksFromAttr, s_sEventTopJunkMaterializingTicksToAttr, false
								, true, 0, false, -1, oGInit.m_nMaterializingTicksFrom, oGInit.m_nMaterializingTicksTo);
	getXmlConditionalParser().parseAttributeFromTo(oCtx, p0Element, s_sEventTopJunkMaterializingMillisecAttr
								, s_sEventTopJunkMaterializingMillisecFromAttr, s_sEventTopJunkMaterializingMillisecToAttr, false
								, true, 0, false, -1, oGInit.m_nMaterializingMillisecFrom, oGInit.m_nMaterializingMillisecTo);
	;
	getXmlConditionalParser().parseAttributeFromTo(oCtx, p0Element, s_sEventTopJunkWaitTicksAttr
								, s_sEventTopJunkWaitTicksFromAttr, s_sEventTopJunkWaitTicksToAttr, false
								, true, 0, false, -1, oGInit.m_nWaitTicksFrom, oGInit.m_nWaitTicksTo);
	getXmlConditionalParser().parseAttributeFromTo(oCtx, p0Element, s_sEventTopJunkWaitMillisecAttr
								, s_sEventTopJunkWaitMillisecFromAttr, s_sEventTopJunkWaitMillisecToAttr, false
								, true, 1, false, -1, oGInit.m_nWaitMillisecFrom, oGInit.m_nWaitMillisecTo);
	;
	const auto oPairMaxJunkPerDrop = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventTopJunkMaxJunkPerDropAttr);
	if (oPairMaxJunkPerDrop.first) {
		const std::string& sMaxJunkPerDrop = oPairMaxJunkPerDrop.second;
		oGInit.m_nMaxJunkPerDrop = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventTopJunkMaxJunkPerDropAttr
																, sMaxJunkPerDrop, false, true, 1, false, -1);
	}
	;
	const auto oPairTopY = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventTopJunkTopYAttr);
	if (oPairTopY.first) {
		const std::string& sTopY = oPairTopY.second;
		oGInit.m_nTopY = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventTopJunkTopYAttr
														, sTopY, false, true, 0, true, oCtx.level().boardHeight() - 1 );
	}
	;
	XmlProbTileGenParser oXmlTileGen{getXmlConditionalParser(), getXmlTraitsParser()};
	getXmlConditionalParser().visitNamedElementChildren(oCtx, p0Element, s_sEventTopJunkProbTileGenNodeName, [&](const xmlpp::Element* p0GenElement)
	{
		auto oNewRowTileGen = oXmlTileGen.parseProbTileGen(oCtx, p0GenElement);
		oGInit.m_aProbTileGens.emplace_back(std::move(oNewRowTileGen));
	});
	if (oGInit.m_aProbTileGens.empty()) {
		throw XmlCommonErrors::errorElementExpected(oCtx, p0Element, s_sEventTopJunkProbTileGenNodeName);
	}

	oCtx.removeChecker(p0Element, true);
	auto refTopJunkEvent = std::make_unique<TopJunkEvent>(std::move(oGInit));
	return integrateAndAdd(oCtx, std::move(refTopJunkEvent), p0Element);
}
int32_t XmlTopJunkEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "DROP_JUNK") {
		nMsg = TopJunkEvent::MESSAGE_DROP_JUNK;
	} else if (sMsgName == "SET_TILE_GEN") {
		nMsg = TopJunkEvent::MESSAGE_SET_TILE_GEN;
	} else if (sMsgName == "NEXT_TILE_GEN") {
		nMsg = TopJunkEvent::MESSAGE_NEXT_TILE_GEN;
	} else if (sMsgName == "PREV_TILE_GEN") {
		nMsg = TopJunkEvent::MESSAGE_PREV_TILE_GEN;
	} else if (sMsgName == "PAUSE") {
		nMsg = TopJunkEvent::MESSAGE_PAUSE;
	} else if (sMsgName == "RESUME") {
		nMsg = TopJunkEvent::MESSAGE_RESUME;
	} else {
		nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlTopJunkEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "PLACED_JUNK") {
		nListenerGroup = TopJunkEvent::LISTENER_GROUP_PLACED_JUNK;
	} else {
		nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg
