#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(void)
{
    //for initializing
    bool skipflag = 1;

    //opening and closing
    int closingCount = 10;
    int openingCount = 10;

    //video to save
    VideoWriter video; // videoはビデオライタのオブジェクト

    //video recording flag and counter
    bool flag = 0;
    int counter = 0;

    //監視カメラ
    //フレーム間差分を検出し、ある一定以上の閾値になったら
    //レコーディング処理に入る。
    VideoCapture camera;
    camera.open(0);

    Mat rgbimage, grayscaleimage, bgimage;
    Mat diffimage, maskimage, resultimage;
    Mat maskbefore, maskxor;
    while (1)
    {
        camera >> rgbimage;
        cvtColor(rgbimage, grayscaleimage, CV_BGR2GRAY);

        //2週目から実行される
        if (!bgimage.empty())
        {
            absdiff(grayscaleimage, bgimage, diffimage);
            threshold(diffimage, maskimage, 10, 255, CV_THRESH_BINARY);
            //Closing
            dilate(maskimage, maskimage, Mat(), Point(-1, -1), closingCount);
            erode(maskimage, maskimage, Mat(), Point(-1, -1), closingCount);

            //Opening
            erode(maskimage, maskimage, Mat(), Point(-1, -1), openingCount);
            dilate(maskimage, maskimage, Mat(), Point(-1, -1), openingCount);

            //findContours Area

            //３週目から実行される
            if (!maskbefore.empty())
            {
                bitwise_xor(maskimage, maskbefore, maskxor);

                vector<vector<Point>> contours;
                double area = 0;
                findContours(maskxor, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
                for (int i = 0; i < contours.size(); i++)
                {
                    area += contourArea(Mat(contours[i]));
                }
                cout << area << endl;
                drawContours(rgbimage, contours, -1, Scalar(0, 0, 255), 1, 8);

                //skipflag is for initializing
                //4週目から実行される
                if (!skipflag)
                {
                    //VIDEO RECORDING HERE
                    if (area > 100000)
                    {
                        if (!flag)
                        {
                            flag = 1;
                            video.open("movie.avi", CV_FOURCC_MACRO('M', 'J', 'P', 'G'), 30, rgbimage.size(), true);
                        }
                        counter = 0;
                    }

                    if (counter == 60)
                    {
                        video.release();
                        flag = 0;
                        break;
                    }

                    if (flag)
                    {
                        video << rgbimage;
                        counter++;
                    }
                }
                else
                {
                    skipflag = 0;
                }
            }
        }

        //表示(debug)

        //imshow("Color Image", rgbimage);
        imshow("Grayscale Image", grayscaleimage);

        if (!bgimage.empty())
        {
            //imshow("Diff Image", diffimage);
            imshow("Mask Image", maskimage);
        }

        if (!maskbefore.empty())
        {
            imshow("Mask XOR", maskbefore);
        }
        if (!maskxor.empty())
        {
            imshow("Mask XOR", maskxor);
        }

        //フレーム間差分
        grayscaleimage.copyTo(bgimage);

        if (!maskimage.empty())
        {
            maskimage.copyTo(maskbefore);
        }

        int key;
        key = waitKey(1);
        if (key == 's')
            break;
    }
}

