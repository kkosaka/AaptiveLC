#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

// glfwのpragma文
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glewのpragma文
#pragma comment(lib,"glew32.lib")

#include <opencv2/opencv.hpp>

#include <direct.h>	// ディレクトリ作成用

#include "vertex.h"
//#include "PGROpenCV.h"
#include "PGRCameraThread.h"
#include "myParam.h"

#include <algorithm>

class LC
{

public:

	LC(PGRCamera *pgrcam, const unsigned int projw = 1280, const unsigned int projh = 800, const unsigned int camw = 1920, const unsigned int camh = 1200)
	{
		pgrCamera = pgrcam;
		LP = new LumiParameter;
		LP->Gamma = cv::Vec3f(1.992f, 2.0715f, 2.1461f);//後に計算するので適当な数値を入れる
	};
	~LC(){};

	// 色混合行列の計算
	int setColorMixingMatrix();

	// 色混合行列の計測用関数
	void color_projection(GLFWwindow *window);

	// パラメータをセットする関数

	// 反射率の更新
	void getProjectorColorSpaceImage(cv::Mat &camera, cv::Mat &projector);

	// 目標画像を作成する関数
	cv::Mat setTargetImage(cv::Mat &src, bool autoBrightness = 0);

	void getImageDifference(cv::Mat &image1, cv::Mat &image2, cv::Mat &dst);
	void getImageRate(cv::Mat &image1, cv::Mat &image2, cv::Mat &dst);

	// パラメータを取得する関数
	cv::Vec3f getGamma(){ return LP->Gamma; };
	cv::Vec3f getCmaxPixel(){ return LP->Cmax; };
	cv::Vec3f getCminPixel(){ return LP->Cmin; };
	cv::Matx33f getCMM(){ return LP->CMM; };
	cv::Matx33f getCMMinv(){ return LP->CMMinv; };

	// 画像評価関数(MSE and PSNR)
	cv::Vec3f calcMSE(cv::Mat &image1, cv::Mat &image2, cv::Mat &mask = cv::Mat());
	template <typename T> float calcPSNR(cv::Mat &image1, cv::Mat &image2, float &dst_MSE, cv::Mat &mask = cv::Mat())
	{
		float E_sum = 0.0f;
		float counter = 0.0;
		for (int y = 0; y < image1.rows; y++){
			for (int x = 0; x < image2.cols; x++){
				if (!mask.empty()){
					T maskColor = mask.ptr<T>(y)[x];
					float m_sum = 0.0;
					for (int c = 0; c < mask.channels(); c++){
						m_sum += maskColor[c];
					}
					if (m_sum == 0)
						continue;
				}

				T img1 = image1.ptr<T>(y)[x];
				T img2 = image2.ptr<T>(y)[x];
				for (int c = 0; c < image1.channels(); c++){
					float error = powf(float(img1[c] - img2[c]), 2.0);
					// 差の二乗の総和(全チャンネル)
					E_sum += error;
					counter += 1.0f;
				}

			}
		}
		float MAX = image1.depth() == CV_8U ? 255.0f : 1.0f;
		//std::cout << "MAX" << MAX << std::endl;
		float MSE = std::fmax(0.0001, E_sum / counter);
		//std::cout << "MSE" << MSE << std::endl;
		dst_MSE = sqrt(E_sum / counter);
		//std::cout << "calc_MSE" << dst_MSE << std::endl;
		float PSNR = 10 * log10(MAX * MAX / MSE);
		//std::cout << "PSNR :" << PSNR << std::endl;
		return PSNR;
	}

	// SSIM
	float calcSSIM(cv::Mat &image1, cv::Mat &image2, cv::Mat &mask = cv::Mat());

	// カラーパターン画像を格納した配列
	std::vector<cv::Mat> colorImages;

private:

	typedef enum color{
		B, G, R, C, M, Y, W, O, PB, PG, PR
	};

	struct LumiParameter{
		cv::Vec3f Gamma;
		cv::Vec3f Gamma_Revers;
		cv::Mat W_image;	// 最大輝度画像(全体)
		cv::Mat B_image;	// 最小輝度画像(全体)
		cv::Vec3f Cmax;		// 最大輝度値(中央pixel)(正規化)
		cv::Vec3f Cmin;		// 最小輝度値(中央pixel)(正規化)
		cv::Matx33f CMM;	// 色混合行列用の変数
		cv::Matx33f CMMinv;	// 色混合行列用の変数
	};
	LumiParameter *LP;


	// カメラスレッド
	PGRCamera *pgrCamera;

	// 時間計測用関数

	/// 色変換行列使う
	bool cmmFlag;

	void getAllGamma(cv::Mat &camImage_r, cv::Mat &camImage_g, cv::Mat &camImage_b);
};
