#include "graycode.h"

// �f�B���N�g���̍쐬
void GrayCode::gcCreateDirs()
{
	// �O���C�R�[�h�p
	_mkdir(path);
	_mkdir(gc_base_dir.c_str());
	_mkdir(std::string(gc_base_dir + "/CaptureImages").c_str());			// �O���C�R�[�h�B�e�摜
	_mkdir(std::string(gc_base_dir + "/ProjectionImages").c_str());		// �O���C�R�[�h���摜
	_mkdir(std::string(gc_base_dir + "/ThresholdImages").c_str());			// �O���C�R�[�h�B�e�摜�̓�l�������摜

}

void GrayCode::init()
{
	gcCreateDirs();
	makeGraycodeImage();
}

// (GrayCode)�p�^�[���R�[�h�摜�쐬
void GrayCode::makeGraycodeImage()
{
	initGraycode();
	cv::Mat posi_img(projHeight, projWidth, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Mat nega_img(projHeight, projWidth, CV_8UC3, cv::Scalar(0, 0, 0));
	int bit = all_bit - 1;

	// �|�W�p�^�[���R�[�h�摜�쐬
	for (unsigned int z = 0; z < all_bit; z++) {
		for (unsigned int y = 0; y < projHeight; y++) {
			for (unsigned int x = 0; x < projWidth; x++) {
				if (((graycode_mat.ptr<int>(y)[x] >> (bit - z)) & 1) == 0) {  // �ŏ�ʃr�b�g���珇�ɒ��o���C���̃r�b�g��0��������
					posi_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(0, 0, 0);
					nega_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(255, 255, 255);
				}
				else if (((graycode_mat.ptr<int>(y)[x] >> (bit - z)) & 1) == 1) {
					posi_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(255, 255, 255);
					nega_img.ptr<cv::Vec3b>(y)[x] = cv::Vec3b(0, 0, 0);
				}
			}
		}
		// �A�ԂŃt�@�C������ۑ��i������X�g���[���j
		cv::imwrite(gcGetProjctionFilePath(z, POSI).str(), posi_img);
		cv::imwrite(gcGetProjctionFilePath(z, NEGA).str(), nega_img);
	}

}

// �g���ɍ��킹���r�b�g���̌v�Z�ƃO���C�R�[�h�̍쐬
void GrayCode::initGraycode()
{
	std::vector<int> bin_code_h;
	std::vector<int> bin_code_w;
	std::vector<int> gray_code_h;
	std::vector<int> gray_code_w;

	/***** 2�i�R�[�h�쐬 *****/
	// �s�ɂ���
	for (unsigned int y = 0; y < waveNumY; y++)
		bin_code_h.emplace_back(y + 1);
	// ��ɂ���
	for (unsigned int x = 0; x < waveNumX; x++)
		bin_code_w.emplace_back(x + 1);

	/***** �O���C�R�[�h�쐬 *****/
	// �s�ɂ���
	for (unsigned int y = 0; y < waveNumY; y++)
		gray_code_h.emplace_back(bin_code_h[y] ^ (bin_code_h[y] >> 1));
	// ��ɂ���
	for (unsigned int x = 0; x < waveNumX; x++)
		gray_code_w.emplace_back(bin_code_w[x] ^ (bin_code_w[x] >> 1));
	// �s������킹��i�s + ��j
	for (unsigned int y = 0; y < waveNumY; y++) {
		for (unsigned int x = 0; x < waveNumX; x++) {
			graycode.ptr<int>(y)[x] = (gray_code_h[y] << w_bit) | gray_code_w[x];
			//���e�T�C�Y�̃O���C�R�[�h�z��쐬
			for (unsigned int i = 0; i < waveLength_y; i++)
			for (unsigned int j = 0; j < waveLength_x; j++)
				graycode_mat.ptr<int>(y*waveLength_y + i)[x*waveLength_x + j] = graycode.ptr<int>(y)[x];
		}
	}
}


// opencv�𗘗p���ĉf�����e(�f����1pixel���Ɖ��ɂ����̂ŁC�P�x�␳�̍ۂ�GL���g������)
void GrayCode::code_projection_opencv()
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

	/***** ���e & �B�e�I�� *****/

	pgrOpenCV->stop();
	//pgrOpenCV->release();

	/***** �I�� *****/

	cv::destroyWindow(CODE_IMG);
	delete[] posi_img;
	delete[] nega_img;
	delete pgrOpenCV;
	pgrOpenCV = NULL;
	posi_img = NULL;
	nega_img = NULL;
}


// �e�N�X�`����ǂݍ��ފ֐�
bool GrayCode::readTexture()
{
	// graycode�摜�̓ǂݍ���
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, POSI).str()));
	}
	// graycode�摜�̓ǂݍ���
	for (unsigned int i = 0; i < gcGetBitNum_all(); ++i){
		code_texture.emplace_back(Texture(GL_TEXTURE_2D, gcGetProjctionFilePath(i, NEGA).str()));
	}

	// �e�N�X�`����
	for (int i = 0; i < code_texture.size(); ++i) {
		code_texture[i].load();
	}

	return true;
}

bool GrayCode::code_projection_opengl(GLFWwindow *window)
{
	// �V�F�[�_�̐ݒ�
	if (!glsl->init("./shader/texture.vert", "./shader/texture.frag"))
		return false;

	glsl->setEnabled(true);
	// uniform�ϐ��ɒl��n��
	glUniform1i(glGetUniformLocation(glsl->getHandle(), "texture"), 0);	//�e�N�X�`�����j�b�g0

	// �E�C���h�E�T�C�Y�̃|���S����ݒ�
	Vertex *vertex = new Vertex(projWidth, projHeight);
	vertex->init();

	// �摜�̓ǂݍ���
	if (!readTexture())
		return false;

	//// PGR�J�����̐ݒ�
	//TPGROpenCV	*pgrOpenCV = new TPGROpenCV;
	//// �J�������X�^�[�g������
	//if (pgrOpenCV->init(FlyCapture2::PIXEL_FORMAT_BGR, FlyCapture2::HQ_LINEAR) == -1)
	//	return false;
	//// �J����������ɓ��삵�Ă����ꍇ
	//pgrOpenCV->setShutterSpeed(pgrOpenCV->getShutter_h());
	//pgrOpenCV->start();	// �J�����X�^�[�g

	cv::Mat cap;
	unsigned int count = 0;
	while (count < code_texture.size())
	{
		// �g�p����摜���o�C���h
		code_texture[count].bind(GL_TEXTURE0);
		// �`��
		vertex->render_mesh();
		// �J���[�o�b�t�@�����ւ�,�C�x���g���擾
		glfwSwapBuffers(window);

		//// �ҋ@
		//Sleep(pgrOpenCV->getDelay() * 2);
		//// �摜��ۑ�
		//pgrOpenCV->CameraCapture(cap);
		// �O���C�R�[�h�̕ۑ�
		// ����
		//if (count < gcGetBitNum_h() * 2){
		//	if (count % 2 == 0)
		//		cv::imwrite(gcGetCaptureFilePath(count / 2, HORIZONTAL, POSI).str(), cap);
		//	else
		//		cv::imwrite(gcGetCaptureFilePath((count - 1) / 2, HORIZONTAL, NEGA).str(), cap);
		//}
		//// �c��
		//else {
		//	int c = count - (gcGetBitNum_h() * 2);
		//	if (count % 2 == 0)
		//		cv::imwrite(gcGetCaptureFilePath(c / 2, VERTICAL, POSI).str(), cap);
		//	else
		//		cv::imwrite(gcGetCaptureFilePath((c - 1) / 2, VERTICAL, NEGA).str(), cap);
		//}
		count++;
		//if (count == code_texture.size()){
		//	finish = true;
		//	code_texture.clear();
		//}
	}

	//pgrOpenCV->stop();
	glsl->setEnabled(false);

	delete vertex;
	vertex = NULL;

	//delete pgrOpenCV;
	//pgrOpenCV = NULL;

	return true;
}

// �B�e�摜��2�l��������C���^�t�F�[�X
void GrayCode::make_thresh()
{
	cv::Mat posi_img;
	cv::Mat nega_img;
	cv::Mat Geometric_thresh_img;  // 2�l�����ꂽ�摜
	cv::Mat mask;

	// �}�X�N�𐶐�
	makeMask(mask);

	std::cout << "��l���J�n" << std::endl;
	// �A�ԂŃt�@�C������ǂݍ���
	// ����
	for (unsigned int i = 0; i < h_bit; i++) {
		// �|�W�p�^�[���ǂݍ���
		posi_img = cv::imread(gcGetCaptureFilePath(i, HORIZONTAL, POSI).str(), 0);
		nega_img = cv::imread(gcGetCaptureFilePath(i, HORIZONTAL, NEGA).str(), 0);

		// 2�l��
		cv::Mat masked_img;
		thresh(posi_img, nega_img, Geometric_thresh_img, 0);
		// �}�X�N��K�p����2�l��
		Geometric_thresh_img.copyTo(masked_img, mask);
		cv::imwrite(gcGetThresholdFilePath(i).str(), masked_img);

		std::cout << i << ", ";
	}
	// �c��
	for (unsigned int i = 0; i < w_bit; i++) {
		posi_img = cv::imread(gcGetCaptureFilePath(i, VERTICAL, POSI).str(), 0);
		nega_img = cv::imread(gcGetCaptureFilePath(i, VERTICAL, NEGA).str(), 0);

		// 2�l��
		cv::Mat masked_img;
		thresh(posi_img, nega_img, Geometric_thresh_img, 0);
		// �}�X�N��K�p����2�l��
		Geometric_thresh_img.copyTo(masked_img, mask);
		cv::imwrite(gcGetThresholdFilePath(i + h_bit).str(), masked_img);

		std::cout << i + h_bit << ", ";
	}
	std::cout << std::endl;

	getMaskForGeometricCorrection();

	std::cout << "��l���I��" << std::endl;
}

void GrayCode::getMaskForGeometricCorrection()
{
	//// �}�X�N�摜�̃A�h���X���Z�b�g
	square->setMask(gc_mask_address);
	// �}�X�N�摜�ɍ��킹�ĉ摜���쐬
	square->adjustSquare(cv::Size(projWidth, projHeight), IMAGE_DIRECTORY, SAVE_DIRECTORY);
}

std::stringstream GrayCode::gcGetProjctionFilePath(unsigned int num, bool posi_nega)
{
	std::stringstream filename;
	if (num > all_bit) {
		std::cerr << num << " ���ڂ̉摜�͂���܂���" << std::endl;
		return filename;
	}
	if (posi_nega == POSI)
		filename << gc_base_dir << "/ProjectionImages/posi" << std::setw(2) << std::setfill('0') << num << ".bmp";
	if (posi_nega == NEGA)
		filename << gc_base_dir << "/ProjectionImages/nega" << std::setw(2) << std::setfill('0') << num << ".bmp";

	return filename;
}
std::stringstream GrayCode::gcGetCaptureFilePath(unsigned int num, bool horizontal, bool pattern)
{
	std::stringstream filename;
	filename << gc_base_dir << "/CaptureImages/CameraImg" << horizontal << "_" << std::setw(2) << std::setfill('0') << num << "_" << pattern << ".bmp";
	return filename;
}
std::stringstream GrayCode::gcGetThresholdFilePath(unsigned int num)
{
	std::stringstream filename;
	filename << gc_base_dir << "/ThresholdImages/Geometric_thresh" << std::setw(2) << std::setfill('0') << num << ".bmp";
	return filename;
}

// �}�X�N���쐬����C���^�t�F�[�X
void GrayCode::makeMask(cv::Mat &mask)
{
	cv::Mat posi_img;
	cv::Mat nega_img;

	// �}�X�N�摜����
	cv::Mat mask_vert, mask_hor;
	static int useImageNumber = 5;
	// y�����̃O���C�R�[�h�摜�ǂݍ���
	posi_img = cv::imread(gcGetCaptureFilePath(useImageNumber, HORIZONTAL, POSI).str(), 0);
	nega_img = cv::imread(gcGetCaptureFilePath(useImageNumber, HORIZONTAL, NEGA).str(), 0);

	// ���̃}�X�N�摜Y����
	makeMaskFromCam(posi_img, nega_img, mask_vert, threshold);

	// x�����̃O���C�R�[�h�摜�ǂݍ���
	posi_img = cv::imread(gcGetCaptureFilePath(useImageNumber, VERTICAL, POSI).str(), 0);
	nega_img = cv::imread(gcGetCaptureFilePath(useImageNumber, VERTICAL, NEGA).str(), 0);

	// ���̃}�X�N�摜X����
	makeMaskFromCam(posi_img, nega_img, mask_hor, threshold);

	// X��Y��OR�����
	// �}�X�N�O�͂ǂ�������Ȃ̂ō�
	// �}�X�N���́i���_�I�ɂ́j�K����������ł�����������Ȃ̂ŁA���ɂȂ�
	// ���ۂ͂��܉��m�C�Y���c���Ă��܂�
	cv::bitwise_or(mask_vert, mask_hor, mask);

	// �c�������܉��m�C�Y�������i���S�}�����S�}���œK�p�����t�ɂȂ�j
	dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);
	erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);

	//for (int i = 0; i < 2; i++)
	//	smallMaskRange(mask, mask);


	cv::imwrite(gc_mask_address, mask);


}

// �O���C�R�[�h�̉摜�𗘗p���ă}�X�N�𐶐�����֐�
// �|�W�ƃl�K�̍����������thresholdValue�ȏ�̋P�x�̃s�N�Z���𔒂ɂ���
void GrayCode::makeMaskFromCam(cv::Mat &posi, cv::Mat &nega, cv::Mat &result, int thresholdValue)
{
	result = cv::Mat(posi.size(), CV_8UC1);

	for (int j = 0; j < result.rows; j++){
		for (int i = 0; i<result.cols; i++){
			int posi_i = posi.at<uchar>(j, i);
			int nega_i = nega.at<uchar>(j, i);

			if (abs(posi_i - nega_i) > thresholdValue){
				result.at<uchar>(j, i) = 255;
			}
			else{
				result.at<uchar>(j, i) = 0;
			}
		}
	}
}

// ���ۂ�2�l������ 
void GrayCode::thresh(cv::Mat &posi, cv::Mat &nega, cv::Mat &thresh_img, int thresh_value)
{
	thresh_img = cv::Mat(posi.rows, posi.cols, CV_8UC1);
	for (int y = 0; y < posi.rows; y++) {
		for (int x = 0; x < posi.cols; x++) {
			int posi_pixel = posi.at<uchar>(y, x);
			int nega_pixel = nega.at<uchar>(y, x);

			// thresh_value���傫�����ǂ����œ�l��
			if (posi_pixel - nega_pixel >= thresh_value)
				thresh_img.at<uchar>(y, x) = 255;
			else
				thresh_img.at<uchar>(y, x) = 0;
		}
	}
}


// 2�l���R�[�h����
void GrayCode::getCorrespondence(bool isCodeProjection)
{
	// �B�e�摜���f�R�[�h
	decodeGrayCode();

	g_code_map->clear();

	int camH = decode.rows;
	int camW = decode.cols;
	// �A�z�z��ŃO���C�R�[�h�̒l�̏ꏊ�ɍ��W���i�[
	for (int y = 0; y < camH; y++) {
		for (int x = 0; x < camW; x++) {
			int a = decode.ptr<int>(y)[x];
			(*g_code_map)[a] = cv::Point(x, y);
		}
	}

	// 0�Ԗڂ͎g��Ȃ�
	(*g_code_map)[0] = cv::Point(-1, -1);

	int graycodeH = graycode.rows;
	int graycodeW = graycode.cols;
	// �f�R�[�h���ꂽ�l�ɑΉ�����v���W�F�N�^���̍��W�l��CamPro�Ɋi�[����
	for (int y = 0; y < graycodeH; y++) {
		for (int x = 0; x < graycodeW; x++) {
			int a = graycode.ptr<int>(y)[x];
			// map���ɑ��݂��Ȃ��R�[�h�i�J�����ŎB�e����肭�����Ȃ����������j�̏ꏊ�ɂ̓G���[�l-1���i�[
			if ((*g_code_map).find(a) == (*g_code_map).end()) {
				coordmap.ptr<cv::Vec2i>(y)[x] = cv::Vec2i(-1, -1);
			}
			// ���݂���ꍇ�́A�v�����ꂽ�O���C�R�[�h�l�ɑΉ�����v���W�F�N�^���W���i�[
			else {
				coordmap.ptr<cv::Vec2i>(y)[x] = cv::Vec2i((*g_code_map)[a]);
			}
		}
	}

	// ���_�z��
	getBufferArrays();

	ready = true;
	std::cout << "finish gray code decoding" << std::endl;

}
void GrayCode::decodeGrayCode()
{

	std::cout << "start gray code decoding" << std::endl;

	int camH = decode.rows;
	int camW = decode.cols;
	// 2�l���R�[�h����
	for (unsigned int i = 0; i < all_bit; i++) {
		cv::Mat a = cv::imread(gcGetThresholdFilePath(i).str(), 0);

		for (int y = 0; y < camH; y++) {
			for (int x = 0; x < camW; x++) {
				if (a.at<uchar>(y, x) == 255)
					decode.ptr<int>(y)[x] = (1 << (all_bit - i - 1)) | decode.ptr<int>(y)[x];
			}
		}
	}

}

// �摜�ό`�E����
// �J�����B�e�̈悩��v���W�F�N�^���e�̈��؂�o��
void GrayCode::reshapeCam2Proj(cv::Mat &src, cv::Mat &dst)
{
	if (!ready) {
		std::cout << "�Ή��}�b�v���擾����Ă��܂���" << std::endl;
		return;
	}
	int height = coordmap.rows;
	int width = coordmap.cols;
	cv::Mat tmp(coordmap.rows, coordmap.cols, src.type());
	if (src.channels() == 1){
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				cv::Point p = coordmap.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<uchar>(y)[x] = src.ptr<uchar>(p.y)[p.x];
				}
			}
		}
	}
	if (src.channels() == 3) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				cv::Point p = coordmap.ptr<cv::Vec2i>(y)[x];
				if (p.x != -1) {
					tmp.ptr<cv::Vec3b>(y)[x] = src.ptr<cv::Vec3b>(p.y)[p.x];
				}
			}
		}
	}
	tmp.copyTo(dst);
}

// ���_�z����擾����
void GrayCode::getBufferArrays()
{
	//cv::Mat vertCoordMap, texCoordMap;

	//// ���߂��Ή��}�b�v���CGL�̍��W�n�ɕϊ�
	//getGLCoordMap(vertCoordMap, texCoordMap);

	////// ���_���
	//cv::Mat indexMap = cv::Mat(projHeight, projWidth, CV_32S);
	//std::vector<Vertex::MyVertex> vertex;
	//std::vector<Vertex::MyIndex_TRIANGLES> index;
	//unsigned int count = 0;

	//// �Ή����Ƃꂽ���ׂẲ�f�� vertex buffer ���擾
	//for (unsigned int y = 0; y < projHeight; ++y) {
	//	int *index = indexMap.ptr<int>(y);
	//	for (unsigned int x = 0; x < projWidth; ++x) {

	//		cv::Vec3f pos = vertCoordMap.ptr<cv::Vec3f>(y)[x];
	//		cv::Vec2f tex = texCoordMap.ptr<cv::Vec2f>(y)[x];
	//		// �Ή��_���v���ł��Ă����f�̒��_���W�ƃe�N�X�`�����W��vector�ɕێ�����
	//		if (pos[0] != -1.1f) {
	//			// ���_�����Z�b�g��vector�ɕێ�
	//			vertex.emplace_back(Vertex::MyVertex(pos, tex));
	//			// �i�[������f�ɔԍ�������U��(�C���f�b�N�X�o�b�t�@�ŎQ�Ƃ��邽��)
	//			index[x] = count;
	//			count++;
	//		}
	//	}
	//}

	//// �Ή����Ƃꂽ���ׂẲ�f�Ƀ��b�V���𒣂邽�߂́Cindex buffer���擾
	//for (unsigned int y = 0; y < projHeight - 1; ++y) {
	//	for (unsigned int x = 0; x < projWidth - 1; ++x) {

	//		// ���ډ�f�C��, �E, �E���̉�f���Q�Ƃ��C����ɂƂ�Ă�����C�O�p���b�V���̒��_�ɂ���
	//		cv::Vec3f pos = vertCoordMap.ptr<cv::Vec3f>(y)[x];
	//		cv::Vec3f right = vertCoordMap.ptr<cv::Vec3f>(y)[x + 1];
	//		cv::Vec3f down = vertCoordMap.ptr<cv::Vec3f>(y + 1)[x];
	//		cv::Vec3f diag = vertCoordMap.ptr<cv::Vec3f>(y + 1)[x + 1];
	//		if (pos[0] != -1.1f && right[0] != -1.1f && down[0] != -1.1f && diag[0] != -1.1f) {
	//			// ���b�V���𒣂钸�_��f�̔ԍ����Cvector�ɕێ�(�Q��)
	//			int pos_index = indexMap.ptr<int>(y)[x];
	//			int right_index = indexMap.ptr<int>(y)[x + 1];
	//			int down_index = indexMap.ptr<int>(y + 1)[x];
	//			int diag_index = indexMap.ptr<int>(y + 1)[x + 1];
	//			index.emplace_back(Vertex::MyIndex_TRIANGLES(pos_index, down_index, right_index));
	//			index.emplace_back(Vertex::MyIndex_TRIANGLES(diag_index, right_index, down_index));
	//		}
	//	}
	//}

	// ���߂����_����p���āC���_�z��Ǘ��N���XVertex���X�V
	//coord_trans_mesh[0]->setBuffers(vertex, index);

}

void GrayCode::getGLCoordMap(cv::Mat &dst_vertex, cv::Mat &dst_texture)
{
	dst_vertex = cv::Mat(projHeight, projWidth, CV_32FC3);
	dst_texture = cv::Mat(projHeight, projWidth, CV_32FC2);

	for (unsigned int y = 0; y < projHeight; ++y) {
		for (unsigned int x = 0; x < projWidth; ++x) {

			cv::Point2f point = coordmap.ptr<cv::Vec2f>(y)[x];
			// ���W�f�[�^������l�ŁA���}�X�N�̈���̏ꍇ�C-1 �` 1�Ɏ��܂�悤���K��
			if (point.x != -1.0){
				// ���_���W���擾
				// �e�N�X�`�����W���擾
				float vx = (float)x / (projWidth - 1.0f) * 2.0f - 1.0f;
				float vy = -(((float)y / (projHeight - 1.0f)) * 2.0f - 1.0f);
				float vz = 0.0;
				dst_vertex.ptr<cv::Vec3f>(y)[x] = cv::Vec3f(vx, vy, vz);
				// �e�N�X�`�����W���擾
				float u = (float)point.x / (camWidth - 1.0f);
				float v = -((float)point.y / (camHeight - 1.0f)) + 1.0f;
				dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(u, v);
			}
			// ���W�f�[�^���G���[�̏ꍇ�C�͈͊O��"-1.1"���i�[
			else{
				dst_vertex.ptr<cv::Vec3f>(y)[x] = cv::Vec3f(-1.1f, -1.1f, -1.1f);
				dst_texture.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(-1.1f, -1.1f);
			}
		}
	}

}