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
	// コンストラクタ
	adaptiveGeomCorresp(unsigned int proj_w = 1280, unsigned int proj_h = 800, unsigned int cam_w = 1920, unsigned int cam_h = 1200)
	{
		projWidth = proj_w;
		projHeight = proj_h;
		camWidth = cam_w;
		camHeight = cam_h;

		calib = new Calibration;
		// キャリブレーションデータの読み込み
		calib->loadCalibParam(PROCAM_CALIBRATION_RESULT_FILENAME);

		// 基礎行列をキャリブレーションデータから求める
		FundametalMat = calib->getFundamentalMat(calib->cam_K, calib->proj_K, calib->R, calib->T);

		fps = new myUtl::FPSMajor(1);

	};
	~adaptiveGeomCorresp()
	{};


	// 初期化関数
	bool init(const cv::Mat &coord_map);

	// エピポーラ線方向に対応を複数パターン偏移させる
	// @param1 : 対応先を保存したマット情報を複数保存しておく
	// @param2 : エラー画素をマスキングした画像
	// @param3 : プロジェクタ座標に対するカメラ座標のエピポーラ線を格納した情報
	void calcNewGeometricCorrespond(std::vector<cv::Mat> &_shiftVectorMats, const std::vector<cv::Point2f> &points, const std::vector<cv::Vec3f> &lines);

	// 各画素毎のエピポーラ線への移動量を格納したMatを保持する配列
	std::vector<cv::Mat> shiftVectorMats;

private:

	// プロジェクタの縦横
	unsigned int projWidth, projHeight;
	// カメラの縦横
	unsigned int camWidth, camHeight;


	// 基礎行列
	cv::Mat FundametalMat;

	// 画素対応が保存してあるマット
	cv::Mat coordMap;
	// vector参照用のマット
	cv::Mat indexMap;

	// エピポーラ線を格納
	std::vector<cv::Vec3f> epiLines;

	// 全画素分のエピ線
	std::vector<cv::Point2f> projPoints;

	std::vector<cv::Point2f> camPoints;

	// クラスオブジェクト
	//boost::shared_ptr<Calibration> calib;
	Calibration *calib;
	myUtl::FPSMajor *fps;

	// 直線の方程式とx座標からy座標を計算する
	inline double solveY(cv::Vec3f efficient, double x){
		return -(efficient[0] * x + efficient[2]) / efficient[1];
	};

	inline double solve(double n){
		double x;

	}

	// 方程式を元に直線を描画する
	void drawEpilines(cv::Mat& image, std::vector<cv::Vec3f> lines, std::vector<cv::Point2f> drawPoints);
	
};

#endif
