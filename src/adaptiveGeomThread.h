#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "glUtl.h"

#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

#include <opencv2/opencv.hpp>
#include "vertex.h"
#include "Calibration.h"
#include "phaseshift.h"
#include "myParam.h"

#include "FPSMajor.h"


class adaptiveGeomCorresp
{
public:
	// �R���X�g���N�^
	adaptiveGeomCorresp(unsigned int proj_w = 1280, unsigned int proj_h = 800, unsigned int cam_w = 1920, unsigned int cam_h = 1200)
	{
		projWidth = proj_w;
		projHeight = proj_h;
		camWidth = cam_w;
		camHeight = cam_h;

		calib = new Calibration;
		// �L�����u���[�V�����f�[�^�̓ǂݍ���
		calib->loadCalibParam(PROCAM_CALIBRATION_RESULT_FILENAME);

		// ��b�s����L�����u���[�V�����f�[�^���狁�߂�
		FundametalMat = calib->getFundamentalMat(calib->cam_K, calib->proj_K, calib->R, calib->T);

		fps = new myUtl::FPSMajor(1);

	};
	~adaptiveGeomCorresp()
	{};


	// �������֐�
	bool init(const cv::Mat &coord_map);

	// �G�s�|�[���������ɑΉ��𕡐��p�^�[���Έڂ�����
	// @param1 : �Ή����ۑ������}�b�g���𕡐��ۑ����Ă���
	// @param2 : �G���[��f���}�X�L���O�����摜
	// @param3 : �v���W�F�N�^���W�ɑ΂���J�������W�̃G�s�|�[�������i�[�������
	void calcNewGeometricCorrespond(std::vector<cv::Mat> &_shiftVectorMats, const std::vector<cv::Point2f> &points, const std::vector<cv::Vec3f> &lines);

	// �e��f���̃G�s�|�[�����ւ̈ړ��ʂ��i�[����Mat��ێ�����z��
	std::vector<cv::Mat> shiftVectorMats;

private:

	// �v���W�F�N�^�̏c��
	unsigned int projWidth, projHeight;
	// �J�����̏c��
	unsigned int camWidth, camHeight;


	// ��b�s��
	cv::Mat FundametalMat;

	// ��f�Ή����ۑ����Ă���}�b�g
	cv::Mat coordMap;
	// vector�Q�Ɨp�̃}�b�g
	cv::Mat indexMap;

	// �G�s�|�[�������i�[
	std::vector<cv::Vec3f> epiLines;

	// �S��f���̃G�s��
	std::vector<cv::Point2f> projPoints;

	std::vector<cv::Point2f> camPoints;

	// �N���X�I�u�W�F�N�g
	//boost::shared_ptr<Calibration> calib;
	Calibration *calib;
	myUtl::FPSMajor *fps;

	// �����̕�������x���W����y���W���v�Z����
	inline double solveY(cv::Vec3f efficient, double x){
		return -(efficient[0] * x + efficient[2]) / efficient[1];
	};

	inline double solve(double n){
		double x;

	}

	// �����������ɒ�����`�悷��
	void drawEpilines(cv::Mat& image, std::vector<cv::Vec3f> lines, std::vector<cv::Point2f> drawPoints);
	
};

#endif
