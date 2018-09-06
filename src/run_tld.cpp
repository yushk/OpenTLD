#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>//��Ӵ˾䲻����˵����װ���óɹ�
#include <tld_utils.h>
#include <iostream>
#include <sstream>
#include <TLD.h>
#include <stdio.h>
#include <time.h>
using namespace cv;
using namespace std;
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

void print_help(char** argv){
  printf("use:\n     %s -p /path/parameters.yml\n",argv[0]);
  printf("-s    source video\n-b        bounding box file\n-tl  track and learn\n-r     repeat\n");
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

int main(int argc, char * argv[]){
  VideoCapture capture;
  capture.open(0);
  FileStorage fs;
  //Read options
  read_options(argc,argv,capture,fs);
  //Init camera
  if (!capture.isOpened())
  {
	cout << "capture device failed to open!" << endl;
    return 1;
  }
  //Register mouse callback to draw the bounding box
  cvNamedWindow("TLD",CV_WINDOW_AUTOSIZE);
  cvSetMouseCallback( "TLD", mouseHandler, NULL );
  //TLD framework
  TLD tld;
  //Read parameters file
  tld.read(fs.getFirstTopLevelNode());
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
  
  Mat mask(frame.rows, frame.cols, CV_8UC1);	// 2值掩膜
	Mat dst(frame);	// 输出图像
  medianBlur(frame, frame, 5); //中值滤波
  
  cvtColor( frame, frameHSV, CV_BGR2HSV ); //转换图像的颜色空间
  Mat dstTemp1(frame.rows, frame.cols, CV_8UC1);
  Mat dstTemp2(frame.rows, frame.cols, CV_8UC1);
  // 对HSV空间进行量化，得到2值图像，亮的部分为手的形状
  inRange(frameHSV, Scalar(0,30,30), Scalar(40,170,256), dstTemp1);
  inRange(frameHSV, Scalar(156,30,30), Scalar(180,170,256), dstTemp2);
  bitwise_or(dstTemp1, dstTemp2, mask);
  imshow("frame", frameHSV);
  imshow("mask", mask);
    
  // 形态学操作，去除噪声，并使手的边界更加清晰
  Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
  imshow("element", element);
  
  erode(mask, mask, element);//侵蚀
  morphologyEx(mask, mask, MORPH_OPEN, element);
  dilate(mask, mask, element);//膨胀
  morphologyEx(mask, mask, MORPH_CLOSE, element);

  frame.copyTo(dst, mask);
  imshow("dst", dst);
  mask.release();
  dst.release();
  // contours.clear();
  // hierarchy.clear();
  // filterContours.clear();

  // int frame_width = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);
  // int frame_height = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);
  // int frame_number = capture.get(CV_CAP_PROP_FRAME_COUNT);
  // cout << "frame_width is " << frame_width << endl;
  // cout << "frame_height is " << frame_height << endl;
  // cout << "frame_number is " << frame_number << endl;
  // srand((unsigned)time(NULL)); //ʱ���
  // long frameToStart = rand() % frame_number;//ȡ  ���֡��֮�ڵ� �����
  // cout <<"֡��ʼ�ĵط�"<< frameToStart << endl;

  // // Mat frame; //Mat����  ��ʵ����ͼ�����
  // char image_name[20];
  // imshow("che", frame);//��ʾ
  // for(int i=0;i<frame_number;i++){
  //   printf("iiii:%d\n",i);
  //   capture.set(CV_CAP_PROP_POS_FRAMES, i);//�Ӵ�ʱ��֡����ʼ��ȡ֡
  //   if (!capture.read(frame))
  //   {
  //       cout << "��ȡ��Ƶʧ��" << endl;
  //   }
  //   sprintf(image_name, "%s%d%s", "image/",i, ".jpg");//�����ͼƬ��
  //   printf("image_name:%s\n",image_name);
  //   imwrite(image_name, frame); //д��  ǰ����  path+name��Ҫ���˺�׺ ������ ֡
  // }
  // waitKey(0);

  ///Initialization
  GETBOUNDINGBOX:
    while(!gotBB) {
      if (!fromfile){
        capture >> frame;
      } else{
      first.copyTo(frame);
      }
      cvtColor(frame, last_gray, CV_RGB2GRAY);
      drawBox(frame,box);
      imshow("TLD", frame);
      if (cvWaitKey(33) == 'q')
        return 0;
    }
    if (min(box.width,box.height)<(int)fs.getFirstTopLevelNode()["min_win"]) {
        cout << "Bounding box too small, try again." << endl;
        gotBB = false;
        goto GETBOUNDINGBOX;
    }
    //Remove callback
    cvSetMouseCallback( "TLD", NULL, NULL );
    printf("Initial Bounding Box = x:%d y:%d h:%d w:%d\n",box.x,box.y,box.width,box.height);
    //Output file
    FILE  *bb_file = fopen("bounding_boxes.txt","w");
    //TLD initialization
    tld.init(last_gray,box,bb_file);

    ///Run-time
    Mat current_gray;
    BoundingBox pbox;
    vector<Point2f> pts1;
    vector<Point2f> pts2;
    bool status=true;
    int frames = 1;
    int detections = 1;
  REPEAT:
    while(capture.read(frame)){
      //get frame
      cvtColor(frame, current_gray, CV_RGB2GRAY);
      //Process Frame
      tld.processFrame(last_gray,current_gray,pts1,pts2,pbox,status,tl,bb_file);
      //Draw Points
      if (status){
        // drawPoints(frame,pts1);
        // drawPoints(frame,pts2);
        drawBox(frame,pbox);
        detections++;
      }
      //Display
      imshow("TLD", frame);
      //swap points and images
      swap(last_gray,current_gray);
      pts1.clear();
      pts2.clear();
      frames++;
      printf("Detection rate: %d/%d\n",detections,frames);
      if (cvWaitKey(33) == 'q')
        break;
    }
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
