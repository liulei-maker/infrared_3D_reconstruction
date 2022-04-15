#include "Visualizer.h"

Visualizer::Visualizer(std::shared_ptr<geometry::PointCloud> geometry_ptr, bool isShowCoordinateFrame)
{
	this->visualizer.CreateVisualizerWindow("open3d", 640, 480, 50, 50, true); //创建一个窗口
	this->visualizer.GetRenderOption().point_size_ = 3;//设置点云点的大小
	this->visualizer.GetRenderOption().background_color_ = Eigen::Vector3d(0,0,0);//设置窗口背景色
	//this->visualizer.GetRenderOption().show_coordinate_frame_ = true;

	std::shared_ptr<geometry::PointCloud> pointcloud_ptr = std::make_shared<geometry::PointCloud>();
	if (isShowCoordinateFrame) {
		Eigen::Vector3d pointTemp;
		Eigen::Vector3d colorTemp;
		for (int k = 0; k < 2; k++) {
			for (int j = 0; j < 2; j++) {
				for (int i = 0; i < 200; i++) {
					pointTemp << i, k, j;
					colorTemp << 1, 0, 0;
					pointcloud_ptr->points_.push_back(pointTemp);
					pointcloud_ptr->colors_.push_back(colorTemp);
					pointTemp << k, i, j;
					colorTemp << 0, 1, 0;
					pointcloud_ptr->points_.push_back(pointTemp);
					pointcloud_ptr->colors_.push_back(colorTemp);
					pointTemp << k, j, i;
					colorTemp << 0, 0, 1;
					pointcloud_ptr->points_.push_back(pointTemp);
					pointcloud_ptr->colors_.push_back(colorTemp);
				}
			}
		}
	}
	this->visualizer.AddGeometry(geometry_ptr);
	if (isShowCoordinateFrame) {
		this->visualizer.AddGeometry(pointcloud_ptr);
	}
}
  
Visualizer::~Visualizer()
{

}

bool Visualizer::AddGeometry(std::shared_ptr<const geometry::Geometry> geometry_ptr)
{
	return this->visualizer.AddGeometry(geometry_ptr);
}

bool Visualizer::UpdateGeometry(std::shared_ptr<const geometry::Geometry> geometry_ptr)
{
	int i = this->visualizer.UpdateGeometry();
	this->visualizer.PollEvents(); //下头这两个不清楚啥用
	this->visualizer.UpdateRender();
	return i;
}

void Visualizer::Run() {
	this->visualizer.Run();
}