#include "luminance.h"

void LC::color_projection(GLFWwindow *window)
{
	// ���e����F�̐�
	const unsigned int color_num = PR + 1;
	// ���e�F��ێ�
	cv::Point3f projColor[color_num];

	projColor[B] = cv::Point3f(0.0f, 0.0f, 1.0f);
	projColor[G] = cv::Point3f(0.0f, 1.0f, 0.0f);
	projColor[R] = cv::Point3f(1.0f, 0.0f, 0.0f);
	projColor[C] = cv::Point3f(0.0f, 1.0f, 1.0f);
	projColor[M] = cv::Point3f(1.0f, 0.0f, 1.0f);
	projColor[Y] = cv::Point3f(1.0f, 1.0f, 0.0f);
	projColor[W] = cv::Point3f(1.0f, 1.0f, 1.0f);
	projColor[O] = cv::Point3f(0.0f, 0.0f, 0.0f);
	// �C�ӂ̋P�x�̒P�F�摜
	int p = 128;
	projColor[PB] = cv::Point3f(0.0f, 0.0f, p / 256.0f);
	projColor[PG] = cv::Point3f(0.0f, p / 256.0f, 0.0f);
	projColor[PR] = cv::Point3f(p / 256.0f, 0.0f, 0.0f);

	myUtl::FPSMajor *fps = new myUtl::FPSMajor(1);

	unsigned int color = 0;
	unsigned int proj_count = 0;

	int proj_num = 2;
	std::vector<cv::Mat> buff;
	std::vector<double> times;
	std::vector<double> proj_times;
	pgrCamera->clear();
	glfwSwapInterval(1);
	while (color < color_num)
	{
		// �J���[��ύX
		// �������h�~���邽�߁C���ꂼ��2�񓊉e
		for (int i = 0; i < proj_num; ++i){
			glClearColor(projColor[color].x, projColor[color].y, projColor[color].z, 1.0f);
			// �J���[�o�b�t�@�ƃf�v�X�o�b�t�@����������
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			fps->begin();
			// �J���[�o�b�t�@�����ւ�,�C�x���g���擾
#if SINGLE_BUFFERING
			glFinish();
			Sleep(15);

#else
			// �J���[�o�b�t�@�����ւ���
			glfwSwapBuffers(window);
			//Sleep(15);
#endif
			proj_times.emplace_back(fps->getElapsed());
			proj_count++;
		}
		color++;
	}
	glfwSwapInterval(0);

	Sleep(85);
	// �o�b�t�@��̉摜�����ׂēǂݍ���
	pgrCamera->getAllTime(times);
	pgrCamera->getAllImages(buff);
	std::cout << "image size" << buff.size() << std::endl;
	//std::cout << "time size" << times.size() << std::endl;
	// (debug)�o�b�t�@�̉摜�����ׂĕۑ�
	for (int i = 0; i < buff.size(); ++i) {
		char buf[256];
		sprintf_s(buf, "./Calibration/pgr/cap%d.jpg", i);
		cv::imwrite(buf, buff[i]);
		std::cout << times[i] << std::endl;
	}

	// ���[���猩�Ă����ĐF���傫���ω������炻�������e�f�����B��n�߂Ă���ꏊ
	cv::Vec3b check, before_color;
	unsigned int firstCaptureIndex = 0;
	for (int i = 0; i < buff.size(); ++i) {
		int lastIndex = buff.size() - 1;
		cv::Point center = cv::Point(buff[0].cols / 2, buff[0].rows / 2);
		cv::Mat image = buff[lastIndex - i].clone();
		if (i == 0) {
			before_color = image.at<cv::Vec3b>(center.y, center.x);
			continue;
		}
		check = image.at<cv::Vec3b>(center.y, center.x);
		if (before_color[2] - check[2] > 100){
			firstCaptureIndex = lastIndex - i;
			break;
		}
	}
	//std::cout << firstCaptureIndex << std::endl;

	// �摜�o�b�t�@����F�f�[�^�𒊏o����
	cv::Vec3f value[color_num];
	for (unsigned int i = 0; i < color_num; ++i) {
		// �摜�̎Q�Əꏊ
		unsigned int index = (firstCaptureIndex - 1) - i * proj_num;
		if (index < 0){
			std::cout << "error : �x�����Ԃ�傫�����čČv��" << std::endl;
			std::cin.ignore();
			exit(0);
			break;
		}
		// ����5x5��f�̍��v
		int size = 5;
		cv::Vec3f ave = cv::Vec3f(.0f, .0f, .0f);
		for (int sy = 0; sy < size; sy++)
		for (int sx = 0; sx < size; sx++)
			ave += (cv::Vec3f)buff[index].at<cv::Vec3b>(cv::Point(buff[index].cols / 2 + sx, buff[index].rows / 2 + sy));
		// 5x5�̕���
		ave /= (size*size);
		value[i] += cv::Vec3f(ave);
	}
	// �Ō�̐F�𓊉e�����ꍇ
	// �v�Z�p�̃J���[�e���v���[�g�摜���쐬
	int size = 100;
	cv::Mat forCalc = cv::Mat(size, 11 * size, CV_8UC3);
	for (int i = 0; i < 11; i++)
	for (int sy = 0; sy < size; sy++)
	for (int sx = 0; sx < size; sx++)
	for (int c = 0; c < 3; c++)
		forCalc.at<cv::Vec3b>(sy, i*size + sx)[c] = (int)(value[i][c] + 0.5);
	cv::imwrite(COLOR_CALIBRATION_ADDRESS, forCalc);

	// �z���C�g�̔w�i
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);
}


int LC::setColorMixingMatrix()
{
	// �ۑ�����Ă���J���[�e���v���[�g�摜�̓ǂݍ���
	int size = 100;
	cv::Mat valMat = cv::imread(COLOR_CALIBRATION_ADDRESS, 1);
	if (valMat.empty()) {
		std::cout << "�F�����s��f�[�^���ǂݍ��߂܂���" << std::endl;
		return -1;
	}
	cv::Vec3f value[11];
	for (int i = 0; i < 11; i++){
		value[i] = (cv::Vec3f)valMat.at<cv::Vec3b>(cv::Point(i*size + size / 2, size / 2));
	}

	float calcCMM[9];
	LP->CMM <<
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f;
	//// B
	//calcCMM[0] = 1.0f;
	//calcCMM[1] = (value[W][G] - value[Y][G]) / (value[B][B] - value[O][B]);
	//calcCMM[2] = (value[W][R] - value[Y][R]) / (value[B][B] - value[O][B]);
	//// G
	//calcCMM[3] = (value[W][B] - value[M][B]) / (value[G][G] - value[O][G]);
	//calcCMM[4] = 1.0f;
	//calcCMM[5] = (value[W][R] - value[M][R]) / (value[G][G] - value[O][G]);
	//// R
	//calcCMM[6] = (value[W][B] - value[C][B]) / (value[R][R] - value[O][R]);
	//calcCMM[7] = (value[W][G] - value[C][G]) / (value[R][R] - value[O][R]);
	//calcCMM[8] = 1.0f;
	//LP->CMM <<
	//	calcCMM[0], calcCMM[3], calcCMM[6],
	//	calcCMM[1], calcCMM[4], calcCMM[7],
	//	calcCMM[2], calcCMM[5], calcCMM[8];


	//�t�s��̌v�Z
	LP->CMMinv = LP->CMM.inv();

	std::cout << "�F�ϊ��s��" << std::endl;
	std::cout << LP->CMM << std::endl;

	cv::Matx31f white(value[W][B], value[W][G], value[W][R]);
	cv::Matx31f black(value[O][B], value[O][G], value[O][R]);
	// �����֐���W,B�̌v�Z
	if (cmmFlag) {
		white = LP->CMMinv * white;
		black = LP->CMMinv * black;
	}
	for (int c = 0; c < 3; c++) {
		LP->Cmax[c] = white(c, 0) / 255.0f;
		LP->Cmin[c] = black(c, 0) / 255.0f;
	}
	// Gamma�̌v�Z
	int p = 128;
	float val = (float)p / 255.0f;
	cv::Vec3f Gamma;
	cv::Matx31f cameraColor(value[PB][B] / 255.0f, value[PG][G] / 255.0f, value[PR][R] / 255.0f);
	cv::Matx31f projColor;
	projColor = /*LP->CMMinv * */cameraColor;
	for (int c = 0; c < 3; c++) {
		Gamma[c] = log((projColor(c, 0) - LP->Cmin[c]) / (LP->Cmax[c] - LP->Cmin[c])) / log(val);
	}

	LP->Gamma = Gamma;
	std::cout << "Cmax :" << LP->Cmax << std::endl;
	std::cout << "Cmin :" << LP->Cmin << std::endl;
	std::cout << "gamma estimate :" << Gamma << std::endl;
	std::cout << "gamma :" << LP->Gamma << std::endl;

	return 0;
}

void LC::getAllGamma(cv::Mat &camImage_r, cv::Mat &camImage_g, cv::Mat &camImage_b)
{

}

// �v���W�F�N�^�F��Ԃ̉摜�ɕϊ����邽�߂̊֐�
void LC::getProjectorColorSpaceImage(cv::Mat &camera, cv::Mat &projector)
{

	int width = camera.cols;
	int height = camera.rows;
	cv::Mat camf, prjf, tmp;
	camera.convertTo(camf, CV_32FC3);
	prjf = camf.clone();
	for (int y = 0; y < height; y++){
		cv::Vec3f* camf_p = camf.ptr<cv::Vec3f>(y);
		cv::Vec3f* prjf_p = prjf.ptr<cv::Vec3f>(y);
		for (int x = 0; x < width; x++){
			cv::Matx31f vec(camf_p[x][0], camf_p[x][1], camf_p[x][2]);
			vec = LP->CMMinv * vec;
			for (int i = 0; i < 3; i++)
				prjf_p[x][i] = vec(i, 0);
		}
	}
	camf.convertTo(projector, CV_8UC3);
}


// �摜���m�̍������Ƃ�֐�
void LC::getImageDifference(cv::Mat &image1, cv::Mat &image2, cv::Mat &dst)
{
	int height = image1.rows;
	int width = image1.cols;
	if (image1.channels() == 1) {
		dst = cv::Mat(image1.size(), CV_8UC1);
		for (int y = 0; y < height; y++){

			uchar* image1_p = image1.ptr<uchar>(y);
			uchar* image2_p = image2.ptr<uchar>(y);
			uchar* dst_p = dst.ptr<uchar>(y);

			for (int x = 0; x < width; x++){
				dst_p[x] = (uchar)std::fmax(0.0f, std::fmin(image1_p[x] - image2_p[x], 255.0f));
			}

		}
	}
	else if (image1.channels() == 3){
		dst = cv::Mat(image1.size(), CV_8UC3);
		for (int y = 0; y < height; y++){

			cv::Vec3b* image1_p = image1.ptr<cv::Vec3b>(y);
			cv::Vec3b* image2_p = image2.ptr<cv::Vec3b>(y);
			cv::Vec3b* dst_p = dst.ptr<cv::Vec3b>(y);

			for (int x = 0; x < width; x++){
				for (int c = 0; c < 3; c++){
					dst_p[x][c] = (uchar)std::fmax(0.0f, std::fmin(image1_p[x][c] - image2_p[x][c], 255.0f));
				}
			}

		}
	}
}

// �ω����̌v�Z
void LC::getImageRate(cv::Mat &image1, cv::Mat &image2, cv::Mat &dst)
{
	int height = image1.rows;
	int width = image1.cols;
	dst = cv::Mat(image1.size(), CV_8UC3);
	for (int y = 0; y < height; y++){

		cv::Vec3b* image1_p = image1.ptr<cv::Vec3b>(y);
		cv::Vec3b* image2_p = image2.ptr<cv::Vec3b>(y);
		cv::Vec3b* dst_p = dst.ptr<cv::Vec3b>(y);

		for (int x = 0; x < width; x++){
			for (int c = 0; c < 3; c++){
				float rate = (image2_p[x][c] - image1_p[x][c]) / std::fmax(image1_p[x][c], 1) * 255.0f;
				dst_p[x][c] = (uchar)std::fmax(0.0f, std::fmin(rate, 255.0f));
			}
		}

	}
}

/**
* @brief   MSE�ɂ��摜�]�����s���֐�
*/
cv::Vec3f LC::calcMSE(cv::Mat &image1, cv::Mat &image2, cv::Mat &mask)
{
	cv::Vec3f E_sum = cv::Vec3f(0.0f, 0.0f, 0.0f);
	for (int y = 0; y < image1.rows; y++){
		for (int x = 0; x < image2.cols; x++){

			cv::Vec3b img1 = image1.ptr<cv::Vec3b>(y)[x];
			cv::Vec3b img2 = image2.ptr<cv::Vec3b>(y)[x];
			cv::Vec3f E;
			for (int c = 0; c < 3; c++){
				// ���̓��
				E[c] = powf(float(img1[c] - img2[c]), 2.0);
				// ���a
				E_sum[c] += E[c];
			}

		}
	}
	cv::Vec3f MSE;
	cv::Vec3f PSNR;
	for (int c = 0; c < 3; c++){
		MSE[c] = E_sum[c] / image1.size().area();
		if (MSE[c] != 0)
			PSNR[c] = 10 * log10(255 * 255 / MSE[c]);
		else
			PSNR[c] = 0;
	}

	return PSNR;

}

float LC::calcSSIM(cv::Mat &image1, cv::Mat &image2, cv::Mat &mask)
{
	float average1 = 0, valiance1 = 0, sd1 = 0, conv = 0;
	float average2 = 0, valiance2 = 0, sd2 = 0;
	
	int width = image1.cols;
	int height = image1.rows;
	
	float div = 1.0 / (height * width);
	float _div = 1.0 / (height * width - 1.0);
	int counter = 0;
	// ���ϒl�̌v�Z
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			// �}�X�N�̈�O�͌v�Z���Ȃ�
			if (!mask.empty()){
				cv::Vec3b maskColor = mask.ptr<cv::Vec3b>(y)[x];
				float m_sum = 0.0;
				for (int c = 0; c < mask.channels(); c++){
					m_sum += maskColor[c];
				}
				if (m_sum == 0)
					continue;
			}

			cv::Vec3b img1 = image1.ptr<cv::Vec3b>(y)[x];
			cv::Vec3b img2 = image2.ptr<cv::Vec3b>(y)[x];
			
			float gray1 = (img1[0] + img1[1] + img1[2]) / 3.0;
			average1 += gray1;
			float gray2 = (img2[0] + img2[1] + img2[2]) / 3.0;
			average2 += gray2;

			counter++;
		}
	}
	// �}�X�N�摜����łȂ��ꍇ�́Cdiv�̒l��ύX
	if (!mask.empty()){
		div = 1.0 / (float)counter;
		_div = 1.0 / ((float)counter - 1.0);
	}

	average1 *= div;
	average2 *= div;

	// ���U�Ƌ����U�̌v�Z
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			// �}�X�N�̈�O�͌v�Z���Ȃ�
			if (!mask.empty()){
				cv::Vec3b maskColor = mask.ptr<cv::Vec3b>(y)[x];
				float m_sum = 0.0;
				for (int c = 0; c < mask.channels(); c++){
					m_sum += maskColor[c];
				}
				if (m_sum == 0)
					continue;
			}

			cv::Vec3b img1 = image1.ptr<cv::Vec3b>(y)[x];
			cv::Vec3b img2 = image2.ptr<cv::Vec3b>(y)[x];

			float gray1 = (img1[0] + img1[1] + img1[2]) / 3.0;
			valiance1 += ((average1 - gray1) * (average1 - gray1));
			float gray2 = (img2[0] + img2[1] + img2[2]) / 3.0;
			valiance2 += ((average2 - gray2) * (average2 - gray2));

			conv += (gray1 - average1) * (gray2 - average2);
		}
	}
	valiance1 *= _div;
	valiance2 *= _div;
	conv *= div;

	// �W���΍��̌v�Z
	sd1 = sqrt(valiance1);
	sd2 = sqrt(valiance2);

	// SSIM�̌v�Z
	float C1 = pow((0.01 * 255), 2.0);
	float C2 = pow((0.03 * 255), 2.0);
	float SSIM = ((2 * average1 * average2 + C1) * (2 * conv + C2)) / ((average1 * average1 + average2 * average2 + C1) * (sd1 * sd1 + sd2 * sd2 + C2));

	return SSIM;
}
