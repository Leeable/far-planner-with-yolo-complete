#include "yolo/detection.h"
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "std_msgs/Int8.h"
#include "std_msgs/String.h"

using namespace std;
using namespace cv;
using namespace dnn;

Detection::Detection()
{
    m_width = 0;
    m_height = 0;
    m_inpWidth = 320;
    m_inpHeight = 320;
    m_confThro = 0.25;
    m_NMSThro = 0.4;
}

Detection::~Detection(){};

void Detection::Init()
{
    this->LoadParmas();
    ReadModel();
    yolo_sub_ = nh.subscribe("/camera/image", 1, &Detection::ImageCallBack, this);
    yolo_pub_ = nh.advertise<std_msgs::String>("yolo4_result",1,this);
    yolo_image_ = nh.advertise<sensor_msgs::Image>("yolo_image",10,this);
}

void Detection::Loop(){
    // ros::Rate loop_rate(10.0);
    while (ros::ok())
    {
        ros::spinOnce();
        // loop_rate.sleep();
    }
}

void Detection::ImageCallBack(const sensor_msgs::ImageConstPtr &msg)
{
    cv::Mat img;
    cv_bridge::CvImageConstPtr cv_ptr;
    cv_ptr = cv_bridge::toCvShare(msg, "bgr8");
    img = cv_ptr->image;
    this->Initalize(img.cols, img.rows);
    this->Detecting(img);
    // imshow("Show RGBImage", detection_.GetFrame());
    // char ch = waitKey(10);
    sensor_msgs::ImagePtr img_msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", this->GetFrame()).toImageMsg();
    yolo_image_.publish(*img_msg);
    string output = this->GetResult();
    std_msgs::String out_msg;
    std::stringstream ss;
    ss << output;
    out_msg.data = ss.str();
    yolo_pub_.publish(out_msg);
}

void Detection::Dump()
{
    m_outs.clear();
    m_boxes.clear();
    m_confs.clear();
    m_classIds.clear();
    m_perfIndx.clear();
}

void Detection::Initalize(int width, int height)
{
    m_width = width;
    m_height = height;
}

void Detection::ReadModel()
{
    // set the absolute path for the coco.names, yolo-cfg and yolo-weights
    string classesFile = "/home/lqz/ros_ws/src/detection_pkg/data/coco.names";
    string modelConfig = "/home/lqz/ros_ws/src/detection_pkg/data/yolov4-tiny.cfg";
    string modelWeights = "/home/lqz/ros_ws/src/detection_pkg/data/yolov4-tiny.weights";

//     std::string classesFile = yl_params_.coconame;
//     ROS_INFO("this is:%s", classesFile.c_str());
//     std::string modelConfig = yl_params_.cfg;
//     std::string modelWeights = yl_params_.weights;
    ifstream ifs(classesFile.c_str());
    string line;
    while (getline(ifs, line))
    {
        m_classes.push_back(line);
    }

    m_model = readNetFromDarknet(modelConfig, modelWeights);
    m_model.setPreferableBackend(DNN_BACKEND_OPENCV);
    m_model.setPreferableTarget(DNN_TARGET_OPENCL);
}

bool Detection::Detecting(Mat frame)
{
    Dump();
    m_frame = frame.clone();
    blobFromImage(m_frame, m_blob, 1 / 255.0, Size(m_inpWidth, m_inpHeight));
    m_model.setInput(m_blob);
    m_model.forward(m_outs, GetOutputNames());
    PostProcess();
    Drawer();
    return true;
}

vector<string> Detection::GetOutputNames()
{
    static vector<string> names;
    if (names.empty())
    {
        vector<int> outLayers = m_model.getUnconnectedOutLayers();
        vector<string> layersNames = m_model.getLayerNames();
        names.resize(outLayers.size());
        for (int i = 0; i < outLayers.size(); ++i)
        {
            names[i] = layersNames[outLayers[i] - 1];
        }
    }
    return names;
}

void Detection::PostProcess()
{
    for (int num = 0; num < m_outs.size(); num++)
    {
        Point Position;
        double confidence;
        float *data = (float *)m_outs[num].data;
        for (int j = 0; j < m_outs[num].rows; j++, data += m_outs[num].cols)
        {
            Mat scores = m_outs[num].row(j).colRange(5, m_outs[num].cols);
            minMaxLoc(scores, 0, &confidence, 0, &Position);
            if (confidence > m_confThro)
            {
                int centerX = (int)(data[0] * m_width);
                int centerY = (int)(data[1] * m_height);
                int width = (int)(data[2] * m_width);
                int height = (int)(data[3] * m_height);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                m_classIds.push_back(Position.x);
                m_confs.push_back((float)confidence);
                m_boxes.push_back(Rect(left, top, width, height));
            }
        }
    }
    NMSBoxes(m_boxes, m_confs, m_confThro, m_NMSThro, m_perfIndx);
}

void Detection::Drawer()
{
    for (int i = 0; i < m_perfIndx.size(); i++)
    {
        int idx = m_perfIndx[i];
        Rect box = m_boxes[idx];
        if (m_classIds[idx] == 0)
        {
            DrawBoxes(m_classIds[idx], m_confs[idx], box.x, box.y, box.x + box.width, box.y + box.height);
        }
    }
}

void Detection::DrawBoxes(int classId, float conf, int left, int top, int right, int bottom)
{
    rectangle(m_frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);
    string label = format("%.2f", conf);
    if (!m_classes.empty())
    {
        CV_Assert(classId < (int)m_classes.size());
        label = m_classes[classId] + ":" + label;
    }

    int baseLine;
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    rectangle(m_frame, Point(left, top - round(1.5 * labelSize.height)), Point(left + round(1.5 * labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
    putText(m_frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
}

Mat Detection::GetFrame()
{
    return m_frame;
}

string Detection::GetResult()
{
    string result = "";

    //获取所有最佳检测框信息
    for (int i = 0; i < m_perfIndx.size(); i++)
    {
        int idx = m_perfIndx[i];
        Rect box = m_boxes[idx];

        // std::cout << "m_classIds[idx]"<< m_classIds[idx] << std::endl;
        // std::cout << "m_confs[idx]"<< m_confs[idx] << std::endl;
        // std::cout << "box.x"<< box.x << std::endl;
        // std::cout << "box.y"<< box.y << std::endl;
        // std::cout << "box.width"<< box.width << std::endl;
        // std::cout << "box.height"<< box.height << std::endl;

        if (m_classIds[idx] == 0) //仅person类
        {
            stringstream strStream;
            // int、float类型都可以塞到stringstream中
            strStream << m_classIds[idx] << "," << m_confs[idx] << "," << box.x << "," << box.y << "," << box.width << "," << box.height;
            result = strStream.str();
        }
    }

    return result;
}

int Detection::GetResWidth()
{
    return m_width;
}

int Detection::GetResHeight()
{
    return m_height;
}



void Detection::LoadParmas()
{
    const std::string yolo_prefix = "/yolo/Yolo/";
    nh.param<std::string>(yolo_prefix + "cfg_path", yl_params_.cfg, "");
    nh.param<std::string>(yolo_prefix + "weight_path", yl_params_.weights, "");
    nh.param<std::string>(yolo_prefix + "coconame_path", yl_params_.coconame, "");
}

// yolo params

int main(int argc, char **argv)
{
    ros::init(argc, argv, "yolo_node");
    Detection detect_node = Detection();
    detect_node.Init();
    detect_node.Loop();
}
