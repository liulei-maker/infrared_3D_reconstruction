#include "OptimizePoseGraph.h"


OptimizePoseGraph::OptimizePoseGraph()
{
}


OptimizePoseGraph::~OptimizePoseGraph()
{
}

void RunPoseGraphOptimization(
    std::string pose_graph_name,
    std::string pose_graph_optimized_name,
    double max_correspondence_distance,
    double preference_loop_closure)
{
    
    auto method   = registration::GlobalOptimizationLevenbergMarquardt();
    auto criteria = registration::GlobalOptimizationConvergenceCriteria();
    auto option   = registration::GlobalOptimizationOption(max_correspondence_distance, 0.25, preference_loop_closure, 0);

    registration::PoseGraph pose_graph;
    if (!io::ReadPoseGraph(pose_graph_name, pose_graph))
        std::cout << "Unable to read pose graph:" << pose_graph_name << std::endl;

    GlobalOptimization(pose_graph, method, criteria, option);

    if (!io::WritePoseGraph(pose_graph_optimized_name, pose_graph))
        std::cout << "Unable to write pose graph:" << pose_graph_optimized_name << std::endl;

}

void OptimizePoseGraph::OptimizePoseGraphForFragment(std::string path, int fragment_id)
{
    auto pose_graph_name = Format(path + template_fragment_posegraph, fragment_id);
    auto pose_graph_name_optimized = Format(path + template_fragment_posegraph_optimized, fragment_id);

    std::cout << "start OptimizePoseGraphForFragment: " << pose_graph_name << std::endl;
    RunPoseGraphOptimization(pose_graph_name, pose_graph_name_optimized, 0.07, 0.1);
    std::cout << "end OptimizePoseGraphForFragment" << std::endl;
}

void OptimizePoseGraph::OptimizePoseGraphForScene(std::string path)
{
    auto pose_graph_name = path + template_global_posegraph;
    auto pose_graph_name_optimized = path + template_global_posegraph_optimized;

    std::cout << "start OptimizePoseGraphForScene: " << pose_graph_name << std::endl;
    RunPoseGraphOptimization(pose_graph_name, pose_graph_name_optimized, 0.07, 2.0);
    std::cout << "end OptimizePoseGraphForScene" << std::endl;
}
