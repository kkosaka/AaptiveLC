#ifndef MYGL_H
#define MYGL_H

#pragma once
// 標準ヘッダをインクルード
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

// glfwのpragma文
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glewのpragma文
#pragma comment(lib,"glew32.lib")

//目標画像のテクスチャユニット
//(レンダリング結果を目標画像とするので0)
#define TEXTURE_UNIT_INPUTIMAGE			0			
#define TEXTURE_UNIT_INPUTIMAGE_ENUM	GL_TEXTURE0

//応答関数計算用のプロジェクタ投影画像のテクスチャユニット
#define TEXTURE_UNIT_PRJIMAGE			1
#define TEXTURE_UNIT_PRJIMAGE_ENUM		GL_TEXTURE1

//応答関数計算用のカメラ画像のテクスチャユニット
#define TEXTURE_UNIT_CAMIMAGE			2
#define TEXTURE_UNIT_CAMIMAGE_ENUM		GL_TEXTURE2

//最小二乗パラメータの分子、分母のテクスチャユニット
#define TEXTURE_UNIT_NUME				3
#define TEXTURE_UNIT_NUME_ENUM			GL_TEXTURE3
#define TEXTURE_UNIT_DENOMI				4
#define TEXTURE_UNIT_DENOMI_ENUM		GL_TEXTURE4

// マスク画像のテクスチャユニット
#define TEXTURE_UNIT_MASK				5
#define TEXTURE_UNIT_MASK_ENUM			GL_TEXTURE5

using namespace glUtl;

class myGL : public Window
{

public:
	// コンストラクタ
	myGL(PGRCamera *pgrcam, int width = 640, int height = 480, const char *title = "Projection") : Window(width, height, title)
	{
		// ウィンドウの垂直同期off
		glfwSwapInterval(0);
		//glfwSwapInterval(1);

		// コールバック
		glfwSetFramebufferSizeCallback(window, resize);
		glfwSetKeyCallback(window, keyboard);

		// インスタンスのthisポインタを記録
		glfwSetWindowUserPointer(window, this);

		loop_flag = false;
		geom_flag = true;
		ready = false;
		agc_calc_flag = false;
		stop_flag = false;
		lc_noRender_flag = false;

		// スレッドの受け渡し
		pgrCamera = pgrcam;
		//pgrOpenCV = new TPGROpenCV(0);

		projWidth = getWidth();
		projHeight = getHeight();
		camWidth = pgrCamera->getCamWidth();
		camHeight = pgrCamera->getCamHeight();

		// コンストラクタの初期化
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

		// フレームサイズに合う四角形メッシュ
		texture_vertex = new Vertex(projWidth, projHeight);

		// 投影画像を格納した配列
		imageFileList = new myUtl::getImageFiles(IMAGE_DIRECTORY);
		// 投影画像のインデックス
		imageIndex = 0;

		// 補正回数をカウントする変数
		lc_counter = 0;

		// 各パラメータの初期化
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

	// 初期処理
	bool init();

	// 毎フレーム行う処理
	void display();

	void swapBuffers();

	// 座標変換を行う関数
	void coord_trans(GLuint cam_texobj, GLuint proj_texobj, cv::Mat &cameraImage = cv::Mat());
	void coord_trans_inv(GLuint proj_texobj, GLuint cam_texobj, GLuint mask);

	// 輝度補正を行う関数
	void lc_projection(float targetImage_ScaleAlpha, float targetImage_ScaleBeta, float ff, float diff_th, GLuint maskTexture, GLuint texture_k_after, GLuint  texture_k_diff);

	// 幾何対応付けを行う関数
	void getCoordinateCorrespondance(bool isProjection);

	// カラーキャリブレーションを行う関数
	void getColorCalibration(bool isProjection);

	// 画像の読み込み

	// コールバック処理
	static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void resize(GLFWwindow *const window, int width, int height);

	// キー操作用関数
	void keyFunc(int key);

	// コンソール画面に表示する内容
	void printInfo();

	void getProCamDelay();

protected:
	unsigned int projWidth, projHeight;	// プロジェクタ縦横
	unsigned int camWidth, camHeight;	// カメラ縦横
	int delay;	//　遅延量

	// カメラスレッド
	PGRCamera *pgrCamera;

	// クラスオブジェクトの生成
	PhaseShift *ps;
	Vertex *texture_vertex;
	SQUARE *sq;
	//TPGROpenCV *pgrOpenCV;
	LC *lc;
	TextureObjectStocker *lumiTexs_RGBA32F;	// 輝度補正に使用するテクスチャ(RGB32F)を管理する変数
	TextureObjectStocker *lumiTexs_RGBA;	// 輝度補正に使用するテクスチャ(RGB)を管理する変数
	TextureObjectStocker *camTexs_RGBA;		// テクスチャ(RGB)を管理する変数
	TextureObjectStocker *camTexs_RGBA32F;		// テクスチャ(RGB)を管理する変数

	adaptiveGeomCorresp *agc;
	
	// 投影画像を読み込んだ配列
	myUtl::getImageFiles *imageFileList;

	// コンソール画面に表示するFPSカウンター
	myUtl::FPSMajor *fps_console;
	// 時間計測用のクラス
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

	// シェーダパラメータのロケーション
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

	//-----各パラメータのテクスチャオブジェクト-----
	GLuint txo_camImageTexture;				//幾何変換済みのカメラ撮影画像 (RGBA)

	GLuint txo_inputImageTexture;			//オリジナル画像 (RGBA)
	GLuint txo_targetImageTexture;			//目標画像 (RGBA)
	GLuint txo_projImageTexture;			//投影画像
	GLuint txo_transCamImageTexture;		//幾何変換済みのカメラ撮影画像 (RGBA)
	GLuint txo_maskTexture;					//マスク画像
	GLuint txo_agc_maskTexture;				//マスク画像

	GLuint txo_projImage_after;				//更新された補正画像(RGBA)
	GLuint txo_numerator_before;			//前回の分子成分 (RGBA32F)
	GLuint txo_denominator_before;			//前回の分母成分 (RGBA32F)
	GLuint txo_numerator_after;				//更新された分子成分 (RGBA32F)
	GLuint txo_denominator_after;			//更新された分母成分 (RGBA32F)
	GLuint txo_k_after;						//更新された反射率K (RGBA32F)、デバッグ用
	std::vector<GLuint> txo_k;				//更新された反射率K (RGBA32F)、デバッグ用
	std::vector<GLuint> txo_minV;				//更新された反射率K (RGBA32F)、デバッグ用
	GLuint txo_diff;

	// Vertex Array Object
	GLuint vao;		
	// Transform Feedback Object
	GLuint feedback;	

	/* Buffer Objects */
	glBuffer<cv::Vec4f> *PBO_projSize;			// 汎用的に使うPBO(プロジェクタ解像度)
	glBuffer<cv::Vec4f> *VBO_valiance;			// 分散値
	glBuffer<cv::Vec4f> *VBO_minValiance;		// 最小分散値
	glBuffer<cv::Vec4f> *VBO_minValiance_Copy;	// 最小分散値の一時的なコピー用
	glBuffer<cv::Vec4f> *VBO_shiftIndex;		// 最小分散値のインデックスを保持
	glBuffer<cv::Vec4f> *VBO_shiftIndex_Copy;	// 最小分散値のインデックスの一時的なコピー用
	glBuffer<cv::Vec4f> *VBO_errorPixels;		// エラー画素検出結果を保持

	glTexBuffObj<cv::Vec4f> *TBO_projSize;		// 汎用的に使うPBO(プロジェクタ解像度)
	glTexBuffObj<cv::Vec2f> *TBO_offsetCopy;	// オフセット値の一時的なコピー用
	glTexBuffObj<GLfloat> *TBO_vboStatus;		// 状態管理用のTBO
	glTexBuffObj<GLfloat> *TBO_vboMask;			// 半分補正用マスクに使うVBO

	// 評価実験用変数
	glBuffer<cv::Vec4b> *PBO_CAMERAIMAGES;	// カメラ画像をVBOへコピーするためのPBO
	glBuffer<cv::Vec4b> *VBO_CAMERAIMAGES[1000];	// カメラ画像を保存しておく変数(1000 frame 分)

	// テクスチャを保存する関数
	void debug_textures();
	cv::Mat saveTexture(GLuint texId, const string &filename, int w, int h, bool isFlip = 1, bool isWrite = 0, bool isSave = 1);
	cv::Mat &textureToMat(GLuint texture, int w, int h);

	// 入力画像を作成する関数
	void getInputTexture(cv::Mat &inputImage, GLuint &textureObj, cv::Mat &camGeomImage = cv::Mat());

	// RLSパラメータを初期化する関数
	void resetRLSParameter();

	// バッファを生成する関数
	void getGLBuffers();

	// 反射率を計算する関数
	void getProjectionSurfaceReflectance(float ff, GLuint maskTexture, GLuint texture_k_after);

	bool writeToExelFile(cv::Mat &src, const std::string &str, cv::Point point = cv::Point(0,0), int range = 0);
	bool readWriteExelFile(const std::vector<std::string> &fileLists, std::vector<cv::Point> points, const std::string &dstStr);

	// 画像用
	int imageIndex;
	cv::Mat srcImage;
	cv::Mat capImage;
	cv::Mat capImage_before;
	cv::Mat whiteImage;
	cv::Mat agc_maskImage;
	cv::Mat cam_mask;

	float brightness;	// 目標画像の明るさ調整
	float fogetFactor;	// 忘却係数の調整
	unsigned int diff_thresh;	// 差分の閾値

	bool ready;

	// 輝度補正用フラグ
	bool loop_flag;
	// 幾何補正用フラグ
	bool geom_flag;
	
	int lc_flag;

		// 輝度補正on/off
	boost::atomic<bool> nonCorrection;

	unsigned int lc_counter;

	GLuint txo_first_k;

	// 幾何対応を修正する関数
	void adaptiveGeom();

	bool agc_calc_flag;
	bool lc_noRender_flag;
	bool stop_flag;
	bool write_flag;	// debug : csvファイルへの書き込み用
	int systemFlag;
	bool blurNum;
	std::vector<float> averageVec;
	cv::Mat coordmap_e;


	// 頂点座標のCVからGLへの座標変換行列を計算する関数
	glm::mat4 VertexTransformMatrix(unsigned int w, unsigned int h)
	{
		float scaling_x = 1.0f / ((float)w - 1.0f) * 2.0f;
		float scaling_y = 1.0f / ((float)h - 1.0f) * 2.0f;
		glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), glm::vec3(-1.0f, 1.0f, 0.0f));
		glm::mat4 RotationMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, 0.0f));
		glm::mat4 ScalingMatrix = glm::scale(glm::mat4(), glm::vec3(scaling_x, scaling_y, 0.0f));

		return TranslationMatrix * RotationMatrix * ScalingMatrix;
	}

	// テクスチャ座標のCVからGLへの座標変換行列を計算する関数
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
