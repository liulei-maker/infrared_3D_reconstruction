#pragma once
#include <Open3D/Open3D.h>

using namespace open3d;

class Visualizer
{
public:
	Visualizer(std::shared_ptr<geometry::PointCloud> geometry_ptr, bool isShowCoordinateFrame = false);
	~Visualizer();
	bool AddGeometry(std::shared_ptr<const geometry::Geometry> geometry_ptr);
	bool UpdateGeometry(std::shared_ptr<const geometry::Geometry> geometry_ptr);
	void Run();
private:
	open3d::visualization::Visualizer visualizer;			//创建一个可视化对象
};