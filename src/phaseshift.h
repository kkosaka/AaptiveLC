#ifndef PHASESHIFT_H
#define PHASESHIFT_H
#pragma warning(disable : 4819)

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>  // 文字列ストリーム
#include <opencv2/opencv.hpp>

#include <direct.h>	// ディレクトリ作成用

#include "graycode.h"
#include "PS_Vertex.h"

#define PI 3.14159265358979323846264338327950288
#define Amp	90.0		// 正弦波画像の振幅
#define Bias 160.0		//正弦波画像のバイアス(底上げ)
#define Proj_num 8				// 投影枚数
#define WaveLength_X 10	// 正弦波画像(X方向)の波長[pixel] 細かいほど色の勾配差が生まれるためよい
#define WaveLength_Y 20	// 正弦波画像(Y方向)の波長[pixel] 

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
		// 位相シフト用ディレクトリパスの設定
		ps_base_dir = std::string(ps_path) + "/PhaseShiftImages";
		ps_image_dir = ps_base_dir + "/SineImages";
		// マスク画像用閾値の設定
		ps_thresh = 40;
		// キャリブレーションに必要な配列の初期化
		phaseValue = cv::Mat(camh, camw, CV_64FC2, cv::Scalar::all(0));
		coord_map_int = cv::Mat(camh, camw, CV_32SC2, cv::Scalar::all(-1));
		coord_map = cv::Mat(camh, camw, CV_32FC2, cv::Scalar::all(-1));
		coord_map_proj = cv::Mat(projh, projw, CV_32SC2, cv::Scalar::all(-1));
		init_flag = false;
		ready = false;
		// 初期化
		this->init();

		// 幾何変換用メッシュの生成
		coord_trans = new PS_Vertex(projWidth, projHeight);

	};

	~PhaseShift()
	{
	};

	// 初期化関数
	void init();

	// opencvを利用して映像投影
	// 呼び出す前に、PGRカメラをストップすること
	//(映像が1pixel左と下にずれるので，輝度補正の際はGLを使うこと)
	void code_projection_opencv();

	// openGLを利用して映像投影
	bool code_projection_opengl(GLFWwindow *window);

	// オーバーライド
	void getCorrespondence(bool isCodeProjection);

	bool getStatus(){ return this->ready; };

	unsigned int psGetProjectionNum() { return Proj_num; };
	std::stringstream psGetProjctionFilePath(unsigned int num, bool vertical);
	std::stringstream psGetCaptureFilePath(unsigned int num, bool vertical);

	// 対応マップを取得する関数
	cv::Mat getCoordMap(){ return coord_map.clone(); };

	// オーバーライド
	//// カメラ撮影領域からプロジェクタ投影領域を切り出し
	void reshapeCam2Proj(cv::Mat &src, cv::Mat &dst);

	// 幾何変換用
	PS_Vertex *coord_trans;

	// ファイルを指定して，座標データを読み込む
	void readPixCorrespData(const std::string &filename, cv::Mat &dst_map);
private:
	const char *ps_path;
	// 生成されるデータの保存場所
	std::string ps_base_dir;
	std::string ps_image_dir;

	// 位相データの軸を0に合わせるために必要な変数
	const int	xplus;
	const int	yplus;

	// 位相情報からマスク画像を作成するための閾値
	unsigned int ps_thresh;

	// 位相シフト法のサイン画像の周期数
	unsigned int waveNumX, waveNumY;

	bool init_flag;

	// 位相値を保持
	cv::Mat phaseValue;
	// カメラ座標対するプロジェクタ座標値を保持したマップ(整数値)
	cv::Mat coord_map_int;
	// カメラ座標対するプロジェクタ座標値を保持したマップ(サブピクセル)
	cv::Mat coord_map;
	// プロジェクタ座標に対するカメラ座標値を保持したマップ(整数値)
	cv::Mat coord_map_proj;

	// 投影領域のマスク画像
	cv::Mat mask;

	bool ready;

	// マスク画像を小さくするための関数
	void smallMaskRange(cv::Mat &src, cv::Mat &dst);

	// ディレクトリの作成
	void psCreateDirs();
	
	// テクスチャを読み込む関数
	bool readTexture();

	// 正弦波画像を作成する関数
	void makeSineWaveImage();

	void code_restore();
	void restore_phase_value();
	void phaseConnection();
	void errorCheck();

	// 対応を保存する
	void writePixCorrespData();

	// 対応を読み込む
	void readPixCorrespData();

	// 整数値に丸めた座標マップ
	void calcIntCoordMap();

	// debug
	void calcNearestNeighbor();

	// glの座標系の計算をする
	void getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture, bool cam2proj);

	// 幾何変換用の頂点配列を取得する
	void getBufferArrays(cv::Mat &in_vertex, cv::Mat &in_texture, PS_Vertex *mesh);

};

#endif