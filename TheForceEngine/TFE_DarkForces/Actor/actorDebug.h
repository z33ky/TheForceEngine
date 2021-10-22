#pragma once
//////////////////////////////////////////////////////////////////////
// TFE Specific Actor debugging tools.
//////////////////////////////////////////////////////////////////////
#include <TFE_System/types.h>
#include <TFE_DarkForces/logic.h>
#include <TFE_Jedi/Collision/collision.h>
#include <TFE_Jedi/Sound/soundSystem.h>

namespace TFE_DarkForces
{
	void actorDebug_init();
	void actorDebug_free();
	void actorDebug_clear();
}  // namespace TFE_DarkForces