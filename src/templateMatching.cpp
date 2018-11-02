#include"templateMatching.h"

using namespace cv;
using namespace std;
// 模板匹配


const char *tmp_names[9] = { "1x.jpg","2x.jpg","3x.jpg","4x.jpg","5x.jpg" ,"6x.jpg" ,"7x.jpg","8x.jpg" ,"9x.jpg" };

vector< vector<Point> > mContoursTemp;  //轮廓模板集  
vector< vector<Point> > mContoursProc;  //待处理轮廓集 

//基于椭圆皮肤模型的皮肤检测
Mat ellipse_detect(Mat& src)
{
	Mat img = src.clone();
	Mat skinCrCbHist = Mat::zeros(Size(256, 256), CV_8UC1);
	//利用opencv自带的椭圆生成函数先生成一个肤色椭圆模型
	ellipse(skinCrCbHist, Point(113, 155.6), Size(23.4, 15.2), 43.0, 0.0, 360.0, Scalar(255, 255, 255), -1);
	Mat ycrcb_image;
	Mat output_mask = Mat::zeros(img.size(), CV_8UC1);
	cvtColor(img, ycrcb_image, CV_BGR2YCrCb); //首先转换成到YCrCb空间
	for (int i = 0; i < img.cols; i++)   //利用椭圆皮肤模型进行皮肤检测
		for (int j = 0; j < img.rows; j++)
		{
			Vec3b ycrcb = ycrcb_image.at<Vec3b>(j, i);
			if (skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) > 0)   //如果该落在皮肤模型椭圆区域内，该点就是皮肤像素点
				output_mask.at<uchar>(j, i) = 255;
		}

	Mat detect;
	img.copyTo(detect, output_mask);  //返回肤色图

	return detect;
}

//载入模板的轮廓  
void init_hand_template(void)
{
	Mat srcImage;
	int g_nGaussianBlurValue = 4; //高斯滤波内核值
	vector< vector<Point> > mContours,mCont;
	vector< Vec4i > mHierarchy;

	for (int i = 0; i < 9; i++)
	{
		// 读取文件  
		srcImage = imread(tmp_names[i]);
		Size sz = srcImage.size();

		GaussianBlur(srcImage, srcImage, Size(g_nGaussianBlurValue * 2 + 1, g_nGaussianBlurValue * 2 + 1), 0);
		srcImage = ellipse_detect(srcImage);
		cvtColor(srcImage, srcImage, CV_BGR2GRAY);
		threshold(srcImage, srcImage, 50, 255, THRESH_BINARY);

		// 寻找轮廓  
		findContours(srcImage, mContours, mHierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));

		int n = 0;
		double contArea = 0, contArea2 = 0;
		double imageArea = sz.width * sz.height;
		const int SIZE = mContours.size();

		for (int j = 0; j < SIZE; j++)
		{
			contArea = contourArea(mContours[j]);

			/*过滤小面积的轮廓  
			if (contArea / imageArea < 0.015)
			{
				continue;
			}
			*/
			if (contArea > contArea2)
			{
				contArea2 = contArea;
				n = j;
			}
			
		}
		mContoursTemp.push_back(mContours[n]);
	}

	cout << "mContoursTemp size = " << mContoursTemp.size() << endl;

}


void hand_contours(Mat &srcImage)
{
	Mat imageProc = srcImage.clone();

	Size sz = srcImage.size();
	Mat draw = Mat::zeros(sz, CV_8UC3);

	vector< vector<Point> > mContours;
	vector< Vec4i > mHierarchy;

	//只查找最外层轮廓  
	findContours(imageProc, mContours, mHierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));

	mContoursProc.clear();  //清空上次图像处理的轮廓  

	if (mContours.size() > 0)
	{
		int n = 0;
		double contArea = 0, contArea2 = 0;
		double imageArea = sz.width * sz.height;
		const int SIZE = mContours.size();

		Rect bound;

		for (int i = 0; i < SIZE; i++)
		{
			contArea = contourArea(mContours[i]);

			/* 过滤小面积的轮廓  
			if (contArea / imageArea < 0.015)
			{
				continue;
			}
			*/
			if (contArea > contArea2)
			{
				contArea2 = contArea;
				n = i;
			}
			
		}
		mContoursProc.push_back(mContours[n]);
		// 绘制过滤后的轮廓  
		draw = Scalar::all(0);          //将矩阵所有元素赋值为某个值  
		drawContours(draw, mContoursProc, -1, Scalar(0, 0, 255), 1, 8);
		// imshow("Filter Contours", draw);
	}
}

// 将目标轮廓与模板轮廓进行匹配  
void hand_template_match(int &match_number)
{
	if ((mContoursProc.size() == 0) || (mContoursTemp.size() == 0))
	{
		return;
	}
	double hu = 1.0, huTmp = 0.0;
	const int SIZE = mContoursProc.size();
	int m = 0, n = 0;
	match_number = 0;

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			huTmp = matchShapes(mContoursTemp[i], mContoursProc[j], CV_CONTOURS_MATCH_I1, 0);

			// hu矩越小，匹配度越高  
			if (huTmp < hu)
			{
				hu = huTmp;

				//保存好，是哪个轮廓和哪个模板匹配上了  
				m = i;
				n = j;
			}
		}
	}

	cout << "************m = " << (m ) << "; n = " << n << "; hu = " << hu << endl;
	// 匹配到的数字  
	if (hu <= 0.3)
	{
		match_number = m+1;
	}
	else
	{
		match_number = 0;
	}
	
}

const char *num_char[] = { "1", "2", "3", "4","5","6","7","8","9" };
// 在图片的左上角标注数字  
void number_draw(Mat &img, int num)
{  

	if (num >= 1)
	{
		string text = num_char[num - 1];
		putText(img, text, Point(5, 100), FONT_HERSHEY_SIMPLEX, 4, Scalar(0, 0, 255), 5);
	}
}
