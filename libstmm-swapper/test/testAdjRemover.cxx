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
 * File:   testAdjRemover.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "adjremover.h"

#include <stmm-games-fake/fixtureTestBase.h>
#include <stmm-games/utile/tilebuffer.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

class AdjRemoverFixture : public TestBaseFixture
{
public:
	AdjRemoverFixture()
	: m_oBoard(NSize{6,4})
	{
	}
	const Tile& boardGetTile(int32_t nX, int32_t nY) const
	{
		if ((nX < 0) || (nY < 0) || (nX >= m_oBoard.getW()) || (nY >= m_oBoard.getH())) {
			return s_oEmptyTile;
		}
		return m_oBoard.get(NPoint{nX, nY});
	}
	void boardSetTile(int32_t nX, int32_t nY, const Tile& oTile)
	{
		if ((nX < 0) || (nY < 0) || (nX >= m_oBoard.getW()) || (nY >= m_oBoard.getH())) {
			return;
		}
		return m_oBoard.set(NPoint{nX, nY}, oTile);
	}
	void boardSetTiles(const NRect& oRect, const Tile& oTile)
	{
		for (int32_t nX = oRect.m_nX; nX < oRect.m_nX + oRect.m_nW; ++ nX) {
			for (int32_t nY = oRect.m_nY; nY < oRect.m_nY + oRect.m_nH; ++ nY) {
				boardSetTile(nX, nY, oTile);
			}
		}
	}
	void boardSetAllTiles(const Tile& oTile)
	{
		NRect oRect;
		oRect.m_nW = m_oBoard.getW();
		oRect.m_nH = m_oBoard.getH();
		boardSetTiles(oRect, oTile);
	}
	bool boardGetBusy(int32_t nX, int32_t nY) const
	{
		if ((nX < 0) || (nY < 0) || (nX >= m_oBoard.getW()) || (nY >= m_oBoard.getH())) {
			return false;
		}
		return m_oBusy.contains(nX, nY);
	}
	void boardSetBusy(int32_t nX, int32_t nY, bool bBusy)
	{
		if (bBusy) {
			m_oBusy.add(nX, nY);
		} else {
			m_oBusy.remove(nX, nY);
		}
	}
	//
	void getToRemove(NPoint oFromXY, const NRect& oArea, Coords& oToRemove)
	{
		AdjRemover::getCoordsToRemove(oFromXY, oArea, oToRemove
		, [&](int32_t nX, int32_t nY) -> const Tile&
		{
			return boardGetTile(nX, nY);
		}, [&](int32_t nX, int32_t nY) -> bool
		{
			return boardGetBusy(nX, nY);
		}, [&](const Tile& oTile1, const Tile& oTile2) -> bool
		{
			return (oTile1.getTileColor() == oTile2.getTileColor());
		});
	}
	void getToRemoveHorizVert(NPoint oFromXY, int32_t nMinSize, const NRect& oArea, Coords& oToRemove)
	{
		AdjRemover::getCoordsToRemoveHorizVert(oFromXY, nMinSize, oArea, oToRemove
		, [&](int32_t nX, int32_t nY) -> const Tile&
		{
			return boardGetTile(nX, nY);
		}, [&](int32_t nX, int32_t nY) -> bool
		{
			return boardGetBusy(nX, nY);
		}, [&](const Tile& oTile1, const Tile& oTile2) -> bool
		{
			return (oTile1.getTileColor() == oTile2.getTileColor());
		});
	}
protected:
	/** The board size
	 * @return The size. Default is 6x5.
	 */
	virtual NSize boardGetSize() const
	{
		return NSize{6,5};
	}
	void setup() override
	{
		TestBaseFixture::setup();
		m_oBoard.reInit(boardGetSize());
		m_oBusy.reInit();
	}
	void teardown() override
	{
		TestBaseFixture::teardown();
	}
protected:
	static const Tile s_oEmptyTile;
private:
	TileBuffer m_oBoard;
	Coords m_oBusy;
};
const Tile AdjRemoverFixture::s_oEmptyTile{};

TEST_CASE_METHOD(STFX<AdjRemoverFixture>, "AllSame")
{
	Tile oTile;
	oTile.getTileColor().setColorIndex(55);
	boardSetAllTiles(oTile);
	NRect oRect;
	oRect.m_nW = 3;
	oRect.m_nH = 4;
	Coords aToRemove;
	getToRemove(NPoint{0,0}, oRect, aToRemove);
	REQUIRE( aToRemove.size() == 3 * 4 );
	//
	aToRemove.reInit();
	getToRemoveHorizVert(NPoint{0,0}, 3, oRect, aToRemove);
	REQUIRE( aToRemove.size() == 3 * 4 );
}
TEST_CASE_METHOD(STFX<AdjRemoverFixture>, "TwoColors")
{
	Tile oTile;
	oTile.getTileColor().setColorIndex(55);
	boardSetAllTiles(oTile);
	NRect oRect;
	//
	oRect.m_nW = 3;
	oRect.m_nH = 5;
	oTile.getTileColor().setColorIndex(77);
	boardSetTiles(oRect, oTile);
	//
	oRect.m_nX = 0;
	oRect.m_nY = 0;
	oRect.m_nW = 6;
	oRect.m_nH = 5;
	Coords aToRemove;
	getToRemove(NPoint{2,2}, oRect, aToRemove);
	REQUIRE( aToRemove.size() == 3 * 5 );
	//
	aToRemove.reInit();
	getToRemoveHorizVert(NPoint{2,2}, 3, oRect, aToRemove);
	REQUIRE( aToRemove.size() == 3 * 5 );
}
TEST_CASE_METHOD(STFX<AdjRemoverFixture>, "ThreeColors")
{
	Tile oTile;
	oTile.getTileColor().setColorIndex(55);
	boardSetAllTiles(oTile);
	//
	NRect oRect;
	oRect.m_nW = 2;
	oRect.m_nH = 5;
	oTile.getTileColor().setColorIndex(77);
	boardSetTiles(oRect, oTile);
	//
	oRect.m_nX = 2;
	oRect.m_nY = 0;
	oRect.m_nW = 2;
	oRect.m_nH = 5;
	oTile.getTileColor().setColorIndex(88);
	boardSetTiles(oRect, oTile);
	//
	oRect.m_nX = 0;
	oRect.m_nY = 0;
	oRect.m_nW = 6;
	oRect.m_nH = 5;
	Coords aToRemove;
	getToRemove(NPoint{2,2}, oRect, aToRemove);
	REQUIRE( aToRemove.size() == 2 * 5 );
	//
	aToRemove.reInit();
	getToRemoveHorizVert(NPoint{2,2}, 3, oRect, aToRemove);
	REQUIRE( aToRemove.size() == 1 * 5 );
}


} // namespace testing

} // namespace stmg
