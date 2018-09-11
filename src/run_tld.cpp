#include <opencv2/opencv.hpp>
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"
 #include <stdlib.h>
#include <tld_utils.h>
#include <iostream>
#include <sstream>
#include <TLD.h>
#include <stdio.h>
#include <time.h>
using namespace cv;
using namespace std;
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

float getTwoPointDistance(Point pointO, Point pointA)
{
    float distance;
    distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
    distance = sqrtf(distance);
    return distance;
}

float getLineSlope(Point a,Point b){
  float k;
   printf("start(%d,%d) end(%d,%d)\n",
                          a.x, a.y, b.x, b.y);
  k = (a.y-b.y)%(a.x-b.x);
  return k;
}
void print_help(char** argv){
  printf("use:\n     %s -p /path/parameters.yml\n",argv[0]);
  printf("-s    source video\n-b        bounding box file\n-tl  track and learn\n-r     repeat\n");
}

 Mat thresh_callback(Mat threshold_output, Mat drawing ){
 
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
    circle(drawing, center, 8 ,Scalar(0, 0, 255), CV_FILLED);  

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
   /// Draw contours + hull results
  //  Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
   for( int i = 0; i< contours.size(); i++ )
      {
        if(maxContour ==contours[i] ){
          Scalar color = Scalar( 0,0,255 );
          // drawContours( drawing, contours, i, color, -1, 8, vector<Vec4i>(), 0, Point() );
          //  drawContours(drawing, contours, i, Scalar(255), -1);
          drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
          vector<Vec4i>::iterator d =defects[i].begin();
          int a=0;
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
                  circle( drawing, ptStart,   4, Scalar(255,0,100), 2 );
                  circle( drawing, ptEnd,   4, Scalar(255,0,100), 2 );
                  circle( drawing, ptFar,   4, Scalar(100,0,255), 2 );
                  sprintf(image_name, "%d", a);
                  printf("aaaa:%d,depth:%d\n",a,depth);
                  putText(drawing,image_name,ptFar,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0),1,8);
                  a++;
                  if(a==3){
                    list[0]=ptStart;
                    list[1]=ptEnd;
                  }else if(a==4) {
                    list[2]=ptStart;
                    list[3]=ptEnd;
                    }
                  printf("start(%d,%d) end(%d,%d), far(%d,%d)\n",
                          ptStart.x, ptStart.y, ptEnd.x, ptEnd.y, ptFar.x, ptFar.y);
                  }
              d++;
          }

          
          if (a==4){
            float kk;
              printf("okokokokokokok\n");
              float dis1= getTwoPointDistance(list[0],list[3]);
              float dis2= getTwoPointDistance(list[1],list[2]);
              if(dis1>dis2){
               kk= getLineSlope(list[0],list[3]);
                printf("dis1 kk:%f\n",kk);
              }else{
                kk= getLineSlope(list[1],list[2]);
                printf("dis2 kk:%f\n",kk);
                
              }
              printf("dis1:%f,dis2:%f\n",dis1,dis2);
             line( drawing, list[0], list[3], CV_RGB(0,0,0), 2 );
             line( drawing, list[1], list[2], CV_RGB(255,255,255), 2 );
            
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
void getpic(Mat* frame){
  // int frame_width = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);
  // int frame_height = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);
  // int frame_number = capture.get(CV_CAP_PROP_FRAME_COUNT);
  // cout << "frame_width is " << frame_width << endl;
  // cout << "frame_height is " << frame_height << endl;
  // cout << "frame_number is " << frame_number << endl;
  // srand((unsigned)time(NULL)); //
  // long frameToStart = rand() % frame_number;//
  // // Mat frame; //
  // char image_name[20];
  // imshow("che", frame);//
  // // for(int i=0;i<frame_number;i++){
  //   capture.set(CV_CAP_PROP_POS_FRAMES, 50);//
  //   if (!capture.read(frame))
  //   {
  //       cout << "error " << endl;
  //   }
  //   sprintf(image_name, "%s%s", "image/1",".jpg");
  //   printf("image_name:%s\n",image_name);
  //   imwrite(image_name, frame); //
  // // }
  // waitKey(0);
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
    // printf("Cr:%d,cb:%d",currentCr[j],currentCb[j]);
			if ((currentCr[j] > 127) && (currentCr[j] < 173) && (currentCb[j] > 77) && (currentCb[j] < 137))
				current[j] = 255;
			else
				current[i] = 0;
		}
    // printf("\n");
	}
	// imshow("result", result);
  return result;
}

int main(int argc, char * argv[]){
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
  // const string address = "http://192.168.8.1:8083/?action=stream.mjpg";
  // if (!capture.open(address))
  // {
	// cout << "wifi failed to open!" << endl;
  //   return 1;
  // }

  Mat frame;
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
     

  
if (!fromfile){
        capture >> frame;
      } else{
      first.copyTo(frame);
      }
    printf("Initial Bounding Box = x:%d y:%d h:%d w:%d\n",box.x,box.y,box.width,box.height);
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
  REPEAT:
  //   while(capture.read(frame)){
      Mat mask(frame.rows, frame.cols, CV_8UC1);	// 2值掩膜
      Mat dst(frame);	// 输出图像
      medianBlur(frame, frame, 5); //中值滤波
      mask = gethand(frame);
      // imshow( "gethand", mask );
      // 形态学操作，去除噪声，并使手的边界更加清晰
      Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
      erode(mask, mask, element);//侵蚀
      morphologyEx(mask, mask, MORPH_OPEN, element);
      // imshow( "侵蚀", mask );
      
      dilate(mask, mask, element);//膨胀
      morphologyEx(mask, mask, MORPH_CLOSE, element);
      // imshow( "膨胀", mask );
      
      medianBlur(mask, mask, 5); //中值滤波
      // imshow( "gethand", mask );
      dst = thresh_callback(mask, frame ); // 寻找手势轮廓
      imshow( "Hull demo", dst );
  
      // // get frame
      // cvtColor(frame, current_gray, CV_RGB2GRAY);
      // //Process Frame
      // tld.processFrame(last_gray,current_gray,pts1,pts2,pbox,status,tl,bb_file);
      // //Draw Points
      // if (status){
      //   // drawPoints(frame,pts1);
      //   // drawPoints(frame,pts2);
      //   drawBox(frame,pbox);
      //   detections++;
      // }
      //Display
      // dst.copyTo(frame);
      // imshow("TLD", frame);
      //swap points and images
      swap(last_gray,current_gray);
      pts1.clear();
      pts2.clear();
      frames++;
      printf("Detection rate: %d/%d\n",detections,frames);
      // if (cvWaitKey(33) == 'q')
        // break;
    // }
    waitKey(0);
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
