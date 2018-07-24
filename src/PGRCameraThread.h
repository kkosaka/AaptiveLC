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
// glfw��pragma��
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glew��pragma��
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
		// �X���b�h�J�n
		thread = boost::thread(&PGRCamera::run, this);
		fps = new myUtl::FPSMajor(1);
	};

	~PGRCamera() {
		threadStopFlag = true;
		thread.join();

		this->release();
		this->stop();
	};

	// �������݂�on/off��ύX����֐�
	void setEnabled(bool enabled){
		ready = enabled;
	}
	// ���݂̏������݂�on/off���擾����֐�
	bool getReady(){ return ready; };
	bool criticalSection(bool write, cv::Mat &image, unsigned int num);

	// �摜���擾����֐�
	bool getImage(cv::Mat &dst, unsigned int num);
	bool getLatencyImage(cv::Mat &dst, unsigned int num);

	// �o�b�t�@���̂��ׂẲ摜���擾����֐�
	bool getAllImages(std::vector<cv::Mat> &dst);

	// �o�b�t�@���̂��ׂĂ̎B�e���Ԃ��擾����֐�
	bool getAllTime(std::vector<double> &dst);

	// �o�b�t�@�̒��̂��ׂẲ摜���폜����֐�
	bool clear();

	bool getProCamLate(GLFWwindow *window);

	// �o�b�t�@�̕ێ�����ύX����
	bool resize(const unsigned int size);

	unsigned int getCamWidth(){ return getWidth(); };
	unsigned int getCamHeight(){ return getHeight(); };

	// �V���b�^�[�X�s�[�h��ݒ肷��
	bool setShutterSpeed(float speed){
		TPGROpenCV::setShutterSpeed(speed);
		return true;
	}
	// ���݂̃V���b�^�[�X�s�[�h���擾����
	float getShutterSpeed() {
		return TPGROpenCV::getShutterSpeed();
	}
	// �␳���̃V���b�^�[�X�s�[�h���擾����
	float getShutterSpeed_LC() {
		return TPGROpenCV::getShutter_LC();
	}
	// �v�����̃V���b�^�[�X�s�[�h���擾����
	float getShutterSpeed_Measure() {
		return TPGROpenCV::getShutter_measure();
	}
protected:

	// �o�b�t�@��ێ�����
	std::deque<cv::Mat> imageBuffers;
	// �B�e���Ԃ�ێ�����
	std::deque<double> timeBuffers;

	// deque�̃o�b�t�@�T�C�Y
	boost::atomic<unsigned int> bufferSize;

	// �X���b�h
	boost::thread thread;

	// �~���[�e�b�N�X
	boost::shared_mutex mutex;

	// �X���b�h��~�t���O
	boost::atomic<bool> threadStopFlag;

	// 
	boost::atomic<bool> ready;

	boost::atomic<int> capCount;

	void run();

	myUtl::FPSMajor *fps;

	// �摜��deque�ɕێ�����֐�
	bool setImage(cv::Mat &cap);
	// �B�e���Ԃ�deque�ɕێ�����֐�
	bool setCaptureTime(double time, bool isCap);

};

#endif