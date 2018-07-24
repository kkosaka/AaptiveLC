#ifndef GRAYCODE_H
#define GRAYCODE_H
#pragma warning(disable : 4819)

#include <iostream>
#include <direct.h>	// ディレクトリ作成用
#include <iomanip>  // 文字列ストリーム

#include <gl/glew.h>
#include <GLFW/glfw3.h>

// glfwのpragma文
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glewのpragma文
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
	// カメラスレッドのポインタ，nega_posi閾値，ディレクトリパス，projw, projh, camW, camH, waveLnegthx, waveLnegthY
	GrayCode(PGRCamera *pgrcam, const int _threshold = 40, const char *dirctorypass = ".", const unsigned int projw = 1280, const unsigned int projh = 800
		, const unsigned int camw = 1920, const unsigned int camh = 1200, int _waveLength_x = 1, int _waveLength_y = 1)
		:path(dirctorypass)
		, projWidth(projw), projHeight(projh), camWidth(camw), camHeight(camh)
		, waveNumX(projw / _waveLength_x), waveNumY(projh / _waveLength_y)
	{
		// 変数の初期化
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

		// ディレクトリパスの設定
		gc_base_dir += std::string(path) + "/GrayCodeImages";
		gc_mask_address = gc_base_dir + "/mask.bmp";

		// 幾何変換用メッシュの生成
		coord_trans_mesh.resize(2);
		coord_trans_mesh[0] = new Vertex(projWidth, projHeight);
		coord_trans_mesh[1] = new Vertex(camWidth, camHeight);

		glsl = new glUtl::glShader;
		square = new SQUARE;

		// スレッドの受け渡し
		pgrCamera = pgrcam;

		ready = false;
	};
	~GrayCode(){

	};

	// 定数
	typedef enum gcflag{
		POSI = true,		// ポジ
		NEGA = false,		// ネガ
		VERTICAL = true,	// 縦縞
		HORIZONTAL = false,	// 横縞
	} gcflag;

	// ファイルパス
	const char *path;
	std::string gc_base_dir;
	std::string gc_mask_address;

	// 投影サイズ
	const unsigned int projWidth;
	const unsigned int projHeight;
	// カメラ解像度
	const unsigned int camWidth;
	const unsigned int camHeight;

	// 幾何変換用の頂点配列([0] : camera→projector, [1] : projector → camera)
	//Vertex **coord_trans_mesh[2];
	//Vertex coord_trans_mesh_proj2cam;

	// 投影時に使用するシェーダ
	glUtl::glShader *glsl;

	SQUARE *square;

	// カメラスレッド
	PGRCamera *pgrCamera;

	// ディレクトリの作成
	void gcCreateDirs();

	// グレイコード画像作成
	void makeGraycodeImage();

	// 2値化処理
	void make_thresh();

	// 初期化関数
	virtual void init();

	// opencvを利用して映像投影
	// 呼び出す前に、PGRカメラをストップすること
	//(映像が1pixel左と下にずれるので，輝度補正の際はGLを使うこと)
	void code_projection_opencv();

	// openGLを利用して映像投影
	// 呼び出す前に、PGRカメラをストップすること
	bool code_projection_opengl(GLFWwindow *window);

	// 2値化コード復元
	virtual void getCorrespondence(bool isCodeProjection);

	// グレイコードをデコードする
	void decodeGrayCode();

	//// カメラ撮影領域からプロジェクタ投影領域を切り出し
	virtual void reshapeCam2Proj(cv::Mat &src, cv::Mat &dst);

	// 縦方向のビット数の取得
	unsigned int gcGetBitNum_h() { return h_bit; };
	// 横方向のビット数の取得
	unsigned int gcGetBitNum_w() { return w_bit; };
	// 縦横方向のビット総数の取得
	unsigned int gcGetBitNum_all() { return all_bit; };
	// 投影画像のパスを取得
	std::stringstream gcGetProjctionFilePath(unsigned int num, bool pos);
	// 撮影画像のパスを取得
	std::stringstream gcGetCaptureFilePath(unsigned int num, bool horizontal, bool posi);
	// 二値化画像のパスを取得
	std::stringstream gcGetThresholdFilePath(unsigned int num);
	// 対応マップの取得
	cv::Mat gcGetGrayCode(){ return graycode; };
	// 対応マップの取得
	cv::Mat gcGetDecodeMap(){ return decode; };
	// 対応マップの取得
	cv::Mat gcGetCoordMap(){ return coordmap; };
	// 幾何補正用のマスク画像を作成
	void getMaskForGeometricCorrection();

	bool getStatus(){ return ready; };

private:

	// ネガポジ差分の閾値
	int threshold;

	cv::Mat graycode;	//<! 波長に合わせたグレイコード（波の数の解像度[高さ][幅]）
	cv::Mat graycode_mat;	//<! グレイコード（プロジェクタ解像度[高さ][幅]）
	cv::Mat coordmap;	// 対応点を保持

	//<! 高さ，幅の必要ビット数
	unsigned int h_bit, w_bit;
	//<! 合計ビット数（h_bit + w_bit
	unsigned int all_bit;
	// 位相シフト法のサイン画像の一周期の長さ
	unsigned int waveLength_x, waveLength_y;
	// 位相シフト法のサイン画像の周期数
	unsigned int waveNumX, waveNumY;

	// 復元されたグレイコードを整理するための連想配列
	std::map<int, cv::Point> *g_code_map;

	// テクスチャを保持する変数
	std::vector<Texture> code_texture;

	std::vector<Vertex*> coord_trans_mesh;

	// テクスチャを読み込む関数
	bool readTexture();

	//デコード情報を保持しておくMat（カメラ解像度）
	cv::Mat decode;

	// 対応取得ができているか判定
	bool ready;

	// グレイコード作成
	void initGraycode();

	// グレイコードの画像を利用してマスクを生成する関数()
	void makeMask(cv::Mat &mask);

	// ポジとネガの差分を取ってMASK_THRESH以上の輝度のピクセルを白にする
	void makeMaskFromCam(cv::Mat &posi, cv::Mat &nega, cv::Mat &result, int thresholdValue);

	// 2値化処理関数 
	void thresh(cv::Mat &posi, cv::Mat &nega, cv::Mat &thresh_img, int thresh_value);

	// glの座標系の計算をする
	void getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture);

	// 幾何変換用の頂点配列を取得する
	void getBufferArrays();
};

#endif