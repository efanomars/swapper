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
 * File:   xmltileremoverevent.cc
 */

#include "xmltileremoverevent.h"

#include <stmm-swapper/tileremoverevent.h>

#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>
#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>

#include <stmm-games/level.h>
#include <stmm-games/util/util.h>

//#include <cassert>
//#include <iostream>

namespace stmg
{

static const std::string s_sEventTileRemoverAreaXAttr = "areaX";
static const std::string s_sEventTileRemoverAreaYAttr = "areaY";
static const std::string s_sEventTileRemoverAreaWAttr = "areaW";
static const std::string s_sEventTileRemoverAreaHAttr = "areaH";
static const std::string s_sEventTileRemoverDeleteAfterAttr = "deleteAfter";

XmlTileRemoverEventParser::XmlTileRemoverEventParser(const std::string& sEventName)
: XmlEventParser(sEventName)
{
}
Event* XmlTileRemoverEventParser::parseEvent(GameCtx& /*oCtx*/, const xmlpp::Element* /*p0Element*/)
{
	return nullptr;
}
TileRemoverEvent::Init XmlTileRemoverEventParser::parseTileRemoverEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventTileRemover" << '\n';
	oCtx.addChecker(p0Element);
	TileRemoverEvent::Init oInit;
	parseEventBase(oCtx, p0Element, oInit);

	oInit.m_nRepeat = parseEventAttrRepeat(oCtx, p0Element);
	oInit.m_nStep = parseEventAttrStep(oCtx, p0Element);
	;
	XmlBasicParser oXmlBasicParser(getXmlConditionalParser());
	oInit.m_oArea = oXmlBasicParser.parseNRect(oCtx, p0Element
												, s_sEventTileRemoverAreaXAttr, s_sEventTileRemoverAreaYAttr, s_sEventTileRemoverAreaWAttr, s_sEventTileRemoverAreaHAttr
												, true, NRect{0, 0, oCtx.level().boardWidth(), oCtx.level().boardHeight()}
												, NSize{1, 1});
	;
	const auto oPairDeleteAfter = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventTileRemoverDeleteAfterAttr);
	if (oPairDeleteAfter.first) {
		const std::string& sDeleteAfter = oPairDeleteAfter.second;
		oInit.m_nDeleteAfter = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventTileRemoverDeleteAfterAttr, sDeleteAfter, false
															, true, 0, true, TileRemoverEvent::s_nMaxDeleteAfter);
	}

	oCtx.removeChecker(p0Element, true);
	return oInit;
}

int32_t XmlTileRemoverEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "REMOVED") {
		nListenerGroup = TileRemoverEvent::LISTENER_GROUP_REMOVED;
	} else {
		nListenerGroup = XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg
