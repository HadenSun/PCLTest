
#include <pcl\visualization\cloud_viewer.h>
#include <iostream>
#include <pcl\io\io.h>
#include <pcl\io\pcd_io.h>
#include <opencv2\opencv.hpp>
#include <fstream>

//�ڲβ���
#define FX 210.88
#define FY 211.12
#define CX 168.340
#define CY 138.013
//�����������
#define K1 -0.369
#define K2 0.134

using namespace cv;
using namespace std;
using namespace pcl;

int user_data;

//PCL��������ʱ���õĺ���
//���룺 ������
//����� ��
void viewerOneOff(pcl::visualization::PCLVisualizer& viewer)
{
	viewer.setBackgroundColor(1.0, 0.5, 1.0);
	pcl::PointXYZ o;
	o.x = 1.0;
	o.y = 0;
	o.z = 0;
	//viewer.addSphere(o, 0.25, "sphere", 0);
	std::cout << "i only run once" << std::endl;
}

//ÿ֡ѭ�����ú���
//���룺 ������
//����� ��
void viewerPsycho(pcl::visualization::PCLVisualizer& viewer)
{
	static unsigned count = 0;
	std::stringstream ss;
	ss << "Once per viewer loop: " << count++;
	viewer.removeShape("text", 0);
	viewer.addText(ss.str(), 200, 300, "text", 0);

	//FIXME: possible race condition here:
	user_data++;
}



//��ȡCSV�ļ�ΪMat��ʽ
//���������������320�У�241�У���һ������Ϊ0,1,...,319
//������ ��
//���أ� Mat��ʽͼ��
Mat csvToMat(void)
{
	fstream p_file;
	p_file.open("123.csv", ios::in);
	if (!p_file.is_open())
	{
		printf("�ļ�������\r\n");
		exit(-1);
	}

	int pixelDep = 0;		//����ֵ
	int colCounter = 0;		//�м���
	int rowCounter = 0;		//�м���

	string line;

	getline(p_file, line);			//���Ե�һ��
	Mat img(240, 320, CV_16U);

	while (getline(p_file, line))
	{
		rowCounter = 0;
		char* tmp;
		tmp = strtok((char*)line.c_str(), ",");		//�Զ��ŷָ��ַ���
		while (tmp)
		{
			img.at<ushort>(colCounter, rowCounter) = (unsigned short)atoi(tmp);		//charתint
			rowCounter++;
			tmp = strtok(NULL, ",");
		}
		colCounter++;
	}

	return img.clone();
}

//�������
//���룺 ��������ͼƬ
//����� У�����ͼƬ
Mat imageUndist(Mat src)
{
	//�������
	Mat img;

	//�ڲξ���
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);		//3*3��λ����
	cameraMatrix.at<double>(0, 0) = FX;
	cameraMatrix.at<double>(0, 1) = 0;
	cameraMatrix.at<double>(0, 2) = CX;
	cameraMatrix.at<double>(1, 1) = FY;
	cameraMatrix.at<double>(1, 2) = CY;
	cameraMatrix.at<double>(2, 2) = 1;
	//�������
	Mat distCoeffs = Mat::zeros(5, 1, CV_64F);		//5*1ȫ0����
	distCoeffs.at<double>(0, 0) = K1;
	distCoeffs.at<double>(1, 0) = K2;
	distCoeffs.at<double>(2, 0) = 0;
	distCoeffs.at<double>(3, 0) = 0;
	distCoeffs.at<double>(4, 0) = 0;

	Size imageSize = src.size();
	Mat map1, map2;

	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), cameraMatrix, imageSize, CV_32FC1, map1, map2);	//
	remap(src, img, map1, map2, INTER_LINEAR);

	return img.clone();
}



//CSV�ļ�תPcd
//���룺 ��
//����� ��
void imageCsvToPcd()
{
	Mat mImageDepth = csvToMat();		//��ȡCSV�ļ�תMat
	Mat img;

	mImageDepth = imageUndist(mImageDepth);		//�������
	

	//16λѹ����8λ
	Mat mImageZip;
	mImageDepth.convertTo(mImageZip, CV_8U, 1/255, 0);

	//����α��ɫͼ��
	Mat mImageColor;
	applyColorMap(mImageZip, mImageColor, COLORMAP_HSV);

	//������������ת�����������
	int imgWidth = mImageDepth.size().width;
	int imgHeight = mImageDepth.size().height;
	PointCloud<PointXYZRGB> pointCloud;

	for (int i = 0; i < imgHeight; i++)
	{
		for (int j = 0; j < imgWidth; j++)
		{
			float picDist = sqrt((i - imgHeight / 2.0)*(i - imgHeight / 2.0) + (j - imgWidth / 2.0)*(j - imgWidth / 2.0));	//ͼ���ϵ㵽���ĵ����ص����
			float picAngle = atan2(i - imgHeight / 2.0, j - imgWidth / 2.0);												//ͼ����x,y�����ĵ�Ƕȹ�ϵ
			float angle = atan(sqrt((j - imgWidth / 2.0)*(j - imgWidth / 2.0) / FX / FX + (i - imgHeight / 2.0)*(i - imgHeight / 2.0) / FY / FY));
			float dist = mImageDepth.at<ushort>(i, j);				//ԭʼͼ�����

			PointXYZRGB p;
			p.z = dist*cos(angle);									//����任������
			p.x = dist*sin(angle)*cos(picAngle);
			p.y = dist*sin(angle)*sin(picAngle);

			p.r = mImageColor.at<Vec3b>(i, j)[0];
			p.g = mImageColor.at<Vec3b>(i, j)[1];
			p.b = mImageColor.at<Vec3b>(i, j)[2];
			pointCloud.points.push_back(p);
		}
	}

	//�������
	io::savePCDFileBinary("pcl.pcd", pointCloud);

}

int main()
{
	//��ȡCSV�ļ���תPcd�����ļ�
	imageCsvToPcd();

	//������
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	//��pcd�����������
	pcl::io::loadPCDFile("pcl.pcd", *cloud);

	//��ʼ�����ƴ���
	pcl::visualization::CloudViewer viewer("Cloud Viewer");

	//��ʾ��������
	viewer.showCloud(cloud);

	//���õ����߳���������
	viewer.runOnVisualizationThreadOnce(viewerOneOff);

	//���õ����߳�ÿ֡���ú���
	viewer.runOnVisualizationThread(viewerPsycho);

	while (!viewer.wasStopped())
	{
		//you can also do cool processing here
		//FIXME: Note that this is running in a separate thread from viewerPsycho
		//and you should guard against race conditions yourself...
		user_data++;
	}

	return 0;
}
