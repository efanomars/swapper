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
 * File:   setup.h
 */

#ifndef STMG_SWAPPER_SETUP_H
#define STMG_SWAPPER_SETUP_H

#include <string>

namespace stmg { struct MainWindowData; }

namespace stmg
{

/* The setup.
 * @return The error string or empty if no error.
 */
std::string swapperSetup(MainWindowData& oMainWindowData, const std::string& sSwapper, const std::string& sAppVersion
						, bool bNoSound, bool bTestMode, bool bFullScreen) noexcept;

} // namespace stmg

#endif	/* STMG_SWAPPER_SETUP_H */

