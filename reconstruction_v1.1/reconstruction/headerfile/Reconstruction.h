#pragma once
#include "MakeFragments.h"
#include "RegisterFragments.h"
#include "IntegrateScene.h"


class Reconstruction
{
public:
	Reconstruction(std::string relativePath,
		camera::PinholeCameraIntrinsicParameters _intrinsic = camera::PinholeCameraIntrinsicParameters::PrimeSenseDefault);
	~Reconstruction();

	void Run();

private:
	camera::PinholeCameraIntrinsic intrinsic;
	std::string path;
};

