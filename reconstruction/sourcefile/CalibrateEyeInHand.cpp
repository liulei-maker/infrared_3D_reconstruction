#include "CalibrateEyeInHand.h"



//机械臂末端13组位姿,x,y,z,rx,ry,rz
//vector<string> positionVec = {
//	"A-0005-5888+3219-178-000-180BC",
//	"A-0517-7166+3219+172+005-173BC",
//	"A-1309-7502+3218+169+009-166BC",
//	"A-1573-7052+2898-178+012-169BC",
//	"A-0605-6588+2898-176+000-174BC",
//	"A+0018-6416+3455-174-004-171BC",
//	"A+1604-6416+3455+179-017+172BC",
//	"A+2228-6834+3515+176-022+171BC",
//	"A+2493-6590+3343+177-025+174BC",
//	"A+2465-6744+3783+171-024-170BC"
//};
vector<string> positionVec = {
	//相机标定坐标
	"A-2002-5914+1299-180-000+180BC",
	"A-1676-5751+1442+180-004+170BC",
	"A-1853-5637+1293-179+000+163BC",
	"A-1617-5411+1485-166-007-172BC",
	"A-2104-6235+1845-168+005-148BC",
	"A+0689-6595+2195+172-024-152BC",
	"A-0518-7032+1754+159-016-175BC"
	//热像仪标定坐标
	//"A+1937-7338+1915+167-011-177BC",
	//"A+2213-7024+2173+179-007+172BC",
	//"A+1049-5809+2336-174-002-169BC",
	//"A-0316-7663+1920+175+016-143BC",
	//"A+0335-6949+2318+169+005+137BC",
	//"A-0940-5548+1984+168+017+123BC",
	//"A+0581-6152+2060+180+005+180BC"
};
//机械臂六轴姿态信息转化为旋转矩阵和转移矩阵
void str2Mat(vector<string> &strPosition, Mat &toolPose) {
	for (int i = 0; i < strPosition.size(); i++) {
		toolPose.at<double>(i, 0) = std::atoi(strPosition[i].substr(1, 5).c_str()) / 10.0;
		toolPose.at<double>(i, 1) = std::atoi(strPosition[i].substr(6, 5).c_str()) / 10.0;
		toolPose.at<double>(i, 2) = std::atoi(strPosition[i].substr(11, 5).c_str()) / 10.0;
		toolPose.at<double>(i, 3) = std::atoi(strPosition[i].substr(16, 4).c_str());
		toolPose.at<double>(i, 4) = std::atoi(strPosition[i].substr(20, 4).c_str());
		toolPose.at<double>(i, 5) = std::atoi(strPosition[i].substr(24, 4).c_str());
	}
	//cout << "toolPose" << endl << toolPose << endl << endl;
}
//R和T转RT矩阵
Mat R_T2RT(Mat& R, Mat& T)
{
	Mat RT;
	Mat_<double> R1 = (cv::Mat_<double>(4, 3) << R.at<double>(0, 0), R.at<double>(0, 1), R.at<double>(0, 2),
		R.at<double>(1, 0), R.at<double>(1, 1), R.at<double>(1, 2),
		R.at<double>(2, 0), R.at<double>(2, 1), R.at<double>(2, 2),
		0.0, 0.0, 0.0);
	cv::Mat_<double> T1 = (cv::Mat_<double>(4, 1) << T.at<double>(0, 0), T.at<double>(1, 0), T.at<double>(2, 0), 1.0);

	cv::hconcat(R1, T1, RT);//C=A+B左右拼接
	return RT;
}

//RT转R和T矩阵
void RT2R_T(Mat& RT, Mat& R, Mat& T)
{
	cv::Rect R_rect(0, 0, 3, 3);
	cv::Rect T_rect(3, 0, 1, 3);
	R = RT(R_rect);
	T = RT(T_rect);
}

//判断是否为旋转矩阵
bool isRotationMatrix(const cv::Mat& R)
{
	cv::Mat tmp33 = R({ 0,0,3,3 });
	cv::Mat shouldBeIdentity;

	shouldBeIdentity = tmp33.t() * tmp33;

	cv::Mat I = cv::Mat::eye(3, 3, shouldBeIdentity.type());

	return  cv::norm(I, shouldBeIdentity) < 1e-6;
}

/** @brief 欧拉角 -> 3*3 的R
*	@param 	eulerAngle		角度值
*	@param 	seq				指定欧拉角xyz的排列顺序如："xyz" "zyx"
*/
cv::Mat eulerAngleToRotatedMatrix(const cv::Mat& eulerAngle, const std::string& seq)
{
	CV_Assert(eulerAngle.rows == 1 && eulerAngle.cols == 3);

	eulerAngle /= 180 / CV_PI; 
	cv::Matx13d m(eulerAngle);
	auto rx = m(0, 0), ry = m(0, 1), rz = m(0, 2);
	auto xs = std::sin(rx), xc = std::cos(rx);
	auto ys = std::sin(ry), yc = std::cos(ry);
	auto zs = std::sin(rz), zc = std::cos(rz);

	cv::Mat rotX = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, xc, -xs, 0, xs, xc);
	cv::Mat rotY = (cv::Mat_<double>(3, 3) << yc, 0, ys, 0, 1, 0, -ys, 0, yc);
	cv::Mat rotZ = (cv::Mat_<double>(3, 3) << zc, -zs, 0, zs, zc, 0, 0, 0, 1);

	cv::Mat rotMat;

	if (seq == "zyx")		rotMat = rotX * rotY * rotZ;
	else if (seq == "yzx")	rotMat = rotX * rotZ * rotY;
	else if (seq == "zxy")	rotMat = rotY * rotX * rotZ;
	else if (seq == "xzy")	rotMat = rotY * rotZ * rotX;
	else if (seq == "yxz")	rotMat = rotZ * rotX * rotY;
	else if (seq == "xyz")	rotMat = rotZ * rotY * rotX;
	else {
		cv::error(cv::Error::StsAssert, "Euler angle sequence string is wrong.",
			__FUNCTION__, __FILE__, __LINE__);
	}

	if (!isRotationMatrix(rotMat)) {
		cv::error(cv::Error::StsAssert, "Euler angle can not convert to rotated matrix",
			__FUNCTION__, __FILE__, __LINE__);
	}

	return rotMat;
	//cout << isRotationMatrix(rotMat) << endl;
}

/** @brief 四元数转旋转矩阵
*	@note  数据类型double； 四元数定义 q = w + x*i + y*j + z*k
*	@param q 四元数输入{w,x,y,z}向量
*	@return 返回旋转矩阵3*3
*/
cv::Mat quaternionToRotatedMatrix(const cv::Vec4d& q)
{
	double w = q[0], x = q[1], y = q[2], z = q[3];

	double x2 = x * x, y2 = y * y, z2 = z * z;
	double xy = x * y, xz = x * z, yz = y * z;
	double wx = w * x, wy = w * y, wz = w * z;

	cv::Matx33d res{
		1 - 2 * (y2 + z2),	2 * (xy - wz),		2 * (xz + wy),
		2 * (xy + wz),		1 - 2 * (x2 + z2),	2 * (yz - wx),
		2 * (xz - wy),		2 * (yz + wx),		1 - 2 * (x2 + y2),
	};
	return cv::Mat(res);
}

/** @brief ((四元数||欧拉角||旋转向量) && 转移向量) -> 4*4 的Rt
*	@param 	m				1*6 || 1*10的矩阵  -> 6  {x,y,z, rx,ry,rz}   10 {x,y,z, qw,qx,qy,qz, rx,ry,rz}
*	@param 	useQuaternion	如果是1*10的矩阵，判断是否使用四元数计算旋转矩阵
*	@param 	seq				如果通过欧拉角计算旋转矩阵，需要指定欧拉角xyz的排列顺序如："xyz" "zyx" 为空表示旋转向量
*/
cv::Mat attitudeVectorToMatrix(cv::Mat m, bool useQuaternion, const std::string& seq)
{
	CV_Assert(m.total() == 6 || m.total() == 10);
	if (m.cols == 1)
		m = m.t();
	cv::Mat tmp = cv::Mat::eye(4, 4, CV_64FC1);

	//如果使用四元数转换成旋转矩阵则读取m矩阵的第第四个成员，读4个数据
	if (useQuaternion)	// normalized vector, its norm should be 1.
	{
		cv::Vec4d quaternionVec = m({ 3, 0, 4, 1 });
		quaternionToRotatedMatrix(quaternionVec).copyTo(tmp({ 0, 0, 3, 3 }));
	}
	else
	{
		cv::Mat rotVec;
		if (m.total() == 6)
			rotVec = m({ 3, 0, 3, 1 });		//6
		else
			rotVec = m({ 7, 0, 3, 1 });		//10

		//如果seq为空表示传入的是旋转向量，否则"xyz"的组合表示欧拉角
		if (0 == seq.compare(""))
			cv::Rodrigues(rotVec, tmp({ 0, 0, 3, 3 }));
		else
			eulerAngleToRotatedMatrix(rotVec, seq).copyTo(tmp({ 0, 0, 3, 3 }));
	}
	tmp({ 3, 0, 1, 3 }) = m({ 0, 0, 3, 1 }).t();
	//std::swap(m,tmp);
	return tmp;
}
void m_calibration(string calibraDataName, vector<string>& FilesName, Size board_size, Size square_size, Mat& cameraMatrix, Mat& distCoeffs, vector<Mat>& rvecsMat, vector<Mat>& tvecsMat)
{
	//参数保存文件
	ofstream fout(calibraDataName + "\\caliberation_result_camera.txt");                       // 保存标定结果的文件

	std::cout << "开始提取角点………………" << endl;
	int image_count = 0;                                            // 图像数量
	Size image_size;                                                // 图像的尺寸

	vector<Point2f> image_points;                                   // 缓存每幅图像上检测到的角点
	vector<vector<Point2f>> image_points_seq;                       // 保存检测到的所有角点

	for (int i = 0; i < FilesName.size(); i++) {
		image_count++;
		// 用于观察检验输出
		std::cout << "image_count = " << image_count << endl;
		Mat originImage = imread(FilesName[i]);
		Mat imageInput;
		cvtColor(originImage, imageInput, COLOR_BGR2GRAY);
		//读入第一张图片时获取图像宽高信息
		if (image_count == 1) {
			image_size.width = imageInput.cols;
			image_size.height = imageInput.rows;
			std::cout << "image_size.width = " << image_size.width << endl;
			std::cout << "image_size.height = " << image_size.height << endl;
		}

		/* 提取角点 */
		bool bRes = findChessboardCorners(imageInput, board_size, image_points, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);

		if (bRes) {
			Mat view_gray;
			std::cout << "imageInput.channels()=" << imageInput.channels() << endl;
			/* 亚像素精确化 */
			cv::cornerSubPix(imageInput, image_points, cv::Size(5, 5), cv::Size(-1, -1), cv::TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.01));
			//保存亚像素角点
			image_points_seq.push_back(image_points);  
			/* 在图像上显示角点位置 */
			drawChessboardCorners(imageInput, board_size, image_points, true);
			//显示图片
			//imshow("Camera Calibration", imageInput);
			//waitKey(1000);//暂停0.1S
		}
		else {
			std::cout << "第" << image_count - 1 << "张照片提取角点失败，请删除后，重新标定！" << endl; //找不到角点
			imshow("失败照片", imageInput);
			waitKey(2000);
		}
	}
	std::cout << "角点提取完成！！！" << endl;


	/*棋盘三维信息*/
	vector<vector<Point3f>> object_points_seq;                     // 保存标定板上角点的三维坐标

	for (int t = 0; t < image_count; t++)
	{
		vector<Point3f> object_points;
		for (int i = 0; i < board_size.height; i++)
		{
			for (int j = 0; j < board_size.width; j++)
			{
				Point3f realPoint;
				/* 假设标定板放在世界坐标系中z=0的平面上 */
				realPoint.x = i * square_size.width;
				realPoint.y = j * square_size.height;
				realPoint.z = 0;
				object_points.push_back(realPoint);
			}
		}
		object_points_seq.push_back(object_points);
	}

	/* 运行标定函数 */
	//double rms = solvePnP(object_points_seq, image_points_seq, cameraMatrix, distCoeffs, rvecsMat, tvecsMat);
	double rms = calibrateCamera(object_points_seq, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, cv::CALIB_FIX_K3);
	std::cout << "RMS：" << rms << "像素" << endl << endl;
	std::cout << "标定完成！！！" << endl;

	cout << "开始评价标定结果………………";
	double total_err = 0.0;            // 所有图像的平均误差的总和
	double err = 0.0;                  // 每幅图像的平均误差
	double totalErr = 0.0;
	double totalPoints = 0.0;
	vector<Point2f> image_points_pro;     // 保存重新计算得到的投影点

	for (int i = 0; i < image_count; i++)
	{

		projectPoints(object_points_seq[i], rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points_pro);   //通过得到的摄像机内外参数，对角点的空间三维坐标进行重新投影计算

		err = norm(Mat(image_points_seq[i]), Mat(image_points_pro), NORM_L2);

		totalErr += err * err;
		totalPoints += object_points_seq[i].size();

		err /= object_points_seq[i].size();
		//fout << "第" << i + 1 << "幅图像的平均误差：" << err << "像素" << endl;
		total_err += err;
	}
	fout << "重投影误差2：" << sqrt(totalErr / totalPoints) << "像素" << endl << endl;
	fout << "重投影误差3：" << total_err / image_count << "像素" << endl << endl;

	//保存定标结果
	std::cout << "开始保存定标结果………………" << endl;
	Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* 保存每幅图像的旋转矩阵 */
	fout << "相机内参数矩阵：" << endl;
	fout << cameraMatrix << endl << endl;
	fout << "畸变系数：\n";
	fout << distCoeffs << endl << endl << endl;
	for (int i = 0; i < image_count; i++)
	{
		fout << "第" << i + 1 << "幅图像的旋转向量： ";
		fout << rvecsMat[i].t() << endl;
		fout << "第" << i + 1 << "幅图像的平移向量： ";
		fout << tvecsMat[i].t() << endl;
	}
	cout << "定标结果完成保存！！！" << endl;
	fout << endl;
}

void cameraCaliration(string calibraDataName, vector<Mat> &tvecsMat, vector<Mat> &rvecsMat)
{
	string file_Directory = calibraDataName + "\\*.jpg";
	vector<cv::String> cvfilenames;
	cv::glob(file_Directory, cvfilenames);

	vector<string> fileNames;
	for (int i = 0; i < cvfilenames.size(); i++)
	{
		fileNames.push_back(cvfilenames[i]);
		cout << cvfilenames[i] << endl;
	}

	Size board_size = Size(8, 11);                         // 标定板上每行、列的角点数
	Size square_size = Size(20, 20);                       // 实际测量得到的标定板上每个棋盘格的物理尺寸，单位mm

	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));        // 摄像机内参数矩阵
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));          // 摄像机的5个畸变系数：k1,k2,p1,p2,k3

	m_calibration(calibraDataName, fileNames, board_size, square_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat);
}

void CalibrateEyeInHand(string calibraDataName = "F:\\00reconstrcuction\\04data\\20211224calibration")
{
	//定义手眼标定矩阵
	std::vector<Mat> R_gripper2base;
	std::vector<Mat> t_gripper2base;
	std::vector<Mat> R_target2cam;
	std::vector<Mat> t_target2cam;
	Mat R_cam2gripper = (Mat_<double>(3, 3));
	Mat t_cam2gripper = (Mat_<double>(3, 1));

	// 读取末端，标定板的姿态矩阵 4*4
	std::vector<cv::Mat> vecHg, vecHc_camera;//vecHc_thermal, 
	cv::Mat Hcg_camera;//定义相机camera到末端grab的位姿矩阵

	//存储标定板的旋转向量和旋转矩阵
	vector<Mat> tvecsMat;
	vector<Mat> rvecsMat;

	//************************************************************************************************************
	//标定棋盘格
	cameraCaliration(calibraDataName + "\\color", tvecsMat, rvecsMat);
	int num_images = tvecsMat.size();

	Mat tempR, tempT;

	//计算标定板位姿
	for (size_t i = 0; i < num_images; i++) {
		cv::Mat calPoseTmp;
		vconcat(tvecsMat[i], rvecsMat[i], calPoseTmp);
		cv::Mat tmp = attitudeVectorToMatrix(calPoseTmp.t(), false, ""); //转移向量转旋转矩阵

		vecHc_camera.push_back(tmp);
		RT2R_T(tmp, tempR, tempT);

		R_target2cam.push_back(tempR);//标定板位姿
		t_target2cam.push_back(tempT);
	}

	//计算机械臂位姿
	Mat ToolPose(num_images, 6, CV_64F, 0.0);
	str2Mat(positionVec, ToolPose);
	for (size_t i = 0; i < num_images; i++) {
		cv::Mat tmp = attitudeVectorToMatrix(ToolPose.row(i), false, "xyz"); //机械臂位姿为欧拉角-旋转矩阵
		
		vecHg.push_back(tmp);
		RT2R_T(tmp, tempR, tempT);

		R_gripper2base.push_back(tempR);
		t_gripper2base.push_back(tempT);
	}

	//手眼标定，CALIB_HAND_EYE_TSAI法为11ms，最快
	calibrateHandEye(R_gripper2base, t_gripper2base, R_target2cam, t_target2cam, R_cam2gripper, t_cam2gripper);

	Mat R_Camera2gripper = R_cam2gripper.clone();
	Mat t_Camera2gripper = t_cam2gripper.clone();
	Hcg_camera = R_T2RT(R_Camera2gripper, t_Camera2gripper);//矩阵合并

	//************************************************************************************************************
	std::cout << "Hcg_camera 矩阵为： " << std::endl;
	std::cout << Hcg_camera << std::endl;

	ofstream fout(calibraDataName + "\\calibrateHandEye.txt");                       // 保存标定结果的文件

	fout << "Hcg_camera 矩阵为： " << std::endl;
	fout << Hcg_camera << std::endl;
	fout.close();


	std::cout << "----手眼系统测试----" << endl;
	std::cout << "机械臂下标定板的空间位置为：" << endl;

	for (int i = 0; i < vecHc_camera.size(); ++i) {
		cv::Mat cheesePos{ 1.0, 1.0, 1.0 , 1.0 };
		cv::Mat worldPos = vecHg[i] * Hcg_camera * vecHc_camera[i] * cheesePos;
		cout << i << ": " << worldPos.t() << endl;
	}

}
