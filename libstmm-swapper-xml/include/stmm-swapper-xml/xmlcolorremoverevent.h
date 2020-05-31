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
 * File:   xmlcolorremoverevent.h
 */

#ifndef STMG_XML_COLOR_REMOVER_EVENT_H
#define STMG_XML_COLOR_REMOVER_EVENT_H

#include "xmltileremoverevent.h"

#include <string>

namespace stmg { class Event; }
namespace stmg { class GameCtx; }
namespace xmlpp { class Element; }

namespace stmg
{

class XmlColorRemoverEventParser : public XmlTileRemoverEventParser
{
public:
	XmlColorRemoverEventParser();

	Event* parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element) override;

	static const std::string s_sEventColorRemoverNodeName;

private:
	Event* parseEventColorRemover(GameCtx& oCtx, const xmlpp::Element* p0Element);

private:
	XmlColorRemoverEventParser(const XmlColorRemoverEventParser& oSource) = delete;
	XmlColorRemoverEventParser& operator=(const XmlColorRemoverEventParser& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_XML_COLOR_REMOVER_EVENT_H */

