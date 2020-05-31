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
 * File:   xmldestroyevent.cc
 */

#include "xmldestroyerevent.h"

#include <stmm-swapper/destroyerevent.h>

#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>

#include <stmm-games/level.h>
#include <stmm-games/util/util.h>

//#include <cassert>
//#include <iostream>
#include <string>

namespace stmg
{

const std::string XmlDestroyerEventParser::s_sEventDestroyerNodeName = "DestroyerEvent";

static const std::string s_sEventDestroyerRemoveAttr = "remove";
static const std::string s_sEventDestroyerAreaXAttr = "areaX";
static const std::string s_sEventDestroyerAreaYAttr = "areaY";
static const std::string s_sEventDestroyerAreaWAttr = "areaW";
static const std::string s_sEventDestroyerAreaHAttr = "areaH";

static const std::string s_sEventDestroyerMessageRadiusStart = "DESTROY_R";

XmlDestroyerEventParser::XmlDestroyerEventParser()
: XmlEventParser(s_sEventDestroyerNodeName)
{
}

Event* XmlDestroyerEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventDestroyer(oCtx, p0Element);
}

Event* XmlDestroyerEventParser::parseEventDestroyer(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "XmlDestroyerEventParser::parseEventDestroyer" << '\n';
	oCtx.addChecker(p0Element);
	DestroyerEvent::Init oInit;
	parseEventBase(oCtx, p0Element, oInit);
	;
	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	oInit.m_oArea = oXmlBasicParser.parseNRect(oCtx, p0Element
												, s_sEventDestroyerAreaXAttr, s_sEventDestroyerAreaYAttr, s_sEventDestroyerAreaWAttr, s_sEventDestroyerAreaHAttr
												, true, NRect{0, 0, oCtx.level().boardWidth(), oCtx.level().boardHeight()}
												, NSize{DestroyerEvent::s_nAreaMinW, DestroyerEvent::s_nAreaMinH});
	//
	;
	const auto oPairRemove = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventDestroyerRemoveAttr);
	if (oPairRemove.first) {
		const std::string& sRemove = oPairRemove.second;
		oInit.m_bRemove = XmlUtil::strToBool(oCtx, p0Element, s_sEventDestroyerRemoveAttr, sRemove);
	}
	;

	oCtx.removeChecker(p0Element, true);
	auto refDestroyerEvent = std::make_unique<DestroyerEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refDestroyerEvent), p0Element);
}
int32_t XmlDestroyerEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "DESTROY_ROW") {
		nMsg = DestroyerEvent::MESSAGE_DESTROY_ROW;
	} else if (sMsgName == "DESTROY_COLUMN") {
		nMsg = DestroyerEvent::MESSAGE_DESTROY_COLUMN;
	} else if (sMsgName == "DESTROY_ROW_COLUMN") {
		nMsg = DestroyerEvent::MESSAGE_DESTROY_ROW_COLUMN;
	} else {
		const auto nRadiusStartSize = s_sEventDestroyerMessageRadiusStart.size();
		if (sMsgName.substr(0, nRadiusStartSize) == s_sEventDestroyerMessageRadiusStart) {
			std::string sNr = sMsgName.substr(nRadiusStartSize);
			constexpr int32_t nMaxIdx = DestroyerEvent::MESSAGE_DESTROY_RMAX - DestroyerEvent::MESSAGE_DESTROY_R0;
			const int32_t nNr = XmlUtil::strToNumber(oCtx, p0Element, sAttr, sNr, false, true, 0, true, nMaxIdx);
			nMsg = DestroyerEvent::MESSAGE_DESTROY_R0 + nNr;
		} else {
			nMsg = XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
		}
	}
	return nMsg;
}

} // namespace stmg
