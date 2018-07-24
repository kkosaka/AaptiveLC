#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

// glfw��pragma��
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glew��pragma��
#pragma comment(lib,"glew32.lib")

#include <opencv2/opencv.hpp>

#include <direct.h>	// �f�B���N�g���쐬�p

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
		LP->Gamma = cv::Vec3f(1.992f, 2.0715f, 2.1461f);//��Ɍv�Z����̂œK���Ȑ��l������
	};
	~LC(){};

	// �F�����s��̌v�Z
	int setColorMixingMatrix();

	// �F�����s��̌v���p�֐�
	void color_projection(GLFWwindow *window);

	// �p�����[�^���Z�b�g����֐�

	// ���˗��̍X�V
	void getProjectorColorSpaceImage(cv::Mat &camera, cv::Mat &projector);

	// �ڕW�摜���쐬����֐�
	cv::Mat setTargetImage(cv::Mat &src, bool autoBrightness = 0);

	void getImageDifference(cv::Mat &image1, cv::Mat &image2, cv::Mat &dst);
	void getImageRate(cv::Mat &image1, cv::Mat &image2, cv::Mat &dst);

	// �p�����[�^���擾����֐�
	cv::Vec3f getGamma(){ return LP->Gamma; };
	cv::Vec3f getCmaxPixel(){ return LP->Cmax; };
	cv::Vec3f getCminPixel(){ return LP->Cmin; };
	cv::Matx33f getCMM(){ return LP->CMM; };
	cv::Matx33f getCMMinv(){ return LP->CMMinv; };

	// �摜�]���֐�(MSE and PSNR)
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
					// ���̓��̑��a(�S�`�����l��)
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

	// �J���[�p�^�[���摜���i�[�����z��
	std::vector<cv::Mat> colorImages;

private:

	typedef enum color{
		B, G, R, C, M, Y, W, O, PB, PG, PR
	};

	struct LumiParameter{
		cv::Vec3f Gamma;
		cv::Vec3f Gamma_Revers;
		cv::Mat W_image;	// �ő�P�x�摜(�S��)
		cv::Mat B_image;	// �ŏ��P�x�摜(�S��)
		cv::Vec3f Cmax;		// �ő�P�x�l(����pixel)(���K��)
		cv::Vec3f Cmin;		// �ŏ��P�x�l(����pixel)(���K��)
		cv::Matx33f CMM;	// �F�����s��p�̕ϐ�
		cv::Matx33f CMMinv;	// �F�����s��p�̕ϐ�
	};
	LumiParameter *LP;


	// �J�����X���b�h
	PGRCamera *pgrCamera;

	// ���Ԍv���p�֐�

	/// �F�ϊ��s��g��
	bool cmmFlag;

	void getAllGamma(cv::Mat &camImage_r, cv::Mat &camImage_g, cv::Mat &camImage_b);
};
