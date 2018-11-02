#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdlib.h>
#include "tld_utils.h"
#include "templateMatching.h"
#include <iostream>
#include <sstream>
#include <TLD.h>
#include <stdio.h>
#include <time.h>
using namespace cv;
using namespace std;

#include<iostream>
#include<string.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <cstdlib>
RNG rng(12345);
//Global variables
Rect box;
bool drawing_box = false;
bool gotBB = false;
bool tl = false;
bool rep = false;
bool fromfile=false;
string video;



void readBB(char* file){
  ifstream bb_file (file);
  string line;
  getline(bb_file,line);
  istringstream linestream(line);
  string x1,y1,x2,y2;
  getline (linestream,x1, ',');
  getline (linestream,y1, ',');
  getline (linestream,x2, ',');
  getline (linestream,y2, ',');
  int x = atoi(x1.c_str());// = (int)file["bb_x"];
  int y = atoi(y1.c_str());// = (int)file["bb_y"];
  int w = atoi(x2.c_str())-x;// = (int)file["bb_w"];
  int h = atoi(y2.c_str())-y;// = (int)file["bb_h"];
  box = Rect(x,y,w,h);
}
//bounding box mouse callback
void mouseHandler(int event, int x, int y, int flags, void *param){
  switch( event ){
  case CV_EVENT_MOUSEMOVE:
    if (drawing_box){
        box.width = x-box.x;
        box.height = y-box.y;
    }
    break;
  case CV_EVENT_LBUTTONDOWN:
    drawing_box = true;
    box = Rect( x, y, 0, 0 );
    break;
  case CV_EVENT_LBUTTONUP:
    drawing_box = false;
    if( box.width < 0 ){
        box.x += box.width;
        box.width *= -1;
    }
    if( box.height < 0 ){
        box.y += box.height;
        box.height *= -1;
    }
    gotBB = true;
    break;
  }
}
// 计算两点距离
float getTwoPointDistance(Point pointO, Point pointA) {
    float distance;
    distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
    distance = sqrtf(distance);
    return distance;
}

// 计算直线斜率
float computeAngle(Point pt0, Point pt1) {
	int dx = pt1.x - pt0.x;
	int dy = pt1.y - pt0.y;
  // printf("dx:%d,dy:%d",dx,dy);
	if(dx == 0)
	{
		if(dy < 0)
		{
			return CV_PI / 2.0;
		}
		else if(dy > 0)
		{
			return -CV_PI / 2.0;
		}
		else
		{
			return 0.0;
		}
	}
	return atanf((float)dy / dx);
}

void print_help(char** argv){
  printf("use:\n     %s -p /path/parameters.yml\n",argv[0]);
  printf("-s    source video\n-b        bounding box file\n-tl  track and learn\n-r     repeat\n");
}
 void  analyseStatic(Mat &threshold_output, Mat &drawing, int &type, BoundingBox& bbnext ){
   vector<vector<Point> > contours;
   vector<Vec4i> hierarchy;
  /// Find contours
   findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
   /// Find the convex hull object for each contour
   vector<vector<Point> >hull( contours.size() );
   // Int type hull
   vector<vector<int> > hullsI( contours.size() );
   // Convexity defects
   vector<vector<Vec4i> > defects( contours.size() );
  // 寻找质心
    Moments moment = moments(threshold_output, true);  
    Point center(moment.m10/moment.m00, moment.m01/moment.m00); 
      double maxArea = 0;
   vector<Point> maxContour;
   for( int i = 0; i < contours.size(); i++ ) {  
      convexHull( Mat(contours[i]), hull[i], false ); 
      // find int type hull
      convexHull( Mat(contours[i]), hullsI[i], false ); 
	    // get convexity defects
      if(hullsI[i].size() > 3 )
	    convexityDefects(Mat(contours[i]),hullsI[i], defects[i]);

     double area = contourArea(contours[i]);
     if (area > maxArea){
       maxArea = area;
       maxContour = contours[i];
     }
    }
     for( int i = 0; i< contours.size(); i++ )
      {
        if(maxContour ==contours[i] ){
          Scalar color = Scalar( 0,0,255 );
          drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
          vector<Vec4i>::iterator d =defects[i].begin();
          int a=0;
          Point listStart[4];
          Point listEnd[4];
          Point list[4];
          while( d!=defects[i].end() ) {
                  Vec4i& v=(*d);
                  int startidx=v[0]; 
                  Point ptStart( contours[i][startidx] ); // point of the contour where the defect begins
                  int endidx=v[1]; 
                  Point ptEnd( contours[i][endidx] ); // point of the contour where the defect ends
                  int faridx=v[2]; 
                  Point ptFar( contours[i][faridx] );// the farthest from the convex hull point within the defect
                  int depth = v[3] / 256; // distance between the farthest point and the convex hull
                  char image_name[20];
                  if(depth > 30 && depth < 120)
                  {
                 
                  line( drawing, ptStart, ptFar, CV_RGB(0,255,0), 2 );
                  line( drawing, ptEnd, ptFar, CV_RGB(0,255,0), 2 );
                  circle( drawing, ptStart,   4, Scalar(0,0,0), 2 );
                  circle( drawing, ptEnd,   4, Scalar(255,0,100), 2 );
                  circle( drawing, ptFar,   4, Scalar(100,0,255), 2 );
                  sprintf(image_name, "%d", a);
                  putText(drawing,image_name,ptFar,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0),1,8);
                  list[a] = ptFar;
                  listStart[a] = ptStart;
                  listEnd[a] = ptEnd;
                  a++;
                  }
              d++;
          }
        }
      }
}

 Mat thresh_callback(Mat threshold_output, Mat drawing,float &angle, int &y, bool &flag, BoundingBox& bbnext ){
 
   vector<vector<Point> > contours;
   vector<Vec4i> hierarchy;
  /// Find contours
   findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
   /// Find the convex hull object for each contour
   vector<vector<Point> >hull( contours.size() );
   // Int type hull
   vector<vector<int> > hullsI( contours.size() );
   // Convexity defects
   vector<vector<Vec4i> > defects( contours.size() );
  // 寻找质心
    Moments moment = moments(threshold_output, true);  
    Point center(moment.m10/moment.m00, moment.m01/moment.m00);  
    

   double maxArea = 0;
   vector<Point> maxContour;
   for( int i = 0; i < contours.size(); i++ ) {  
      convexHull( Mat(contours[i]), hull[i], false ); 
      // find int type hull
      convexHull( Mat(contours[i]), hullsI[i], false ); 
	    // get convexity defects
      if(hullsI[i].size() > 3 )
	    convexityDefects(Mat(contours[i]),hullsI[i], defects[i]);

     double area = contourArea(contours[i]);
     if (area > maxArea){
       maxArea = area;
       maxContour = contours[i];
     }
    }
   for( int i = 0; i< contours.size(); i++ )
      {
        if(maxContour ==contours[i] ){
          Scalar color = Scalar( 0,0,255 );
          drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
          vector<Vec4i>::iterator d =defects[i].begin();
          int a=0;
          Point listStart[4];
          Point listEnd[4];
          Point list[4];
          while( d!=defects[i].end() ) {
                  Vec4i& v=(*d);
                  int startidx=v[0]; 
                  Point ptStart( contours[i][startidx] ); // point of the contour where the defect begins
                  int endidx=v[1]; 
                  Point ptEnd( contours[i][endidx] ); // point of the contour where the defect ends
                  int faridx=v[2]; 
                  Point ptFar( contours[i][faridx] );// the farthest from the convex hull point within the defect
                  int depth = v[3] / 256; // distance between the farthest point and the convex hull
                  char image_name[20];
                  if(depth > 30 && depth < 120)
                  {
                 
                  line( drawing, ptStart, ptFar, CV_RGB(0,255,0), 2 );
                  line( drawing, ptEnd, ptFar, CV_RGB(0,255,0), 2 );
                  circle( drawing, ptStart,   4, Scalar(0,0,0), 2 );
                  circle( drawing, ptEnd,   4, Scalar(255,0,100), 2 );
                  circle( drawing, ptFar,   4, Scalar(100,0,255), 2 );
                  sprintf(image_name, "%d", a);
                  putText(drawing,image_name,ptFar,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0),1,8);
                  list[a] = ptFar;
                  listStart[a] = ptStart;
                  listEnd[a] = ptEnd;
                  a++;
                  }
              d++;
          }
          // 判断出现5指情况下
          if (a==4){
              circle(drawing, center, 8 ,Scalar(0, 0, 255), CV_FILLED);  
              bbnext.x = center.x-100;   // weighted average trackers trajectory with the close detections
              bbnext.y = center.y-100;
              bbnext.width = 200;
              bbnext.height =200;
              flag = true;
              float dis[6];
              dis[0]= getTwoPointDistance(list[0],list[1]);
              dis[1]= getTwoPointDistance(list[1],list[2]);
              dis[2]= getTwoPointDistance(list[2],list[3]);
              dis[3]= getTwoPointDistance(list[0],list[3]);
              dis[4]= getTwoPointDistance(list[1],list[3]);
              dis[5]= getTwoPointDistance(list[0],list[2]);
              float max=dis[5];
              for (int m=0;m<5;m++){
                if(dis[m]>max){
                  max = dis[m];
                }
              }
              Point start;
              Point end;
              for (int n=0;n<6;n++){
                if(max ==dis[n]){
                  switch(n){
                    case 0:
                      start = list[0];
                      end = list[1];
                      break;
                    case 1: 
                      start = list[1];
                      end = list[2];
                      break;
                    case 2:
                      start = list[2];
                      end = list[3];
                      break;
                    case 3:
                      start = list[0];
                      end = list[3];
                      break;
                    case 4:
                      start = list[0];
                      end = list[2];
                      break;
                    case 5:
                      start = list[1];
                      end = list[3];
                      break;
                  }
                }
              }
             
              angle= computeAngle(list[0],list[1]);
              y = center.y;
              line( drawing, start, end, CV_RGB(0,0,0), 2 );
          }else{
            flag = false;
            bbnext.width = 0;
            bbnext.height =0;
          }
          break;
        }
      }
  contours.clear();
  hierarchy.clear();
  return drawing;
 }

void read_options(int argc, char** argv,VideoCapture& capture,FileStorage &fs){
  for (int i=0;i<argc;i++){
      if (strcmp(argv[i],"-b")==0){
          if (argc>i){
              readBB(argv[i+1]);
              gotBB = true;
          }
          else
            print_help(argv);
      }
      if (strcmp(argv[i],"-s")==0){
          if (argc>i){
              video = string(argv[i+1]);
              capture.open(video);
              fromfile = true;
          }
          else
            print_help(argv);

      }
      if (strcmp(argv[i],"-p")==0){
          if (argc>i){
              fs.open(argv[i+1], FileStorage::READ);
          }
          else
            print_help(argv);
      }
      if (strcmp(argv[i],"-tl")==0){
          tl = true;
      }
      if (strcmp(argv[i],"-r")==0){
          rep = true;
      }
  }
}

// 根据皮肤颜色分割 获取手势
Mat gethand(Mat frame){
	Mat srcImage ;
  frame.copyTo(srcImage);
	if (!srcImage.data)
	{
		printf("could not load image...\n");
		return -1;
	}
	// imshow("srcImage", srcImage);
	Mat result, tmp;
	Mat Y, Cr, Cb;
	std::vector<Mat> channels;//定义一些Mat的变量用来存储Y Cr Cb的变量
 
	srcImage.copyTo(tmp);//将原图拷贝一份到tmp中
	cvtColor(tmp, tmp, CV_BGR2YCrCb);//转换到YCrCb空间
	split(tmp, channels);//通道分离的图存在channels中
	Y = channels.at(0);
	Cr = channels.at(1);
	Cb = channels.at(2);
 
	result = Mat::zeros(srcImage.size(), CV_8UC1);
	for (int i = 0; i < result.rows; i++)
	{
		//各个图首行的指针
		uchar* currentCr = Cr.ptr< uchar>(i);
		uchar* currentCb = Cb.ptr< uchar>(i);
		uchar* current = result.ptr< uchar>(i);
		for (int j = 0; j < result.cols; j++)
		{
			/*
			据资料显示，正常黄种人的Cr分量大约在133至173之间，
			Cb分量大约在77至127之间。大家可以根据自己项目需求放大或缩小这两个分量的范围，会有不同的效果。
      cr 反应rgb红色之间的差异
      cb反应rgb蓝色之间的差异
			*/
    // fprintf(bb_file,"Cr:%d,cb:%d",currentCr[j],currentCb[j]);
			if ((currentCr[j] > 133) && (currentCr[j] < 173) && (currentCb[j] > 77) && (currentCb[j] < 137))
				current[j] = 255;
			else
				current[i] = 0;
		}
	}
	// imshow("result", result);
  return result;
}

void client() {
    const unsigned short SERVERPORT = 2001;
    const int MAXSIZE = 1024;
    const char* SERVER_IP = "192.168.8.1";
    const char* DATA = "this is a client message ";

    int sock, recvBytes;
    char buf[MAXSIZE];
    //    hostent *host;
    sockaddr_in serv_addr;

    if( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cerr<<"socket create fail!"<<endl;
        exit(1);
    }
    bzero( &serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family =  AF_INET;
    serv_addr.sin_port = htons(SERVERPORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if( connect(sock, (sockaddr*)&serv_addr, sizeof(sockaddr)) == -1)
    {
        cerr<<"connect error"<<endl;
        exit(1);
    }

    write(sock, const_cast<char*>(DATA), strlen(DATA) );
    if( (recvBytes = recv(sock, buf, MAXSIZE, 0)) == -1)
    {
        cerr<<"recv error!"<<endl;
        exit(1);
    }
    cerr<<"ssssssssssss"<<endl;
    buf[recvBytes] = '\0';
    cout<<buf<<endl;
    close(sock);
}
int match_number = 0;
int g_nGaussianBlurValue = 4; //高斯滤波内核值

int main(int argc, char * argv[]){
  const unsigned short SERVERPORT = 2001;
  const int MAXSIZE = 1024;
  const char* SERVER_IP = "192.168.8.1";
  char DATA[4] = "";
  char wifisend[20];
  char wifisenderror[20] = "A000000000000000000";

  VideoCapture capture;
  capture.open(0);
  FileStorage fs;
  //Read options
  read_options(argc,argv,capture,fs);
  // Init camera
  if (!capture.isOpened())
  {
	cout << "capture device failed to open!" << endl;
    return 1;
  }
  // 通过wifi 获取小车摄像头视频数据

  // const string address = "http://192.168.8.1:8083/?action=stream.mjpg";
  // if (!capture.open(address))
  // {
	// cout << "wifi failed to open!" << endl;
  //   return 1;
  // }

    int sock, recvBytes;
    char buf[MAXSIZE];
    sockaddr_in serv_addr;
  // 与小车通信，与路由简历socket 链接 
    // if( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    // {
    //     cerr<<"socket create fail!"<<endl;
    //     exit(1);
    // }
    // bzero( &serv_addr, sizeof(serv_addr) );
    // serv_addr.sin_family =  AF_INET;
    // serv_addr.sin_port = htons(SERVERPORT);
    // serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // if( connect(sock, (sockaddr*)&serv_addr, sizeof(sockaddr)) == -1)
    // {
    //     cerr<<"connect error"<<endl;
    //     exit(1);
    // }
  

  Mat frame,result1;
  Mat frameHSV;	// hsv空间
  Mat last_gray;
  Mat first;
  if (fromfile){
      capture >> frame;
      cvtColor(frame, last_gray, CV_RGB2GRAY);
      frame.copyTo(first);
  }else{
      capture.set(CV_CAP_PROP_FRAME_WIDTH,340);
      capture.set(CV_CAP_PROP_FRAME_HEIGHT,240);
  }
  
  Mat staticpic (frame.rows, frame.cols, CV_8UC1);	// 2值掩膜
  Mat outPut(frame);	// 输出图像
  BoundingBox box;
  
  // int type=0;
  // medianBlur(frame, frame, 5); //中值滤波
  // staticpic = gethand(frame);
  // Mat staticelement = getStructuringElement(MORPH_RECT, Size(3,3));
  // erode(staticpic, staticpic, staticelement);//侵蚀
  // morphologyEx(staticpic, staticpic, MORPH_OPEN, staticelement);
  // dilate(staticpic, staticpic, staticelement);//膨胀
  // morphologyEx(staticpic, staticpic, MORPH_CLOSE, staticelement);
  // medianBlur(staticpic, staticpic, 5); //中值滤波
  // analyseStatic(staticpic,outPut,type,box);
  // imshow("outPut", outPut);//显示
	  init_hand_template();// 载入模板的轮廓
    //Output file
    FILE  *bb_file = fopen("bounding_boxes.txt","w");

    ///Run-time
    Mat current_gray;
    BoundingBox pbox;
    vector<Point2f> pts1;
    vector<Point2f> pts2;
    bool status=true;
    int frames = 1;
    int detections = 1;
    float angle;
    int y;
    float angleList[15000];
    int yList[15000];
    int i=0;
  REPEAT:
    while(capture.read(frame)){
      printf("startstartstartstart \n");
      
      Mat mask(frame.rows, frame.cols, CV_8UC1);	// 2值掩膜
      Mat dst(frame);	// 输出图像
      frame.copyTo(dst);
      medianBlur(frame, frame, 5); //中值滤波
      // imshow("zhongzhi",frame);
      mask = gethand(frame);
      // imshow( "gethand", mask );
      // 形态学操作，去除噪声，并使手的边界更加清晰
      Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
      erode(mask, mask, element);//侵蚀
      morphologyEx(mask, mask, MORPH_OPEN, element);
      dilate(mask, mask, element);//膨胀
      morphologyEx(mask, mask, MORPH_CLOSE, element);
      medianBlur(mask, mask, 5); //中值滤波
      thresh_callback(mask, frame, angle, y ,status, pbox); // 寻找手势轮廓
      // imshow( "dst demo", dst );
      // drawBox(dst,pbox);
      GaussianBlur(frame, frame, Size(g_nGaussianBlurValue * 2 + 1, g_nGaussianBlurValue * 2 + 1), 0);
      //基于椭圆皮肤模型的皮肤检测
      result1 = ellipse_detect(frame);
      cvtColor(result1, result1, CV_BGR2GRAY);
      threshold(result1, result1, 50, 255, THRESH_BINARY);
      //imshow("效果图-椭圆皮肤模型", result1);

      hand_contours(result1); // 对肤色分割、滤波去噪、开运算后图像进行轮廓提取并过滤  
      hand_template_match(match_number); //将目标轮廓与模板轮廓进行匹配
      number_draw(dst, match_number);
      printf("match_number:%d\n",match_number);
      switch(match_number){
        case 2:
          printf("right\n");
          DATA[0]='R';
          write(sock, DATA, strlen(DATA));
          break;
        case 3:
          DATA[0]='L';
          printf("left\n");
          write(sock, DATA, strlen(DATA));
          break;
        case 4:
          DATA[0]='U';
          printf("up\n");
          write(sock, DATA, strlen(DATA));
        break;
      }
      imshow("dst",frame);
      match_number = 0;
      
      if(status){
        angleList[i] = angle;
        yList[i] = y;
        DATA[0]='A';
        if(angleList[i]-angleList[0] > 0.4){
            printf("right\n");
            DATA[0]='R';
            write(sock, DATA, strlen(DATA));
          }else if(angleList[i]-angleList[0] < -0.4){
            DATA[0]='L';
            printf("left\n");
            write(sock, DATA, strlen(DATA));
          }else if(yList[i]-yList[0]>30){
            DATA[0]='D';
            printf("down\n");
            write(sock, DATA, strlen(DATA));
          }else if(yList[i]-yList[0]<-30){
            DATA[0]='U';
            printf("up\n");
            write(sock, DATA, strlen(DATA));
          }
            // printf("\nstart angle=%f,y=%d,aaaaa=%d\n",angleList[0],yList[0],0);
            printf("end angle=%f,y=%d,aaaaaaa=%d\n",angleList[i],yList[i],i);
            // printf("RES angle:%f,y:%d\n",angleList[i]-angleList[0],yList[i]-yList[0]);
          i++;
        
      }else if(i>0){
        i=0;
        DATA[0]='D';
        write(sock, DATA, strlen(DATA));
        memset(angleList, 0, sizeof(angleList));
        memset(yList, 0, sizeof(yList));
      }
      
      if (cvWaitKey(33) == 'q')
        break;
    }
    printf("endendendend \n");
    
    if (rep){
      rep = false;
      tl = false;
      fclose(bb_file);
      bb_file = fopen("final_detector.txt","w");
      //capture.set(CV_CAP_PROP_POS_AVI_RATIO,0);
      capture.release();
      capture.open(video);
      goto REPEAT;
    }
    fclose(bb_file);
    return 0;
}
