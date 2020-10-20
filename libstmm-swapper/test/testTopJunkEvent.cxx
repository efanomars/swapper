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
 * File:   testTopJunkEvent.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "topjunkevent.h"

#include <stmm-games-fake/fixtureGame.h>
#include <stmm-games-fake/fakelevelview.h>
#include <stmm-games-fake/mockevent.h>

#include <stmm-games/events/logevent.h>
#include <stmm-games/traitsets/tiletraitsets.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

class TopJunkEventGameFixture : public GameFixture
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

class TopJunkEventCase1GameFixture : public TopJunkEventGameFixture
{
protected:
	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		TopJunkEventGameFixture::fillBoard(nBoardW, nBoardH, aBoard);
		Tile oTile;
		oTile.getTileChar().setChar(65);
		aBoard[Level::Init::getBoardIndex(NPoint{3,2}, NSize{nBoardW, nBoardH})] = oTile;
	}

	virtual int32_t getMaterializingTicks() const
	{
		return 2;
	}
	virtual int32_t getWaitTicks() const
	{
		return 0;
	}
	virtual int32_t getMaxJunkPerDrop() const
	{
		return 1;
	}
	void setup() override
	{
		TopJunkEventGameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		TopJunkEvent::Init oTJInit;
		oTJInit.m_p0Level = p0Level;
		oTJInit.m_nMaterializingTicksFrom = getMaterializingTicks();
		oTJInit.m_nMaterializingTicksTo = oTJInit.m_nMaterializingTicksFrom;
		oTJInit.m_nWaitMillisecFrom = getWaitTicks();
		oTJInit.m_nWaitMillisecTo = oTJInit.m_nWaitMillisecFrom;
		oTJInit.m_nMaxJunkPerDrop = getMaxJunkPerDrop();
		{
			RandomTiles::ProbTileGen oProbTileGen;
			std::vector< std::unique_ptr<TraitSet> > aTraitSets;
			std::unique_ptr<CharUcs4TraitSet> refITS = std::make_unique<CharUcs4TraitSet>(65,67);
			auto refCharTraitSet = std::make_unique<CharTraitSet>(std::move(refITS));
			aTraitSets.push_back(std::move(refCharTraitSet));
			RandomTiles::ProbTraitSets oPTS;
			oPTS.m_nProb = 10;
			oPTS.m_aTraitSets = std::move(aTraitSets);
			oProbTileGen.m_aProbTraitSets.push_back(std::move(oPTS));
			//
			oTJInit.m_aProbTileGens.push_back(std::move(oProbTileGen));
		}
		auto refTopJunkEvent = make_unique<TopJunkEvent>(std::move(oTJInit));
		m_p0TopJunkEvent = refTopJunkEvent.get();
		p0Level->addEvent(std::move(refTopJunkEvent));
		p0Level->activateEvent(m_p0TopJunkEvent, 0);
	}
	void teardown() override
	{
		TopJunkEventGameFixture::teardown();
	}
protected:
	shared_ptr<Level> m_refLevel;
	TopJunkEvent* m_p0TopJunkEvent;
};


TEST_CASE_METHOD(STFX<TopJunkEventCase1GameFixture>, "Case1_Choose")
{
	Level* p0Level = m_refLevel.get();
	const int32_t nBoardW = p0Level->boardWidth();

	const int32_t nMaterializingIdx = p0Level->getNamed().tileAnis().addName("TILEANI:MATERIALIZING");

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 64738;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );
	
	LogEvent::msgLog().reset();
	
	m_p0TopJunkEvent->addListener(TopJunkEvent::LISTENER_GROUP_PLACED_JUNK, p0LogEvent, 55);

	MockEvent::Init oMockInit;
	oMockInit.m_p0Level = p0Level;
	auto refMockEvent = make_unique<MockEvent>(std::move(oMockInit));
	MockEvent* p0MockEvent = refMockEvent.get();
	p0Level->addEvent(std::move(refMockEvent));

	p0MockEvent->addListener(8889, m_p0TopJunkEvent, TopJunkEvent::MESSAGE_DROP_JUNK);

	auto oCheckTiles = [&](int32_t nExpectedTiles, int32_t nExpectedTileAnimators) {
		const int32_t nTopY = 1;
		int32_t nNotEmpty = 0;
		int32_t nTotTileAnimators = 0;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			const Tile& oTile = p0Level->boardGetTile(nX, nTopY);
			const TileAnimator* p0TA = p0Level->boardGetTileAnimator(nX, nTopY, nMaterializingIdx);
			if (! oTile.isEmpty()) {
				++nNotEmpty;
				if (p0TA == m_p0TopJunkEvent) {
					++nTotTileAnimators;
				}
			} else {
				REQUIRE( p0TA == nullptr);
			}
		}
		REQUIRE(nNotEmpty == nExpectedTiles);
		REQUIRE(nTotTileAnimators == nExpectedTileAnimators);
	};
	
	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 1 );

	const int32_t nJunkTiles = 4;
	p0MockEvent->setTriggerValue(8889, nJunkTiles, 1); // group 8889, value, trigger after one tick

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	oCheckTiles(0, 0);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 3 );

	oCheckTiles(1, 1);
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE( oEntry.m_nGameTick == 2 );
	}

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 4 );

	oCheckTiles(2, 2);
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE( oEntry.m_nGameTick == 3 );
	}

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 5 );

	oCheckTiles(3, 2);
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE( oEntry.m_nGameTick == 4 );
	}

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 6 );

	oCheckTiles(4, 2);
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE( oEntry.m_nGameTick == 5 );
	}

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 7 );

	oCheckTiles(4, 1);

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 8 );

	oCheckTiles(4, 0);

	const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
	REQUIRE_FALSE( oEntry.isEmpty() );
	REQUIRE( oEntry.m_nTag == 64738 );
	REQUIRE( oEntry.m_nGameTick == 5 );
	REQUIRE( oEntry.m_nLevel == 0 );
	REQUIRE( oEntry.m_nMsg == 55 );
	REQUIRE( oEntry.m_nValue == 1 );
	REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0TopJunkEvent) );
}


} // namespace testing

} // namespace stmg
