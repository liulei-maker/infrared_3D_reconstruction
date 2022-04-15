#pragma once

#include <string>
#include "Helpers.h"
#include "Common.h"

#include <Open3D/Registration/GlobalOptimization.h>
#include <Open3D/Open3D.h>

using namespace open3d;
class OptimizePoseGraph
{
public:
	OptimizePoseGraph();
	~OptimizePoseGraph();

	void OptimizePoseGraphForFragment(std::string path, int fragment_id);
	void OptimizePoseGraphForScene(std::string path);
};

