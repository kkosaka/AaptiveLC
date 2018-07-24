#ifndef GRAYCODE_H
#define GRAYCODE_H
#pragma warning(disable : 4819)

#include <iostream>
#include <direct.h>	// �f�B���N�g���쐬�p
#include <iomanip>  // ������X�g���[��

#include <gl/glew.h>
#include <GLFW/glfw3.h>

// glfw��pragma��
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glew��pragma��
#pragma comment(lib,"glew32.lib")

#include <opencv2/opencv.hpp>

#include "texture.h"
#include "vertex.h"
#include "PGRCameraThread.h"
//#include "PGROpenCV.h"
#include "glUtl.h"
#include "square.h"
#include "FPSMajor.h"

class GrayCode
{
public:
	// �J�����X���b�h�̃|�C���^�Cnega_posi臒l�C�f�B���N�g���p�X�Cprojw, projh, camW, camH, waveLnegthx, waveLnegthY
	GrayCode(PGRCamera *pgrcam, const int _threshold = 40, const char *dirctorypass = ".", const unsigned int projw = 1280, const unsigned int projh = 800
		, const unsigned int camw = 1920, const unsigned int camh = 1200, int _waveLength_x = 1, int _waveLength_y = 1)
		:path(dirctorypass)
		, projWidth(projw), projHeight(projh), camWidth(camw), camHeight(camh)
		, waveNumX(projw / _waveLength_x), waveNumY(projh / _waveLength_y)
	{
		// �ϐ��̏�����
		waveLength_x = _waveLength_x;
		waveLength_y = _waveLength_y;
		w_bit = (((int)ceil(log(waveNumX + 1) / log(2))));
		h_bit = ((int)ceil(log(waveNumY + 1) / log(2)));
		all_bit = (w_bit + h_bit);
		graycode = cv::Mat(waveNumY, waveNumX, CV_32S, cv::Scalar::all(0));
		graycode_mat = cv::Mat(projh, projw, CV_32S, cv::Scalar::all(0));
		decode = cv::Mat(camh, camw, CV_32S, cv::Scalar::all(0));
		coordmap = cv::Mat(waveNumY, waveNumX, CV_32SC2, cv::Scalar::all(0));
		threshold = _threshold;
		g_code_map = new std::map<int, cv::Point>();

		// �f�B���N�g���p�X�̐ݒ�
		gc_base_dir += std::string(path) + "/GrayCodeImages";
		gc_mask_address = gc_base_dir + "/mask.bmp";

		// �􉽕ϊ��p���b�V���̐���
		coord_trans_mesh.resize(2);
		coord_trans_mesh[0] = new Vertex(projWidth, projHeight);
		coord_trans_mesh[1] = new Vertex(camWidth, camHeight);

		glsl = new glUtl::glShader;
		square = new SQUARE;

		// �X���b�h�̎󂯓n��
		pgrCamera = pgrcam;

		ready = false;
	};
	~GrayCode(){

	};

	// �萔
	typedef enum gcflag{
		POSI = true,		// �|�W
		NEGA = false,		// �l�K
		VERTICAL = true,	// �c��
		HORIZONTAL = false,	// ����
	} gcflag;

	// �t�@�C���p�X
	const char *path;
	std::string gc_base_dir;
	std::string gc_mask_address;

	// ���e�T�C�Y
	const unsigned int projWidth;
	const unsigned int projHeight;
	// �J�����𑜓x
	const unsigned int camWidth;
	const unsigned int camHeight;

	// �􉽕ϊ��p�̒��_�z��([0] : camera��projector, [1] : projector �� camera)
	//Vertex **coord_trans_mesh[2];
	//Vertex coord_trans_mesh_proj2cam;

	// ���e���Ɏg�p����V�F�[�_
	glUtl::glShader *glsl;

	SQUARE *square;

	// �J�����X���b�h
	PGRCamera *pgrCamera;

	// �f�B���N�g���̍쐬
	void gcCreateDirs();

	// �O���C�R�[�h�摜�쐬
	void makeGraycodeImage();

	// 2�l������
	void make_thresh();

	// �������֐�
	virtual void init();

	// opencv�𗘗p���ĉf�����e
	// �Ăяo���O�ɁAPGR�J�������X�g�b�v���邱��
	//(�f����1pixel���Ɖ��ɂ����̂ŁC�P�x�␳�̍ۂ�GL���g������)
	void code_projection_opencv();

	// openGL�𗘗p���ĉf�����e
	// �Ăяo���O�ɁAPGR�J�������X�g�b�v���邱��
	bool code_projection_opengl(GLFWwindow *window);

	// 2�l���R�[�h����
	virtual void getCorrespondence(bool isCodeProjection);

	// �O���C�R�[�h���f�R�[�h����
	void decodeGrayCode();

	//// �J�����B�e�̈悩��v���W�F�N�^���e�̈��؂�o��
	virtual void reshapeCam2Proj(cv::Mat &src, cv::Mat &dst);

	// �c�����̃r�b�g���̎擾
	unsigned int gcGetBitNum_h() { return h_bit; };
	// �������̃r�b�g���̎擾
	unsigned int gcGetBitNum_w() { return w_bit; };
	// �c�������̃r�b�g�����̎擾
	unsigned int gcGetBitNum_all() { return all_bit; };
	// ���e�摜�̃p�X���擾
	std::stringstream gcGetProjctionFilePath(unsigned int num, bool pos);
	// �B�e�摜�̃p�X���擾
	std::stringstream gcGetCaptureFilePath(unsigned int num, bool horizontal, bool posi);
	// ��l���摜�̃p�X���擾
	std::stringstream gcGetThresholdFilePath(unsigned int num);
	// �Ή��}�b�v�̎擾
	cv::Mat gcGetGrayCode(){ return graycode; };
	// �Ή��}�b�v�̎擾
	cv::Mat gcGetDecodeMap(){ return decode; };
	// �Ή��}�b�v�̎擾
	cv::Mat gcGetCoordMap(){ return coordmap; };
	// �􉽕␳�p�̃}�X�N�摜���쐬
	void getMaskForGeometricCorrection();

	bool getStatus(){ return ready; };

private:

	// �l�K�|�W������臒l
	int threshold;

	cv::Mat graycode;	//<! �g���ɍ��킹���O���C�R�[�h�i�g�̐��̉𑜓x[����][��]�j
	cv::Mat graycode_mat;	//<! �O���C�R�[�h�i�v���W�F�N�^�𑜓x[����][��]�j
	cv::Mat coordmap;	// �Ή��_��ێ�

	//<! �����C���̕K�v�r�b�g��
	unsigned int h_bit, w_bit;
	//<! ���v�r�b�g���ih_bit + w_bit
	unsigned int all_bit;
	// �ʑ��V�t�g�@�̃T�C���摜�̈�����̒���
	unsigned int waveLength_x, waveLength_y;
	// �ʑ��V�t�g�@�̃T�C���摜�̎�����
	unsigned int waveNumX, waveNumY;

	// �������ꂽ�O���C�R�[�h�𐮗����邽�߂̘A�z�z��
	std::map<int, cv::Point> *g_code_map;

	// �e�N�X�`����ێ�����ϐ�
	std::vector<Texture> code_texture;

	std::vector<Vertex*> coord_trans_mesh;

	// �e�N�X�`����ǂݍ��ފ֐�
	bool readTexture();

	//�f�R�[�h����ێ����Ă���Mat�i�J�����𑜓x�j
	cv::Mat decode;

	// �Ή��擾���ł��Ă��邩����
	bool ready;

	// �O���C�R�[�h�쐬
	void initGraycode();

	// �O���C�R�[�h�̉摜�𗘗p���ă}�X�N�𐶐�����֐�()
	void makeMask(cv::Mat &mask);

	// �|�W�ƃl�K�̍����������MASK_THRESH�ȏ�̋P�x�̃s�N�Z���𔒂ɂ���
	void makeMaskFromCam(cv::Mat &posi, cv::Mat &nega, cv::Mat &result, int thresholdValue);

	// 2�l�������֐� 
	void thresh(cv::Mat &posi, cv::Mat &nega, cv::Mat &thresh_img, int thresh_value);

	// gl�̍��W�n�̌v�Z������
	void getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture);

	// �􉽕ϊ��p�̒��_�z����擾����
	void getBufferArrays();
};

#endif