#pragma once

#include <iostream>
#include <tuple>
#include <cstdio>
#include <string>

#include "Helpers.h"
#include "Common.h"

#include <Open3D/Open3D.h>
#include <Open3D/Registration/FastGlobalRegistration.h>


using namespace open3d;

namespace open3d
{
	class PinholeCameraIntrinsic;
}

class RealtimeSystem
{
public:
	RealtimeSystem();
	~RealtimeSystem();

	void Run(std::string path, camera::PinholeCameraIntrinsic intrinsic);
};

