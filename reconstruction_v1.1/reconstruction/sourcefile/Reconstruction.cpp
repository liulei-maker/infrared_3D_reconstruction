#include "Reconstruction.h"


Reconstruction::Reconstruction(
    std::string relativePath,
    camera::PinholeCameraIntrinsicParameters _intrinsic)
{
    //相机参数设置
    intrinsic = _intrinsic;
    //dataset路径设置
    path = FullPath(relativePath);
    std::cout << "start reconstruction" << std::endl;
}


Reconstruction::~Reconstruction()
{
    std::cout << "end reconstruction" << std::endl;
}

void Reconstruction::Run()
{
    utility::ScopeTimer timer("Full reconstruction");

    std::cout << "start MakeFragments" << std::endl;
    auto frag = MakeFragments();
    frag.Run(path, intrinsic);
    std::cout << "end MakeFragments" << std::endl;

    std::cout << "start RegisterFragments" << std::endl;
    auto reg = RegisterFragments();
    reg.Run(path);
    std::cout << "end RegisterFragments" << std::endl;

    std::cout << "start IntegrateScene" << std::endl;
    auto integration = IntegrateScene();
    integration.Run(path, intrinsic);
    std::cout << "end RegisterFragments" << std::endl;
}


