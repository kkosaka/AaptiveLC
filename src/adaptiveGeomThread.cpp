#include "adaptiveGeomThread.h"

/**
* @brief   �������֐�
*
* @param   coord_map[in] : �Ή����ۑ������}�b�g����ێ�����vecotr�z��
*
* @param   points[in]	:	�J�������W
*
* @param   lines[in]	:	�v���W�F�N�^���W�ɑ΂���J�������W�̃G�s�|�[�������i�[�������
*/
bool adaptiveGeomCorresp::init(const cv::Mat &coord_map)
{
	// ���W�f�[�^������
	camPoints.clear();
	projPoints.clear();
	if (coord_map.empty())
		return false;

	// �Ή��}�b�v��ێ�
	coordMap = coord_map.clone();
	indexMap = cv::Mat(camHeight, camWidth, CV_32SC1);
	int count = 0;
	for (int y = 0; y < camHeight; y++) {
		for (int x = 0; x < camWidth; x++) {
			cv::Point2f point = coordMap.ptr<cv::Vec2f>(y)[x];
			// �Ή��G���[�_�͏���
			if (point.x == -1.0f || point.y == -1.0f)
				continue;

			camPoints.emplace_back(cv::Point2f(x, y));
			projPoints.emplace_back(point);
			indexMap.ptr<int>(y)[x] = count;
			count++;
		}
	}

	// �J������f���̉�f�Ή���ύX����̂ŁC�J������f��̃G�s�|�[���������߂�
	// �J������̃G�s�|�[���������߂�ꍇ������,(�v���W�F�N�^���W, 2, F, lines)
	// ���Ȃ݂ɁC�v���W�F�N�^��̃G�s�|�[���������߂����ꍇ, (�J�������W, 1, F, lines)
	cv::computeCorrespondEpilines(projPoints, 2, FundametalMat, epiLines);

	//// debug(�G�s�|�[������`��)
	//cv::Mat camImage = cv::imread("./Calibration/projection.jpg");
	//cv::Mat projImage = cv::imread("./Calibration/reshape.jpg");
	//std::vector<cv::Vec3f> lines;
	//std::vector<cv::Point2f> draw_points;
	//draw_points.emplace_back(camPoints[indexMap.ptr<int>(900)[1000]]);
	//draw_points.emplace_back(camPoints[indexMap.ptr<int>(600)[960]]);
	//draw_points.emplace_back(camPoints[indexMap.ptr<int>(300)[300]]);
	//lines.emplace_back(epiLines[indexMap.ptr<int>(900)[1000]]);
	//lines.emplace_back(epiLines[indexMap.ptr<int>(600)[960]]);
	//lines.emplace_back(epiLines[indexMap.ptr<int>(300)[300]]);
	//// �G�s�|�[������`��
	//drawEpilines(camImage, lines, draw_points);

	// ���ׂẴJ������f�ɂ�����ړ��ʂ����߂�
	calcNewGeometricCorrespond(shiftVectorMats, camPoints, epiLines);

	return true;
}

// �G�s�|�[������`��
void adaptiveGeomCorresp::drawEpilines(cv::Mat& image, std::vector<cv::Vec3f> lines, std::vector<cv::Point2f> drawPoints)
{
	for (int i = 0; i < drawPoints.size(); ++i){
		if (lines[i][1]){
			// Y���ƌ���钼���̏ꍇ
			double width = image.size().width;
			// left �� x=0 �ɂ����钼���̒ʉߍ��W
			cv::Point2d left = cv::Point2d(0.0, solveY(lines[i], 0.0));
			// right �� x=width �ɂ����钼���̒ʉߍ��W
			cv::Point2d right = cv::Point2d(width, solveY(lines[i], width));
			// ���[����E�[�ɒ���������
			cv::line(image, left, right, cv::Scalar(0, 0, 255), 2);
		}
		else{
			// Y���ɕ��s�Ȓ����̏ꍇ
			// x���W�͈��Ȃ̂ŁC���炩���ߋ��߂�
			double x = -(lines[i])[2] / (lines[i])[0];
			// �摜�̏�[��ʉ߂�����W
			cv::Point2d top = cv::Point2d(x, 0.0);
			// �摜�̉��[��ʉ߂�����W
			cv::Point2d bottom = cv::Point2d(x, image.size().height);
			// ��[���牺�[�ɒ���������
			cv::line(image, top, bottom, cv::Scalar(0, 0, 255), 2);
		}
		cv::circle(image, drawPoints[i], 5, cv::Scalar(255, 0, 0));
	}
	cv::resize(image, image, cv::Size(), 0.5, 0.5);
	cv::imshow("epiline", image);
	cv::waitKey(0);
}

/**
* @brief   �e��f���ƂɃG�s�|�[���������ɑΉ������炷���߂̎Q�ƃe�[�u��(Mat)���쐬���邽�߂̊֐�
*		   ����ʐ���̂Ƃ��́C�􉽕ϊ����s���Ƃ��ɏ����v�������Ή��}�b�v(+���肵�������)�ɂ��̂���ʂ����Z�������̂�p����
*
* @param   _shiftVectorMats[in][out] : �Ή����ۑ������}�b�g����ێ�����vecotr�z��
*
* @param   points[in]	:	�J�������W
*
* @param   lines[in]	:	�v���W�F�N�^���W�ɑ΂���J�������W�̃G�s�|�[�������i�[�������
*/
void adaptiveGeomCorresp::calcNewGeometricCorrespond(std::vector<cv::Mat> &_shiftVectorMats, const std::vector<cv::Point2f> &points, const std::vector<cv::Vec3f> &lines)
{
	// �ړ��ʂ� -d[pixel]  ���� d[pixel] �܂� 1[pixel] �����₵�Ă���
	for (float d = -(maxPixelSize); d <= maxPixelSize; d += 1.0f) {
		// �ړ��ʂ��i�[
		cv::Mat shiftVecMat(camHeight, camWidth, CV_32FC2, cv::Scalar::all(0.0f));

		// �S��f�ɂ����āA�G�s�|�[������̈ړ��ʂ��Z�o����
		auto it = lines.begin();
		auto it_points = points.begin();
		for (; it != lines.end(); ++it, ++it_points)
		{
			cv::Vec2f move_vector;
			// y���ɕ��s�łȂ��ꍇ
			if ((*it)[1]){
				// �X�������߂�
				float a = -(*it)[0] / (*it)[1];
				// �X����1�����傫���ꍇ
				if (abs(a) >= 1.0f) {
					// y�������ւ̒l�̑�����D��
					float x_vector = d / a;
					move_vector = cv::Vec2f(x_vector, (float)d);
				}
				// �X����1�����������ꍇ
				else {
					// x�������ւ̒l�̑�����D��
					int y_vector = d * a;
					move_vector = cv::Vec2f((float)d, (float)y_vector);
				}
			}
			else{
				// Y���ɕ��s�Ȓ����̏ꍇ
				// x���W�͈��Ȃ̂ŁC���炩���ߋ��߂�
				move_vector = cv::Vec2f((float)d, 0.0f);
			}

			shiftVecMat.ptr<cv::Vec2f>((int)(*it_points).y)[(int)(*it_points).x] = move_vector;
		}

		// Mat�ɕۑ�
		_shiftVectorMats.emplace_back(shiftVecMat);
	}

}
//
//void adaptiveGeomCorresp::getValiance(GLuint &k_texture_objs, GLuint &mask_objs, int current_num)
//{
//	//fps->begin();
//	std::vector<GLuint> v_tex_objs;
//	// �V�F�[�_�̗L��
//	shader_getv->setEnabled(true);
//	// �ϐ��̐ݒ�
//	glUniform1i(shl_corrent_index, current_num);
//	// �f�o�b�O�p
//	GLuint txo_valiance = texobj32F->getTextureObject();
//	// ���U���v�Z����
//	calcReliability(k_texture_objs, mask_objs, txo_valiance);
//	texobj32F->releaseTextureObject(txo_valiance);
//	cv::Mat	sd = saveTexture(txo_valiance, "adaptiveLC/Result/result" + to_string(current_num), projWidth, projHeight);
//	images.emplace_back(sd);
//	// �V�F�[�_�̖���
//	shader_getv->setEnabled(false);
//	//std::cout << "�Y���̐��菈��" << fps->getElapsed() << "[ms]" << std::endl;
//}
//
//// ���U���v�Z����
//void adaptiveGeomCorresp::calcReliability(GLuint &srcTexture, GLuint &maskTexture, GLuint &texture_valiance)
//{
//	glActiveTexture(GL_TEXTURE0);	//���j�b�g0
//	glBindTexture(GL_TEXTURE_2D, srcTexture);
//	glActiveTexture(GL_TEXTURE1);	//���j�b�g1
//	glBindTexture(GL_TEXTURE_2D, txo_min_valiance_before);
//	glActiveTexture(GL_TEXTURE2);	//���j�b�g2
//	glBindTexture(GL_TEXTURE_2D, txo_sigma_before);
//	glActiveTexture(GL_TEXTURE3);	//���j�b�g3
//	glBindTexture(GL_TEXTURE_2D, txo_index_before);
//	glActiveTexture(GL_TEXTURE4);	//���j�b�g4
//	glBindTexture(GL_TEXTURE_2D, maskTexture);
//
//	// �I�t�X�N���[�������_�����O�J�n
//	agc_offscreen_render->setEnabled(true);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_min_valiance_after);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, txo_sigma_after);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, txo_index_after);
//	agc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, texture_valiance);
//	agc_offscreen_render->drawBuffers();
//	vertex->render_mesh();
//	// �I�t�X�N���[�������_�����O�I��
//	agc_offscreen_render->setEnabled(false);
//
//	// �e�N�X�`���̓���ւ�
//	GLuint tmp;
//	tmp = txo_min_valiance_before;
//	txo_min_valiance_before = txo_min_valiance_after;
//	txo_min_valiance_after = tmp;
//
//	tmp = txo_sigma_before;
//	txo_sigma_before = txo_sigma_after;
//	txo_sigma_after = tmp;
//
//	tmp = txo_index_before;
//	txo_index_before = txo_index_after;
//	txo_index_after = tmp;
//}
//
//bool adaptiveGeomCorresp::Copy(GLuint pbo, void* dest, GLuint fbo, GLuint src_tex, GLint x, GLint y, GLsizei width, GLsizei height)
//{
//	GL_PIXEL_PACK_BUFFER_ARB;
//	bool ret = false;
//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_tex, 0);
//	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
//	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
//	glReadPixels(x, y, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
//	void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
//	if (ptr != NULL)
//	{
//		memcpy(dest, ptr, width * height * 4);
//		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//		ret = true;
//	}
//	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	return ret;
//
//	// PBO�̐���
//	//int channels = 1;
//	//PBO pbo;
//	//pbo.getPBO(camWidth, camHeight, channels, GL_FLOAT);
//	//fps->begin();
//	//gt_offscreen_render_cam->setEnabled(true);
//	//gt_offscreen_render_cam->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_tex_cam);
//	//glReadBuffer(GL_COLOR_ATTACHMENT0);
//	//pbo.bind(GL_PIXEL_PACK_BUFFER, true);
//	//glReadPixels(0, 0, camWidth, camHeight, GL_RED, GL_FLOAT, 0);
//	//// Mat �փR�s�[
//	//cv::Mat dst(camHeight, camWidth, CV_32F);
//	//void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
//	//if (ptr != NULL)
//	//{
//	//	memcpy(dst.data, ptr, camWidth * camHeight * channels * sizeof(GL_FLOAT));
//	//	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
//	//}
//	//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
//	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}



//void adaptiveGeomCorresp::correctGeometricCorresp(const cv::Mat &moveVectorMat, cv::Mat maskMat)
//{
//	cv::Mat dst = cv::Mat(camHeight, camWidth, CV_32FC2);
//	moveVectorsResult.clear();
//
//	// �l��⊮����
//	for (int y = 0; y < camHeight; ++y) {
//		int *index_p = indexMap.ptr<int>(y);
//		cv::Vec2f *coord_p = coordMap.ptr<cv::Vec2f>(y);
//		for (int x = 0; x < camWidth; ++x) {
//
//			// �Ή��G���[�_�͏���
//			if (coord_p[x][0] == -1.0f || coord_p[x][1] == -1.0f)
//				continue;
//
//			// ���O�ɗp�ӂ��ꂽ���F�N�^�[����Q��
//			unsigned int num = moveVectorMat.at<cv::Vec4f>(y, x)[0];
//			// �ړ���[pixel]���i�[
//			cv::Vec3f movePixel;
//			movePixel[0] = moveVectors[num][index_p[x]][0];
//			movePixel[1] = moveVectors[num][index_p[x]][1];
//			movePixel[2] = index_p[x];
//			moveVectorsResult.emplace_back(movePixel);
//
//			// debug�p
//			dst.ptr<cv::Vec2f>(y)[x] = cv::Vec2f(movePixel[0], movePixel[1]);
//		}
//	}
//	std::cout << "aa" << std::endl;
//}


/**
* @brief   �����_�����O���ʂ̕ۑ�(RGB)
*
* @param   texId[in]		�ۑ�����e�N�X�`����ID
*
* @param   filename[in]		�ۑ�����摜�̃t�@�C����
*
* @param   w[in]		�ۑ�����摜�̉���[pixel]
*
* @param   h[in]		�ۑ�����摜�̏c��[pixel]
*/
//cv::Mat adaptiveGeomCorresp::saveTexture(GLuint texId, const string &filename, int w, int h, bool isFlip)
//{
//	cv::Mat img(h, w, CV_32FC4);
//
//	glBindTexture(GL_TEXTURE_2D, texId);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, img.data);
//	glBindTexture(GL_TEXTURE_2D, 0);
//
//	//ofstream ofs("./debugout/" + filename + ".txt");
//	//ofs << img;
//	cv::cvtColor(img, img, CV_RGBA2BGR);
//	if (isFlip)
//		cv::flip(img, img, 0);// �摜�̔��](x������)
//
//	cv::Mat uc_mat;
//	img.convertTo(uc_mat, CV_8UC3, 255.0);
//	cv::imwrite("./debugout/" + filename + ".bmp", uc_mat);
//
//	return img;
//}
