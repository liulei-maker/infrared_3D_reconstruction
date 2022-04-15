#include "RegisterFragments.h"



RegisterFragments::RegisterFragments()
{
}


RegisterFragments::~RegisterFragments()
{
}

std::tuple<std::shared_ptr<geometry::PointCloud>, std::shared_ptr<registration::Feature>> PreprocessPointCloud(geometry::PointCloud& pcd)
{
    auto pcd_down = pcd.VoxelDownSample(0.05);

    pcd_down->EstimateNormals(geometry::KDTreeSearchParamHybrid(0.1, 30));

    auto pcd_fpfh = registration::ComputeFPFHFeature(*pcd_down, geometry::KDTreeSearchParamHybrid(0.25, 100));
    
    return std::make_tuple(pcd_down, pcd_fpfh);
}

std::tuple<bool, Eigen::Matrix4d> RegisterPointCloudFpfh(
    geometry::PointCloud& source,
    geometry::PointCloud& target,
    registration::Feature& source_fpfh,
    registration::Feature& target_fpfh)
{
    auto result = FastGlobalRegistration(source, target, source_fpfh, target_fpfh, registration::FastGlobalRegistrationOption(1.4, false, true, 0.07));

    if (result.transformation_.trace() == 4.0)
    {
        return std::make_tuple(false, Eigen::Matrix4d::Identity());
    }
    else
    {
        return std::make_tuple(true, result.transformation_);
    }
}

std::tuple<bool, Eigen::Matrix4d> ComputeInitialRegistration(
    int s, int t,
    geometry::PointCloud& source_down,
    geometry::PointCloud& target_down,
    registration::Feature& source_fpfh,
    registration::Feature& target_fpfh,
    std::string path,
    bool draw_result = false)
{
    Eigen::Matrix4d transformation;
    bool success_reg;

    if (t == s + 1)
    {
        // odometry case
        PrintInfo("Using RGBD odometry\n");
        registration::PoseGraph pose_graph_frag;
        io::ReadPoseGraph(Format(path + template_fragment_posegraph_optimized, s), pose_graph_frag);

        int n_nodes = (int)pose_graph_frag.nodes_.size();
        transformation = pose_graph_frag.nodes_[n_nodes - 1].pose_.inverse();
    }
    else
    {
        // loop closure
        PrintInfo("Register point cloud fpfh\n");
        std::tie(success_reg, transformation) = RegisterPointCloudFpfh(source_down, target_down, source_fpfh, target_fpfh);
        if (!success_reg)
        {
            return std::make_tuple(false, Eigen::Matrix4d::Identity());
        }
    }

    if (draw_result)
    {

    }

    return std::make_tuple(true, transformation);
}

// colored pointcloud registration
// This is implementation of following paper
// J.Park, Q. - Y.Zhou, V.Koltun,
// Colored Point Cloud Registration Revisited, ICCV 2017
std::tuple<Eigen::Matrix4d, Eigen::Matrix6d> RegisterColoredPointCloudICP(
    geometry::PointCloud& source,
    geometry::PointCloud& target,
    Eigen::Matrix4d init_transformation = Eigen::Matrix4d::Identity(),
    std::vector<double> voxel_radius = { 0.05, 0.025, 0.0125 },
    std::vector<int> max_iter = { 50, 30, 14 },
    bool draw_result = false
)
{
    auto current_transformation = init_transformation;
    registration::RegistrationResult result_icp;

    for (int scale = 0; scale < max_iter.size(); scale++)
    {
        auto iter = max_iter[scale];
        auto radius = voxel_radius[scale];
        auto source_down = source.VoxelDownSample(radius);
        auto target_down = target.VoxelDownSample(radius);

        source_down->EstimateNormals(geometry::KDTreeSearchParamHybrid(radius * 2, 30));
        target_down->EstimateNormals(geometry::KDTreeSearchParamHybrid(radius * 2, 30));

        result_icp = registration::RegistrationICP(
            *source_down, *target_down, radius,
            current_transformation,
            registration::TransformationEstimationPointToPoint(false),
            registration::ICPConvergenceCriteria(1e-6, 1e-6, iter));
        current_transformation = result_icp.transformation_;
    }

    auto information_matrix = registration::GetInformationMatrixFromPointClouds(source, target, 0.07, result_icp.transformation_);

    if (draw_result)
    {

    }

    return std::tie(result_icp.transformation_, information_matrix);
}

std::tuple<bool, Eigen::Matrix4d, Eigen::Matrix6d> LocalRefinement(
    int s, int t,
    geometry::PointCloud& source,
    geometry::PointCloud& target,
    Eigen::Matrix4d& transformation_init,
    bool draw_result = false)
{
    Eigen::Matrix4d transformation;
    Eigen::Matrix6d information;

    if (t == s + 1)
    {
        // odometry case
        std::tie(transformation, information) = RegisterColoredPointCloudICP(source, target, transformation_init, { 0.0125 }, { 30 });
    }
    else
    {
        // loop closure case
        std::tie(transformation, information) = RegisterColoredPointCloudICP(source, target, transformation_init);
    }

    bool success_local = false;

    if (information(5, 5) / (double)std::min((double)source.points_.size(), (double)target.points_.size()) > 0.3)
    {
        success_local = true;
    }

    if (draw_result)
    {
    }

    return std::make_tuple(success_local, transformation, information);
}

std::tuple<Eigen::Matrix4d, registration::PoseGraph> UpdateOdometryPoseGraph(
    int s, int t,
    Eigen::Matrix4d& transformation,
    Eigen::Matrix6d& information,
    Eigen::Matrix4d& odometry,
    registration::PoseGraph pose_graph
)
{
    if (t == s + 1)
    {
        // odometry case
        odometry = transformation * odometry;
        auto odometry_inv = odometry.inverse();
        pose_graph.nodes_.push_back(registration::PoseGraphNode(odometry_inv));
        pose_graph.edges_.push_back(registration::PoseGraphEdge(s, t, transformation, information, false));
    }
    else
    {
        // loop closure case
        pose_graph.edges_.push_back(registration::PoseGraphEdge(s, t, transformation, information, true));
    }

    return std::make_tuple(odometry, pose_graph);
}

void RegisterPointCloud(std::string path, std::vector<std::string> ply_file_names, bool draw_result = false)
{
    registration::PoseGraph pose_graph;
    Eigen::Matrix4d odometry = Eigen::Matrix4d::Identity();

    pose_graph.nodes_.push_back(registration::PoseGraphNode(odometry));

    geometry::PointCloud source, target;
    std::shared_ptr<geometry::PointCloud> source_down, target_down;
    std::shared_ptr<registration::Feature> source_fpfh, target_fpfh;
    bool success_global, success_local;
    Eigen::Matrix4d transformation_init, transformation_icp;
    Eigen::Matrix6d information_icp;

    for (int s = 0; s < ply_file_names.size(); s++)
    {
        for (int t = s + 1; t < ply_file_names.size(); t++)
        {
            PrintInfo("Reading %s...\n", ply_file_names[s].c_str());
            io::ReadPointCloud(ply_file_names[s], source);
            PrintInfo("Reading %s...\n", ply_file_names[t].c_str());
            io::ReadPointCloud(ply_file_names[t], target);

            std::tie(source_down, source_fpfh) = PreprocessPointCloud(source);
            std::tie(target_down, target_fpfh) = PreprocessPointCloud(target);
            std::tie(success_global, transformation_init) = ComputeInitialRegistration(
                s, t, *source_down, *target_down, *source_fpfh, *target_fpfh, path, draw_result);

            if (!success_global)
                continue;

            std::tie(success_local, transformation_icp, information_icp) = LocalRefinement(
                s, t, source, target, transformation_init, draw_result);

            if (!success_local)
                continue;

            std::tie(odometry, pose_graph) = UpdateOdometryPoseGraph(
                s, t, transformation_icp, information_icp,
                odometry, pose_graph);

        }
    }

    io::WritePoseGraph(path + template_global_posegraph, pose_graph);
}

void RegisterFragments::Run(std::string path)
{
    utility::ScopeTimer timer("Register Fragments");
    auto ply_file_names = GetFileList(path + folder_fragment, ".ply");
    MakeFolder(path + folder_scene);

    RegisterPointCloud(path, ply_file_names);

    auto optimize = OptimizePoseGraph();
    optimize.OptimizePoseGraphForScene(path);
}
