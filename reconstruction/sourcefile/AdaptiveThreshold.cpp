#include "AdaptiveThreshold.h"

// 局部二值化
void AdaptiveThreshold(string inputFile, string outputFile)
{
	string file_Directory = inputFile + "\\*.bmp";
	vector<cv::String> cvfilenames;
	cv::glob(file_Directory, cvfilenames);
	vector<string> fileNames;
	for (int i = 0; i < cvfilenames.size(); i++) {
		fileNames.push_back(cvfilenames[i]);	
	}

	for (int i = 0; i < fileNames.size(); i++) {
		cv::Mat src = cv::imread(fileNames[i]);
		cv::Mat dst;
		if (src.empty()) {
			return;
		}
		if (src.channels() > 1)
			cv::cvtColor(src, src, CV_RGB2GRAY);
		cv::flip(src, src, 1);
		cv::adaptiveThreshold(src, dst, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 45, 1.5);
		int pos = fileNames[i].find_last_of('\\');
		cout << pos << endl;
		cv::imwrite(outputFile + fileNames[i].substr(pos), dst);
	}
	cout << "局部二值化完成" << endl;
}