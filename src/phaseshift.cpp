#include "phaseshift.h"
#include "TextureObjectStocker.h"
#include <memory>

using namespace glUtl;

// �f�B���N�g���̍쐬
void PhaseShift::psCreateDirs()
{
	_mkdir(ps_path);
	gcCreateDirs();	// �t�H���_�������������錴�����Ǝv��
	_mkdir(ps_base_dir.c_str());
	_mkdir(ps_image_dir.c_str());
	_mkdir(std::string(ps_image_dir + "/ProjectionImages").c_str());	// �����g���e�摜
	_mkdir(std::string(ps_image_dir + "/CaptureImages").c_str());	// �����g�B�e�摜
}

void PhaseShift::init()
{
	// �f�B���N�g���̍쐬
	psCreateDirs();
	makeGraycodeImage();
	makeSineWaveImage();

	init_flag = true;
}

// �����g�摜�쐬
void PhaseShift::makeSineWaveImage()
{
	cv::Mat x_img(projHeight, projWidth, CV_8UC1);
	cv::Mat y_img(projHeight, projWidth, CV_8UC1);
	for (int i = 0; i < Proj_num; i++) {
		for (unsigned int x = 0; x < projWidth; x++){
			for (unsigned int y = 0; y < projHeight; y++){
				x_img.at<uchar>(y, x) = (uchar)(Amp * sin(2.0*PI*((double)i / Proj_num + (double)(x + xplus) / WaveLength_X)) + Bias);
				y_img.at<uchar>(y, x) = (uchar)(Amp * sin(2.0*PI*((double)i / Proj_num + (double)(y + yplus) / WaveLength_Y)) + Bias);
			}
		}
		// �A�ԂŃt�@�C������ۑ��i������X�g���[���j
		cv::imwrite(psGetProjctionFilePath(i, VERTICAL).str(), x_img);
		// �A�ԂŃt�@�C������ۑ��i������X�g���[���j
		cv::imwrite(psGetProjctionFilePath(i, HORIZONTAL).str(), y_img);
	}
	x_img.release();
	y_img.release();
}


// opencv�𗘗p���ĉf�����e
void PhaseShift::code_projection_opencv()
{
	// �萔
	typedef enum flag{
		POSI = true,
		NEGA = false,
		VERTICAL = true,
		HORIZONTAL = false,
	} flag;

	TPGROpenCV	*pgrOpenCV = new TPGROpenCV;
	//�����ݒ�
	pgrOpenCV->init(FlyCapture2::PIXEL_FORMAT_BGR, FlyCapture2::HQ_LINEAR);
	pgrOpenCV->setShutterSpeed(pgrOpenCV->getShutter_measure());
	pgrOpenCV->start();
	int delay = (int)(2.5 * pgrOpenCV->getDelay());

	char *CODE_IMG = "GC";
	// �S��ʕ\���p�E�B���h�E�̍쐬  
	cv::namedWindow(CODE_IMG, 0);

	HWND windowHandle = ::FindWindowA(NULL, CODE_IMG);
	SetWindowLong(windowHandle, GWL_STYLE, WS_POPUP);
	SetWindowPos(windowHandle, HWND_TOP,
		1680, 0,
		projWidth, projHeight, SWP_SHOWWINDOW);
	cv::imshow(CODE_IMG, cv::Mat(projHeight, projWidth, CV_8UC3, cv::Scalar::all(255)));
	cv::waitKey(delay);

	/******* GrayCode�Ǎ� *********/

	cv::Mat *posi_img = new cv::Mat[gcGetBitNum_all()];  // �|�W�p�^�[���p
	cv::Mat *nega_img = new cv::Mat[gcGetBitNum_all()];  // �l�K�p�^�[���p

	// �A�ԂŃt�@�C������ǂݍ��ށi������X�g���[���j
	std::cout << "���e�p�O���C�R�[�h�摜�ǂݍ��ݒ�" << std::endl;
	for (unsigned int i = 0; i < gcGetBitNum_all(); i++) {
		// �ǂݍ���
		posi_img[i] = cv::imread(gcGetProjctionFilePath(i, POSI).str(), 1);
		nega_img[i] = cv::imread(gcGetProjctionFilePath(i, NEGA).str(), 1);
		// �ǂݍ��ޖ���������Ȃ�������O���C�R�[�h�摜����蒼��
		if (posi_img[i].empty() || nega_img[i].empty()){
			std::cout << "ERROR(1)�F���e�p�̃O���C�R�[�h�摜���s�����Ă��܂��B" << std::endl;
			std::cout << "ERROR(2)�F�O���C�R�[�h�摜���쐬���܂��B" << std::endl;
			makeGraycodeImage();
			code_projection_opencv();
			return;
		}
	}

	/******* sin�g�Ǎ� *********/

	cv::Mat *phase_x_img = new cv::Mat[Proj_num];
	cv::Mat *phase_y_img = new cv::Mat[Proj_num];
	// sine �摜�̓ǂݍ���
	for (unsigned int i = 0; i < psGetProjectionNum(); ++i) {
		phase_x_img[i] = cv::imread(psGetProjctionFilePath(i, VERTICAL).str(), 1);
		phase_y_img[i] = cv::imread(psGetProjctionFilePath(i, HORIZONTAL).str(), 1);
	}

	/***** �O���C�R�[�h���e & �B�e *****/
	cv::Mat cap;
	// �|�W�p�^�[�����e & �B�e
	std::cout << "�|�W�p�^�[���B�e��" << std::endl;
	for (unsigned int i = 0; i < gcGetBitNum_all(); i++) {
		// ���e
		cv::imshow(CODE_IMG, posi_img[i]);
		// �J�����B�e
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);

		// �|�W�p�^�[���B�e���ʂ�ۑ�
		// ����
		if (i < gcGetBitNum_h())
			cv::imwrite(gcGetCaptureFilePath(i, HORIZONTAL, POSI).str(), cap);
		// �c��
		else
			cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_h(), VERTICAL, POSI).str(), cap);
	}

	cv::waitKey(delay);
	// �l�K�p�^�[�����e & �B�e
	std::cout << "�l�K�p�^�[���B�e��" << std::endl;
	for (unsigned int i = 0; i < gcGetBitNum_all(); i++) {
		// ���e
		cv::imshow(CODE_IMG, nega_img[i]);

		// �J�����B�e
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);

		// �|�W�p�^�[���B�e���ʂ�ێ�
		// ����
		if (i < gcGetBitNum_h())
			cv::imwrite(gcGetCaptureFilePath(i, HORIZONTAL, NEGA).str(), cap);
		// �c��
		else
			cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_h(), VERTICAL, NEGA).str(), cap);
	}

	/***** �����g ���e & �B�e *****/

	std::cout << "�����g�p�^�[���B�e��" << std::endl;
	//�@�c��
	for (int i = 0; i < Proj_num; i++) {
		// ���e
		cv::imshow(CODE_IMG, phase_x_img[i]);
		// �J�����B�e
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);
		// �p�^�[���B�e���ʂ�ۑ�
		cv::imwrite(psGetCaptureFilePath(i, VERTICAL).str(), cap);
	}

	//�@����
	for (int i = 0; i < Proj_num; i++) {
		// ���e
		cv::imshow(CODE_IMG, phase_y_img[i]);
		// �J�����B�e
		cv::waitKey(delay);
		pgrOpenCV->CameraCapture(cap);
		// �p�^�[���B�e���ʂ�ۑ�
		cv::imwrite(psGetCaptureFilePath(i, HORIZONTAL).str(), cap);
	}
	std::cout << "�����g�p�^�[���B�e�I��" << std::endl;

	/***** ���e & �B�e�I�� *****/

	pgrOpenCV->stop();
	//pgrOpenCV->release();

	/***** �I�� *****/

	cv::destroyWindow(CODE_IMG);
	delete[] posi_img;
	delete[] nega_img;
	delete[] phase_x_img;
	delete[] phase_y_img;
	delete pgrOpenCV;
	pgrOpenCV = NULL;
	posi_img = NULL;
	nega_img = NULL;
	phase_x_img = NULL;
	phase_y_img = NULL;
}

// �e�N�X�`����ǂݍ��ފ֐�
bool PhaseShift::readTexture()
{

	//// graycode�摜�̓ǂݍ���
	//for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, POSI).str()));
	//}
	//// graycode�摜�̓ǂݍ���
	//for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, NEGA).str()));
	//}
	//// sine �摜�̓ǂݍ���
	//for (unsigned int i = 0; i < psGetProjectionNum(); ++i)
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, VERTICAL).str()));
	//// sine �摜�̓ǂݍ���
	//for (unsigned int i = 0; i < psGetProjectionNum(); ++i)
	//	code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, HORIZONTAL).str()));

	//// �e�N�X�`����
	//for (unsigned int i = 0; i < code_texture.size(); ++i) {
	//	code_texture[i].load();
	//}

	// GPU�ւ̓]���x���҂�
	Sleep(3 * 44);

	return true;
}
bool PhaseShift::code_projection_opengl(GLFWwindow *window)
{
	if (!init_flag){
		std::cout << "init ���Ă΂�Ă��܂���" << std::endl;
		return false;
	}

	// �V�F�[�_�̐ݒ�
	if (!GrayCode::glsl->init("./shader/texture.vert", "./shader/texture.frag")){
		std::cout << "shader�t�@�C�����ǂݍ��߂܂���" << std::endl;
		return false;
	}
	// �V�F�[�_�̗L��
	glsl->setEnabled(true);

	// uniform�ϐ��ɒl��n��
	glUniform1i(getShlWithErrorDetect(glsl->getHandle(), "projection_texture"), 0);	//�e�N�X�`�����j�b�g0

	// �E�C���h�E�T�C�Y�̃|���S����ݒ�
	std::unique_ptr<Vertex> vert(new Vertex(projWidth, projHeight));
	//Vertex *vert = new Vertex(projWidth, projHeight);
	vert->init();

	// �e�N�X�`����ێ�����ϐ�
	std::vector<Texture> code_texture;
	// graycode�摜�̓ǂݍ���
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, POSI).str()));
	}
	// graycode�摜�̓ǂݍ���
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, NEGA).str()));
	}
	// sine �摜�̓ǂݍ���
	for (unsigned int i = 0; i < psGetProjectionNum(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, VERTICAL).str()));
	}
	// sine �摜�̓ǂݍ���
	for (unsigned int i = 0; i < psGetProjectionNum(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, psGetProjctionFilePath(i, HORIZONTAL).str()));
	}

	// �e�N�X�`����
	for (unsigned int i = 0; i < code_texture.size(); ++i) {
		code_texture[i].load();
	}

	//// �摜�̓ǂݍ���
	//if (!readTexture())
	//	return false;

	unsigned int proj_num = 2;
	unsigned int proj_count = 0;
	unsigned int count = 0;
	pgrCamera->resize(100);
	// ���e
	glfwSwapInterval(1);
	while (count < code_texture.size())
	{
		for (int i = 0; i < proj_num; ++i){
			// �g�p����摜���o�C���h
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, code_texture[count].m_textureObj);
			// �`��
			vert->render_mesh();
			// �J���[�o�b�t�@�����ւ�,�C�x���g���擾

			glfwSwapBuffers(window);
			proj_count++;
		}
		count++;
	}
	glfwSwapInterval(0);
	// �x�����ҋ@
	Sleep(90);

	// �o�b�t�@��̉摜�����ׂēǂݍ���
	std::vector<cv::Mat> buff;
	pgrCamera->getAllImages(buff);

	// �z���C�g�̔w�i
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

	//// (debug)�o�b�t�@�̉摜�����ׂĕۑ�
	//for (int i = 0; i < buff.size(); ++i) {
	//	if (!buff[i].empty()){
	//		char buf[256];
	//		sprintf_s(buf, "./Calibration/pgr/cap%d.jpg", i);
	//		cv::imwrite(buf, buff[i]);
	//	}
	//}

	// ���[���猩�Ă����ĐF���傫���ω������炻�������e�f�����B��n�߂Ă���ꏊ
	cv::Vec3b check, before_color;
	unsigned int firstCaptureIndex = 0;
	for (int i = 0; i < buff.size(); ++i) {
		int lastIndex = buff.size() - 1;
		cv::Point center = cv::Point(camWidth / 2, camHeight / 2);
		cv::Mat image = buff[lastIndex - i].clone();
		if (i == 0) {
			before_color = image.at<cv::Vec3b>(center.y, center.x);
			continue;
		}
		check = image.at<cv::Vec3b>(center.y, center.x);
		cv::Point3i diff = cv::Point3i(abs(before_color[0] - check[0]), abs(before_color[1] - check[1]), abs(before_color[2] - check[2]));
		std::cout << diff << std::endl;
		if (diff.x > 30 || diff.y > 30 || diff.z > 30){
			firstCaptureIndex = lastIndex - i;
			break;
		}
		else {
			before_color = check;
		}
	}

	//std::cout << firstCaptureIndex << std::endl;

	// �摜�o�b�t�@����R�[�h���e�����̂ݒ��o����
	for (unsigned int i = 0; i < code_texture.size(); ++i) {
		// �摜�̎Q�Əꏊ
		unsigned int index = (firstCaptureIndex - 1) - (i * proj_num);
		//// debug
		//char buf[256];
		//sprintf_s(buf, "./Calibration/pgr/cap%d.jpg", i);
		//cv::imwrite(buf, buff[index]);

		// �摜��ۑ�
		// posi
		if (i < gcGetBitNum_all()) {
			if (i < gcGetBitNum_h())
				cv::imwrite(gcGetCaptureFilePath(i, HORIZONTAL, POSI).str(), buff[index]);
			else
				cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_h(), VERTICAL, POSI).str(), buff[index]);
		}
		// nega
		else if (i < gcGetBitNum_all() * 2){
			if (i - gcGetBitNum_all() < gcGetBitNum_h())
				cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_all(), HORIZONTAL, NEGA).str(), buff[index]);
			else
				cv::imwrite(gcGetCaptureFilePath(i - gcGetBitNum_all() - gcGetBitNum_h(), VERTICAL, NEGA).str(), buff[index]);
		}
		// sine
		else if (i < gcGetBitNum_all() * 2 + psGetProjectionNum()) {
			// �c��
			cv::imwrite(psGetCaptureFilePath(i - gcGetBitNum_all() * 2, VERTICAL).str(), buff[index]);
		}
		// sine
		else {
			// ����
			cv::imwrite(psGetCaptureFilePath(i - gcGetBitNum_all() * 2 - psGetProjectionNum(), HORIZONTAL).str(), buff[index]);
		}

	}

	// ���e�e�N�X�`�������ׂč폜
	code_texture.clear();
	code_texture.shrink_to_fit();

	// �V�F�[�_�̖���
	GrayCode::glsl->setEnabled(false);

	// PGR�J�����X���b�h�̃o�b�t�@�T�C�Y��ύX
	pgrCamera->resize(30);

	return true;
}

std::stringstream PhaseShift::psGetProjctionFilePath(unsigned int num, bool vertical)
{
	std::stringstream filename;
	//if (num >= Proj_num) {
	//	std::cerr << num << " ���ڂ̐����g�摜�͂���܂���" << std::endl;
	//	return filename;
	//}
	// �c��
	if (vertical)
		filename << ps_image_dir << "/ProjectionImages/x_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";
	// ����
	else
		filename << ps_image_dir << "/ProjectionImages/y_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";

	return filename;
}

std::stringstream PhaseShift::psGetCaptureFilePath(unsigned int num, bool vertical)
{
	std::stringstream filename;
	// �c��
	if (vertical)
		filename << ps_image_dir << "/CaptureImages/x_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";
	// ����
	else
		filename << ps_image_dir << "/CaptureImages/y_pattern" << std::setw(2) << std::setfill('0') << num << ".bmp";
	return filename;

}

/***********************************
** �v���W�F�N�^�ƃJ�����̑Ή��t�� **
************************************/

void PhaseShift::getCorrespondence(bool isCodeProjection)
{
	std::cout << "�􉽑Ή��擾�J�n" << std::endl;
	mask = cv::imread(gc_mask_address, 0);// �}�X�N�摜�̓ǂݍ���
	if (!init_flag)
		init();

	if (isCodeProjection) {
		//��������
		code_restore();
		// �f�[�^��ۑ�
		writePixCorrespData();
	}
	else {
		// �f�[�^�̓ǂݏo��
		readPixCorrespData();
		// debug�p�ɗ��U���������}�b�v���ꉞ�쐬����
		calcIntCoordMap();
	}

	// (debug�p)�T�u�s�N�Z�����x�̑Ή��𐮐��l�Ɋۂ߂āC�Ή��}�b�v���擾
	calcNearestNeighbor();

	// opengl�̃��b�V����p�����􉽕ϊ��p��VAO���쐬����
	cv::Mat vertCoordMap, texCoordMap;
	// �J�������W�n���v���W�F�N�^���W�n�̊􉽕ϊ��pVAO
	//getGLCoordMap(vertCoordMap, texCoordMap, true);
	getBufferArrays(vertCoordMap, texCoordMap, coord_trans);
	ready = true;
}


// ��������
void PhaseShift::code_restore()
{
	std::cout << "�ʑ������J�n " << std::endl;

	// �ʑ�����
	restore_phase_value();

	// �A������
	phaseConnection();

	// �Ή��G���[���C������
	errorCheck();

	std::cout << "�ʑ������I�� : ";
}

/* �ʑ����� */
void PhaseShift::restore_phase_value()
{

	cv::Point2d lumi[Proj_num];
	cv::Mat *phase_x_img = new cv::Mat[Proj_num];
	cv::Mat *phase_y_img = new cv::Mat[Proj_num];

	cv::Mat xphase_img = cv::Mat(camHeight, camWidth, CV_32F);
	cv::Mat yphase_img = cv::Mat(camHeight, camWidth, CV_32F);

	// �摜�̓ǂݍ���
	for (int i = 0; i < Proj_num; ++i)
	{
		phase_x_img[i] = cv::imread(psGetCaptureFilePath(i, VERTICAL).str(), 0);
		phase_y_img[i] = cv::imread(psGetCaptureFilePath(i, HORIZONTAL).str(), 0);
	}
	int height = (*phase_x_img).rows;
	int width = (*phase_x_img).cols;

	//��������
	//�iDebug�p�j�摜
	cv::Mat phasemask((*phase_x_img).rows, (*phase_x_img).cols, CV_8UC1, cv::Scalar::all(0));

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){

			//�}�X�N�O�͉������Ȃ�
			if (mask.ptr<uchar>(y)[x] == 0){
				continue;
			}

			cv::Point2d cosValue = cv::Point2d(0.0, 0.0);
			cv::Point2d sinValue = cv::Point2d(0.0, 0.0);

			//���q�ɂ���
			for (int i = 0; i < Proj_num; i++){
				lumi[i] = cv::Point2d((double)phase_x_img[i].at<uchar>(y, x), (double)phase_y_img[i].at<uchar>(y, x));
				lumi[i] *= cos(2.0*PI*i / Proj_num);
				cosValue += lumi[i];
			}

			//����ɂ���
			for (int i = 0; i < Proj_num; i++){
				lumi[i] = cv::Point2d((double)phase_x_img[i].at<uchar>(y, x), (double)phase_y_img[i].at<uchar>(y, x));
				lumi[i] *= sin(2.0*PI*i / Proj_num);
				sinValue += lumi[i];
			}

			//0���h�~����
			if (((0 <= abs(sinValue.x)) && (abs(sinValue.x) <= 1)) && cosValue.x > 0)
				sinValue.x = 1;
			if (((0 <= abs(sinValue.x)) && (abs(sinValue.x) <= 1)) && cosValue.x < 0)
				sinValue.x = -1;
			if (((0 <= abs(sinValue.y)) && (abs(sinValue.y) <= 1)) && cosValue.x > 0)
				sinValue.y = 1;
			if (((0 <= abs(sinValue.y)) && (abs(sinValue.y) <= 1)) && cosValue.x < 0)
				sinValue.y = -1;

			//�ʑ�����
			cv::Point2d phase_value = cv::Point2d(atan2(cosValue.x, sinValue.x), atan2(cosValue.y, sinValue.y));

			//�i�[
			phaseValue.ptr<cv::Vec2d>(y)[x] = cv::Vec2d(phase_value.x, phase_value.y);
			//�ʑ���񂩂�}�X�N���쐬����
			phasemask.at<uchar>(y, x) = 255;
			if (fabs(cosValue.x) < ps_thresh &&  fabs(cosValue.y) < ps_thresh){
				if (fabs(sinValue.x) < ps_thresh &&  fabs(sinValue.y) < ps_thresh)
					phasemask.at<uchar>(y, x) = 255;
			}

			// (Debug�p) ���K�����邽�߂ɒl�� 0< value <2�� �ɂ���
			xphase_img.at<float>(y, x) = (float)(phase_value.x + (PI));
			yphase_img.at<float>(y, x) = (float)(phase_value.y + (PI));


		}
	}

	/* �}�X�N�摜�����菬��������*/
	for (int i = 0; i < 2; i++)
		smallMaskRange(phasemask, phasemask);

	// (Debug�p)���n��̉摜���쐬&�ۑ�
	cv::Mat temp;
	xphase_img.convertTo(temp, CV_8U, 255 / (2.0*PI));
	cv::imwrite(std::string(ps_base_dir + "/phase_x.bmp").c_str(), temp);
	yphase_img.convertTo(temp, CV_8U, 255 / (2.0*PI));
	cv::imwrite(std::string(ps_base_dir + "/phase_y.bmp").c_str(), temp);

	// �}�X�N�摜��ύX
	cv::imwrite(std::string(ps_base_dir + "/mask.bmp").c_str(), phasemask);
	mask = phasemask.clone();

	delete[] phase_x_img;
	delete[] phase_y_img;
	temp.release();

}


/* �A������ */
void PhaseShift::phaseConnection()
{
	// �B�e�摜���f�R�[�h
	GrayCode::decodeGrayCode();
	std::map<int, cv::Point> *gray_code_map = new std::map<int, cv::Point>;
	cv::Mat graycode = GrayCode::gcGetGrayCode();

	//�R�[�h�}�b�v���쐬( Key:�O���C�R�[�h�Cvalue:�g�̍��W�f�[�^(�������ڂ�) )
	for (unsigned int y = 0; y < waveNumY; y++) {
		for (unsigned int x = 0; x < waveNumX; x++) {
			int a = graycode.ptr<int>(y)[x];
			if (a != 0)
				(*gray_code_map)[a] = cv::Point(x, y);
		}
	}

	// 0�Ԗڂ͎g��Ȃ�
	(*gray_code_map)[0] = cv::Point(-1, -1);

	//�A������(�ʑ��l��A���ɂ��鏈��)���s��
	for (unsigned int y = 0; y < camHeight; y++){

		// �|�C���^���擾
		cv::Vec2d *phaseValue_p = phaseValue.ptr<cv::Vec2d>(y);
		cv::Vec2f *coordmap_p = coord_map.ptr<cv::Vec2f>(y);
		cv::Vec2i *coordmap_int_p = coord_map_int.ptr<cv::Vec2i>(y);

		for (unsigned int x = 0; x < camWidth; x++){

			//�}�X�N�O�͉������Ȃ�
			if (mask.ptr<uchar>(y)[x] == 0){
				continue;
			}

			int a = GrayCode::gcGetDecodeMap().ptr<int>(y)[x];

			//�R�[�h��������Ȃ��ꍇ�G���[����
			if ((*gray_code_map).find(a) != (*gray_code_map).end())
			{
				// �g�̍��W(+1)���i�[
				int nx = ((*gray_code_map)[a]).x + 1;
				int ny = ((*gray_code_map)[a]).y + 1;

				// 2n�Ή��Z
				double phase_x = phaseValue_p[x][0] + (nx*2.0*PI);
				double phase_y = phaseValue_p[x][1] + (ny*2.0*PI);
				// �ʑ��l���X�V
				phaseValue_p[x] = cv::Vec2d(phase_x, phase_y);

				//�Ή�����v���W�F�N�^���W(�T�u�s�N�Z�����x)���Z�o
				float point_x = (float)((phase_x * WaveLength_X) / (2.0 * PI) - (double)xplus);
				float point_y = (float)((phase_y * WaveLength_Y) / (2.0 * PI) - (double)yplus);

				// �R�[�h�}�b�v�ɕێ�
				coordmap_p[x] = cv::Vec2f(point_x, point_y);

				// �Ή�������W���ߖT�̉�f�Ƃ���(Nearest Neighbor�⊮)
				coordmap_int_p[x] = cv::Vec2i((int)(point_x + 0.5f), (int)(point_y + 0.5f));

			}
			else
			{
				coordmap_p[x] = cv::Vec2f(-1.0f, -1.0f);
				coordmap_int_p[x] = cv::Vec2i(-1, -1);
			}
		}
	}
	delete gray_code_map;
	gray_code_map = NULL;
}

// �ʑ��̘A���G���[���s���֐�
void PhaseShift::errorCheck()
{

	// �ʑ��摜���쐬
	cv::Mat xphase_img(camHeight, camWidth, CV_64F, cv::Scalar::all(0));
	cv::Mat yphase_img(camHeight, camWidth, CV_64F, cv::Scalar::all(0));
	int error;
	for (unsigned int y = 1; y < camHeight; y++){
		cv::Vec2i *int_cm_p = coord_map_int.ptr<cv::Vec2i>(y);
		cv::Vec2i *int_cm_p_up = coord_map_int.ptr<cv::Vec2i>(y - 1);

		cv::Vec2f *cm_p = coord_map.ptr<cv::Vec2f>(y);
		cv::Vec2f *cm_p_up = coord_map.ptr<cv::Vec2f>(y - 1);

		cv::Vec2d *phase_p = phaseValue.ptr<cv::Vec2d>(y);
		for (unsigned int x = 1; x < camWidth; x++){

			//�}�X�N�E�G���[�l�̏ꍇ�������Ȃ�
			if (mask.ptr<uchar>(y)[x] == 0)
				continue;

			// ���ډ�f���G���[�l�������ꍇ�������Ȃ�
			if (int_cm_p[x] == cv::Vec2i(-1, -1))
				continue;

			/****************/
			/*  x �ɂ���	*/
			/****************/
			// ���ډ�f�̈�O�̉�f���G���[�l�������ꍇ�������Ȃ�
			if (int_cm_p[x - 1] == cv::Vec2i(-1, -1))
				continue;

			int point = int_cm_p[x][0];
			int point_up = int_cm_p[x - 1][0];
			error = point - point_up;

			// ���W�̐��ڂ����j�A�ł͂Ȃ��ꍇ(error < 0),���ډ�f�̈ʑ��l��2�Α����č��W���Z�o���C
			// ���̂Ƃ��̍��W�l���Ó��Ȃ��̂��𔻒f����
			if (error < 0){
				double value = phase_p[x][0] + 2.0 * PI;
				double newPoint = (value * WaveLength_X) / (2.0 * PI) - (double)xplus;
				int p = (int)(newPoint + 0.5);
				// �ēx1�O�̍��W�l�Ƃ̌덷�ʂ��v�Z
				error = p - point_up;

				// �C����̍��W�����j�A�ɐ��ڂ��Ă����ꍇ
				if (-1 <= error){
					//-�O���C�R�[�h�̋��E�덷
					if (error < 5){
						// ���������W�l������
						cm_p[x][0] = (float)newPoint;
						int_cm_p[x][0] = p;
						phase_p[x][0] = value;
					}
					//-�ʑ��̌덷
					else{
						//c->p.pointdata[y][x].x = c->p.pointdata[y][x-1].x;
					}
				}
			}


			/****************/
			/*  y �ɂ���	*/
			/****************/
			// ���ډ�f�̈��̉�f���G���[�l�������ꍇ�������Ȃ�
			if (int_cm_p_up[x] == cv::Vec2i(-1, -1))
				continue;

			point = int_cm_p[x][1];
			point_up = int_cm_p_up[x][1];
			error = point - point_up;
			//-���W�̐��ڂ����j�A�ł͂Ȃ��ꍇ
			if (error < 0){
				double value = phase_p[x][1] + 2.0 * PI;
				double newPoint = (value * WaveLength_Y) / (2 * PI) - (double)yplus;
				int p = (int)(newPoint + 0.5);
				error = p - point_up;
				if (0 <= error){
					//-�O���C�R�[�h�̋��E�덷
					if (error < 5){
						cm_p[x][1] = (float)newPoint;
						int_cm_p[x][1] = p;
						phase_p[x][1] = value;
					}
					//-�ʑ��̌덷
					else{
						//c->p.pointdata[y][x].y = c->p.pointdata[y-1][x].y;
					}
				}
			}

			// �v���W�F�N�^���W�͈̔͂𒴂����ꍇ�̃G���[����
			if (cm_p[x][0] < 0.0f || cm_p[x][0]  > projWidth - 1.0f) {
				cm_p[x][0] = -1.0f;
				int_cm_p[x][0] = -1;

			}
			if (cm_p[x][1] < 0.0f || cm_p[x][1] > projHeight - 1.0f) {
				cm_p[x][1] = -1.0f;
				int_cm_p[x][1] = -1;
			}

			// (Debug�p)
			xphase_img.at<double>(y, x) = phase_p[x][0];
			yphase_img.at<double>(y, x) = phase_p[x][1];

		}
	}

	// �i�m�F�p�j�摜�Ŋm�F
	double nx = (int)(2.0*PI*(projWidth / WaveLength_X) + PI - 2.0*PI * 1 * (0 + xplus) / WaveLength_X);	//x�̈ʑ��l�̎�蓾��͈�(max - min)
	double ny = (int)(2.0*PI*(projHeight / WaveLength_Y) + PI - 2.0*PI * 1 * (0 + yplus) / WaveLength_Y);	//y�̈ʑ��l�̎�蓾��͈�(max - min)
	xphase_img.convertTo(xphase_img, CV_8U, 255 / nx);
	yphase_img.convertTo(yphase_img, CV_8U, 255 / ny);
	cv::imwrite(std::string(ps_base_dir + "/phase_restore_x.bmp").c_str(), xphase_img);
	cv::imwrite(std::string(ps_base_dir + "/phase_restore_y.bmp").c_str(), yphase_img);

	//�i�m�F�p�j
	xphase_img.release();
	yphase_img.release();
}

/*
�}�X�N�̈��1pixel����������֐�
���e�̈�̉��͐M�������Ⴂ���߁C���͊􉽑Ή��t�������Ȃ��D
*/
void PhaseShift::smallMaskRange(cv::Mat &src, cv::Mat &dst)
{
	cv::Mat newMask(camHeight, camWidth, CV_8UC1);

	for (unsigned int y = 0; y < camHeight; y++) {
		for (unsigned int x = 0; x < camWidth; x++) {
			if ((x > 0 && src.at<uchar>(y, x - 1) == 0 && src.at<uchar>(y, x) == 255)
				|| (y > 0 && src.at<uchar>(y - 1, x) == 0 && src.at<uchar>(y, x) == 255))
				newMask.at<uchar>(y, x) = 0;
			else if ((x < camWidth - 1 && src.at<uchar>(y, x) == 255 && src.at<uchar>(y, x + 1) == 0)
				|| (y < camHeight - 1 && src.at<uchar>(y, x) == 255 && src.at<uchar>(y + 1, x) == 0))
				newMask.at<uchar>(y, x) = 0;
			else if (src.at<uchar>(y, x) == 0)
				newMask.at<uchar>(y, x) = 0;
			else if (src.at<uchar>(y, x) == 255)
				newMask.at<uchar>(y, x) = 255;
		}
	}

	dst = newMask.clone();
}

void PhaseShift::writePixCorrespData()
{
	std::vector<float> sub_pointdataVector_X;
	std::vector<float> sub_pointdataVector_Y;
	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *float_pointer = coord_map.ptr<cv::Vec2f>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			sub_pointdataVector_X.emplace_back(float_pointer[x][0]);
			sub_pointdataVector_Y.emplace_back(float_pointer[x][1]);
		}
	}
	std::string fileName = COORDMAP_ADDRESS;
	cv::FileStorage fs(fileName, CV_STORAGE_WRITE);
	cv::write(fs, "subPointdata_x", sub_pointdataVector_X);
	cv::write(fs, "subPointdata_y", sub_pointdataVector_Y);

	sub_pointdataVector_X.clear();
	sub_pointdataVector_Y.clear();
}

void PhaseShift::readPixCorrespData()
{
	std::vector<float> sub_pointdataVector_X;
	std::vector<float> sub_pointdataVector_Y;

	std::string fileName = COORDMAP_ADDRESS;
	cv::FileStorage fs(fileName, cv::FileStorage::READ);
	fs["subPointdata_x"] >> sub_pointdataVector_X;
	fs["subPointdata_y"] >> sub_pointdataVector_Y;

	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *float_pointer = coord_map.ptr<cv::Vec2f>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			int key = y * camWidth + x;
			float_pointer[x] = cv::Vec2f(sub_pointdataVector_X[key], sub_pointdataVector_Y[key]);
		}
	}
	sub_pointdataVector_X.clear();
	sub_pointdataVector_Y.clear();

}

void PhaseShift::readPixCorrespData(const std::string &filename, cv::Mat &dst_map)
{
	cv::Mat map = cv::Mat(camHeight, camWidth, CV_32FC2);

	std::vector<float> sub_pointdataVector_X;
	std::vector<float> sub_pointdataVector_Y;

	std::string fileName = "./Data/" + filename;
	cv::FileStorage fs(fileName, cv::FileStorage::READ);
	fs["subPointdata_x"] >> sub_pointdataVector_X;
	fs["subPointdata_y"] >> sub_pointdataVector_Y;

	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *float_pointer = map.ptr<cv::Vec2f>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			int key = y * camWidth + x;
			float_pointer[x] = cv::Vec2f(sub_pointdataVector_X[key], sub_pointdataVector_Y[key]);
		}
	}
	sub_pointdataVector_X.clear();
	sub_pointdataVector_Y.clear();
	sub_pointdataVector_X.shrink_to_fit();
	sub_pointdataVector_Y.shrink_to_fit();

	dst_map = map.clone();
	map.release();

}
// �T�u�s�N�Z�����x�̑Ή��_�𐮐��l�Ɋۂ߂�֐�
void PhaseShift::calcIntCoordMap()
{

	for (unsigned int y = 0; y < camHeight; y++) {
		cv::Vec2f *coord_p = coord_map.ptr<cv::Vec2f>(y);
		cv::Vec2i *coord_int_p = coord_map_int.ptr<cv::Vec2i>(y);
		for (unsigned int x = 0; x < camWidth; x++) {
			if (coord_p[x][0] != -1.0f)
				// �Ή�������W���ߖT�̉�f�Ƃ���(Nearest Neighbor�⊮)
				coord_int_p[x] = cv::Vec2i((int)(coord_p[x][0] + 0.5f), (int)(coord_p[x][1] + 0.5f));
		}
	}
}

//�@��A���X�g�l�C�o�[�⊮�̏ꍇ
void PhaseShift::calcNearestNeighbor()
{

	for (unsigned int y = 0; y < camHeight; y++) {
		for (unsigned int x = 0; x < camWidth; x++) {
			cv::Point prj_point = coord_map_int.ptr<cv::Vec2i>(y)[x];
			if (prj_point.x != -1 && prj_point.y != -1)
				coord_map_proj.ptr<cv::Vec2i>(prj_point.y)[prj_point.x] = cv::Point(x, y);
		}
	}
}

//�@�􉽕ϊ�
void PhaseShift::reshapeCam2Proj(cv::Mat &src, cv::Mat &dst)
{

	if (!ready) {
		std::cout << "�Ή��}�b�v���擾����Ă��܂���" << std::endl;
		return;
	}
	cv::Mat tmp(projHeight, projWidth, src.type());
	//dst = cv::Mat(src.size(), src.type(), cv::Scalar(0, 0, 0)); 

	if (src.channels() == 1){
		for (unsigned int y = 0; y < projHeight; y++) {
			for (unsigned int x = 0; x < projWidth; x++) {
				cv::Point p = coord_map_proj.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<uchar>(y)[x] = src.ptr<uchar>(p.y)[p.x];
				}
			}
		}
	}
	if (src.channels() == 3) {
		for (unsigned int y = 0; y < projHeight; y++) {
			for (unsigned int x = 0; x < projWidth; x++) {
				cv::Point p = coord_map_proj.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<cv::Vec3b>(y)[x] = src.ptr<cv::Vec3b>(p.y)[p.x];
				}
			}
		}
	}
	tmp.copyTo(dst);
	std::cout << "�􉽕␳ : ";
}

// �􉽕ϊ��p�̒��_�z����擾����
void PhaseShift::getBufferArrays(cv::Mat &in_vertex, cv::Mat &in_texture, PS_Vertex *mesh)
{

	// ���_���
	cv::Mat indexMap = cv::Mat(camHeight, camWidth, CV_32S, cv::Scalar::all(-1));
	std::vector<PS_Vertex::MyVertex> vertex;
	std::vector<float> referVec;	// �Q�Ɨp

	// �Ή����Ƃꂽ���ׂẲ�f�� vertex buffer ���擾
	unsigned int count = 0;
	for (unsigned int y = 0; y < camHeight; ++y) {
		int *index_p = indexMap.ptr<int>(y);

		for (unsigned int x = 0; x < camWidth; ++x) {

			// ���������_�Ƃ����Ƃ��̃K������f�̔ԍ����擾
			int cameraIndex = (y * -1 + (camHeight - 1)) * camWidth + x;

			// �Ή��}�b�v����l���擾
			cv::Vec2f projPoint = coord_map.ptr<cv::Vec2f>(y)[x];

			//// �Ή��_���v���ł��Ă����f�̒��_���W�ƃe�N�X�`�����W��vector�ɕێ�����
			//if (projPoint[0] != -1.0f && projPoint[1] != -1.0f){
			//	// ���_�����Z�b�g��vector�ɕێ�
			//	cv::Vec2f camPoint = cv::Vec2f(x, y);
			//	vertex.emplace_back(PS_Vertex::MyVertex(projPoint, camPoint));

			//	// �J������f�Q�Ɨp�̃o�b�t�@���쐬����(openGL�̃e�N�X�`�����W�𒆐S�Ƃ����J������f�̔ԍ����擾)
			//	referVec.emplace_back((float)cameraIndex);
			//	// �i�[������f�ɔԍ�������U��(�C���f�b�N�X�o�b�t�@�ŎQ�Ƃ��邽��)
			//	index_p[x] = count;
			//	count++;
			//}

			cv::Vec2f camPoint = cv::Vec2f(x, y);
			vertex.emplace_back(PS_Vertex::MyVertex(projPoint, camPoint));
			// �J������f�Q�Ɨp�̃o�b�t�@���쐬����(op
			referVec.emplace_back((float)cameraIndex);
			// �i�[������f�ɔԍ�������U��(�C���f�b�N�X�o�b�t�@�ŎQ�Ƃ��邽��)
			index_p[x] = count;
			count++;

		}
	}
	// ���b�V���𒣂�ꍇ
	std::vector<PS_Vertex::MyIndex_TRIANGLES> index;
	// �Ή����Ƃꂽ���ׂẲ�f�Ƀ��b�V���𒣂邽�߂́Cindex buffer���擾
	for (unsigned int y = 0; y < camHeight - 1; ++y) {
		for (unsigned int x = 0; x < camWidth - 1; ++x) {

			// ���ډ�f�C��, �E, �E���̉�f���Q�Ƃ��C����ɂƂ�Ă�����C�O�p���b�V���̒��_�ɂ���
			cv::Vec2f pos = coord_map.ptr<cv::Vec2f>(y)[x];
			cv::Vec2f right = coord_map.ptr<cv::Vec2f>(y)[x + 1];
			cv::Vec2f down = coord_map.ptr<cv::Vec2f>(y + 1)[x];
			cv::Vec2f diag = coord_map.ptr<cv::Vec2f>(y + 1)[x + 1];

			// ���ډ�f�����Ă��Ȃ��ꍇ
			if (pos[0] == -1.0f) {
				// ���A�E�A���A�E���̉�f�����Ă���ꍇ
				if (right[0] != -1.0f && down[0] != -1.0f && diag[0] != -1.0f) {
					// ���b�V���𒣂钸�_��f�̔ԍ����Cvector�ɕێ�(�Q��)
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(diag_index, right_index, down_index));
				}
			}
			// ���ډ�f�����Ă���ꍇ
			else {
				// 3�_�Ƃ�Ă���ꍇ
				if (right[0] != -1.0f && down[0] != -1.0f && diag[0] != -1.0f) {
					// ���b�V���𒣂钸�_��f�̔ԍ����Cvector�ɕێ�(�Q��)
					int pos_index = indexMap.ptr<int>(y)[x];
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, down_index, right_index));
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(diag_index, right_index, down_index));
				}
				// �E��f���Ƃ�Ă��Ȃ��ꍇ
				else if (right[0] == -1.0f && down[0] != -1.0f && diag[0] != -1.0f) {
					// ���b�V���𒣂钸�_��f�̔ԍ����Cvector�ɕێ�(�Q��)
					int pos_index = indexMap.ptr<int>(y)[x];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, down_index, diag_index));

				}
				// ����f���Ƃ�Ă��Ȃ��ꍇ
				else if (right[0] != -1.0f && down[0] == -1.0f && diag[0] != -1.0f) {
					// ���b�V���𒣂钸�_��f�̔ԍ����Cvector�ɕێ�(�Q��)
					int pos_index = indexMap.ptr<int>(y)[x];
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, diag_index, right_index));

				}
				// �Ίp��f���������Ă��Ȃ��ꍇ
				else if (right[0] != -1.0f && down[0] != -1.0f && diag[0] == -1.0f) {
					// ���b�V���𒣂钸�_��f�̔ԍ����Cvector�ɕێ�(�Q��)
					int pos_index = indexMap.ptr<int>(y)[x];
					int right_index = indexMap.ptr<int>(y)[x + 1];
					int down_index = indexMap.ptr<int>(y + 1)[x];
					index.emplace_back(PS_Vertex::MyIndex_TRIANGLES(pos_index, down_index, right_index));

				}
			}

		}
	}

	// ���߂����_����p���āC���_�z��Ǘ��N���XPS_Vertex���X�V
	mesh->setBuffers(vertex, index);
	//mesh->setCameraIndex(referVec);
}

void PhaseShift::getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture, bool cam2proj)
{
	dst_vertex = cv::Mat(camHeight, camWidth, CV_32FC2);
	dst_texture = cv::Mat(camHeight, camWidth, CV_32FC2);

	//float texel_x = GrayCode::coord_trans_mesh->getTexel().x;
	//float texel_y = GrayCode::coord_trans_mesh->getTexel().y;
	float texel_x = 0.0f;
	float texel_y = 0.0f;

	if (cam2proj) {

		// �J�������W�n���v���W�F�N�^���W�n�ւ̕ϊ�
		for (unsigned int y = 0; y < camHeight; ++y) {
			for (unsigned int x = 0; x < camWidth; ++x) {

				cv::Point2f point = coord_map.ptr<cv::Vec2f>(y)[x];
				// ���W�f�[�^���G���[�̏ꍇ�C�͈͊O��"-1.1"���i�[
				if (point.x == -1.0f || point.y == -1.0f){
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
				}
				// ���W�f�[�^������l�ŁA���}�X�N�̈���̏ꍇ�C-1 �` 1�Ɏ��܂�悤���K��
				else{
					// ���_���W���擾
					float vx = (point.x / (projWidth - 1.0f)) * 2.0f - 1.0f;
					float vy = -((point.y / (projHeight - 1.0f)) * 2.0f - 1.0f);
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(vx - texel_x, vy - texel_y);
					// �e�N�X�`�����W���擾
					float u = (float)(x) / (camWidth - 1.0f);
					float v = -((float)(y) / (camHeight - 1.0f)) + 1.0f;
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(u, v);
				}
			}
		}
	}
	else {

		// �v���W�F�N�^���W�n���J�������W�n�ւ̕ϊ�
		for (unsigned int y = 0; y < camHeight; ++y) {
			for (unsigned int x = 0; x < camWidth; ++x) {

				cv::Point2f point = coord_map.ptr<cv::Vec2f>(y)[x];
				// ���W�f�[�^������l�ŁA���}�X�N�̈���̏ꍇ�C-1 �` 1�Ɏ��܂�悤���K��
				if (point.x != -1.0f && point.y != -1.0f){
					// ���_���W���擾
					//float vx = (point.x / (projWidth - 1.0f)) * 2.0f - 1.0f;
					//float vy = -((point.y / (projHeight - 1.0f)) * 2.0f - 1.0f);
					float vx = ((float)x / (camWidth - 1.0f)) * 2.0f - 1.0f;
					float vy = -(((float)y / (camHeight - 1.0f)) * 2.0f - 1.0f);
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(vx, vy);
					// �e�N�X�`�����W���擾
					float u = (float)(point.x) / (projWidth - 1.0f);
					float v = -((float)(point.y) / (projHeight - 1.0f)) + 1.0f;
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(u, v);
				}
				// ���W�f�[�^���G���[�̏ꍇ�C�͈͊O��"-1.1"���i�[
				else{
					dst_vertex.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
					dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
				}
			}
		}
	}
}