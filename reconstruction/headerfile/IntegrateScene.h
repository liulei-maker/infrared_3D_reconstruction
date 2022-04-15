#pragma once

#include <string>
#include "Helpers.h"
#include "Common.h"
#include "OptimizePoseGraph.h"

#include <Open3D/Open3D.h>

#include <iostream>
#include <tuple>
#include <cstdio>

using namespace open3d;


class IntegrateScene
{
public:
	IntegrateScene();
	~IntegrateScene();

	void Run(std::string path, camera::PinholeCameraIntrinsic intrinsic);
};

