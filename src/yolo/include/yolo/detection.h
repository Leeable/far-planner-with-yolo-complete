#pragma once
#ifndef __DETECTION_H__
#define __DETECTION_H__

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <string.h>
#include <vector>
#include <fstream>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>

using namespace std;
using namespace cv;
using namespace dnn;

struct DetectionParams {
    DetectionParams() = default;
    std::string cfg;
    std::string coconame;
    std::string weights;
};


class Detection {
public:
    Detection();
    ~Detection();
    void Init();
    void Loop();
    void Initalize(int width, int height);
    void ReadModel();
    bool Detecting(Mat frame);
    vector<String> GetOutputNames();
    void PostProcess();
    void Drawer();

    void DrawBoxes(int classId, float conf, int left, int top, int right, int bottom);
    Mat GetFrame();
    string GetResult();
    int GetResWidth();
    int GetResHeight();
    void LoadParmas();
    void ImageCallBack(const sensor_msgs::ImageConstPtr &msg);
private:
    ros::NodeHandle nh;
    ros::Subscriber yolo_sub_;
    ros::Publisher yolo_pub_;
    ros::Publisher yolo_image_;
    DetectionParams yl_params_;
    int m_width;
    int m_height;
    Net m_model;
    Mat m_frame;
    Mat m_blob;
    vector<Mat> m_outs;
    vector<float> m_confs;
    vector<Rect> m_boxes;
    vector<int> m_classIds;
    vector<int> m_perfIndx;
    int m_inpWidth;
    int m_inpHeight;
    float m_confThro;
    float m_NMSThro;
    vector<string> m_classes;

    void Dump();
};

#endif