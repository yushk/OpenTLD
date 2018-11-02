// #include<tld_utils.h>
#include <opencv2/opencv.hpp>
using namespace cv;
#pragma once


cv::Mat ellipse_detect(cv::Mat& src); //基于椭圆皮肤模型的皮肤检测
void hand_contours(cv::Mat& srcImage); //对肤色分割、滤波去噪、开运算后图像进行轮廓提取并过滤 
void init_hand_template(void); ////载入模板的轮廓						    
void hand_template_match(int &match_number);// 将目标轮廓与模板轮廓进行匹配 							     
void number_draw(cv::Mat& img, int num);// 在图片的左上角标注数字		