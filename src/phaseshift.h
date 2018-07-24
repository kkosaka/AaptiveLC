#ifndef PHASESHIFT_H
#define PHASESHIFT_H
#pragma warning(disable : 4819)

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>  // ������X�g���[��
#include <opencv2/opencv.hpp>

#include <direct.h>	// �f�B���N�g���쐬�p

#include "graycode.h"
#include "PS_Vertex.h"

#define PI 3.14159265358979323846264338327950288
#define Amp	90.0		// �����g�摜�̐U��
#define Bias 160.0		//�����g�摜�̃o�C�A�X(��グ)
#define Proj_num 8				// ���e����
#define WaveLength_X 10	// �����g�摜(X����)�̔g��[pixel] �ׂ����قǐF�̌��z�������܂�邽�߂悢
#define WaveLength_Y 20	// �����g�摜(Y����)�̔g��[pixel] 

class PhaseShift : public GrayCode
{
public:

	PhaseShift(PGRCamera *pgrcam, int graycode_threshold = 20, const char *dirctorypass = ".", const unsigned int projw = 1280
		, const unsigned int projh = 800, const unsigned int camw = 1920, const unsigned int camh = 1200)
		:
		ps_path(dirctorypass),
		GrayCode(pgrcam, graycode_threshold, (std::string(dirctorypass) + "/PhaseShiftImages").c_str(), projw, projh, camw, camh, WaveLength_X, WaveLength_Y),
		xplus((int)(WaveLength_X / 2) + 1), yplus((int)(WaveLength_Y / 2) + 1),
		waveNumX(projw / WaveLength_X), waveNumY(projh / WaveLength_Y)
	{
		// �ʑ��V�t�g�p�f�B���N�g���p�X�̐ݒ�
		ps_base_dir = std::string(ps_path) + "/PhaseShiftImages";
		ps_image_dir = ps_base_dir + "/SineImages";
		// �}�X�N�摜�p臒l�̐ݒ�
		ps_thresh = 40;
		// �L�����u���[�V�����ɕK�v�Ȕz��̏�����
		phaseValue = cv::Mat(camh, camw, CV_64FC2, cv::Scalar::all(0));
		coord_map_int = cv::Mat(camh, camw, CV_32SC2, cv::Scalar::all(-1));
		coord_map = cv::Mat(camh, camw, CV_32FC2, cv::Scalar::all(-1));
		coord_map_proj = cv::Mat(projh, projw, CV_32SC2, cv::Scalar::all(-1));
		init_flag = false;
		ready = false;
		// ������
		this->init();

		// �􉽕ϊ��p���b�V���̐���
		coord_trans = new PS_Vertex(projWidth, projHeight);

	};

	~PhaseShift()
	{
	};

	// �������֐�
	void init();

	// opencv�𗘗p���ĉf�����e
	// �Ăяo���O�ɁAPGR�J�������X�g�b�v���邱��
	//(�f����1pixel���Ɖ��ɂ����̂ŁC�P�x�␳�̍ۂ�GL���g������)
	void code_projection_opencv();

	// openGL�𗘗p���ĉf�����e
	bool code_projection_opengl(GLFWwindow *window);

	// �I�[�o�[���C�h
	void getCorrespondence(bool isCodeProjection);

	bool getStatus(){ return this->ready; };

	unsigned int psGetProjectionNum() { return Proj_num; };
	std::stringstream psGetProjctionFilePath(unsigned int num, bool vertical);
	std::stringstream psGetCaptureFilePath(unsigned int num, bool vertical);

	// �Ή��}�b�v���擾����֐�
	cv::Mat getCoordMap(){ return coord_map.clone(); };

	// �I�[�o�[���C�h
	//// �J�����B�e�̈悩��v���W�F�N�^���e�̈��؂�o��
	void reshapeCam2Proj(cv::Mat &src, cv::Mat &dst);

	// �􉽕ϊ��p
	PS_Vertex *coord_trans;

	// �t�@�C�����w�肵�āC���W�f�[�^��ǂݍ���
	void readPixCorrespData(const std::string &filename, cv::Mat &dst_map);
private:
	const char *ps_path;
	// ���������f�[�^�̕ۑ��ꏊ
	std::string ps_base_dir;
	std::string ps_image_dir;

	// �ʑ��f�[�^�̎���0�ɍ��킹�邽�߂ɕK�v�ȕϐ�
	const int	xplus;
	const int	yplus;

	// �ʑ���񂩂�}�X�N�摜���쐬���邽�߂�臒l
	unsigned int ps_thresh;

	// �ʑ��V�t�g�@�̃T�C���摜�̎�����
	unsigned int waveNumX, waveNumY;

	bool init_flag;

	// �ʑ��l��ێ�
	cv::Mat phaseValue;
	// �J�������W�΂���v���W�F�N�^���W�l��ێ������}�b�v(�����l)
	cv::Mat coord_map_int;
	// �J�������W�΂���v���W�F�N�^���W�l��ێ������}�b�v(�T�u�s�N�Z��)
	cv::Mat coord_map;
	// �v���W�F�N�^���W�ɑ΂���J�������W�l��ێ������}�b�v(�����l)
	cv::Mat coord_map_proj;

	// ���e�̈�̃}�X�N�摜
	cv::Mat mask;

	bool ready;

	// �}�X�N�摜�����������邽�߂̊֐�
	void smallMaskRange(cv::Mat &src, cv::Mat &dst);

	// �f�B���N�g���̍쐬
	void psCreateDirs();
	
	// �e�N�X�`����ǂݍ��ފ֐�
	bool readTexture();

	// �����g�摜���쐬����֐�
	void makeSineWaveImage();

	void code_restore();
	void restore_phase_value();
	void phaseConnection();
	void errorCheck();

	// �Ή���ۑ�����
	void writePixCorrespData();

	// �Ή���ǂݍ���
	void readPixCorrespData();

	// �����l�Ɋۂ߂����W�}�b�v
	void calcIntCoordMap();

	// debug
	void calcNearestNeighbor();

	// gl�̍��W�n�̌v�Z������
	void getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture, bool cam2proj);

	// �􉽕ϊ��p�̒��_�z����擾����
	void getBufferArrays(cv::Mat &in_vertex, cv::Mat &in_texture, PS_Vertex *mesh);

};

#endif