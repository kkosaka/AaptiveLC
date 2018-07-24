#ifndef CALIBRATION_H
#define CALIBRATION_H
#include <opencv2\opencv.hpp>


class Calibration
{
public:
	Calibration(){};
	~Calibration(){};

	// キャリブレーション結果の読み込み
	void loadCalibParam(const std::string &fileName);
	void loadPGR2PGRCalibParam(const std::string &fileName); // pgr-pgrのキャリブレーション結果
	void loadPGR2KINECTCalibParam(const std::string &fileName); // PGR-KinectColorのキャリブレーション結果
	void loadKINECT2KINECTCalibParam(const std::string &fileName); // KinectColor-IRのキャリブレーション結果

	// 透視投影変換行列の取得(カメラ)
	cv::Mat getCamPerspectiveMat();
	// 透視投影変換行列の取得(プロジェクタ)
	cv::Mat getProjPerspectiveMat();

	// カメラ位置をワールド座標とした際の対象物体の位置の取得
	void getCameraWorldPoint(std::vector<cv::Point3f> &camWorldPoint, const std::vector<cv::Point2f> &imagePoint);

	// 3次元復元
	void reconstruction(std::vector<cv::Point3f> &reconstructPoint, const std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint);

	cv::Mat getHomogeneousCoodinateMatrix(cv::Mat &R, cv::Mat &T);

	// kinect座標系からPGRカメラ座標系へ変換する行列
	void getKinectSpaceToCameraSpaceMatrix(cv::Mat &matrix);

	// projector→カメラへの回転並進行列
	void getCamToProjMatrix(cv::Mat &matrix);
	void getProjToCamMatrix(cv::Mat &matrix);

	// 基本行列を求める関数(カメラ座標を中心としたときの、プロジェクタのR,T行列を入れる)
	cv::Mat getFundamentalMat(const cv::Mat &camA, const cv::Mat &projA, const cv::Mat &R, const cv::Mat &T);

	/***** メンバ変数 *****/
	cv::Size checkerPattern;		// チェッカーパターンの交点の数
	float checkerSize;				// チェッカーパターンのマス目のサイズ(mm)

	std::vector<cv::Point3f> worldPoint;		// チェッカー交点座標と対応する世界座標の値を格納する行列

	// カメラ
	cv::Mat cam_K;					// 内部パラメータ行列
	cv::Mat cam_dist;				// レンズ歪み
	std::vector<cv::Mat> cam_R;		// 回転ベクトル
	std::vector<cv::Mat> cam_T;		// 平行移動ベクトル

	// プロジェクタ
	cv::Mat proj_K;					// 内部パラメータ行列
	cv::Mat proj_dist;				// レンズ歪み
	std::vector<cv::Mat> proj_R;	// 回転ベクトル
	std::vector<cv::Mat> proj_T;	// 平行移動ベクトル

	// ステレオパラメータ
	cv::Mat R;						// カメラ-プロジェクタ間の回転行列
	cv::Mat T;						// カメラ-プロジェクタ間の並進ベクトル
	cv::Mat E;						// 基本行列
	cv::Mat F;						// 基礎行列
	cv::Mat PGR2PGR_R;
	cv::Mat PGR2PGR_T;
	cv::Mat PGR2KINECT_R;
	cv::Mat PGR2KINECT_T;
	cv::Mat KINECT2KINECT_R;				// PGRカメラ-kinect間の回転行列
	cv::Mat KINECT2KINECT_T;				// PGRカメラ-kinect間の並進ベクトル


	// フラグ
	bool calib_flag;
};

#endif