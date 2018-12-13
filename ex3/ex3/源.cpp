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
#include <pcl/features/vfh.h>
#include <pcl/visualization/range_image_visualizer.h>
#include <pcl/features/range_image_border_extractor.h>
#include <pcl/keypoints/narf_keypoint.h>
#include <pcl/features/narf_descriptor.h>
#include <pcl/console/parse.h>

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

	cout << "fpfh feature size:" << fpfhs->points.size() << endl;	//Ӧ���������ͬ

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

//VFH����
//�ӵ�����ֱ��ͼVFH(Viewpoint Feature Histogram)�����ӣ�
//����һ���µ�������ʾ��ʽ��Ӧ���ڵ��ƾ���ʶ��������ɶ�λ�˹������⡣
//���������������ּ�����������������Ӧ����Ŀ��ʶ�������λ�˹��ƣ�
//1.��չFPFH��ʹ�������������ƶ��������м�����ƣ�
//�ڼ���FPFHʱ���������ĵ�����������������е�֮��ĵ����Ϊ���㵥Ԫ��
//2.����ӵ㷽����ÿ������Ʒ���֮������ͳ����Ϣ��Ϊ�˴ﵽ���Ŀ�ģ�
//���ǵĹؼ��뷨����FPFH�����н��ӵ㷽�����ֱ�����뵽��Է��߽Ǽ��㵱��
//����չ��FPFH������˵��Ĭ�ϵ�VFH��ʵ��ʹ��45�����������ͳ�ƣ��������ӵ����Ҫʹ��128�����������ͳ�ƣ�
//����VFH����һ��308��������������С�
//��PCL������pcl::VFHSignature308�ĵ��������洢��ʾ��
//����һ����֪�ĵ������ݼ���ֻһ����һ��VFH�����ӣ����ϳɵ�PFH/FPFH��������Ŀ�͵����еĵ���Ŀ��ͬ��
//���룺��
//�������������0
int vfhEstimationFeatures()
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);

	//��ȡ��������
	PCDReader reader;
	reader.read("1258107463333_cluster_0_nxyz.pcd", *cloud);

	//���㷨��
	NormalEstimationOMP<PointXYZ, Normal> ne;
	PointCloud<Normal>::Ptr normal(new PointCloud<Normal>);
	search::KdTree<PointXYZ>::Ptr tree1(new search::KdTree<PointXYZ>());
	ne.setInputCloud(cloud);		//���������������
	ne.setSearchMethod(tree1);		//���������㷨KD��
	ne.setRadiusSearch(0.03);		//���������뾶0.03
	ne.setNumberOfThreads(4);		//���ü����߳�4
	ne.compute(*normal);			//���㷨��

	//����VFH
	VFHEstimation<PointXYZ, Normal, VFHSignature308> vfh;
	PointCloud<VFHSignature308>::Ptr vfhs(new PointCloud<VFHSignature308>());
	search::KdTree<PointXYZ>::Ptr tree2(new search::KdTree<PointXYZ>());
	vfh.setInputCloud(cloud);		//���������������
	vfh.setInputNormals(normal);	//�������뷨��
	vfh.setSearchMethod(tree2);		//���������㷨KD��
	vfh.compute(*vfhs);				//����VFH����

	cout << "vfh feature size: " << vfhs->points.size() << endl;		//Ӧ�õ���1

	//ֱ��ͼ���ӻ�
	visualization::PCLPlotter plotter;
	plotter.addFeatureHistogram(*vfhs, 300);
	plotter.plot();

	return 0;
}

//NARF����
//���룺angular_resolution �Ƕȷֱ��ʣ�Ĭ��0.5
//���룺coordinate_frame ����ϵ��Ĭ���������ϵ
//���룺setUnseenToMaxRange �Ƿ����в��ɼ��㿴�������룬Ĭ��false
//���룺support_size ����Ȥ��ĳߴ磨����ֱ������Ĭ��0.2
//���룺roation_invariant ������ת�������ԣ�Ĭ�Ͽ�
//�������������0
int narfDescriptor(float angular_resolution = 0.5f,
	RangeImage::CoordinateFrame coordinate_frame = RangeImage::CAMERA_FRAME,
	bool setUnseenToMaxRange = false,
	float support_size = 0.2f,
	bool rotation_invariant = true)
{
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>& point_cloud = *cloud;
	Eigen::Affine3f scene_sensor_pose(Eigen::Affine3f::Identity()); //����任
	PointCloud<PointWithViewpoint> far_ranges;						//���ӽǵĵ���

	
	//��ȡ��������
	PCDReader reader;
	reader.read("office_scene.pcd", *cloud);

	//���ô���������
	scene_sensor_pose = Eigen::Affine3f(Eigen::Translation3f(cloud->sensor_origin_[0],
		cloud->sensor_origin_[1],
		cloud->sensor_origin_[2])) *
		Eigen::Affine3f(cloud->sensor_orientation_);

	//��ȡԶ�����ļ�
	//reader.read("frame_00000_far_ranges.pcd", far_ranges);

	/*
	setUnseenToMaxRange = true;//�����в��ɼ��ĵ� ���� ������
	cout << "\nNo *.pcd file given => Genarating example point cloud.\n\n";
	for (float x = -0.5f; x <= 0.5f; x += 0.01f)
	{
		for (float y = -0.5f; y <= 0.5f; y += 0.01f)
		{
			PointXYZ point;  point.x = x;  point.y = y;  point.z = 2.0f - y;
			point_cloud.points.push_back(point);//���õ����е������
		}
	}
	point_cloud.width = (int)point_cloud.points.size();
	point_cloud.height = 1;
	*/
	

	//�ӵ������ݣ��������ͼ��
	//ֱ�Ӱ���ά����Ͷ��ɶ�άͼ��
	float noise_level = 0.0;	//�ݲ��ʣ���Ϊ1��X1��ռ��ڿ��ܲ�ֹһ�㣬0��ʾȥ�����ľ�����Ϊ����ֵ��0.05��ʾ������5cm��ƽ��
	float min_range = 0.0f;		//�����Сֵ��0��ʾȡ1��X1��ռ�����Զ��
	int border_size = 1;		//ͼ���ܱߵ�
	boost::shared_ptr<RangeImage> range_image_ptr(new RangeImage);
	RangeImage& range_image = *range_image_ptr;
	
	range_image.createFromPointCloud(point_cloud, angular_resolution, deg2rad(360.0f), deg2rad(180.0f),
		scene_sensor_pose, coordinate_frame, noise_level, min_range, border_size);
	//range_image.integrateFarRanges(far_ranges);		//����Զ�������
	if (setUnseenToMaxRange)
		range_image.setUnseenToMaxRange();

	//3D������ʾ
	visualization::PCLVisualizer viewer("3D Viewer");
	viewer.setBackgroundColor(1, 1, 1);			//������ɫ
	visualization::PointCloudColorHandlerCustom<PointWithRange> range_image_color_handler(range_image_ptr, 0, 0, 0);
	viewer.addPointCloud(range_image_ptr, range_image_color_handler, "range image");
	viewer.setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_POINT_SIZE, 2, "range image");
	viewer.initCameraParameters();

	//��ʾ���ͼ��ƽ��ͼ��
	visualization::RangeImageVisualizer range_image_widget("Range image");
	range_image_widget.showRangeImage(range_image);

	//��ȡNARF�ؼ���
	RangeImageBorderExtractor range_image_border_extractor;				//�������ͼ��ı߽���ȡ����������ȡNARF�ؼ���
	NarfKeypoint narf_keypoint_detector(&range_image_border_extractor);	//����NARF����
	narf_keypoint_detector.setRangeImage(&range_image);					//���õ��ƶ�Ӧ���ͼ
	narf_keypoint_detector.getParameters().support_size = support_size; //����Ȥ��ߴ磨�����ֱ����

	PointCloud<int> keypoint_indices;	//���ڴ洢�ؼ�������� PointCloud<int>
	narf_keypoint_detector.compute(keypoint_indices);	//����NARF
	cout << "�ҵ��ؼ��㣺" << keypoint_indices.points.size() << " key points." << endl;

	//3D��ʾ�ؼ���
	PointCloud<PointXYZ>::Ptr keypoints_ptr(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>& keypoints = *keypoints_ptr;
	keypoints.points.resize(keypoint_indices.points.size());		//��ʼ����С
	for (size_t i = 0; i < keypoint_indices.points.size(); i++)
	{
		keypoints.points[i].getVector3fMap() = range_image.points[keypoint_indices.points[i]].getVector3fMap();
	}

	visualization::PointCloudColorHandlerCustom<PointXYZ> keypoints_color_handler(keypoints_ptr, 255, 0, 0);
	viewer.addPointCloud<PointXYZ>(keypoints_ptr, keypoints_color_handler, "keypoints");		//�����ʾ�ؼ���
	viewer.setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_POINT_SIZE, 7, "keypoints");

	//��ȡNARF����
	vector<int> keypoints_indices2;
	keypoints_indices2.resize(keypoint_indices.points.size());
	for (size_t i = 0; i < keypoint_indices.size(); i++)
		keypoints_indices2[i] = keypoint_indices.points[i];				//narf�ؼ�������
	NarfDescriptor narf_descriptor(&range_image, &keypoints_indices2);	//narf����������
	narf_descriptor.getParameters().support_size = support_size;
	narf_descriptor.getParameters().rotation_invariant = rotation_invariant;
	PointCloud<Narf36> narf_descriptors;
	narf_descriptor.compute(narf_descriptors);
	cout << "Extracted " << narf_descriptors.size() << " descriptors for " << keypoint_indices.points.size() << " keypoints." << endl;


	while (!viewer.wasStopped())
	{
		range_image_widget.spinOnce();
		viewer.spinOnce();
		pcl_sleep(0.01);
	}

	return 0;

}

int main()
{
	//estimatingTheNormalsFeatures();			//PCA���Ʒ�����ʾ
	//integralImageNormalEstimationFeatures();	//����ͼ���������֯���Ƶķ��߹���
	//����PFH��FPFH�������ο�ex_etc���pfh_demo��fpfh_demo�������
	//pfhEstimationFeatures();					//PFH����
	//fpfhEstimationFeatures();					//FPFH����
	//vfhEstimationFeatures();					//VFH����
	narfDescriptor();							//NARF����

	system("pause");

	return 0;
}