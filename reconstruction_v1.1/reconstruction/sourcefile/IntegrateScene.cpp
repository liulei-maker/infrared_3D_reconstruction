#include "IntegrateScene.h"


IntegrateScene::IntegrateScene()
{
}


IntegrateScene::~IntegrateScene()
{
}

void IntegrateScene::Run(std::string path, camera::PinholeCameraIntrinsic intrinsic)
{
    utility::ScopeTimer timer("Integrate Scene");

    std::vector<std::string> colorFiles, depthFiles;

    std::tie(colorFiles, depthFiles) = ReadRGBDColorFiles(path);

    int n_fragments = (int)std::ceil((float)colorFiles.size() / (float)n_frames_per_fragment);

    integration::ScalableTSDFVolume volume(3.0 / 512.0, 0.04, integration::TSDFVolumeColorType::RGB8);

    registration::PoseGraph pose_graph_fragment;
    io::ReadPoseGraph(path + template_global_posegraph_optimized, pose_graph_fragment);

    registration::PoseGraph pose_graph_rgbd;
    geometry::Image color, depth;
    
    for (int fragment_id = 0; fragment_id < pose_graph_fragment.nodes_.size(); fragment_id++)
    {
        io::ReadPoseGraph(Format(path + template_fragment_posegraph_optimized, fragment_id), pose_graph_rgbd);

        for (int frame_id = 0; frame_id < pose_graph_rgbd.nodes_.size(); frame_id++)
        {
            auto frame_id_abs = fragment_id * n_frames_per_fragment + frame_id;
            PrintInfo("Fragment %03d / %03d :: integrate rgbd frame %d (%d of %d).\n",
                fragment_id, n_fragments - 1, frame_id_abs, frame_id + 1, pose_graph_rgbd.nodes_.size());

            io::ReadImage(colorFiles[frame_id_abs], color);
            io::ReadImage(depthFiles[frame_id_abs], depth);

            auto rgbd = geometry::RGBDImage::CreateFromColorAndDepth(color, depth, 1000.0, 3.0, false);

            auto pose = pose_graph_fragment.nodes_[fragment_id].pose_ * pose_graph_rgbd.nodes_[frame_id].pose_;
            volume.Integrate(*rgbd, intrinsic, pose.inverse());
        }
    }

    auto mesh = volume.ExtractTriangleMesh();
    mesh->ComputeVertexNormals();

    auto mesh_name = path + template_global_mesh;
    io::WriteTriangleMesh(mesh_name, *mesh, false, true);
}
