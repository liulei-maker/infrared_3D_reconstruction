#include "MakeFragments.h"


int n_frames_per_fragment = 100;    //每一个片段由多少帧数据构成
int n_keyframes_per_n_frame = 5;

std::string folder_fragment = "fragments\\";
std::string template_fragment_posegraph = folder_fragment + "fragment_%03d.json";
std::string template_fragment_posegraph_optimized = folder_fragment + "fragment_optimized_%03d.json";

std::string template_fragment_mesh = folder_fragment + "fragment_%03d.ply";
std::string folder_scene = "scene\\";
std::string template_global_posegraph = folder_scene + "global_registration.json";
std::string template_global_posegraph_optimized = folder_scene + "global_registration_optimized.json";
std::string template_global_mesh = folder_scene + "integrated.ply";


MakeFragments::MakeFragments()
{
}


MakeFragments::~MakeFragments()
{
}

/****************************************************************************
 * @brief function   : to estimate 6D rigid motion from two RGBD image pairs.
 * @param s          : the first index of RGBD image
 * @param t          : the second index of RGBD image
 * @param colorFiles : all of the color images' path
 * @param depthFiles : all of the depth images' path
 * @param intrinsic  : camera' intrinsic parameters
 * @return           : 6D rigid motion matrix and information
 ****************************************************************************/
std::tuple<bool, Eigen::Matrix4d, Eigen::Matrix6d> RegisterOneRGBDPair(
    int s, int t,
    std::vector<std::string> colorFiles,
    std::vector<std::string> depthFiles,
    camera::PinholeCameraIntrinsic& intrinsic)
{
    if (std::abs(s - t) != 1)
    {
        return std::make_tuple(false, Eigen::Matrix4d::Identity(), Eigen::Matrix6d::Identity());
    }
    
    auto color_s = io::CreateImageFromFile(colorFiles[s]);
    auto depth_s = io::CreateImageFromFile(depthFiles[s]);
    auto color_t = io::CreateImageFromFile(colorFiles[t]);
    auto depth_t = io::CreateImageFromFile(depthFiles[t]);

    auto rgbd_s = geometry::RGBDImage::CreateFromColorAndDepth(*color_s, *depth_s);
    auto rgbd_t = geometry::RGBDImage::CreateFromColorAndDepth(*color_t, *depth_t);

    Eigen::Matrix4d odo_init = Eigen::Matrix4d::Identity();

    return odometry::ComputeRGBDOdometry(*rgbd_s, *rgbd_t, intrinsic, odo_init, odometry::RGBDOdometryJacobianFromHybridTerm(), odometry::OdometryOption());

}

/****************************************************************************
 * @brief function   : calcula 6D rigid motion matrix and information, then save it
 * @param path       : this path is used to save the results
 * @param sid        : the start index of current fragments
 * @param eid        : the end index of current fragments
 * @param colorFiles : all of the color images' path
 * @param depthFiles : all of the depth images' path
 * @param n_fragments: the total number of fragments
 * @param intrinsic  : camera' intrinsic parameters
 * @return           : void
 ****************************************************************************/
void MakePoseGraphForFragment(
    std::string path,
    int sid, 
    int eid,
    std::vector<std::string> colorFiles,
    std::vector<std::string> depthFiles,
    int fragment_id, int n_fragments,
    camera::PinholeCameraIntrinsic& intrinsic)
{
    auto fragment_posegraph_name = Format(path + template_fragment_posegraph, fragment_id);

    registration::PoseGraph pose_graph;
    Eigen::Matrix4d trans_odometry = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d trans_odometry_inv = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d trans = Eigen::Matrix4d::Identity();
    Eigen::Matrix6d info = Eigen::Matrix6d::Identity();
    bool success;

    pose_graph.nodes_.push_back(registration::PoseGraphNode(trans_odometry));

    for (int s = sid; s < eid; s++)
    {
        for (int t = s + 1; t < eid; t++)
        {
            // odometry
            if (t == s + 1)
            {
                PrintInfo("Fragment %03d / %03d :: RGBD matching between frame : %d and %d\n", fragment_id, n_fragments - 1, s, t);

                std::tie(success, trans, info) = RegisterOneRGBDPair(s, t, colorFiles, depthFiles, intrinsic);

                trans_odometry = trans * trans_odometry;
                trans_odometry_inv = trans_odometry.inverse();

                pose_graph.nodes_.push_back(registration::PoseGraphNode(trans_odometry_inv));
                pose_graph.edges_.push_back(registration::PoseGraphEdge(s - sid, t - sid, trans, info, false));
            }

            // keyframe loop closure
            if (s % n_keyframes_per_n_frame == 0 && t % n_keyframes_per_n_frame == 0)
            {
                PrintInfo("Fragment %03d / %03d :: RGBD matching between frame : %d and %d\n", fragment_id, n_fragments - 1, s, t);

                std::tie(success, trans, info) = RegisterOneRGBDPair(s, t, colorFiles, depthFiles, intrinsic);

                if (success)
                {
                    pose_graph.edges_.push_back(registration::PoseGraphEdge(s - sid, t - sid, trans, info, true));
                }
            }
        }
    }
    io::WritePoseGraph(fragment_posegraph_name, pose_graph);
}


std::shared_ptr<geometry::TriangleMesh> IntegrateRGBFramesForFragment(
    std::vector<std::string> colorFiles,
    std::vector<std::string> depthFiles,
    int fragment_id, int n_fragments,
    std::string poseGraphName,
    camera::PinholeCameraIntrinsic& intrinsic
)
{
    registration::PoseGraph pose_graph;
    io::ReadPoseGraph(poseGraphName, pose_graph);

    integration::ScalableTSDFVolume volume(3.0 / 512.0, 0.04, integration::TSDFVolumeColorType::RGB8);

    int i_abs;
    geometry::Image color, depth;
    for (int i = 0; i < pose_graph.nodes_.size(); i++)
    {
        i_abs = fragment_id * n_frames_per_fragment + i;
        
        PrintInfo("Fragment %03d / %03d :: integrate rgbd frame %d (%d of %d).\n", fragment_id, n_fragments - 1, i_abs, i + 1, pose_graph.nodes_.size());

        io::ReadImage(colorFiles[i_abs], color);
        io::ReadImage(depthFiles[i_abs], depth);

        auto rgbd = geometry::RGBDImage::CreateFromColorAndDepth(color, depth, 1000.0, 3.0, false);

        auto pose = pose_graph.nodes_[i].pose_;
        //fusion to generate RGB+depth graph
        volume.Integrate(*rgbd, intrinsic, pose.inverse());
    }

    auto mesh = volume.ExtractTriangleMesh();//三角化表面模型
    mesh->ComputeVertexNormals();//计算法向量

    return mesh;
}

void MakeMeshForFragment(
    std::string path,
    std::vector<std::string> colorFiles,
    std::vector<std::string> depthFiles,
    int fragment_id, int n_fragments,
    camera::PinholeCameraIntrinsic& intrinsic)
{
    auto mesh = IntegrateRGBFramesForFragment(
        colorFiles, depthFiles, fragment_id, n_fragments,
        Format(path + template_fragment_posegraph_optimized, fragment_id),
        intrinsic);

    auto mesh_name = Format(path + template_fragment_mesh, fragment_id);

    io::WriteTriangleMesh(mesh_name, *mesh, false, true);
}
/****************************************************************************
 * @brief function   : process single fragment
 * @param path       : this path is used to save the results
 * @param fragment_id: fragments' id
 * @param colorFiles : all of the color images' path
 * @param depthFiles : all of the depth images' path
 * @param n_fragments: the total number of fragments 
 * @param intrinsic  : camera' intrinsic parameters
 * @return           : void
 ****************************************************************************/
void ProcessSingleFragment(
    std::string path, 
    int fragment_id, 
    std::vector<std::string> colorFiles, 
    std::vector<std::string> depthFiles,
    int n_fragments, 
    camera::PinholeCameraIntrinsic intrinsic)
{
    auto optimize = OptimizePoseGraph();

    int sid = fragment_id * n_frames_per_fragment;  //the start index of current fragments
    int eid = (int)(std::min)((float)(sid + n_frames_per_fragment), (float)colorFiles.size());//the end index of current fragments

    MakePoseGraphForFragment(path, sid, eid, colorFiles, depthFiles, fragment_id, n_fragments, intrinsic);
    optimize.OptimizePoseGraphForFragment(path, fragment_id);
    MakeMeshForFragment(path, colorFiles, depthFiles, fragment_id, n_fragments, intrinsic);
}

void MakeFragments::Run(std::string path, camera::PinholeCameraIntrinsic intrinsic)
{
    utility::ScopeTimer timer("Make Fragments");
    std::vector<std::string> colorFiles, depthFiles;
    //create the folder for save fragments
    MakeFolder(path + folder_fragment);
    //get the path of RGB image and depth image 
    std::tie(colorFiles, depthFiles) = ReadRGBDColorFiles(path);
    //calculate the nums of fragments
    int n_fragments = (int)std::ceil((float)colorFiles.size() / (float)n_frames_per_fragment);//ceil向上取整   floor向下取整
    
    for (int fragment_id = 0; fragment_id < n_fragments; fragment_id++)
    {
        ProcessSingleFragment(path, fragment_id, colorFiles, depthFiles, n_fragments, intrinsic);
    }
}
