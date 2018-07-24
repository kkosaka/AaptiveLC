#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <iostream>
#include <memory>

#include <deque>

#include "PGROpenCV.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include "FPSMajor.h"

#include <gl/glew.h>
#include <GLFW/glfw3.h>
// glfwのpragma文
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glewのpragma文
#pragma comment(lib,"glew32.lib")

class PGRCamera : private TPGROpenCV{
public:
	PGRCamera(FlyCapture2::PixelFormat _format = FlyCapture2::PIXEL_FORMAT_BGR, int ColorProcessingAlgorithm = FlyCapture2::ColorProcessingAlgorithm::HQ_LINEAR, int buffer_size = 30)
		: TPGROpenCV()
	{
		bufferSize = buffer_size;
		if (this->init(_format, ColorProcessingAlgorithm) == -1){
			exit(0);
		}
		ready = true;
		this->start();
		threadStopFlag = false;
		// スレッド開始
		thread = boost::thread(&PGRCamera::run, this);
		fps = new myUtl::FPSMajor(1);
	};

	~PGRCamera() {
		threadStopFlag = true;
		thread.join();

		this->release();
		this->stop();
	};

	// 書き込みのon/offを変更する関数
	void setEnabled(bool enabled){
		ready = enabled;
	}
	// 現在の書き込みのon/offを取得する関数
	bool getReady(){ return ready; };
	bool criticalSection(bool write, cv::Mat &image, unsigned int num);

	// 画像を取得する関数
	bool getImage(cv::Mat &dst, unsigned int num);
	bool getLatencyImage(cv::Mat &dst, unsigned int num);

	// バッファ中のすべての画像を取得する関数
	bool getAllImages(std::vector<cv::Mat> &dst);

	// バッファ中のすべての撮影時間を取得する関数
	bool getAllTime(std::vector<double> &dst);

	// バッファの中のすべての画像を削除する関数
	bool clear();

	bool getProCamLate(GLFWwindow *window);

	// バッファの保持数を変更する
	bool resize(const unsigned int size);

	unsigned int getCamWidth(){ return getWidth(); };
	unsigned int getCamHeight(){ return getHeight(); };

	// シャッタースピードを設定する
	bool setShutterSpeed(float speed){
		TPGROpenCV::setShutterSpeed(speed);
		return true;
	}
	// 現在のシャッタースピードを取得する
	float getShutterSpeed() {
		return TPGROpenCV::getShutterSpeed();
	}
	// 補正時のシャッタースピードを取得する
	float getShutterSpeed_LC() {
		return TPGROpenCV::getShutter_LC();
	}
	// 計測時のシャッタースピードを取得する
	float getShutterSpeed_Measure() {
		return TPGROpenCV::getShutter_measure();
	}
protected:

	// バッファを保持する
	std::deque<cv::Mat> imageBuffers;
	// 撮影時間を保持する
	std::deque<double> timeBuffers;

	// dequeのバッファサイズ
	boost::atomic<unsigned int> bufferSize;

	// スレッド
	boost::thread thread;

	// ミューテックス
	boost::shared_mutex mutex;

	// スレッド停止フラグ
	boost::atomic<bool> threadStopFlag;

	// 
	boost::atomic<bool> ready;

	boost::atomic<int> capCount;

	void run();

	myUtl::FPSMajor *fps;

	// 画像をdequeに保持する関数
	bool setImage(cv::Mat &cap);
	// 撮影時間をdequeに保持する関数
	bool setCaptureTime(double time, bool isCap);

};

#endif