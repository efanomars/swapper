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
 * File:   setupstdconfig.cc
 */

#include "setupstdconfig.h"

#include <stmm-swapper/swapperevent.h>

#include <stmm-games/stdconfig.h>
#include <stmm-games/options/booloption.h>

#include <stmm-games/appconstraints.h>

#include <stmm-input-ev/stmm-input-ev.h>

#include <stmm-input/hardwarekey.h>

#include <stmm-input/devicemanager.h>

//#include <iostream>
#include <cassert>
#include <vector>


namespace stmg
{

void swapperSetupStdConfig(shared_ptr<StdConfig>& refStdConfig, const shared_ptr<stmi::DeviceManager>& refDeviceManager
								, const std::string& sSwapper, const std::string& sAppVersion
								, bool bNoSound, bool bTestMode, bool bTouchMode) noexcept
{
	assert(refStdConfig.get() == nullptr);
	StdConfig::Init oStdConfigInit;
	oStdConfigInit.m_sAppName = sSwapper;
	oStdConfigInit.m_sAppVersion = sAppVersion;
	oStdConfigInit.m_bTestMode = bTestMode;

	oStdConfigInit.m_bSoundEnabled = ! bNoSound;
	oStdConfigInit.m_bSoundPerPlayerAllowed = ! bNoSound; // Per player sound is useful for team vs team games

	{
		{
		const bool bReadonly = ! bTestMode;
		const bool bVisible = bTestMode;
		auto refOption = std::make_shared<BoolOption>(OwnerType::GAME, "TouchMode", bTouchMode, "TouchMode"
													, bReadonly, bVisible, shared_ptr<Option>{}, std::vector<Variant>{});
		oStdConfigInit.m_aOptions.push_back(refOption);
		}
	}

	oStdConfigInit.m_refDeviceManager = refDeviceManager;

	AppConstraints& oAppConstraints = oStdConfigInit.m_oAppConstraints;
	oAppConstraints.m_nAIMatesPerTeamMax = 0;
	if (bTouchMode) {
		oAppConstraints.m_nPlayersMax = 1;
		oAppConstraints.m_nTeamsMax = 1;
		oAppConstraints.m_nMatesPerTeamMax = 1;
	}

	StdConfig::CapabilityAssignment& oCapabilityAssignment = oStdConfigInit.m_oCapabilityAssignment;
	oCapabilityAssignment.m_bCapabilitiesAutoAssignedToActivePlayer = true;

	std::vector<StdConfig::KeyAction>& aKeyActions = oStdConfigInit.m_aKeyActions;
	std::vector< std::pair<stmi::Capability::Class, std::vector<stmi::HARDWARE_KEY> > > aDefaultClassKeys;
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_UP, stmi::HK_W}));
	aKeyActions.emplace_back(std::vector<std::string>{SwapperEvent::s_sKeyActionUp}, "Up", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_LEFT, stmi::HK_A}));
	aKeyActions.emplace_back(std::vector<std::string>{SwapperEvent::s_sKeyActionLeft}, "Left", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_RIGHT, stmi::HK_D}));
	aKeyActions.emplace_back(std::vector<std::string>{SwapperEvent::s_sKeyActionRight}, "Right", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_DOWN, stmi::HK_S}));
	aKeyActions.emplace_back(std::vector<std::string>{SwapperEvent::s_sKeyActionDown}, "Down", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_B, stmi::HK_1}));
	aKeyActions.emplace_back(std::vector<std::string>{SwapperEvent::s_sKeyActionPushRow}, "Push row", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_SPACE, stmi::HK_Q}));
	aKeyActions.emplace_back(std::vector<std::string>{SwapperEvent::s_sKeyActionSwap}, "Swap", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_N, stmi::HK_TAB}));
	aKeyActions.emplace_back(std::vector<std::string>{SwapperEvent::s_sKeyActionNext}, "Next", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	refStdConfig = std::make_shared<StdConfig>(std::move(oStdConfigInit));
}

} //namespace stmg

