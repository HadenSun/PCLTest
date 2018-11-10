/*
* Copyright (c) 2018 HadenSun
* Contact: http://www.sunhx.cn
* E-mail: admin@sunhx.cn
*
* Description:
*	 
*
*/

#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/features/integral_image_normal.h>
#include <pcl/features/pfh.h>

using namespace std;
using namespace pcl;

int user_data;

//PCA���Ʒ�����ʾ
//���룺��
//�������������0
int estimatingTheNormalsFeatures()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<Normal>::Ptr cloud_normals(new PointCloud<Normal>);
	
	//��ȡ��������
	PCDReader reader;
	reader.read("pcl.pcd", *cloud);

	//������kd���������ٽ��㼯����
	search::KdTree<PointXYZ>::Ptr tree(new search::KdTree<PointXYZ>());

	//�������߹�����
	NormalEstimation<PointXYZ, Normal> ne;
	ne.setInputCloud(cloud);	//���������������
	ne.setSearchMethod(tree);	//���ÿռ������������ָ��
	ne.setRadiusSearch(100);	//���������ھӰ뾶
	ne.compute(*cloud_normals);	//���㷨��
	
	//��ʾ���
	boost::shared_ptr<visualization::PCLVisualizer> viewer(new visualization::PCLVisualizer("3D Viewer"));
	viewer->setBackgroundColor(0, 0, 0.7);																	//���ñ�����ɫ
	visualization::PointCloudColorHandlerCustom<PointXYZ> single_color(cloud, 0, 255, 0);					//���õ�����ɫ
	viewer->addPointCloud<PointXYZ>(cloud, single_color, "sample cloud");									//�����Ҫ��ʾ�ĵ�������
	viewer->setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_POINT_SIZE, 1, "sample cloud");	//���õ��С
	//����1������ԭʼ����
	//����2��������Ϣ
	//����3����ʾ����ĵ��Ƽ����15������ʾһ�η���
	//����4�����򳤶�
	viewer->addPointCloudNormals<PointXYZ, Normal>(cloud, cloud_normals, 15, 300, "normals");				//�����Ҫ��ʾ�ķ���

	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		user_data++;
	}

	return 0;
}

//����ͼ���������֯���Ƶķ��߹���
//���룺��
//�������������0
int integralImageNormalEstimationFeatures()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);

	//��������
	PCDReader reader;
	reader.read("table_scene_mug_stereo_textured.pcd", *cloud);

	//���Ʒ���
	PointCloud<Normal>::Ptr normals(new PointCloud<Normal>);
	IntegralImageNormalEstimation<PointXYZ, Normal> ne;
	//COVARIANCE_MATRIXģʽ����9������ͼ���Ը�����ֲ������Э�����������ض���ķ��ߡ� 
	//AVERAGE_3D_GRADIENTģʽ����6������ͼ���Լ���ˮƽ�ʹ�ֱ3D�����ƽ���汾����ʹ������������֮��Ľ���������㷨�ߡ� 
	//AVERAGE_DEPTH_CHANGEģʽ��������������ͼ�񣬲�����ƽ����ȱ仯���㷨�ߡ�
	ne.setNormalEstimationMethod(ne.AVERAGE_3D_GRADIENT);	//���ù��Ʒ�����ƽ��3D�ݶ�
	ne.setMaxDepthChangeFactor(0.02f);						//���������Եʱ����ȱ仯��ֵ
	ne.setNormalSmoothingSize(10.0f);						//����ƽ����������Ĵ�С
	ne.setInputCloud(cloud);								//���������������
	ne.compute(*normals);									//���㷨��

	//��ʾ���
	visualization::PCLVisualizer viewer("PCL Viewer");
	viewer.setBackgroundColor(0.0, 0.0, 0.5);
	viewer.addPointCloudNormals<PointXYZ, Normal>(cloud, normals);

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);
	}

	return 0;
}

//PFH��������ֱ��ͼ������
int pfhEstimationFeatures()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<Normal>::Ptr normals(new PointCloud<Normal>);

	//��������
	PCDReader reader;
	reader.read("table_scene_mug_stereo_textured.pcd", *cloud);

	//����PFH������
	PFHEstimation<PointXYZ, Normal, PFHSignature125> pfh;
	pfh.setInputCloud(cloud);
	pfh.setInputNormals(normals);
	

	return 0;
}

int main()
{
	//estimatingTheNormalsFeatures();			//PCA���Ʒ�����ʾ
	integralImageNormalEstimationFeatures();	//����ͼ���������֯���Ƶķ��߹���

	return 0;
}