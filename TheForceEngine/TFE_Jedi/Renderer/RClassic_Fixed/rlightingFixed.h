#pragma once
//////////////////////////////////////////////////////////////////////
// Wall
// Dark Forces Derived Renderer - Wall functions
//////////////////////////////////////////////////////////////////////
#include <TFE_System/types.h>
#include <TFE_Jedi/Math/fixedPoint.h>
#include <TFE_Jedi/Math/core_math.h>

namespace TFE_Jedi
{
	namespace RClassic_Fixed
	{
		struct CameraLight
		{
			vec3_fixed lightWS;
			vec3_fixed lightVS;
			fixed16_16 brightness;
		};
		extern CameraLight s_cameraLight[];

		const u8* computeLighting(fixed16_16 depth, s32 lightOffset);
	}
}