/*
* Copyright (c) 2018 HadenSun
* Contact: http://www.sunhx.cn
* E-mail: admin@sunhx.cn
*
* Description:
*	 estimatingTheNormalsFeatures��PCA���Ʒ�����
*	 integralImageNormalEstimationFeatures������ͼ���������֯���Ƶķ��߹���
*	 pfhEstimationFeatures��PFH��������
*	 fpfhEstimationFeatures��FPFH��������
*
*/

#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/features/integral_image_normal.h>
#include <pcl/features/pfh.h>
#include <pcl/visualization/histogram_visualizer.h>
#include <pcl/visualization/pcl_plotter.h>
#include <pcl/features/normal_3d_omp.h>
#include <omp.h>
#include <pcl/features/fpfh.h>
#include <pcl/features/fpfh_omp.h>

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
//ע��÷���ֻ�������������
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
//������ֱ��ͼ(Point Feature Histograms)
//�����������ʾ����ʾ�����淨�ߺ����ʹ�����ĳ������Χ�ļ�������������ʾ����
//��Ȼ����ǳ��������ף������޷����̫����Ϣ����Ϊ����ֻʹ�ú��ٵ�
//��������ֵ�����Ʊ�ʾһ�����k����ļ���������Ȼ���󲿷ֳ����а�����������㣬
//��Щ����������ͬ�Ļ��߷ǳ����������ֵ����˲��õ�������ʾ����
//��ֱ�ӽ���ͼ�����ȫ�ֵ�������Ϣ��
//ÿһ��ԣ�ԭ��12��������6������ֵ��6��������̬�����ڷ��ߣ�
//PHF����ûһ��Ե� �������ǶȲ�ֵ����ֵ�� �����֮���ŷ�Ͼ��� d
//��12���������ٵ�4������
//Ĭ��PFH��ʵ��ʹ��5��������ࣨ���磺�ĸ�����ֵ�е�ÿ����ʹ��5��������ͳ�ƣ���
//���в��������루���������Ѿ����͹��ˡ��������������Ҫ�Ļ���
//Ҳ����ͨ���û�����computePairFeatures��������þ���ֵ����
//�����������һ��125������Ԫ�ص�����������15����
//�䱣����һ��pcl::PFHSignature125�ĵ������С�
//ע�⣺����PFH����ʱ��ϳ�
//���룺��
//�������������0
int pfhEstimationFeatures()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<Normal>::Ptr normals(new PointCloud<Normal>);

	//PFH����ʱ��̫����������ʾ
	cout << "��ʼ��������" << endl;

	//��������
	PCDReader reader;
	reader.read("table_scene_lms400_downsampled.pcd", *cloud);
	cout << "�����������" << endl;
	
	//���㷨��
	cout << "���㷨��" << endl;
	NormalEstimationOMP<PointXYZ, Normal> ne;		//����OpenMPʵ�ֶ��̼߳��㷨��
	search::KdTree<PointXYZ>::Ptr tree1(new search::KdTree<PointXYZ>());
	ne.setInputCloud(cloud);			//���ü�������
	ne.setSearchMethod(tree1);			//���������㷨KDtree����
	ne.setRadiusSearch(0.03);			//���������뾶
	ne.setNumberOfThreads(4);			//���ü����߳���
	ne.compute(*normals);
	cout << "���߼������" << endl;

	//����PFH������
	cout << "����PFH����" << endl;
	PFHEstimation<PointXYZ, Normal, PFHSignature125> pfh;
	search::KdTree<PointXYZ>::Ptr tree2(new search::KdTree<PointXYZ>());
	PointCloud<PFHSignature125>::Ptr pfhs(new PointCloud<PFHSignature125>());		//������ݼ�
	pfh.setInputCloud(cloud);			//����ԭʼ��������
	pfh.setInputNormals(normals);		//���뷨������
	pfh.setSearchMethod(tree2);			//����һ����KD��������PFH���ƶ���
	pfh.setRadiusSearch(0.05);			//5cm�������ھӲ�����㣬ע����ڷ��߹���ʱ�İ뾶
	pfh.compute(*pfhs);					//����PFH����
	cout << "PFH�����������" << endl;

	//ֱ��ͼ���ӻ�
	cout << "���չʾ" << endl;
	visualization::PCLPlotter plotter;
	plotter.addFeatureHistogram(*pfhs, "pfh",300);	//��ʾ��300����FPHֱ��ͼ
	plotter.plot();
	
	//ֱ��ͼ���ӻ�����2
	//1.8�汾�н�visualization\include\pcl\visualization\common\ren_win_interact_map.h�е�RenWinInteract�Ŀչ��캯����ɾ���ˡ�
	//���ʹ����ʹ��PCLHistogramVisualizerʱ������ȱ��RenWinInteract���������Ӵ���
	//��Դ��http://www.pclcn.org/bbs/forum.php?mod=viewthread&tid=1300&extra=page%3D1
	//visualization::PCLHistogramVisualizer view;//ֱ��ͼ���ӻ�
	//view.setBackgroundColor(255, 0, 0);//������ɫ
	//view.addFeatureHistogram<pcl::PFHSignature125>(*pfhs, "pfh", 100);
	//view.spinOnce();  //ѭ���Ĵ���

	return 0;
}

//FPFH����
//phf������ֱ��ͼ ���㸴�ӶȻ���̫��
//���㷨��-- - �����ٽ���ԽǶȲ�ֵ---- - ֱ��ͼ--
//��˴���һ��O(nk ^ 2) �ļ��㸴���ԡ�
//k����֮���໥�ĵ�� k��k��������
//���ٵ�����ֱ��ͼFPFH��Fast Point Feature Histograms�����㷨�ļ��㸴�ӶȽ��͵���O(nk) ��
//������Ȼ������PFH�󲿷ֵ�ʶ�����ԡ�
//��ѯ�����Χk��������� ��4��������
//Ҳ����1��k = k����
//Ĭ�ϵ�FPFHʵ��ʹ��11��ͳ�������䣨���磺�ĸ�����ֵ�е�ÿ���������Ĳ�������ָ�Ϊ11������
//����ֱ��ͼ���ֱ����Ȼ��ϲ��ó��˸���ֵ��һ��33Ԫ�ص�����������
//��Щ������һ��pcl::FPFHSignature33�������С�
//���룺��
//�������������0
int fpfhEstimationFeatures()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);

	//��������
	PCDReader reader;
	reader.read("table_scene_lms400_downsampled.pcd", *cloud);

	//���㷨��
	NormalEstimationOMP<PointXYZ, Normal> ne;
	PointCloud<Normal>::Ptr normal(new PointCloud<Normal>);
	search::KdTree<PointXYZ>::Ptr tree1(new search::KdTree<PointXYZ>());
	ne.setInputCloud(cloud);			//����ԭʼ��������
	ne.setSearchMethod(tree1);			//���������㷨
	ne.setRadiusSearch(0.03);			//���������뾶
	ne.setNumberOfThreads(4);			//���ü����߳�
	ne.compute(*normal);

	//����FPFH����
	FPFHEstimationOMP<PointXYZ, Normal, FPFHSignature33> fpfh;
	PointCloud<FPFHSignature33>::Ptr fpfhs(new PointCloud<FPFHSignature33>());
	search::KdTree<PointXYZ>::Ptr tree2(new search::KdTree<PointXYZ>());
	fpfh.setInputCloud(cloud);				//����ԭʼ��������
	fpfh.setInputNormals(normal);			//���÷�������
	fpfh.setSearchMethod(tree2);			//���ý��������㷨
	fpfh.setRadiusSearch(0.05);				//���������뾶��ע����ڷ��߹���ʱ�İ뾶
	fpfh.setNumberOfThreads(4);				//�����߳���
	fpfh.compute(*fpfhs);					

	//ֱ��ͼ���ӻ�
	visualization::PCLPlotter plotter;
	plotter.addFeatureHistogram(*fpfhs, 300);	//��ʾ��300����FPHֱ��ͼ
	plotter.plot();

	//ֱ��ͼ���ӻ�����2
	//1.8�汾�н�visualization\include\pcl\visualization\common\ren_win_interact_map.h�е�RenWinInteract�Ŀչ��캯����ɾ���ˡ�
	//���ʹ����ʹ��PCLHistogramVisualizerʱ������ȱ��RenWinInteract���������Ӵ���
	//��Դ��http://www.pclcn.org/bbs/forum.php?mod=viewthread&tid=1300&extra=page%3D1
	//visualization::PCLHistogramVisualizer view;//ֱ��ͼ���ӻ�
	//view.setBackgroundColor(255, 0, 0);//������ɫ
	//view.addFeatureHistogram<pcl::FPFHSignature33>(*pfhs, "fpfh", 100);
	//view.spinOnce();  //ѭ���Ĵ���

	return 0;
}

int main()
{
	//estimatingTheNormalsFeatures();			//PCA���Ʒ�����ʾ
	//integralImageNormalEstimationFeatures();	//����ͼ���������֯���Ƶķ��߹���
	//pfhEstimationFeatures();					//PFH����
	fpfhEstimationFeatures();					//FPFH����

	system("pause");

	return 0;
}