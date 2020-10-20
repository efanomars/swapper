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
 * File:   destroyerevent.h
 */

#ifndef STMG_DESTROYER_EVENT_H
#define STMG_DESTROYER_EVENT_H

//#include <stmm-games/level.h>
#include <stmm-games/event.h>
#include <stmm-games/util/circularbuffer.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/util/coords.h>

#include <stdint.h>

namespace stmg
{

/** Instances of this class destroy tiles around a specific position.
 */
class DestroyerEvent : public Event
{
public:
	struct LocalInit
	{
		NRect m_oArea; /**< the area the destruction of cell should be limited to. Default: all board. */
		bool m_bRemove = false; /**< Whether to remove rather than destroy tiles. Default is false. */
		//int32_t m_nPerTileAnimationId = -1;
		//float m_fPerTileAnimationSize = 1.0;
		//int32_t m_nCenterAnimationId = -1;
		//float m_fCenterAnimationSize = -1.0; /**< -1 means depends on input. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit The initialization data.
	 */
	explicit DestroyerEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	// Event iface
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;

	// input
	/** All messages that have a postion expect nValue to be packed with Util::packPointToInt32(x,y).
	 */
	enum {
		MESSAGE_DESTROY_ROW = 100 /**< Destroy row of (x,y) passed through nValue. */
		, MESSAGE_DESTROY_COLUMN = 101 /**< Destroy column of (x,y) passed through nValue. */
		, MESSAGE_DESTROY_ROW_COLUMN = 102 /**< Destroy row and column of (x,y) passed through nValue. */
		, MESSAGE_DESTROY_R0 = 150 /**< Destroy tile at (x,y) passed through nValue. */
		, MESSAGE_DESTROY_R1 = 151 /**< Destroy tiles at &lt;= 1 tiles distance from (x,y) passed through nValue. */
		, MESSAGE_DESTROY_R2 = 152 /**< Destroy tiles at &lt;= 2 tiles distance from (x,y) passed through nValue. */
		, MESSAGE_DESTROY_R3 = 153 /**< Destroy tiles at &lt;= 3 tiles distance from (x,y) passed through nValue. */
		, MESSAGE_DESTROY_R4 = 154 /**< Destroy tiles at &lt;= 4 tiles distance from (x,y) passed through nValue. */
		, MESSAGE_DESTROY_R5 = 155 /**< Destroy tiles at &lt;= 5 tiles distance from (x,y) passed through nValue. */
		, MESSAGE_DESTROY_RMAX = 155 /**<  */
	};
	// Outputs
	//enum {
	//};

	static constexpr const int32_t s_nAreaMinW = 1;
	static constexpr const int32_t s_nAreaMinH = 1;
private:
	void commonInit() noexcept;

	struct ToExplode
	{
		NPoint m_oOrigin;
		int32_t m_nMsg;
	};
	void createExplosionDestroy(const ToExplode& oTE) noexcept;
	void createExplosionRemove(const ToExplode& oTE) noexcept;
	bool createExplosion(Coords& oCoords, const ToExplode& oTE) const noexcept;
	void addRow(Coords& oCoords, int32_t nY) const noexcept;
	void addColumn(Coords& oCoords, int32_t nX) const noexcept;
	void addCircle(Coords& oCoords, int32_t nX, int32_t nY, int32_t nRadius) const noexcept;

	enum DESTROYER_STATE
	{
		DESTROYER_STATE_ACTIVATE = 0
		, DESTROYER_STATE_RUN = 1
	};
private:
	LocalInit m_oData;

	DESTROYER_STATE m_eState;

	CircularBuffer<ToExplode> m_oToExplodes;
private:
	DestroyerEvent() = delete;
	DestroyerEvent(const DestroyerEvent& oSource) = delete;
	DestroyerEvent& operator=(const DestroyerEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_DESTROYER_EVENT_H */

