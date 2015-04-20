/*************************************************************************
	> File Name: laserToImg.cpp
	> Author: 
	> Mail: 
	> Created Time: 2015��03��25�� ������ 15ʱ29��28��
 ************************************************************************/

#include<iostream>
#include"ros/ros.h"
#include"std_msgs/String.h"
#include <sensor_msgs/LaserScan.h>
#include <math.h>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include <sstream>
#include <vector>
using namespace cv;
using namespace std;

#define Z -120
#define dX -50
#define dY 50
#define F1 7.1695537050642884e+02
#define F2 7.1275153332443017e+02
#define D1 3.3311050187491378e+02
#define D2 2.8336132788893195e+02
#define LENGTH 100
#define RANGE 10

class laser2Img{
public:
	int width;
    int count,num;
	int height;
	ros::NodeHandle nh;
	ros::Subscriber laser_sub;
	Mat m_coord;
	Mat depth;
public:
	laser2Img(int iwidth, int iheight,const ros::NodeHandle& n){
		nh = n;
		width = iwidth;
		height = iheight;
		laser_sub = nh.subscribe("/scan",1000, &laser2Img::laserReceive,this);
        count = 0;
        num = 1;
	}
	void laserReceive(const sensor_msgs::LaserScan& data){
		int m_size = data.ranges.size();
		float angle_min, angle_max, angle_increment;
		angle_increment = data.angle_increment;
		float radian = data.angle_min;
		
		msg = Mat::zeros(0, 3, CV_32FC1);
		Mat temp = Mat::zeros(1,3,CV_32FC1);
		m_coord = Mat::zeros(m_size, 2, CV_32FC1);
		depth = Mat::zeros(m_size, 1, CV_32FC1);
		for (int i = 0; i < m_size; i++)
		{
			float l = data.ranges[i]*1000;
			depth.at<float>(i, 0) = l;
			float x = static_cast<float>(l * cos(radian));
			float y = static_cast<float>(l * sin(radian));
			m_coord.at<float>(i, 0) = x;
			m_coord.at<float>(i, 1) = y;
			radian += angle_increment;
		}
		int X = 0;
		float x, y;
		for (int i = 0; i < m_size; i++)
		{
			m_coord.at<float>(i,0) += dX;
			X = m_coord.at<float>(i, 0);
			x = m_coord.at<float>(i, 1) * F1 / X + D1;
			y = Z * F2 / X + D2;
	
			x = width - x;
			y = height - y;
			x += dY;
            		if(x < 0 || x > width || y < 0 || y > height)
            		    continue;
			temp.at<float>(0,0) = x;
			temp.at<float>(0,1) = y;
			temp.at<float>(0,2) = data.ranges[i]*1000;
			msg.push_back(temp);
		}
	};
};

int main(int argc,char** argv){
	ros::init(argc,argv,"test");
	ros::NodeHandle n;
	ros::Rate loop_rate(100);
	CvCapture *capture = cvCreateCameraCapture(1);
	IplImage* frame;
	frame = cvQueryFrame(capture);
	CvPoint point;
	laser2Img app(frame->width, frame->height, n);
	int count = 0;
	while(ros::ok()){
		ros::spinOnce();
		loop_rate.sleep();
		
		frame = cvQueryFrame(capture);
		int size = app.msg.rows;
		//ROS_INFO("3 %d", size);
		count++;
        if (count == 20){
			std::stringstream sstm_depth;
			sstm_depth << "/home/wangh09/data/depth/" << num << ".yml";
			cv::FileStorage storage1(sstm_depth.str().c_str(), cv::FileStorage::WRITE);
			storage1 << "depth" << app.depth;
			storage1.release();

			std::stringstream sstm_point;
			sstm_point << "/home/wangh09/data/point/" << num << ".yml";
			cv::FileStorage storage2(sstm_point.str().c_str(), cv::FileStorage::WRITE);
			storage2 << "point" << app.msg;
			storage2.release();

			string str;
			std::stringstream sstm;
			sstm << "/home/wangh09/data/image/" << num << ".jpg";
			str = sstm.str();
			cvSaveImage(str.c_str(), frame);

			ROS_INFO("%d", num);
		}
		for(int i=0;i<size;i++){
			point.x = app.msg.at<float>(i, 0);
			point.y = app.msg.at<float>(i, 1);
			//ROS_INFO("%d x:%d,y:%d", i, int(point.x), int(point.y));
			cvDrawLine(frame, point, point, cvScalar(255, 0, 0), 5);
		}
		cvShowImage(" ", frame);
        if (count == 20){
			string str;
			std::stringstream sstm;
			sstm << "/home/wangh09/data/image_plot/" << num << ".jpg";
			str = sstm.str();
			cvSaveImage(str.c_str(), frame);

			count = 0;
			num++;
		}
		char c = cvWaitKey(1);
		if(c == 32)
			break;
	}
}
