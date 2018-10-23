# This will be a C++ implementations of the OpenTLD (aka Predator)
---
This is a work in progress, as of right now the code is functional but pretty slow.
---
## Installation openCV 2.4.9

---
[ubuntu 安装opencv链接](https://blog.csdn.net/u013066730/article/details/79411767)

## Installation Notes
---
    git clone git@github.com:yushk/OpenTLD.git
    cd OpenTLD
    mkdir build
    cd build
    cmake ../src/
    make
    cd ../bin/
### To run from camera
    ./run_tld -p ../parameters.yml -tl
### To run from file
    ./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg -tl
    ./run_tld -p ../parameters.yml -s ../datasets/11_hand/right.mp4 -tl
    ./run_tld -p ../parameters.yml -s ../datasets/11_hand/hand.png -tl
    
### To init bounding box from file
    ./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt -tl
### To train only in the firs frame (no tracking, no learning)
    ./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt 
### To test the final detector (Repeat the video, first time learns, second time detects)
    ./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt -tl -r
### head test
    ./run_tld -p ../parameters.yml -s ../datasets/11_hand/right.mp4
    
---
Evaluation
---
The output of the program is a file called bounding_boxes.txt which contains all the detections made through the video. This file should be compared with the ground truth file to evaluate the performance of the algorithm. This is done using a python script:
python ../datasets/evaluate_vis.py ../datasets/06_car/car.mpg bounding_boxes.txt ../datasets/06_car/gt.txt

---
Thanks
---
To Zdenek Kalal for realeasing his awesome algorithm

Reference
---
https://blog.csdn.net/taily_duan/article/details/52130135
http://www.elecfans.com/consume/483057.html 手势
https://blog.csdn.net/qq5132834/article/details/42682947?locationNum=2 轮廓识别
https://blog.csdn.net/wangshuai610/article/details/80040317 手势动作识别剪刀石头布
https://blog.csdn.net/yuan1125/article/details/62226382 肤色检测
https://blog.csdn.net/linqianbi/article/details/79155823 肤色检测
https://blog.csdn.net/logan_lin/article/details/79517571 手势检测及手掌质心的运动轨迹（opencv）

### Issue
1. 怎么找到具体的某个手指

2. 如何建立socket 链接
http://cache.baiducontent.com/c?m=9d78d513d9c04aaf4fece4690d61c067695784692bd6a0027fa38449e36e4c403771e3cc30236007c4c40c7000dc5e5d9af03470341767f7c5c7d20c9bf985295c953a713559914165964aeb9c077f9260c74de9d845b0fced73d5e98498c207138c085424d7e78b2c0512cd78f06232f4a6ee1253004e&p=8b2a975381934eae5e8185216153&newp=882a9443969801fc57efca64594792695c0fc13423938b5712d7984e8732002c0331a3fa7c7f4c0bd7c17e6104ae4d5ce0f13774330524bd91c9895e9bd6d37f6d877068265cdd01&user=baidu&fm=sc&query=192%2E168%2E8%2E1%3A2001&qid=ce5a8a9e001167eb&p1=5

局域网聊天 socket
https://www.cnblogs.com/diligenceday/p/6241021.html