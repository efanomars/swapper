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
 * File:   xmlswapperevent.h
 */

#ifndef STMG_XML_SWAPPER_EVENT_H
#define STMG_XML_SWAPPER_EVENT_H

#include <stmm-games-xml-game/xmleventparser.h>

#include <string>

#include <stdint.h>

namespace stmg { class Event; }
namespace stmg { class GameCtx; }
namespace stmg { class ConditionalCtx; }
namespace xmlpp { class Element; }

namespace stmg
{

class XmlSwapperEventParser : public XmlEventParser
{
public:
	XmlSwapperEventParser();

	Event* parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element) override;

	int32_t parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
							, const std::string& sMsgName) override;
	int32_t parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
										, const std::string& sListenerGroupName) override;

	static const std::string s_sEventSwapperNodeName;

private:
	Event* parseEventSwapper(GameCtx& oCtx, const xmlpp::Element* p0Element);

private:
	XmlSwapperEventParser(const XmlSwapperEventParser& oSource) = delete;
	XmlSwapperEventParser& operator=(const XmlSwapperEventParser& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_XML_SWAPPER_EVENT_H */

