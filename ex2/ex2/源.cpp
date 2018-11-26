/*
 * Copyright (c) 2018 HadenSun 
 * Contact: http://www.sunhx.cn
 * E-mail: admin@sunhx.cn
 *
 * Description:
 *	 �������ڷֱ���ò�ͬ������ʵ�ֲ�ͬ�˲�Ч����
 *   showPCD����ȡPCD�ļ�Ȼ��Ա���ʾ��
 *   pathThroughFilter����Χ���ˡ���ָ����Χ��ĵ�������ɾ����
 *	 voxelGridFilter���������ػ����緽ʽ��������
 *   statisticalOutlierRemovalFilter�����ھ���ͳ��ȥ���쳣����
 *   extractIndicedFilter���ӵ�����������ȡ����
 *	 radiusOutlierRemovalFilter�������ھ�����ȥ���쳣����
 *	 conditionalRemovalFilter������ָ������ȥ���쳣����
 *
 */


#include <iostream>
#include <pcl/point_types.h>
#include <pcl/filters/passthrough.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/conditional_removal.h>
#include <pcl/filters/radius_outlier_removal.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/segmentation/sac_segmentation.h>

using namespace std;
using namespace pcl;

int user_data;

//��ȡPCD�ļ�Ȼ��Ա���ʾ
//���˲������������Ѿ������ӦPCD�ļ����򿪿ɶԱȲ鿴���
//���룺file1 ��һ���ļ���
//���룺file2 �ڶ����ļ���
//����� ��
void showPCD(string file1, string file2)
{
	PointCloud<PointXYZ>::Ptr cloud_1(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_2(new PointCloud<PointXYZ>);

	//�ļ���ȡ
	PCDReader reader;
	reader.read<PointXYZ>(file1, *cloud_1);
	reader.read<PointXYZ>(file2, *cloud_2);

	//��ʾ����Ա�
	int v1(0), v2(0);
	visualization::PCLVisualizer viewer;
	viewer.setWindowName("�����ʾ");
	viewer.createViewPort(0, 0, 0.5, 1.0, v1);
	viewer.setBackgroundColor(0, 0, 0, v1);
	viewer.addPointCloud(cloud_1, "before", v1);

	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);
	viewer.setBackgroundColor(0.3, 0.3, 0.3, v2);
	viewer.addPointCloud(cloud_2, "after", v2);

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);
		user_data++;
	}
}


//��Χ���ˣ���ָ����Χ��ĵ�ᱻ�޳�
//���룺��
//�������������0
int pathThroughFilter()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_filtered(new PointCloud<PointXYZ>);

	//��ʼ����������
	cloud->width = 5;
	cloud->height = 1;
	cloud->points.resize(cloud->width * cloud->height);

	srand((int)time(0));		//��ʼ���������
	for (size_t i = 0; i < cloud->points.size(); ++i)
	{
		cloud->points[i].x = rand() / (RAND_MAX + 1.0f) - 0.5;
		cloud->points[i].y = rand() / (RAND_MAX + 1.0f) - 0.5;
		cloud->points[i].z = rand() / (RAND_MAX + 1.0f) - 0.5;
	}

	//չʾ���Ƹ�ֵ���
	cerr << "Cloud before filtering: " << endl;
	for (rsize_t i = 0; i < cloud->points.size(); ++i)
	{
		cerr << " " << cloud->points[i].x << " " << cloud->points[i].y << " " << cloud->points[i].z << " " << endl;
	}

	//���˶���
	PassThrough<PointXYZ> pass;
	pass.setInputCloud(cloud);
	pass.setFilterFieldName("z");		//�����ֶ�����Ϊz����
	pass.setFilterLimits(0.0, 1.0);		//���յļ��ֵ����Ϊ��0.0, 1.0��
	//pass.setFilterLimitsNegative(true);	//���շ�Χ��ת
	pass.filter(*cloud_filtered);

	//��ʾ���˽��
	cerr << "Cloud after filtering: " << endl;
	for (rsize_t i = 0; i < cloud_filtered->points.size(); i++)
	{
		cerr << " " << cloud_filtered->points[i].x << " " << cloud_filtered->points[i].y << " " << cloud_filtered->points[i].z << " " << endl;
	}

	//��ʾ����Ա�
	int v1(0), v2(0);
	visualization::PCLVisualizer viewer;
	viewer.createViewPort(0, 0, 0.5, 1.0, v1);			//�½����ڣ�λ�ò������ðٷֱ�
	viewer.setBackgroundColor(0, 0, 0, v1);				//���ô��ڱ���
	viewer.addPointCloud(cloud, "before", v1);			//��v1����ӵ�������

	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);		//�½�����v2
	viewer.setBackgroundColor(0.3, 0.3, 0.3, v2);		//����v2����
	viewer.addPointCloud(cloud_filtered, "after", v2);	//��v2����ӵ�������
	//viewer.spin();		//��������ͣ��3D���治���ִ��

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);		//3D��������100ms���������߳�
		user_data++;
	}

	return 0;
}

//�������ػ����緽ʽ������
//VoxelGrid����������������ϴ���3D�������񣨽�����������Ϊ�ռ��е�һ��΢С3D�򣩡�
//Ȼ����ÿ�����أ���3D���У����д��ڵĵ㽫�����ǵ����Ľ��ƣ������²������� 
//���ַ����������ص����ıƽ�����Ҫ��һЩ��������׼ȷ�ش���������ı��档
//���룺��
//�������������0
int voxelGridFilter()
{
	PCLPointCloud2::Ptr cloud(new PCLPointCloud2());
	PCLPointCloud2::Ptr cloud_filtered(new PCLPointCloud2());
	PointCloud<PointXYZ>::Ptr cloud_xyz(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_xyz_filtered(new PointCloud<PointXYZ>);

	//�������
	PCDReader reader;
	reader.read("table_scene_lms400.pcd", *cloud);		//��ȡ����
	cerr << "PointCloud before filtering: " << cloud->width * cloud->height << " data points (" << getFieldsList(*cloud) << ")." << endl;

	//�������˶���
	VoxelGrid<PCLPointCloud2> sor;
	sor.setInputCloud(cloud);
	sor.setLeafSize(0.01f, 0.01f, 0.01f);		//���ù�����Ҷ��С1cm 1cm 1cm
	sor.filter(*cloud_filtered);

	cerr << "PointCloud after filtering: " << cloud_filtered->width * cloud_filtered->height << " data points (" << getFieldsList(*cloud_filtered) << ")." << endl;

	//���˽��д��pcd�ļ�
	PCDWriter writer;
	writer.write("table_scene_lms400_downsampled.pcd", *cloud_filtered, Eigen::Vector4f::Zero(), Eigen::Quaternionf::Identity(), false);

	//���Ƹ�ʽת��
	fromPCLPointCloud2(*cloud, *cloud_xyz);
	fromPCLPointCloud2(*cloud_filtered, *cloud_xyz_filtered);

	//��ʾ����Ա�
	int v1(0), v2(0);
	visualization::PCLVisualizer viewer;
	viewer.setWindowName("�����ʾ");
	viewer.createViewPort(0, 0, 0.5, 1.0, v1);
	viewer.setBackgroundColor(0, 0, 0, v1);
	viewer.addPointCloud(cloud_xyz, "before",v1);
	
	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);
	viewer.setBackgroundColor(0.3, 0.3, 0.3, v2);
	viewer.addPointCloud(cloud_xyz_filtered, "after",v2);

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);
		user_data++;
	}
	
	return 0;
}

//���ھ���ͳ��ȥ���쳣����
//���ǵ�ϡ���쳣ֵȥ�������������ݼ��е㵽�ھӾ���ķֲ��ļ��㡣����ÿ���㣬���Ǽ�������������ھӵ�ƽ�����롣
//ͨ���������õ��ķֲ��Ǿ���ƽ��ֵ�ͱ�׼ƫ��ĸ�˹�ֲ���
//����ƽ����������ȫ�־���ƽ��ֵ�ͱ�׼ƫ��������֮��ĵ���Ա���Ϊ���쳣ֵ���Ҵ����ݼ����޼���
//���룺��
//������������0
int statisticalOutlierRemovalFilter()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_filtered(new PointCloud<PointXYZ>);

	//�������
	PCDReader reader;
	reader.read<PointXYZ>("table_scene_lms400.pcd", *cloud);	

	cerr << "Cloud before filtering: " << std::endl;
	cerr << *cloud << endl;

	//�����˲�������
	StatisticalOutlierRemoval<PointXYZ> sor;
	//���㷨���������������Σ��ڵ�һ�ε����ڼ䣬��������ÿ�������������k���ھ�֮���ƽ�����롣 ����ʹ��setMeanK��������k��ֵ��
	//������������������Щ�����ƽ��ֵ�ͱ�׼ƫ��Ա�ȷ��������ֵ��
	//������ֵ�����ڣ�mean + stddev_mult * stddev�� ����ʹ��setStddevMulThresh�������ñ�׼ƫ��ĳ�����
	//����һ�ε����ڼ䣬������ǵ�ƽ�����ھ���ֱ���ڻ���ڸ���ֵ����㽫������Ϊ�ڲ����쳣ֵ��
	//Ϊÿ����ѯ���ҵ����ھӽ���setInputCloud���������е����ҵ���������������setIndices������������Щ�㡣 
	//setIndices������������������Ϊ������ѯ������ĵ㡣
	sor.setInputCloud(cloud);
	sor.setMeanK(50);					//������50���ھӼ���ƽ������
	sor.setStddevMulThresh(1.0);		//���ñ�׼��Ŵ�ϵ��
	sor.filter(*cloud_filtered);

	cerr << "Cloud after filtering: " << endl;
	cerr << *cloud_filtered << endl;

	//д��PCD�ļ�
	PCDWriter writer;
	writer.write<PointXYZ>("table_scene_lms400_inliers.pcd", *cloud_filtered, false);	

	//����ȡ������ȡ����
	//sor.setNegative(true);
	//sor.filter(*cloud_filtered);
	//writer.write<PointXYZ>("table_scene_lms400_outliers.pcd", *cloud_filtered, false);

	//��ʾ����Ա�
	int v1(0), v2(0);
	visualization::PCLVisualizer viewer;
	viewer.setWindowName("�����ʾ");
	viewer.createViewPort(0, 0, 0.5, 1.0, v1);
	viewer.setBackgroundColor(0, 0, 0, v1);
	viewer.addPointCloud(cloud, "before", v1);

	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);
	viewer.setBackgroundColor(0.3, 0.3, 0.3, v2);
	viewer.addPointCloud(cloud_filtered, "after", v2);

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);
		user_data++;
	}

	return 0;
}

//�ӵ�����������ȡ����
//ʹ�������ػ��˲���Ľ��������
//����SACSegmentation�ָ���ƣ�����ExtractIndices������ȡ�ָ�ĵ��ơ�
//���룺��
//�������������0
int extractIndicedFilter()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_p(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_f(new PointCloud<PointXYZ>);

	//��ȡ��������
	PCDReader reader;
	PCDWriter writer;
	reader.read<PointXYZ>("table_scene_lms400_downsampled.pcd", *cloud);

	ModelCoefficients::Ptr coefficients(new ModelCoefficients());		//�ṹ�壬�洢��ϳ���ģ��ϵ�����������ƽ�棬��ѧ����ax+by+cz+d=0��һ��ƽ�棬����洢a,b,c,d
	PointIndices::Ptr inliers(new PointIndices());						//�ṹ�壬�洢��ϳ��ķ���ģ�͵ĵ�����
	//�����ָ����
	SACSegmentation<PointXYZ> seg;
	//��ѡ
	seg.setOptimizeCoefficients(true);
	//��ѡ
	seg.setModelType(SACMODEL_PLANE);		//����ģ�����ͣ�ƽ��/��/Բ2D3D/��/Բ��/׵��/ƽ���ߵȿ�ѡ
	seg.setMethodType(SAC_RANSAC);			//���÷�������SAC_RANSAC���������һ�£�
	seg.setMaxIterations(1000);				//��������������
	seg.setDistanceThreshold(0.01);			//��ģ�;������ֵ

	//�����˲�����
	ExtractIndices<PointXYZ> extract;

	int i = 0;
	int nr_points = (int)cloud->points.size();

	//30% ��ԭʼ�㻹���������ȡ
	while (cloud->points.size() > 0.3*nr_points)
	{
		//��ʣ������зָ������ƽ�����
		seg.setInputCloud(cloud);
		seg.segment(*inliers, *coefficients);			//��ϳ���ƽ��ĵ�������ƽ������ֱ����inliers��coefficients
		if (inliers->indices.size() == 0)
		{
			cerr << "Could not estimate a planar model for the given dataset. " << endl;
			break;
		}

		//��ȡ����
		extract.setInputCloud(cloud);		//����ԭʼ��������
		extract.setIndices(inliers);		//���õ�������
		extract.setNegative(false);
		extract.filter(*cloud_p);			//��ȡ����
		cerr << "PointCloud representing the planar component: " << cloud_p->width * cloud_p->height << " data points." << endl;

		stringstream ss;
		ss << "table_scene_lms400_plane_" << i << ".pcd";
		writer.write<PointXYZ>(ss.str(), *cloud_p, false);		//�洢�ָ��ĵ�������

		//��ȡʣ�µĵ�
		extract.setNegative(true);
		extract.filter(*cloud_f);
		cloud.swap(cloud_f);
		i++;
	}

	return 0;
}

//�����ھ�������������
//���룺��
//������������0
int radiusOutlierRemovalFilter()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_filtered(new PointCloud<PointXYZ>);

	//��������
	cloud->width = 5;
	cloud->height = 1;
	cloud->points.resize(cloud->height * cloud->width);

	srand((int)time(0));		//��ʼ���������
	for (size_t i = 0; i < cloud->points.size(); ++i)
	{
		cloud->points[i].x = rand() / (RAND_MAX + 1.0f) - 0.5;
		cloud->points[i].y = rand() / (RAND_MAX + 1.0f) - 0.5;
		cloud->points[i].z = rand() / (RAND_MAX + 1.0f) - 0.5;
	}

	//��ӡ����
	cerr << "Cloud before filtering: " << endl;
	for (size_t i = 0; i < cloud->points.size(); ++i)
	{
		cerr << " " << cloud->points[i].x << " " << cloud->points[i].y << " " << cloud->points[i].z << endl;
	}

	//����������
	RadiusOutlierRemoval<PointXYZ> outrem;
	//������������һ�Σ����Ҷ���ÿ���㣬�����ض��뾶�ڵ��ھӵ�������
	//����ھ�̫�٣���õ㽫����Ϊ�쳣ֵ����setMinNeighborsInRadius����ȷ���� 
	//����ʹ��setRadiusSearch�������İ뾶��
	//Ϊÿ����ѯ���ҵ����ھӽ���setInputCloud���������е����ҵ���������������setIndices������������Щ�㡣
	//setIndices������������������Ϊ������ѯ������ĵ㡣
	outrem.setInputCloud(cloud);			//���������������
	outrem.setRadiusSearch(0.8);			//���������뾶0.8
	outrem.setMinNeighborsInRadius(2);		//�����ھ���ֵ2
	outrem.filter(*cloud_filtered);

	//��ӡ���
	cerr << "Cloud after filtering: " << endl;
	for (size_t i = 0; i < cloud_filtered->points.size(); ++i)
	{
		cerr << " " << cloud_filtered->points[i].x << " " << cloud_filtered->points[i].y << " " << cloud_filtered->points[i].z << endl;
	}

	//��ʾ����Ա�
	int v1(0), v2(0);
	visualization::PCLVisualizer viewer;
	viewer.setWindowName("�����ʾ");
	viewer.createViewPort(0, 0, 0.5, 1.0, v1);
	viewer.setBackgroundColor(0, 0, 0, v1);
	viewer.addPointCloud(cloud, "before", v1);

	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);
	viewer.setBackgroundColor(0.3, 0.3, 0.3, v2);
	viewer.addPointCloud(cloud_filtered, "after", v2);

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);
		user_data++;
	}

	return 0;
}

//���ڸ�����������������
//���룺��
//������������0
int conditionalRemovalFilter()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_filtered(new PointCloud<PointXYZ>);

	//��������
	cloud->width = 5;
	cloud->height = 1;
	cloud->points.resize(cloud->height * cloud->width);

	srand((int)time(0));		//��ʼ���������
	for (size_t i = 0; i < cloud->points.size(); ++i)
	{
		cloud->points[i].x = rand() / (RAND_MAX + 1.0f) - 0.5;
		cloud->points[i].y = rand() / (RAND_MAX + 1.0f) - 0.5;
		cloud->points[i].z = rand() / (RAND_MAX + 1.0f) - 0.5;
	}

	//��ӡ����
	cerr << "Cloud before filtering: " << endl;
	for (size_t i = 0; i < cloud->points.size(); ++i)
	{
		cerr << " " << cloud->points[i].x << " " << cloud->points[i].y << " " << cloud->points[i].z << endl;
	}

	//��������
	ConditionAnd<PointXYZ>::Ptr range_cond(new ConditionAnd<PointXYZ>());
	range_cond->addComparison(FieldComparison<PointXYZ>::ConstPtr(new FieldComparison<PointXYZ>("z", ComparisonOps::GT, 0.0)));		//greater than(GT)����0.0
	range_cond->addComparison(FieldComparison<PointXYZ>::ConstPtr(new FieldComparison<PointXYZ>("z", ComparisonOps::LT, 0.8)));		//less than(LT)С��0.8

	//����������
	ConditionalRemoval<PointXYZ> condrem;
	//ConditionalRemoval���������ض����������ݡ�
	//����ΪConditionalRemoval�ṩ������ 
	//���������͵�������ConditionAnd��ConditionOr�� 
	//������Ҫһ�������ȽϺ� / ������������ �ȽϾ������ƣ��Ƚ��������ֵ��
	//���������ıȽ����ͣ����ƿ��Զ�Ӧ��PointCloud�ֶ����ƣ���rgb��ɫ�ռ��hsi��ɫ�ռ��е���ɫ������
	condrem.setCondition(range_cond);
	condrem.setInputCloud(cloud);
	condrem.setKeepOrganized(true);
	condrem.filter(*cloud_filtered);

	//��ӡ���
	cerr << "Cloud after filtering: " << endl;
	for (size_t i = 0; i < cloud_filtered->points.size(); ++i)
	{
		cerr << " " << cloud_filtered->points[i].x << " " << cloud_filtered->points[i].y << " " << cloud_filtered->points[i].z << endl;
	}

	//��ʾ����Ա�
	int v1(0), v2(0);
	visualization::PCLVisualizer viewer;
	viewer.setWindowName("�����ʾ");
	viewer.createViewPort(0, 0, 0.5, 1.0, v1);
	viewer.setBackgroundColor(0, 0, 0, v1);
	viewer.addPointCloud(cloud, "before", v1);

	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);
	viewer.setBackgroundColor(0.3, 0.3, 0.3, v2);
	viewer.addPointCloud(cloud_filtered, "after", v2);

	while (!viewer.wasStopped())
	{
		viewer.spinOnce(100);
		user_data++;
	}

	return 0;
}

//������
//������Ч���ĵ���
int main(int argc, char** argv)
{
	//pathThroughFilter();					//ָ����Χ���޳�
	//voxelGridFilter();					//�²��������ٵ����
	statisticalOutlierRemovalFilter();	//���ھ���ͳ���˲����˳�����
	//radiusOutlierRemovalFilter();			//���ڷ�Χ���ھ����˲����˳�����
	//conditionalRemovalFilter();			//���ڸ�����������������
	//extractIndicedFilter();				//�ӵ�����������ȡ����


	//����������Ľ���������ڸ��Ե�PCD�ļ�������ֱ�Ӵ򿪲鿴
	//showPCD("table_scene_lms400_plane_1.pcd", "table_scene_lms400_plane_0.pcd");

	system("pause");

	return 0;
}