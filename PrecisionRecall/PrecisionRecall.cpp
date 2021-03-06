// PrecisionRecall.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


pcl::PointCloud<pcl::PointXYZI>::Ptr cloudRGB2GRAY(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud) {
	pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_gray(new pcl::PointCloud<pcl::PointXYZI>);
	cloud_gray->height = cloud->height;
	cloud_gray->width = cloud->width;

	for (pcl::PointCloud<pcl::PointXYZRGB>::iterator it = cloud->begin(); it != cloud->end(); it++) {
		// Color conversion
		cv::Mat pixel(1, 1, CV_8UC3, cv::Scalar(it->r, it->g, it->b));
		cv::Mat temp;
		cv::cvtColor(pixel, temp, CV_RGB2GRAY);

		pcl::PointXYZI pointI;
		pointI.x = it->x;
		pointI.y = it->y;
		pointI.z = it->z;
		pointI.intensity = temp.at<uchar>(0, 0);

		cloud_gray->push_back(pointI);

	}
	return cloud_gray;
}

pcl::PointCloud<pcl::PointXYZI>::Ptr cloudRGB2S(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud) {
	pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_gray(new pcl::PointCloud<pcl::PointXYZI>);
	cloud_gray->height = cloud->height;
	cloud_gray->width = cloud->width;

	for (pcl::PointCloud<pcl::PointXYZRGB>::iterator it = cloud->begin(); it != cloud->end(); it++) {
		// Color conversion
		cv::Mat pixel(1, 1, CV_8UC3, cv::Scalar(it->r, it->g, it->b));
		cv::Mat temp;
		cv::cvtColor(pixel, temp, CV_RGB2HSV);

		pcl::PointXYZI pointI;
		pointI.x = it->x;
		pointI.y = it->y;
		pointI.z = it->z;
		pointI.intensity = 255 - temp.at<uchar>(0, 1);

		cloud_gray->push_back(pointI);

	}
	return cloud_gray;
}

void cloud_compare(pcl::PointCloud <pcl::PointXYZI>::Ptr cloud_ref, pcl::PointCloud <pcl::PointXYZI>::Ptr cloud_pred) {
	float TN = 0.f, TP = 0.f, FN = 0.f, FP = 0.f;

	pcl::KdTreeFLANN<pcl::PointXYZI> tree_ref;
	tree_ref.setInputCloud(cloud_ref);

	std::vector<int> nn_indices(1);
	std::vector<float> nn_dists(1);
	for (pcl::PointCloud<pcl::PointXYZI>::iterator it = cloud_pred->begin(); it != cloud_pred->end(); it++) {
		tree_ref.nearestKSearch(*it, 1, nn_indices, nn_dists);

		float i_ref = cloud_ref->points[nn_indices[0]].intensity;
		float i_pred = it->intensity;

		if (i_ref == 255 & i_pred == 255) TP++;
		else if (i_ref == 0 & i_pred == 0) TN++;
		else if (i_ref == 0 & i_pred == 255) FP++;
		else if (i_ref == 255 & i_pred == 0) FN++;

	}

	float precision = TP / (TP + FP);
	float recall = TP / (TP + FN);
	float accu = (TP + TN) / (TP + TN + FP + FN);
	float tpr = TP / (TP + FN);
	float tnr = TN / (TN + FP);
	float f_m = 2 * ((precision*recall) / (precision + recall));

	std::cout << "Precision: " << precision << std::endl;
	std::cout << "Recall: " << recall << std::endl;
	std::cout << "True Positive Rate: " << tpr << std::endl;
	std::cout << "True Negative Rate: " << tnr << std::endl;
	std::cout << "Accuracy: " << accu << std::endl;
	std::cout << "F measure: " << f_m << std::endl;
}

int main(int argc, char** argv) {

	pcl::PointCloud <pcl::PointXYZRGB>::Ptr cloud_org(new pcl::PointCloud <pcl::PointXYZRGB>);
	pcl::PointCloud <pcl::PointXYZRGB>::Ptr cloud_pb(new pcl::PointCloud <pcl::PointXYZRGB>);
	pcl::PLYReader reader;
	if (reader.read("lucy.ply", *cloud_org) == -1 & reader.read("lucy_pb.ply", *cloud_pb) == -1) {
		std::cout << "Cloud reading failed." << std::endl;
		return (-1);
	}

	pcl::PointCloud <pcl::PointXYZI>::Ptr cloud_pred = cloudRGB2GRAY(cloud_org);
	pcl::PointCloud <pcl::PointXYZI>::Ptr cloud_ref = cloudRGB2GRAY(cloud_pb);

	//  pcl::io::savePCDFile("lucy_gray.pcd",*cloud);

	cv::Mat gray_values(1, cloud_pred->size(), CV_8U);
	cv::Mat temp;

	int counter = 0;
	for (pcl::PointCloud<pcl::PointXYZI>::iterator it = cloud_pred->begin(); it != cloud_pred->end(); it++) {
		gray_values.at<uchar>(0, counter) = it->intensity;
		counter++;
	}

	double thres_v = cv::threshold(gray_values, temp, 0, 255, CV_THRESH_OTSU);
	std::cout << "Otsu threshold value = " << thres_v << std::endl;

	for (pcl::PointCloud<pcl::PointXYZI>::iterator it = cloud_pred->begin(); it != cloud_pred->end(); it++) {
		float v = it->intensity;
		if (v < thres_v) { it->intensity = 0; }
		else { it->intensity = 255; }
	}

	cloud_compare(cloud_ref, cloud_pred);

	int vp_1, vp_2;
	
	/*pcl::visualization::PCLVisualizer *viewer = new pcl::visualization::PCLVisualizer(argc, argv, "exemple");
	viewer->createViewPort(0.0, 0, 0.5, 1.0, vp_1);
	viewer->createViewPort(0.5, 0, 1.0, 1.0, vp_2);
	pcl::visualization::PointCloudColorHandlerGenericField<pcl::PointXYZI> pred_c(cloud_pred, "intensity");
	pcl::visualization::PointCloudColorHandlerGenericField<pcl::PointXYZI> ref_c(cloud_ref, "intensity");

	viewer->addPointCloud(cloud_pred, pred_c, "Predicted", vp_1);
	viewer->addPointCloud(cloud_ref, ref_c, "Reference", vp_2);

	viewer->spin();*/
	//System("pause");

	getchar();
	return (1);
}


