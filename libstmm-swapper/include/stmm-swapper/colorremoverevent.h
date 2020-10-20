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
 * File:   colorremoverevent.h
 */

#ifndef STMG_COLOR_REMOVER_EVENT_H_
#define STMG_COLOR_REMOVER_EVENT_H_

#include "tileremoverevent.h"

//#include <stmm-games/util/coords.h>
//#include <stmm-games/util/basictypes.h>
//#include <stmm-games/utile/extendedboard.h>

#include <stdint.h>

namespace stmg { class Coords; }
namespace stmg { struct NPoint; }
namespace stmg { struct NRect; }

namespace stmg
{

class ExtendedBoard;

class ColorRemoverEvent : public TileRemoverEvent
{
public:
	static constexpr int32_t s_nDefaultMinAdj = 4;

	struct LocalInit
	{
		int32_t m_nMinAdj = s_nDefaultMinAdj; /**< The minimum number of adjacent
												 * "same" tiles needed for them to be removed. Must be &gt; 1. */
		bool m_bHorizVert = false; /**< Whether only vertical and horitontal tiles
									 * are considered adjacent. Default is false. */
		//TODO Combine the following with a new event that decreases the char number of a tile
		// (count down) and when 0 the char is set to empty. The tiles with char 1..9
		// are unbreakable. So if one of the two in adjSame in unbreakable false is returned.
		//unique_ptr<TileSelector> m_refUnbreakable; /* *< The tiles that cannot be removed. */
	};
	struct Init : public TileRemoverEvent::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit The initialization data.
	 */
	explicit ColorRemoverEvent(Init&& oInit) noexcept;

protected:
	/** Reinitialization.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;

	bool getCoordsToRemove(NPoint oFromXY, const NRect& oArea, Coords& aToRemove) noexcept override;
	bool getCoordsToRemove(NPoint oFromXY, const NRect& oArea, const ExtendedBoard& oBoard, Coords& aToRemove) noexcept override;
private:
	LocalInit m_oData;

private:
	ColorRemoverEvent() = delete;
	ColorRemoverEvent(const ColorRemoverEvent& oSource) = delete;
	ColorRemoverEvent& operator=(const ColorRemoverEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_COLOR_REMOVER_EVENT_H_ */

