#include "Calibration.h"


// キャリブレーション結果の読み込み
void Calibration::loadCalibParam(const std::string &fileName)
{
	// xmlファイルの読み込み
	cv::FileStorage cvfs(fileName, cv::FileStorage::READ);

	cvfs["cam_K"] >> cam_K;
	cvfs["cam_dist"] >> cam_dist;
	cvfs["cam_R"] >> cam_R;
	cvfs["cam_T"] >> cam_T;
	cvfs["proj_K"] >> proj_K;
	cvfs["proj_dist"] >> proj_dist;
	cvfs["proj_R"] >> proj_R;
	cvfs["proj_T"] >> proj_T;
	cvfs["R"] >> R;
	cvfs["T"] >> T;
	cvfs["E"] >> E;
	cvfs["F"] >> F;

	calib_flag = true;

}

void Calibration::loadPGR2PGRCalibParam(const std::string &fileName)
{
	// xmlファイルの読み込み
	cv::FileStorage cvfs(fileName, cv::FileStorage::READ);

	// kinect pgrcamera 結果
	cvfs["R"] >> PGR2PGR_R;
	cvfs["T"] >> PGR2PGR_T;
}

void Calibration::loadPGR2KINECTCalibParam(const std::string &fileName)
{
	// xmlファイルの読み込み
	cv::FileStorage cvfs(fileName, cv::FileStorage::READ);

	// kinect pgrcamera 結果
	cvfs["R"] >> PGR2KINECT_R;
	cvfs["T"] >> PGR2KINECT_T;
}
void Calibration::loadKINECT2KINECTCalibParam(const std::string &fileName)
{
	// xmlファイルの読み込み
	cv::FileStorage cvfs(fileName, cv::FileStorage::READ);

	// kinect pgrcamera 結果
	cvfs["R"] >> KINECT2KINECT_R;
	cvfs["T"] >> KINECT2KINECT_T;
}

// 透視投影変換行列の取得(カメラ)
cv::Mat Calibration::getCamPerspectiveMat()
{
	// 回転と並進を結合
	cv::Mat extrinsic = cv::Mat::eye(4, 4, CV_64F);

	// 内部パラメータの変形
	cv::Mat intrinsic(3, 4, CV_64F);
	intrinsic.at<double>(0,0) = cam_K.at<double>(0,0);
	intrinsic.at<double>(0,1) = cam_K.at<double>(0,1);
	intrinsic.at<double>(0,2) = cam_K.at<double>(0,2);
	intrinsic.at<double>(1,0) = cam_K.at<double>(1,0);
	intrinsic.at<double>(1,1) = cam_K.at<double>(1,1);
	intrinsic.at<double>(1,2) = cam_K.at<double>(1,2);
	intrinsic.at<double>(2,0) = cam_K.at<double>(2,0);
	intrinsic.at<double>(2,1) = cam_K.at<double>(2,1);
	intrinsic.at<double>(2,2) = cam_K.at<double>(2,2);
	intrinsic.at<double>(0,3) = 0.0;
	intrinsic.at<double>(1,3) = 0.0;
	intrinsic.at<double>(2,3) = 0.0;

	return intrinsic * extrinsic;
}


// 透視投影変換行列の取得(プロジェクタ)
cv::Mat Calibration::getProjPerspectiveMat()
{
	// 回転と並進を結合
	cv::Mat extrinsic(4, 4, CV_64F);
	extrinsic.at<double>(0,0) = R.at<double>(0,0);
	extrinsic.at<double>(0,1) = R.at<double>(0,1);
	extrinsic.at<double>(0,2) = R.at<double>(0,2);
	extrinsic.at<double>(1,0) = R.at<double>(1,0);
	extrinsic.at<double>(1,1) = R.at<double>(1,1);
	extrinsic.at<double>(1,2) = R.at<double>(1,2);
	extrinsic.at<double>(2,0) = R.at<double>(2,0);
	extrinsic.at<double>(2,1) = R.at<double>(2,1);
	extrinsic.at<double>(2,2) = R.at<double>(2,2);
	extrinsic.at<double>(0,3) = T.at<double>(0,0);
	extrinsic.at<double>(1,3) = T.at<double>(1,0);
	extrinsic.at<double>(2,3) = T.at<double>(2,0);
	extrinsic.at<double>(3,0) = 0.0;
	extrinsic.at<double>(3,1) = 0.0;
	extrinsic.at<double>(3,2) = 0.0;
	extrinsic.at<double>(3,3) = 1.0;

	// 内部パラメータの変形
	cv::Mat intrinsic(3, 4, CV_64F);
	intrinsic.at<double>(0,0) = proj_K.at<double>(0,0);
	intrinsic.at<double>(0,1) = proj_K.at<double>(0,1);
	intrinsic.at<double>(0,2) = proj_K.at<double>(0,2);
	intrinsic.at<double>(1,0) = proj_K.at<double>(1,0);
	intrinsic.at<double>(1,1) = proj_K.at<double>(1,1);
	intrinsic.at<double>(1,2) = proj_K.at<double>(1,2);
	intrinsic.at<double>(2,0) = proj_K.at<double>(2,0);
	intrinsic.at<double>(2,1) = proj_K.at<double>(2,1);
	intrinsic.at<double>(2,2) = proj_K.at<double>(2,2);
	intrinsic.at<double>(0,3) = 0.0;
	intrinsic.at<double>(1,3) = 0.0;
	intrinsic.at<double>(2,3) = 0.0;

	return intrinsic * extrinsic;
}


// カメラ位置をワールド座標とした際の対象物体の位置の取得
void Calibration::getCameraWorldPoint(std::vector<cv::Point3f> &camWorldPoint, const std::vector<cv::Point2f> &imagePoint)
{
	cv::Mat rvec, tvec, rmat;

	// チェッカーパターンの位置検出
	cv::solvePnP(worldPoint, imagePoint, cam_K, cv::Mat(), rvec, tvec);		

	cv::Rodrigues(rvec, rmat);		// 回転行列に変換

	// チェッカーパターン中心からカメラ中心に座標変換
	rmat = rmat.t();	// 転置行列

	cv::Mat extrinsic(4, 4, CV_64F);
	extrinsic.at<double>(0,0) = rmat.at<double>(0,0);
	extrinsic.at<double>(0,1) = rmat.at<double>(0,1);
	extrinsic.at<double>(0,2) = rmat.at<double>(0,2);
	extrinsic.at<double>(1,0) = rmat.at<double>(1,0);
	extrinsic.at<double>(1,1) = rmat.at<double>(1,1);
	extrinsic.at<double>(1,2) = rmat.at<double>(1,2);
	extrinsic.at<double>(2,0) = rmat.at<double>(2,0);
	extrinsic.at<double>(2,1) = rmat.at<double>(2,1);
	extrinsic.at<double>(2,2) = rmat.at<double>(2,2);
	extrinsic.at<double>(0,3) = cv::Mat(-rmat*tvec).at<double>(0,0);
	extrinsic.at<double>(1,3) = cv::Mat(-rmat*tvec).at<double>(1,0);
	extrinsic.at<double>(2,3) = cv::Mat(-rmat*tvec).at<double>(2,0);
	extrinsic.at<double>(3,0) = 0.0;
	extrinsic.at<double>(3,1) = 0.0;
	extrinsic.at<double>(3,2) = 0.0;
	extrinsic.at<double>(3,3) = 1.0;

	// チェッカーパターンの交点位置
	for(int i=0; i<worldPoint.size(); ++i)
	{
		cv::Mat checker_pos = extrinsic.inv() * cv::Mat((cv::Mat_<double>(4,1) << worldPoint[i].x, worldPoint[i].y, worldPoint[i].z, 1.0));		// チェッカーパターンの位置
		camWorldPoint.emplace_back(cv::Point3f(checker_pos.at<double>(0)/checker_pos.at<double>(3), checker_pos.at<double>(1)/checker_pos.at<double>(3), checker_pos.at<double>(2)/checker_pos.at<double>(3)));
	}
}


// 3次元復元
void Calibration::reconstruction(std::vector<cv::Point3f> &reconstructPoint, const std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint)
{
	// 透視投影変換行列
	cv::Mat cam_pers = getCamPerspectiveMat();
	cv::Mat proj_pers = getProjPerspectiveMat();


	static cv::Mat f(4, 1, CV_64FC1, cv::Scalar(0.0));
	static cv::Mat q(4, 3, CV_64FC1, cv::Scalar(0.0));
	static cv::Mat v(3, 1, CV_64FC1, cv::Scalar(0.0));

	// object spaceの最小化による3次元復元
	//for(int i=0; i<projPoint.size(); ++i) //original
	for(int i=0; i<imagePoint.size(); ++i)
	{
		f.at<double>(0, 0) = imagePoint[i].x * cam_pers.at<double>(2, 3) - cam_pers.at<double>(0, 3);
		f.at<double>(1, 0) = imagePoint[i].y * cam_pers.at<double>(2, 3) - cam_pers.at<double>(1, 3);
		f.at<double>(2, 0) = projPoint[i].x * proj_pers.at<double>(2, 3) - proj_pers.at<double>(0, 3);
		f.at<double>(3, 0) = projPoint[i].y * proj_pers.at<double>(2, 3) - proj_pers.at<double>(1, 3);

		q.at<double>(0, 0) = cam_pers.at<double>(0, 0) - cam_pers.at<double>(2, 0) * imagePoint[i].x;
		q.at<double>(0, 1) = cam_pers.at<double>(0, 1) - cam_pers.at<double>(2, 1) * imagePoint[i].x;
		q.at<double>(0, 2) = cam_pers.at<double>(0, 2) - cam_pers.at<double>(2, 2) * imagePoint[i].x;
		q.at<double>(1, 0) = cam_pers.at<double>(1, 0) - cam_pers.at<double>(2, 0) * imagePoint[i].y;
		q.at<double>(1, 1) = cam_pers.at<double>(1, 1) - cam_pers.at<double>(2, 1) * imagePoint[i].y;
		q.at<double>(1, 2) = cam_pers.at<double>(1, 2) - cam_pers.at<double>(2, 2) * imagePoint[i].y;
		q.at<double>(2, 0) = proj_pers.at<double>(0, 0) - proj_pers.at<double>(2, 0) * projPoint[i].x;
		q.at<double>(2, 1) = proj_pers.at<double>(0, 1) - proj_pers.at<double>(2, 1) * projPoint[i].x;
		q.at<double>(2, 2) = proj_pers.at<double>(0, 2) - proj_pers.at<double>(2, 2) * projPoint[i].x;
		q.at<double>(3, 0) = proj_pers.at<double>(1, 0) - proj_pers.at<double>(2, 0) * projPoint[i].y;
		q.at<double>(3, 1) = proj_pers.at<double>(1, 1) - proj_pers.at<double>(2, 1) * projPoint[i].y;
		q.at<double>(3, 2) = proj_pers.at<double>(1, 2) - proj_pers.at<double>(2, 2) * projPoint[i].y;

		v = q.inv(cv::DECOMP_SVD) * f;
		//v *= 0.001f;
		reconstructPoint.emplace_back(cv::Point3f(v.at<double>(0, 0), v.at<double>(1, 0), v.at<double>(2, 0)));
	}
}

// 外部パラメータを同次座標の行列で表現
cv::Mat Calibration::getHomogeneousCoodinateMatrix(cv::Mat &R, cv::Mat &T)
{
	// カメラを中心とした各デバイスへの回転・並進ベクトルを求めたいときは外部パラメータを逆にする必要がある
	cv::Mat rmat = R.t();
	cv::Mat tvec = T;
	cv::Mat extrinsic(4, 4, CV_64F);
	extrinsic.at<double>(0, 0) = rmat.at<double>(0, 0);	// 回転
	extrinsic.at<double>(0, 1) = rmat.at<double>(0, 1);
	extrinsic.at<double>(0, 2) = rmat.at<double>(0, 2);
	extrinsic.at<double>(1, 0) = rmat.at<double>(1, 0);
	extrinsic.at<double>(1, 1) = rmat.at<double>(1, 1);
	extrinsic.at<double>(1, 2) = rmat.at<double>(1, 2);
	extrinsic.at<double>(2, 0) = rmat.at<double>(2, 0);
	extrinsic.at<double>(2, 1) = rmat.at<double>(2, 1);
	extrinsic.at<double>(2, 2) = rmat.at<double>(2, 2);
	extrinsic.at<double>(0, 3) = cv::Mat(-rmat*tvec).at<double>(0, 0);	// 並進
	extrinsic.at<double>(1, 3) = cv::Mat(-rmat*tvec).at<double>(1, 0);
	extrinsic.at<double>(2, 3) = cv::Mat(-rmat*tvec).at<double>(2, 0);
	extrinsic.at<double>(3, 0) = 0.0;
	extrinsic.at<double>(3, 1) = 0.0;
	extrinsic.at<double>(3, 2) = 0.0;
	extrinsic.at<double>(3, 3) = 1.0;
	return extrinsic;

}

// Kinect→PGRカメラ座標系へ変換するのに必要な行列を計算
void Calibration::getKinectSpaceToCameraSpaceMatrix(cv::Mat &matrix)
{
	cv::Mat KIENCT2KINECT_Mat = getHomogeneousCoodinateMatrix(KINECT2KINECT_R, KINECT2KINECT_T);	//KINECTカラーカメラからIRカメラへの外部パラメータ行列
	cv::Mat PGR2KINECT_Mat = getHomogeneousCoodinateMatrix(PGR2KINECT_R, PGR2KINECT_T);	// PGRカメラからKINECTCOLORカメラへの外部パラメータ行列

	// KINECTColor-IRの回転並進行列，PGR-KINECTの回転並進行列の順にかけていく
	matrix = cv::Mat(4, 4, CV_64F);
	matrix = PGR2KINECT_Mat * KIENCT2KINECT_Mat;

}

void Calibration::getCamToProjMatrix(cv::Mat &matrix)
{
	matrix = cv::Mat(4, 4, CV_64F);
	matrix = getHomogeneousCoodinateMatrix(R, T);

}
void Calibration::getProjToCamMatrix(cv::Mat &matrix)
{
	matrix = cv::Mat(4, 4, CV_64F);
	matrix.at<double>(0, 0) = R.at<double>(0, 0);	// 回転
	matrix.at<double>(0, 1) = R.at<double>(0, 1);
	matrix.at<double>(0, 2) = R.at<double>(0, 2);
	matrix.at<double>(1, 0) = R.at<double>(1, 0);
	matrix.at<double>(1, 1) = R.at<double>(1, 1);
	matrix.at<double>(1, 2) = R.at<double>(1, 2);
	matrix.at<double>(2, 0) = R.at<double>(2, 0);
	matrix.at<double>(2, 1) = R.at<double>(2, 1);
	matrix.at<double>(2, 2) = R.at<double>(2, 2);
	matrix.at<double>(0, 3) = T.at<double>(0, 0);	// 並進
	matrix.at<double>(1, 3) = T.at<double>(1, 0);
	matrix.at<double>(2, 3) = T.at<double>(2, 0);
	matrix.at<double>(3, 0) = 0.0;
	matrix.at<double>(3, 1) = 0.0;
	matrix.at<double>(3, 2) = 0.0;
	matrix.at<double>(3, 3) = 1.0;
}

cv::Mat Calibration::getFundamentalMat(const cv::Mat &camA, const cv::Mat &projA, const cv::Mat &R, const cv::Mat &T)
{
	// 歪対称行列をつくる
	//*******
	//	T = [T1, T2, T3]^tとしたとき、歪対称行列は以下の式で表される
	//	[T]x = [0, -T3, T2;
	//			T3, 0, -T1;
	//			-T2, T1, 0]
	//********
	cv::Mat alt_T(3, 3, CV_64F);
	alt_T.at<double>(0, 0) = 0;
	alt_T.at<double>(0, 1) = -T.at<double>(0, 2);
	alt_T.at<double>(0, 2) = T.at<double>(0, 1);
	alt_T.at<double>(1, 0) = T.at<double>(0, 2);
	alt_T.at<double>(1, 1) = 0;
	alt_T.at<double>(1, 2) = -T.at<double>(0, 0);
	alt_T.at<double>(2, 0) = -T.at<double>(0, 1);
	alt_T.at<double>(2, 1) = T.at<double>(0, 0);
	alt_T.at<double>(2, 2) = 0;

	// 基礎行列(E)を求める
	cv::Mat E = alt_T * R;

	// プロジェクタの内部行列Aの逆行列の転値を左から乗算する。
	// カメラの内部行列の逆行列を左から乗算する。
	cv::Mat inv_projK = projA.inv();
	cv::Mat inv_camK = camA.inv();

	cv::Mat F = inv_projK.t() * E * inv_camK;

	//std::cout << "基礎行列 F :" << std::endl;
	//std::cout << F << std::endl;

	return F;
}

