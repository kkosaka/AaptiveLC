#ifndef MYGL_H
#define MYGL_H

#pragma once
// �W���w�b�_���C���N���[�h
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "window.h"
#include "offscreenRender.h"
#include "TextureObjectStocker.h"
#include "vertex.h"
#include "myParam.h"

//#include "glsl.h"
#include "glUtl.h"
#include "myUtl.h"

#include "phaseshift.h"
#include "square.h"
#include "luminance.h"
//#include "PGROpenCV.h"
#include "PGRCameraThread.h"
#include "FPSMajor.h"
#include "offscreenRender.h"
#include "adaptiveGeomThread.h"

// glfw��pragma��
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glew��pragma��
#pragma comment(lib,"glew32.lib")

//�ڕW�摜�̃e�N�X�`�����j�b�g
//(�����_�����O���ʂ�ڕW�摜�Ƃ���̂�0)
#define TEXTURE_UNIT_INPUTIMAGE			0			
#define TEXTURE_UNIT_INPUTIMAGE_ENUM	GL_TEXTURE0

//�����֐��v�Z�p�̃v���W�F�N�^���e�摜�̃e�N�X�`�����j�b�g
#define TEXTURE_UNIT_PRJIMAGE			1
#define TEXTURE_UNIT_PRJIMAGE_ENUM		GL_TEXTURE1

//�����֐��v�Z�p�̃J�����摜�̃e�N�X�`�����j�b�g
#define TEXTURE_UNIT_CAMIMAGE			2
#define TEXTURE_UNIT_CAMIMAGE_ENUM		GL_TEXTURE2

//�ŏ����p�����[�^�̕��q�A����̃e�N�X�`�����j�b�g
#define TEXTURE_UNIT_NUME				3
#define TEXTURE_UNIT_NUME_ENUM			GL_TEXTURE3
#define TEXTURE_UNIT_DENOMI				4
#define TEXTURE_UNIT_DENOMI_ENUM		GL_TEXTURE4

// �}�X�N�摜�̃e�N�X�`�����j�b�g
#define TEXTURE_UNIT_MASK				5
#define TEXTURE_UNIT_MASK_ENUM			GL_TEXTURE5

using namespace glUtl;

class myGL : public Window
{

public:
	// �R���X�g���N�^
	myGL(PGRCamera *pgrcam, int width = 640, int height = 480, const char *title = "Projection") : Window(width, height, title)
	{
		// �E�B���h�E�̐�������off
		glfwSwapInterval(0);
		//glfwSwapInterval(1);

		// �R�[���o�b�N
		glfwSetFramebufferSizeCallback(window, resize);
		glfwSetKeyCallback(window, keyboard);

		// �C���X�^���X��this�|�C���^���L�^
		glfwSetWindowUserPointer(window, this);

		loop_flag = false;
		geom_flag = true;
		ready = false;
		agc_calc_flag = false;
		stop_flag = false;
		lc_noRender_flag = false;

		// �X���b�h�̎󂯓n��
		pgrCamera = pgrcam;
		//pgrOpenCV = new TPGROpenCV(0);

		projWidth = getWidth();
		projHeight = getHeight();
		camWidth = pgrCamera->getCamWidth();
		camHeight = pgrCamera->getCamHeight();

		// �R���X�g���N�^�̏�����
		const char *calibDir = "./Calibration";
		ps = new PhaseShift(pgrcam, GrayCodeThreshodlValue, calibDir, projWidth, projHeight, camWidth, camHeight);
		lc = new LC(pgrcam, projWidth, projHeight, camWidth, camHeight);
		sq = new SQUARE(projWidth, projHeight, camWidth, camHeight);
		fps_console = new myUtl::FPSMajor(1);
		timeGetter = new myUtl::FPSMajor(1);

		agc = new adaptiveGeomCorresp;

		shader_geom = new glShader;
		shader_geomInv = new glShader;
		shader_lc = new glShader;
		shader_texture = new glShader;
		shader_mapUpdater = new glShader;
		shader_mapBlur = new glShader;
		shader_getShiftVec = new glShader;
		shader_getValiance = new glShader;
		shader_getMinVIndex = new glShader;
		shader_errorTest = new glShader;

		lumiTexs_RGBA32F = new TextureObjectStocker(projWidth, projHeight, GL_RGBA32F);
		lumiTexs_RGBA	 = new TextureObjectStocker(projWidth, projHeight, GL_RGBA);

		camTexs_RGBA = new TextureObjectStocker(camWidth, camHeight, GL_RGBA);
		camTexs_RGBA32F = new TextureObjectStocker(camWidth, camHeight, GL_RGBA32F);

		gt_offscreen_render = new offscreenRender;
		lc_offscreen_render = new offscreenRender;
		offscreen_render_proj = new offscreenRender;

		// �t���[���T�C�Y�ɍ����l�p�`���b�V��
		texture_vertex = new Vertex(projWidth, projHeight);

		// ���e�摜���i�[�����z��
		imageFileList = new myUtl::getImageFiles(IMAGE_DIRECTORY);
		// ���e�摜�̃C���f�b�N�X
		imageIndex = 0;

		// �␳�񐔂��J�E���g����ϐ�
		lc_counter = 0;

		// �e�p�����[�^�̏�����
		delay = DlayTime;
		brightness = TargetScale;
		fogetFactor = FogetFactor;
		diff_thresh = ErrorDitectThreshold;
		nonCorrection = false;
		write_flag = false;
		blurNum = true;
	};

	~myGL()
	{
		delete lumiTexs_RGBA32F;
		lumiTexs_RGBA32F = NULL;

		delete lumiTexs_RGBA;
		lumiTexs_RGBA = NULL;

		delete camTexs_RGBA;
		camTexs_RGBA = NULL;

		delete texture_vertex;
		texture_vertex = NULL;

		delete sq;
		sq = NULL;

		//delete lc;
		//lc = NULL;
	};

	// ��������
	bool init();

	// ���t���[���s������
	void display();

	void swapBuffers();

	// ���W�ϊ����s���֐�
	void coord_trans(GLuint cam_texobj, GLuint proj_texobj, cv::Mat &cameraImage = cv::Mat());
	void coord_trans_inv(GLuint proj_texobj, GLuint cam_texobj, GLuint mask);

	// �P�x�␳���s���֐�
	void lc_projection(float targetImage_ScaleAlpha, float targetImage_ScaleBeta, float ff, float diff_th, GLuint maskTexture, GLuint texture_k_after, GLuint  texture_k_diff);

	// �􉽑Ή��t�����s���֐�
	void getCoordinateCorrespondance(bool isProjection);

	// �J���[�L�����u���[�V�������s���֐�
	void getColorCalibration(bool isProjection);

	// �摜�̓ǂݍ���

	// �R�[���o�b�N����
	static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void resize(GLFWwindow *const window, int width, int height);

	// �L�[����p�֐�
	void keyFunc(int key);

	// �R���\�[����ʂɕ\��������e
	void printInfo();

	void getProCamDelay();

protected:
	unsigned int projWidth, projHeight;	// �v���W�F�N�^�c��
	unsigned int camWidth, camHeight;	// �J�����c��
	int delay;	//�@�x����

	// �J�����X���b�h
	PGRCamera *pgrCamera;

	// �N���X�I�u�W�F�N�g�̐���
	PhaseShift *ps;
	Vertex *texture_vertex;
	SQUARE *sq;
	//TPGROpenCV *pgrOpenCV;
	LC *lc;
	TextureObjectStocker *lumiTexs_RGBA32F;	// �P�x�␳�Ɏg�p����e�N�X�`��(RGB32F)���Ǘ�����ϐ�
	TextureObjectStocker *lumiTexs_RGBA;	// �P�x�␳�Ɏg�p����e�N�X�`��(RGB)���Ǘ�����ϐ�
	TextureObjectStocker *camTexs_RGBA;		// �e�N�X�`��(RGB)���Ǘ�����ϐ�
	TextureObjectStocker *camTexs_RGBA32F;		// �e�N�X�`��(RGB)���Ǘ�����ϐ�

	adaptiveGeomCorresp *agc;
	
	// ���e�摜��ǂݍ��񂾔z��
	myUtl::getImageFiles *imageFileList;

	// �R���\�[����ʂɕ\������FPS�J�E���^�[
	myUtl::FPSMajor *fps_console;
	// ���Ԍv���p�̃N���X
	myUtl::FPSMajor *timeGetter;

	// frame buffer object

	offscreenRender *gt_offscreen_render;
	offscreenRender *lc_offscreen_render;
	offscreenRender *offscreen_render_proj;

	// glsl
	glShader *shader_geom;
	glShader *shader_geomInv;
	glShader *shader_lc;
	glShader *shader_texture;
	glShader *shader_mapUpdater;
	glShader *shader_mapBlur;
	glShader *shader_getShiftVec;
	glShader *shader_getValiance;
	glShader *shader_getMinVIndex;
	glShader *shader_errorTest;

	// �V�F�[�_�p�����[�^�̃��P�[�V����
	GLint shl_shiftPixel;
	GLuint shl_corrent_index;

	GLint shl_inputImageTexture;
	GLint shl_inputImage_ScalingAlpha;
	GLint shl_inputImage_ScalingBeta;
	GLint shl_prjImageTexture;
	GLint shl_camImageTexture;
	GLint shl_gamma;
	GLint shl_b_pixel;
	GLint shl_w_pixel;
	GLint shl_color_mixing_matrix;
	GLint shl_color_mixing_matrix_inv;
	GLint shl_numerator_before;
	GLint shl_denominator_before;
	GLint shl_k_before;
	GLint shl_forgetFactor;
	GLint shl_diffThreshold;
	GLint shl_nonCorection;
	GLint shl_mask;
	GLint shl_getShift_shiftIndexTex;
	GLint shl_getShift_correntIndex;
	GLint shl_minV_correntIndex;
	GLint shl_getV_correntIndex;
	GLint shl_mapUpdater_statusTex;
	GLint shl_mapUpdater_resultTex;
	GLint shl_mapUpdater_maskTex;
	GLint shl_mapBlur_offsetTex;
	GLint shl_mapBlur_maskTex;
	GLint shl_mapBlur_blurNum;

	//-----�e�p�����[�^�̃e�N�X�`���I�u�W�F�N�g-----
	GLuint txo_camImageTexture;				//�􉽕ϊ��ς݂̃J�����B�e�摜 (RGBA)

	GLuint txo_inputImageTexture;			//�I���W�i���摜 (RGBA)
	GLuint txo_targetImageTexture;			//�ڕW�摜 (RGBA)
	GLuint txo_projImageTexture;			//���e�摜
	GLuint txo_transCamImageTexture;		//�􉽕ϊ��ς݂̃J�����B�e�摜 (RGBA)
	GLuint txo_maskTexture;					//�}�X�N�摜
	GLuint txo_agc_maskTexture;				//�}�X�N�摜

	GLuint txo_projImage_after;				//�X�V���ꂽ�␳�摜(RGBA)
	GLuint txo_numerator_before;			//�O��̕��q���� (RGBA32F)
	GLuint txo_denominator_before;			//�O��̕��ꐬ�� (RGBA32F)
	GLuint txo_numerator_after;				//�X�V���ꂽ���q���� (RGBA32F)
	GLuint txo_denominator_after;			//�X�V���ꂽ���ꐬ�� (RGBA32F)
	GLuint txo_k_after;						//�X�V���ꂽ���˗�K (RGBA32F)�A�f�o�b�O�p
	std::vector<GLuint> txo_k;				//�X�V���ꂽ���˗�K (RGBA32F)�A�f�o�b�O�p
	std::vector<GLuint> txo_minV;				//�X�V���ꂽ���˗�K (RGBA32F)�A�f�o�b�O�p
	GLuint txo_diff;

	// Vertex Array Object
	GLuint vao;		
	// Transform Feedback Object
	GLuint feedback;	

	/* Buffer Objects */
	glBuffer<cv::Vec4f> *PBO_projSize;			// �ėp�I�Ɏg��PBO(�v���W�F�N�^�𑜓x)
	glBuffer<cv::Vec4f> *VBO_valiance;			// ���U�l
	glBuffer<cv::Vec4f> *VBO_minValiance;		// �ŏ����U�l
	glBuffer<cv::Vec4f> *VBO_minValiance_Copy;	// �ŏ����U�l�̈ꎞ�I�ȃR�s�[�p
	glBuffer<cv::Vec4f> *VBO_shiftIndex;		// �ŏ����U�l�̃C���f�b�N�X��ێ�
	glBuffer<cv::Vec4f> *VBO_shiftIndex_Copy;	// �ŏ����U�l�̃C���f�b�N�X�̈ꎞ�I�ȃR�s�[�p
	glBuffer<cv::Vec4f> *VBO_errorPixels;		// �G���[��f���o���ʂ�ێ�

	glTexBuffObj<cv::Vec4f> *TBO_projSize;		// �ėp�I�Ɏg��PBO(�v���W�F�N�^�𑜓x)
	glTexBuffObj<cv::Vec2f> *TBO_offsetCopy;	// �I�t�Z�b�g�l�̈ꎞ�I�ȃR�s�[�p
	glTexBuffObj<GLfloat> *TBO_vboStatus;		// ��ԊǗ��p��TBO
	glTexBuffObj<GLfloat> *TBO_vboMask;			// �����␳�p�}�X�N�Ɏg��VBO

	// �]�������p�ϐ�
	glBuffer<cv::Vec4b> *PBO_CAMERAIMAGES;	// �J�����摜��VBO�փR�s�[���邽�߂�PBO
	glBuffer<cv::Vec4b> *VBO_CAMERAIMAGES[1000];	// �J�����摜��ۑ����Ă����ϐ�(1000 frame ��)

	// �e�N�X�`����ۑ�����֐�
	void debug_textures();
	cv::Mat saveTexture(GLuint texId, const string &filename, int w, int h, bool isFlip = 1, bool isWrite = 0, bool isSave = 1);
	cv::Mat &textureToMat(GLuint texture, int w, int h);

	// ���͉摜���쐬����֐�
	void getInputTexture(cv::Mat &inputImage, GLuint &textureObj, cv::Mat &camGeomImage = cv::Mat());

	// RLS�p�����[�^������������֐�
	void resetRLSParameter();

	// �o�b�t�@�𐶐�����֐�
	void getGLBuffers();

	// ���˗����v�Z����֐�
	void getProjectionSurfaceReflectance(float ff, GLuint maskTexture, GLuint texture_k_after);

	bool writeToExelFile(cv::Mat &src, const std::string &str, cv::Point point = cv::Point(0,0), int range = 0);
	bool readWriteExelFile(const std::vector<std::string> &fileLists, std::vector<cv::Point> points, const std::string &dstStr);

	// �摜�p
	int imageIndex;
	cv::Mat srcImage;
	cv::Mat capImage;
	cv::Mat capImage_before;
	cv::Mat whiteImage;
	cv::Mat agc_maskImage;
	cv::Mat cam_mask;

	float brightness;	// �ڕW�摜�̖��邳����
	float fogetFactor;	// �Y�p�W���̒���
	unsigned int diff_thresh;	// ������臒l

	bool ready;

	// �P�x�␳�p�t���O
	bool loop_flag;
	// �􉽕␳�p�t���O
	bool geom_flag;
	
	int lc_flag;

		// �P�x�␳on/off
	boost::atomic<bool> nonCorrection;

	unsigned int lc_counter;

	GLuint txo_first_k;

	// �􉽑Ή����C������֐�
	void adaptiveGeom();

	bool agc_calc_flag;
	bool lc_noRender_flag;
	bool stop_flag;
	bool write_flag;	// debug : csv�t�@�C���ւ̏������ݗp
	int systemFlag;
	bool blurNum;
	std::vector<float> averageVec;
	cv::Mat coordmap_e;


	// ���_���W��CV����GL�ւ̍��W�ϊ��s����v�Z����֐�
	glm::mat4 VertexTransformMatrix(unsigned int w, unsigned int h)
	{
		float scaling_x = 1.0f / ((float)w - 1.0f) * 2.0f;
		float scaling_y = 1.0f / ((float)h - 1.0f) * 2.0f;
		glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), glm::vec3(-1.0f, 1.0f, 0.0f));
		glm::mat4 RotationMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, 0.0f));
		glm::mat4 ScalingMatrix = glm::scale(glm::mat4(), glm::vec3(scaling_x, scaling_y, 0.0f));

		return TranslationMatrix * RotationMatrix * ScalingMatrix;
	}

	// �e�N�X�`�����W��CV����GL�ւ̍��W�ϊ��s����v�Z����֐�
	glm::mat4 TextureTransformMatrix(unsigned int w, unsigned int h)
	{
		float scaling_s = 1.0f / ((float)w - 1.0f);
		float scaling_t = 1.0f / ((float)h - 1.0f);
		glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 RotetionMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, 0.0f));
		glm::mat4 ScalingMatrix = glm::scale(glm::mat4(), glm::vec3(scaling_s, scaling_t, 0.0f));

		return TranslationMatrix * RotetionMatrix * ScalingMatrix;
	}

};

#endif
