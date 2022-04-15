# infrared_3D_reconstruction
<font size = "6">光激励红外三维重构目标检测</font><br>
此项目是将光激励红外结合深度相机，联合机械臂自动扫查扫查系统，实现自动扫查与三维重构<br>
![流程](https://user-images.githubusercontent.com/54426524/163543969-5eef0645-95ee-4053-a975-aaa4c35f2fe9.PNG)
![流程2](https://user-images.githubusercontent.com/54426524/163543973-1bf3b3b7-81a2-4e2a-b0b4-ae56588ca05e.PNG)<br>
代码环境：<br>
1.depth camera astra SDK<br>
2.Open3D_v0.10.0<br>
3.OpenCV3416<br>

代码流程：<br>
1.标定红外系统、深度相机和机器人的变换关系<br>
2.采集实验数据，包含红外图像、深度图像<br>
3.三维重构+匹配<br>

实验结果<br>
![结果1](https://user-images.githubusercontent.com/54426524/163544509-78d6ee3d-0d8c-4aca-bc2f-cca8226b957e.PNG)
![结果2](https://user-images.githubusercontent.com/54426524/163544515-513c394e-4ecb-44a9-9a5b-124f0d90bd2a.PNG)
![结果3](https://user-images.githubusercontent.com/54426524/163544521-eaaed732-175a-4793-acda-e93f60a25552.PNG)
