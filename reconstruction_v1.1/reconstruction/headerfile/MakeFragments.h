#pragma once

#include <string>
#include <iostream>
#include <tuple>
#include <cstdio>

#include "Helpers.h"
#include "Common.h"
#include "OptimizePoseGraph.h"

#include <Open3D/Open3D.h>
#include <Open3D/Registration/FastGlobalRegistration.h>


using namespace open3d;


class MakeFragments
{
public:
	MakeFragments();
	~MakeFragments();

	void Run(std::string path, camera::PinholeCameraIntrinsic intrinsic);
private:


};

