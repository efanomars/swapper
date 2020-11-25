/*
 * Copyright © 2020  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   xmlcharremoverevent.cc
 */

#include "xmlcharremoverevent.h"

#include <stmm-swapper/tileremoverevent.h>
#include <stmm-swapper/charremoverevent.h>

#include <stmm-games-xml-base/xmlcommonparser.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>
#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmltraitsparser.h>

#include <stmm-games/util/util.h>

//#include <cassert>
//#include <iostream>

namespace stmg
{

const std::string XmlCharRemoverEventParser::s_sEventCharRemoverNodeName = "CharRemoverEvent";
static const std::string s_sEventCharRemoverMinAdjAttr = "minAdj";
static const std::string s_sEventCharRemoverHorizVertAttr = "horizVert";
static const std::string s_sEventCharRemoverIrremovableNodeName = "Irremovable";

XmlCharRemoverEventParser::XmlCharRemoverEventParser()
: XmlTileRemoverEventParser(s_sEventCharRemoverNodeName)
{
}

Event* XmlCharRemoverEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventCharRemover(oCtx, p0Element);
}
static void assignTileRemover(TileRemoverEvent::Init& oTRE, TileRemoverEvent::Init&& oNew)
{
	oTRE = std::move(oNew);
}
Event* XmlCharRemoverEventParser::parseEventCharRemover(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventCharRemover" << '\n';
	oCtx.addChecker(p0Element);
	CharRemoverEvent::Init oInit;

	assignTileRemover(oInit, parseTileRemoverEvent(oCtx, p0Element));

	const auto oPairMinAdj = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventCharRemoverMinAdjAttr);
	if (oPairMinAdj.first) {
		const std::string& sMinAdj = oPairMinAdj.second;
		oInit.m_nMinAdj = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventCharRemoverMinAdjAttr, sMinAdj, false
															, true, 2, false, -1);
	}
	;
	const auto oPairHorizVert = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventCharRemoverHorizVertAttr);
	if (oPairHorizVert.first) {
		oInit.m_bHorizVert = XmlUtil::strToBool(oCtx, p0Element, s_sEventCharRemoverHorizVertAttr, oPairHorizVert.second);
	}
	;
	const xmlpp::Element* p0IrremovableElement = getXmlConditionalParser().parseUniqueElement(oCtx, p0Element, s_sEventCharRemoverIrremovableNodeName, false);
	if (p0IrremovableElement != nullptr) {
		oInit.m_refIrremovable = getXmlTraitsParser().parseTileSelectorAnd(oCtx, p0IrremovableElement);
	}

	oCtx.removeChecker(p0Element, true);
	auto refCharRemoverEvent = std::make_unique<CharRemoverEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refCharRemoverEvent), p0Element);
}

} // namespace stmg
