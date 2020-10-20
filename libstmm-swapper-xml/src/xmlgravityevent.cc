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
 * File:   xmlgravityevent.cc
 */

#include "xmlgravityevent.h"

#include <stmm-swapper/gravityevent.h>

#include <stmm-games-xml-game/gamectx.h>

//#include <cassert>
//#include <iostream>
#include <memory>
#include <utility>

namespace stmg
{

const std::string XmlGravityEventParser::s_sEventGravityNodeName = "GravityEvent";

XmlGravityEventParser::XmlGravityEventParser()
: XmlEventParser(s_sEventGravityNodeName)
{
}

Event* XmlGravityEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventGravity(oCtx, p0Element);
}

Event* XmlGravityEventParser::parseEventGravity(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventGravity" << '\n';
	oCtx.addChecker(p0Element);
	GravityEvent::Init oGInit;
	parseEventBase(oCtx, p0Element, oGInit);

	oGInit.m_nRepeat = parseEventAttrRepeat(oCtx, p0Element);
	oGInit.m_nStep = parseEventAttrStep(oCtx, p0Element);
	oCtx.removeChecker(p0Element, true);
	auto refGravityEvent = std::make_unique<GravityEvent>(std::move(oGInit));
	return integrateAndAdd(oCtx, std::move(refGravityEvent), p0Element);
}

} // namespace stmg
