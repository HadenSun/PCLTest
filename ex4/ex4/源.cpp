/*
* Copyright (c) 2018 HadenSun
* Contact: http://www.sunhx.cn
* E-mail: admin@sunhx.cn
*
* Description:
*	 �������ڷֱ���ò�ͬ������ʵ�ֲ�ͬ��׼Ч����
*	 ICPAlgorithms������ICP����������㣩�㷨��ȡ״̬ת�ƾ���
*	 NDTAlgorithms������NDT����̫�ֲ��任���㷨��׼��
*	 SACIAAlgorithms��������ȡ��FPFH������SAC-IAƥ��
*/


#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/ndt.h>
#include <pcl/filters/approximate_voxel_grid.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/features/fpfh_omp.h>
#include <pcl/registration/ia_ransac.h>

using namespace std;
using namespace pcl;


// ����ICP����������㣩�㷨��ȡ״̬ת�ƾ���
// ���룺��
// �������������0
int ICPAlgorithms()
{
	PointCloud<PointXYZ>::Ptr cloud_in(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_out(new PointCloud<PointXYZ>);

	//�������ԭʼ����
	cloud_in->width = 5;
	cloud_in->height = 1;
	cloud_in->is_dense = false;
	cloud_in->points.resize(cloud_in->width * cloud_in->height);
	for (size_t i = 0; i < cloud_in->points.size(); ++i)
	{
		cloud_in->points[i].x = 1024 * rand() / (RAND_MAX + 1.0f);
		cloud_in->points[i].y = 1024 * rand() / (RAND_MAX + 1.0f);
		cloud_in->points[i].z = 1024 * rand() / (RAND_MAX + 1.0f);
	}

	//�����������
	std::cout << "Saved " << cloud_in->points.size() << " data points to input:"
		<< std::endl;
	for (size_t i = 0; i < cloud_in->points.size(); ++i) std::cout << "    " <<
		cloud_in->points[i].x << " " << cloud_in->points[i].y << " " <<
		cloud_in->points[i].z << std::endl;

	//���ɶԱ�����
	*cloud_out = *cloud_in;
	std::cout << "size:" << cloud_out->points.size() << std::endl;
	for (size_t i = 0; i < cloud_in->points.size(); ++i)
		cloud_out->points[i].x = cloud_in->points[i].x + 0.7f;					//ÿ����ƫ��0.7
	std::cout << "Transformed " << cloud_in->points.size() << " data points:" << std::endl;
	for (size_t i = 0; i < cloud_out->points.size(); ++i)
		std::cout << "    " << cloud_out->points[i].x << " " <<
		cloud_out->points[i].y << " " << cloud_out->points[i].z << std::endl;

	//��ȡ״̬ת�ƾ���
	IterativeClosestPoint<PointXYZ, PointXYZ> icp;
	icp.setInputSource(cloud_in);			//����Դ����
	icp.setInputTarget(cloud_out);			//����Ŀ�����
	PointCloud<PointXYZ> Final;
	icp.align(Final);

	//��������
	std::cout << "Final " << Final.size() << " data points:" << std::endl;
	for (size_t i = 0; i < Final.size(); i++)
	{
		std::cout << "    " << Final.at(i).x << " " << Final.at(i).y << " " << Final.at(i).z << std::endl;
	}

	//���ת�ƾ���
	std::cout << "has converged:" << icp.hasConverged() << " score: " <<
		icp.getFitnessScore() << std::endl;
	std::cout << icp.getFinalTransformation() << std::endl;


	return (0);
}

// NDTƥ��
// ���룺��
// �������������0
int NDTAlgorithms()
{
	PointCloud<PointXYZ>::Ptr target_cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr source_cloud(new PointCloud<PointXYZ>);

	//��������
	io::loadPCDFile<PointXYZ>("room_scan1.pcd", *target_cloud);
	io::loadPCDFile<PointXYZ>("room_scan2.pcd", *source_cloud);

	//��������ߴ����ٶ�
	PointCloud<PointXYZ>::Ptr filtered_cloud(new PointCloud<PointXYZ>);
	ApproximateVoxelGrid<PointXYZ> approximate_voxel_filter;
	approximate_voxel_filter.setLeafSize(0.2, 0.2, 0.2);
	approximate_voxel_filter.setInputCloud(source_cloud);		//ֻ��Ҫ�Դ�ƥ����ƽ�����
	approximate_voxel_filter.filter(*filtered_cloud);
	cout << "Filtered Cloud contains " << filtered_cloud->size() << " data points from room_scan2.pcd" << endl;

	//NDTƥ��
	NormalDistributionsTransform<PointXYZ, PointXYZ> ndt;
	PointCloud<PointXYZ>::Ptr output_cloud(new PointCloud<PointXYZ>);
	Eigen::AngleAxisf init_rotation(0.6931, Eigen::Vector3f::UnitZ());			//��ʼ��ת�Ƕ�
	Eigen::Translation3f init_translation(1.79387, 0.720047, 0);				//��ʼλ��
	Eigen::Matrix4f init_guess = (init_translation * init_rotation).matrix();	//��ʼλ��
	ndt.setTransformationEpsilon(0.01);		//����ֹͣʱ��Сת�����
	ndt.setStepSize(0.1);					//������������
	ndt.setResolution(1.0);					//����NDT���ؽṹ�ֱ���
	ndt.setMaximumIterations(35);			//����NDT��������
	ndt.setInputSource(filtered_cloud);		//���ô�ƥ�����
	ndt.setInputTarget(target_cloud);		//����ƥ��Ŀ�����
	ndt.align(*output_cloud, init_guess);	//ƥ��

	cout << "NDT has converged: " << ndt.hasConverged() << " score: " << ndt.getFitnessScore() << endl;

	//�任���
	transformPointCloud(*source_cloud, *output_cloud, ndt.getFinalTransformation());

	//������ӻ�
	boost::shared_ptr<visualization::PCLVisualizer> viewer(new visualization::PCLVisualizer("3D Viewer"));
	viewer->setBackgroundColor(0, 0, 0);
	visualization::PointCloudColorHandlerCustom<PointXYZ> target_color(target_cloud, 255, 0, 0);
	viewer->addPointCloud<PointXYZ>(target_cloud, target_color, "target cloud");
	viewer->setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_POINT_SIZE, 1, "target_cloud");
	visualization::PointCloudColorHandlerCustom<PointXYZ> output_color(output_cloud, 0, 255, 0);
	viewer->addPointCloud<PointXYZ>(output_cloud, output_color, "output cloud");
	viewer->setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_POINT_SIZE, 1, "output_cloud");

	viewer->addCoordinateSystem(1.0, "global");
	viewer->initCameraParameters();

	while (!viewer->wasStopped())
	{
		viewer->spinOnce();
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}

	return 0;
}


//������ȡ��FPFH������SAC-IAƥ��
//���룺��
//�������������0
int SACIAAlgorithms()
{
	PointCloud<PointXYZ>::Ptr source(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr target(new PointCloud<PointXYZ>);
	search::KdTree<PointXYZ>::Ptr tree(new search::KdTree<PointXYZ>());

	//��������
	io::loadPCDFile("bun0.pcd", *source);
	io::loadPCDFile("bun4.pcd", *target);

	//���㷨����
	PointCloud<Normal>::Ptr source_normal(new PointCloud<Normal>);
	PointCloud<Normal>::Ptr target_normal(new PointCloud<Normal>);
	NormalEstimationOMP<PointXYZ, Normal> ne;
	ne.setNumberOfThreads(4);
	ne.setSearchMethod(tree);
	ne.setKSearch(10);
	ne.setInputCloud(source);
	ne.compute(*source_normal);		//����source�㷨��
	ne.setInputCloud(target);
	ne.compute(*target_normal);		//����target�㷨��	

	//����FPFH����
	PointCloud<FPFHSignature33>::Ptr source_fpfh(new PointCloud<FPFHSignature33>);
	PointCloud<FPFHSignature33>::Ptr target_fpfh(new PointCloud<FPFHSignature33>);
	FPFHEstimationOMP<PointXYZ, Normal, FPFHSignature33> fe;
	fe.setNumberOfThreads(4);
	fe.setSearchMethod(tree);
	fe.setKSearch(10);
	fe.setInputCloud(source);
	fe.setInputNormals(source_normal);
	fe.compute(*source_fpfh);		//����source FPFH����
	fe.setInputCloud(target);
	fe.setInputNormals(target_normal);
	fe.compute(*target_fpfh);		//����target FPFH����
	
	//����
	SampleConsensusInitialAlignment<PointXYZ, PointXYZ, FPFHSignature33> sac_ia;
	PointCloud<PointXYZ>::Ptr align(new PointCloud<PointXYZ>);
	sac_ia.setInputSource(source);			//����Դ����
	sac_ia.setSourceFeatures(source_fpfh);	//����Դ��������
	sac_ia.setInputTarget(target);			//����Ŀ�����
	sac_ia.setTargetFeatures(target_fpfh);	//����Ŀ���������
	sac_ia.setNumberOfSamples(20);			//����ÿ�ε���������ʹ����������
	sac_ia.setCorrespondenceRandomness(6);	//���ü���Э����ʱѡ������ٽ���
	sac_ia.setMaximumIterations(500);		//��������������
	sac_ia.setTransformationEpsilon(0.01);	//����������
	sac_ia.align(*align);					//��׼

	//���ӻ�
	boost::shared_ptr<visualization::PCLVisualizer> view(new visualization::PCLVisualizer("result"));
	int v1;
	int v2;

	view->createViewPort(0.0, 0.0, 0.5, 1.0, v1);
	view->createViewPort(0.5, 0.0, 1.0, 1.0, v2);
	view->setBackgroundColor(0, 0, 0, v1);
	view->setBackgroundColor(0.1, 0.1, 0.1, v2);

	visualization::PointCloudColorHandlerCustom<PointXYZ> source_cloud_color(source, 255, 0, 0);
	view->addPointCloud(source, source_cloud_color, "source_cloud_v1", v1);
	visualization::PointCloudColorHandlerCustom<PointXYZ> target_cloud_color(target, 0, 255, 0);
	view->addPointCloud(target, target_cloud_color, "target_cloud_v1", v1);

	visualization::PointCloudColorHandlerCustom<PointXYZ>aligend_cloud_color(align, 0, 0, 255);
	view->addPointCloud(align, aligend_cloud_color, "aligend_cloud_v2", v2);
	view->addPointCloud(target, target_cloud_color, "target_cloud_v2", v2);

	view->setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_POINT_SIZE, 4, "aligend_cloud_v2");
	view->setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_POINT_SIZE, 2, "target_cloud_v2");

	while (!view->wasStopped())
	{
		view->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(10000));
	}

	return 0;
}



int main()
{

	//ICPAlgorithms();			//����ICP����������㣩�㷨��ȡ״̬ת�ƾ���
	//NDTAlgorithms();			//����NDT�㷨��׼
	SACIAAlgorithms();			//������ȡ��FPFH������SAC-IAƥ��

	system("pause");

	return 0;
}