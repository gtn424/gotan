// どういう処理をするコードなのかを必ずコメントアウトする。
// 無人レジ本屋の実装を行う
// 本領域の抽出とフレーム間差分(人が本を取ったか取ってないか)を利用する。

#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main(void) {
	namedWindow("slider");
	resizeWindow("slider", 640, 480);
	int huemax = 181, huemin = 0, satmax = 255, satmin = 0, valmax = 255, valmin = 0;
	int opening = 0, closing = 0, border = 0, framecount = 0, framecount2 = 0,framecount3=0,movecount=0,nomove=0,bluecount=0,moveframe=0;
	double area = 0, threshold_area = 0, result_area = 0,result_area2=0,resultmove=0,blue_area=0,blue_area2=0,blue_move=0;
	bool move=false;
	vector<int> linear_array;
	vector<vector<Point>>contours;
	Mat gray_image,hsv_image, original_image, mask_image, dilate, erode, opened_image, closed_image, abs_image, background_image, threshold_image,blue_mask_image;

	/*----スライダーバー設置-----*/
	createTrackbar("Hue Max", "slider", &huemax, 181);
	createTrackbar("Hue Min", "slider", &huemin, 181);
	createTrackbar("Sat Max", "slider", &satmax, 255);
	createTrackbar("Sat Min", "slider", &satmin, 255);
	createTrackbar("Val Max", "slider", &valmax, 255);
	createTrackbar("Val Min", "slider", &valmin, 255);
	createTrackbar("Opening", "slider", &opening, 50);
	createTrackbar("Colosing", "slider", &closing, 50);
	createTrackbar("threshold", "slider", &border, 256);

	VideoCapture camera; // cameraは映像ソースを表現するためのオブジェクト
	camera.open(1);      // 映像ソースとして0番目のカメラを指定


	while (1) {
		
		camera >> original_image;

		cvtColor(original_image, hsv_image, CV_BGR2HSV); //(2)
		cvtColor(original_image, gray_image, CV_BGR2GRAY); //(2)s

		inRange(hsv_image, Scalar(huemin, satmin, valmin), Scalar(huemax, satmax, valmax), mask_image); //(3)

		//Opening処理
		cv::dilate(mask_image, dilate, Mat(), Point(-1, -1), opening);
		cv::erode(dilate, opened_image, Mat(), Point(-1, -1), opening);

		//Closing処理
		/*cv::erode(mask_image, erode, Mat(), Point(-1, -1), closing);
		cv::dilate(erode, closed_image, Mat(), Point(-1, -1), closing);*/

		//Opening処理 ➡ Closing処理
		cv::erode(opened_image, erode, Mat(), Point(-1, -1), closing);
		cv::dilate(erode, closed_image, Mat(), Point(-1, -1), closing);

		//contoursArea：輪郭線を取得
		for (int i = 0; i < contours.size(); i++) {
			area = contourArea(Mat(contours[i]));
			//cout << "contours : " << contours[i] << endl;
		}

		//Contours 輪郭線を絵画
		for (int i = 0; i < linear_array.size(); i++) {
			linear_array[i] = 10;
		}
		findContours(mask_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		drawContours(original_image, contours, -1, Scalar(0, 0, 255), 1, 8);

		Mat image(480, 640, CV_8SC3); //(4)
		image.setTo(Scalar(0, 0, 0)); //(5)
		
		original_image.copyTo(image, mask_image); //(6)

		//threshold：フレーム間差分を行う(人の手と本が増えた、減ったを認識する。)
		if (background_image.empty() == false) {
			//std::cout << "Background" << std::endl;
			absdiff(gray_image, background_image, abs_image);
			threshold(abs_image, threshold_image, (double)border, 255, CV_THRESH_BINARY);
			imshow("abs", abs_image);
			imshow("threshold", threshold_image);

			Mat image2(480, 640, CV_8SC3); //(4)
			image2.setTo(Scalar(0, 0, 0)); //(5)
			original_image.copyTo(image2, threshold_image); //(6)
			imshow("Threshold_Result image", image2);
		}

		//threshold,resultの二値化面積を取得
		if (framecount == 5) {
			threshold_area = countNonZero(threshold_image);
			if (threshold_area > 1000) {
				movecount++;
				if (movecount > 5) {
					std::cout << "Move" << std::endl;
					move = true;
					nomove = 0;
				}
			}
			else {
				nomove++;
				if (nomove > 10) {
					move = false;
				}
			}
			//std::cout << "Threshold_area: " << threshold_area << std::endl;
			framecount = 0;
		}
		/*if (framecount2 == 5) {
			result_area = countNonZero(closed_image);
			std::cout << "Result_area: " << result_area << std::endl;
			framecount2 = 0;
		}*/

		//(実験) 青色領域を抽出
		/*-----------------------------------------------*/
		inRange(hsv_image,Scalar(90, 50, 70),Scalar(128, 255, 255),blue_mask_image);
		imshow("Blue", blue_mask_image);
		if (framecount3 == 5) {
			blue_area = countNonZero(blue_mask_image);
			//std::cout << "Blue area : " << blue_area << std::endl;

			result_area = countNonZero(closed_image);
			//std::cout << "Result_area: " << result_area << std::endl;

			if (moveframe==0 && move == false) {
				result_area2 = result_area;
				blue_area2 = blue_area;
			}

			if (move) {
				moveframe=1;
			}

			if (moveframe > 0 && move==false) {
				resultmove = result_area2 - result_area;
				blue_move = blue_area2 - blue_area;
				//std::cout << "a" << std::endl;
				if (blue_move > 3000) {
					std::cout << "Blue book got" << std::endl;
				}
				if (blue_move < -3000) {
					std::cout << "Blue book entry" << std::endl;
				}
				else{
					if (resultmove > 3000) {
						std::cout << "Other color book got" << std::endl;
					}
					if (resultmove < -3000) {
						std::cout << "Other color book entry" << std::endl;
					}
				}
				moveframe = 0;
			}
			framecount3 = 0;
		}
		/*-----------------------------------------------*/


		// 映像出力
		imshow("Camera Image", original_image); //(7)
		imshow("Mask Image", mask_image);
		imshow("Result Image", image);
		imshow("Opening Image", opened_image);
		imshow("Closing Image", closed_image);

		//プログラム停止
		if (waitKey(1) == 'q') {
			break;
		}

		//フレーム間差分を行うためにbackgroundに画像をコピー
		gray_image.copyTo(background_image);
		imshow("background", background_image);
		framecount++;
		framecount2++;
		framecount3++;
	}
	return 0;
}


