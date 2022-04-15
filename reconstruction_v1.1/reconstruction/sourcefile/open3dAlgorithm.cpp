#include "open3dAlgorithm.h"

//姿态的欧拉角转换为旋转矩阵，用于刚体变换
Eigen::Matrix3d eulerAnglesToRotationMatrix(Eigen::Vector3d& theta) {
	// Calculate rotation about x axis
	for (int i = 0; i < 3; i++)
		theta[i] = theta[i] / 57.296;

	Eigen::Matrix3d R_x;
	R_x <<	1,	0,				0,
			0,	cos(theta[0]),	-sin(theta[0]),
			0,	sin(theta[0]),	cos(theta[0]);

	// Calculate rotation about y axis
	Eigen::Matrix3d R_y;
	R_y <<	cos(theta[1]),	0,	sin(theta[1]),
			0,				1,	0,
			-sin(theta[1]), 0,	cos(theta[1]);

	// Calculate rotation about z axis
	Eigen::Matrix3d R_z;
	R_z <<	cos(theta[2]),	-sin(theta[2]), 0,
			sin(theta[2]),	cos(theta[2]),	0,
			0,				0,				1;   

	// Combined rotation matrix
	return R_z * R_y * R_x;
}

//PCA  计算个别点云的主法向量
void PCA(Eigen::MatrixXd& X, Eigen::MatrixXd& vec, Eigen::MatrixXd& val) {
	//计算每一维度均值
	Eigen::MatrixXd meanval = X.colwise().mean();
	Eigen::RowVectorXd meanvecRow = meanval;
	//样本均值化为0
	X.rowwise() -= meanvecRow;

	//计算协方差矩阵C = XTX / n-1;
	Eigen::MatrixXd Cov;
	//C = X.adjoint() * X;
	//C = C.array() / X.rows() - 1;
	Cov = X.transpose() * X;

	//计算特征值和特征向量，使用selfadjont按照对阵矩阵的算法去计算，可以让产生的vec和val按照有序排列
	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(Cov);

	//获取特征值与特征向量
	val = eig.eigenvalues();
	vec = eig.eigenvectors();

	if (vec(2, 0) > 0)
		vec = -vec;
}

void DownSampleAndEstimateNormals(std::shared_ptr<geometry::PointCloud> pointcloud, double voxelsize, int kNum = 300) {
	//采用体素下采样
	auto downsamplePoint = pointcloud->VoxelDownSample(voxelsize);
	
	//创建KDTree 用于K近邻搜索           
	geometry::KDTreeFlann pointTree(*pointcloud);

	//迭代计算体素中心  300近邻点的主法向量
	for (int index = 0; index < downsamplePoint->points_.size(); index++) {
		//体素中心使用KNN近邻搜索  寻找原点云中的最近点
		std::vector<int> indices;
		std::vector<double> distance;
		pointTree.SearchKNN(downsamplePoint->points_[index], 1, indices, distance);
		indices.clear();
		distance.clear();

		//体素中心使用KNN近邻搜索  寻找附近最近kNum个点云数据
		pointTree.SearchKNN(pointcloud->points_[indices[0]], kNum, indices, distance);

		//修正体素中的值
		downsamplePoint->points_[index] = pointcloud->points_[indices[0]];

		//添加K近邻点到矩阵中
		Eigen::MatrixXd knearestPointData(kNum, 3);
		for (int i = 0; i < kNum; i++)
			knearestPointData.row(i) = pointcloud->points_[indices[i]];

		//PCA  主法向量求解
		Eigen::MatrixXd vec, val;
		PCA(knearestPointData, vec, val);

		//第0列为特征值最小的列，也就是主法向量对应的方向
		downsamplePoint->normals_.push_back(vec.col(0));
	}
}