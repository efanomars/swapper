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
 * File:   testGravityEvent.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "gravityevent.h"

#include <stmm-games-fake/fixtureGame.h>
#include <stmm-games-fake/fakelevelview.h>

#include <stmm-games/events/sysevent.h>
#include <stmm-games/events/logevent.h>
//#include <stmm-games/tiletraitsets.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

class GravityEventGameFixture : public GameFixture
							//default , public FixtureVariantDevicesKeys_Two, public FixtureVariantDevicesJoystick_Two
							, public FixtureVariantPrefsTeams<1>
							, public FixtureVariantPrefsMates<0,1>
							//default , public FixtureVariantMatesPerTeamMax_Three, public FixtureVariantAIMatesPerTeamMax_Zero
							//default , public FixtureVariantAllowMixedAIHumanTeam_False, public FixtureVariantPlayersMax_Six
							//default , public FixtureVariantTeamsMin_One, public FixtureVariantTeamsMax_Two
							//default , public FixtureVariantLayoutTeamDistribution_AllTeamsInOneLevel
							//default , public FixtureVariantLayoutShowMode_Show
							//default , public FixtureVariantLayoutCreateVarWidgetsFromVariables_False
							//default , public FixtureVariantLayoutCreateActionWidgetsFromKeyActions_False
							, public FixtureVariantVariablesGame_Time
							//, public FixtureVariantVariablesTeam
							, public FixtureVariantVariablesPlayer_Lives<3>
							//default , public FixtureVariantLevelInitBoardWidth<10>
							//default , public FixtureVariantLevelInitBoardHeight<6>
							//default , public FixtureVariantLevelInitShowWidth<10>
							//default , public FixtureVariantLevelInitShowHeight<6>
{
protected:
	void setup() override
	{
		GameFixture::setup();
	}
	void teardown() override
	{
		GameFixture::teardown();
	}

	void fillBoard(int32_t /*nBoardW*/, int32_t /*nBoardH*/, std::vector<Tile>& /*aBoard*/) override
	{
	}
};

class GravityEventCase1GameFixture : public GravityEventGameFixture
{
protected:
	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		GravityEventGameFixture::fillBoard(nBoardW, nBoardH, aBoard);
		Tile oTile;
		oTile.getTileChar().setChar(65);
		aBoard[Level::Init::getBoardIndex(NPoint{3,2}, NSize{nBoardW, nBoardH})] = oTile;
	}

	void setup() override
	{
		GravityEventGameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		GravityEvent::Init oGInit;
		oGInit.m_p0Level = p0Level;
		oGInit.m_nRepeat = -1;
		oGInit.m_nStep = 1;
		auto refGravityEvent = make_unique<GravityEvent>(std::move(oGInit));
		m_p0GravityEvent = refGravityEvent.get();
		p0Level->addEvent(std::move(refGravityEvent));
		p0Level->activateEvent(m_p0GravityEvent, 0);
	}
	void teardown() override
	{
		GravityEventGameFixture::teardown();
	}
protected:
	shared_ptr<Level> m_refLevel;
	GravityEvent* m_p0GravityEvent;
};


TEST_CASE_METHOD(STFX<GravityEventCase1GameFixture>, "Case1_Down")
{
	Level* p0Level = m_refLevel.get();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 64738;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	LogEvent::msgLog().reset();

	m_p0GravityEvent->addListener(GravityEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 55);

	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place on board
	REQUIRE( m_refGame->gameElapsed() == 1 );
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		LevelBlock* p0LB = aLBs[0];
		assert(p0LB != nullptr);
		const NPoint oPos = p0LB->blockPos();
		REQUIRE( oPos.m_nX == 3);
		REQUIRE( oPos.m_nY == 2);
		REQUIRE( p0LB->blockBricksTotVisible() == 1);
	}
}


} // namespace testing

} // namespace stmg
