#pragma once

#include <string>
#include "Helpers.h"
#include "Common.h"
#include "OptimizePoseGraph.h"
#include <Windows.h>

#include <Open3D/Registration/FastGlobalRegistration.h>


#include <iostream>
#include <tuple>
#include <cstdio>

using namespace open3d;


class RegisterFragments
{
public:
	RegisterFragments();
	~RegisterFragments();

	void Run(std::string path);
};

