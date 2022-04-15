#pragma once
#include <iostream>
#include <Eigen/Dense>
#include <Open3D/Open3D.h>

using namespace open3d;
using namespace Eigen;

#define pi 3.1415926


Eigen::Matrix3d eulerAnglesToRotationMatrix(Eigen::Vector3d& theta);
void PCA(Eigen::MatrixXd& X, Eigen::MatrixXd& vec, Eigen::MatrixXd& val);
void DownSampleAndEstimateNormals(std::shared_ptr<geometry::PointCloud> pointcloud, double voxelsize, int kNum);