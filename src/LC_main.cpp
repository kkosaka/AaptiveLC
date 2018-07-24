#include "LC_main.h"
#include <regex>

cv::Rect rect(278, 130, 648, 405);
cv::Rect rect_old(200, 140, 450, 450);
cv::Rect rect_new(660, 140, 450, 450);

/* �ŏ���1�񂾂��Ă΂��֐� */
bool myGL::init()
{
	// glew�̏�����
	glewExperimental = true;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		printf("Error: %s\n", glewGetErrorString(err));
		return false;
	}

	//-----�e�N�X�`���}�b�s���O�p�V�F�[�_�̓ǂݍ���-----
	shader_texture->init("./shader/texture.vert", "./shader/texture.frag");
	shader_texture->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	GLuint shl_transMat_vert = getShlWithErrorDetect(shader_texture->getHandle(), "transformMatrix_vertex");
	GLuint shl_transMat_tex = getShlWithErrorDetect(shader_texture->getHandle(), "transformMatrix_texture");
	// �ϊ��s����v�Z����
	glm::mat4 transMat_vert = VertexTransformMatrix(projWidth, projHeight);
	glm::mat4 transMat_tex = TextureTransformMatrix(projWidth, projHeight);
	// �o�[�e�b�N�X�V�F�[�_�Ŏg�p����ϊ��s���]��
	glUniformMatrix4fv(shl_transMat_vert, 1, GL_FALSE, &transMat_vert[0][0]);
	glUniformMatrix4fv(shl_transMat_tex, 1, GL_FALSE, &transMat_tex[0][0]);
	GLuint shl_texture = getShlWithErrorDetect(shader_texture->getHandle(), "projection_texture");
	glUniform1i(shl_texture, TEXTURE_UNIT_INPUTIMAGE);	//�e�N�X�`�����j�b�g0
	shader_texture->setEnabled(0);
	texture_vertex->init();

	//-----�􉽕␳�p�V�F�[�_�̓ǂݍ���-----

	shader_geom->init("./shader/geometricTrans.vert", "./shader/geometricTrans.frag");
	shader_geom->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	GLuint shl_camtex = getShlWithErrorDetect(shader_geom->getHandle(), "imageTexture");
	GLuint shl_geom_transMat_vert = getShlWithErrorDetect(shader_geom->getHandle(), "transformMatrix_vertex");
	GLuint shl_geom_transMat_tex = getShlWithErrorDetect(shader_geom->getHandle(), "transformMatrix_texture");
	// �t���O�����g�V�F�[�_�Ŏg�p����e�N�X�`�����e�N�X�`�����j�b�g0�ɐݒ�
	glUniform1i(shl_camtex, 0);
	// �ϊ��s����v�Z����
	glm::mat4 geom_transMat_vert = VertexTransformMatrix(projWidth, projHeight);
	glm::mat4 geom_transMat_tex = TextureTransformMatrix(camWidth, camHeight);
	// �o�[�e�b�N�X�V�F�[�_�Ŏg�p����ϊ��s���]��
	glUniformMatrix4fv(shl_geom_transMat_vert, 1, GL_FALSE, &geom_transMat_vert[0][0]);
	glUniformMatrix4fv(shl_geom_transMat_tex, 1, GL_FALSE, &geom_transMat_tex[0][0]);

	txo_camImageTexture = camTexs_RGBA->getTextureObject();
	shader_geom->setEnabled(0);

	//-----�P�x�␳�p�V�F�[�_�̐ݒ�-----
	shader_lc->init("./shader/luminance.vert", "./shader/luminance.frag");
	shader_lc->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	shl_inputImageTexture = getShlWithErrorDetect(shader_lc->getHandle(), "inputImageTexture");
	shl_inputImage_ScalingAlpha = getShlWithErrorDetect(shader_lc->getHandle(), "inputImage_ScalingAlpha");
	shl_inputImage_ScalingBeta = getShlWithErrorDetect(shader_lc->getHandle(), "inputImage_ScalingBeta");
	shl_prjImageTexture = getShlWithErrorDetect(shader_lc->getHandle(), "prjImageTexture");
	shl_camImageTexture = getShlWithErrorDetect(shader_lc->getHandle(), "camImageTexture");
	shl_gamma = getShlWithErrorDetect(shader_lc->getHandle(), "gamma");
	shl_b_pixel = getShlWithErrorDetect(shader_lc->getHandle(), "b_pixel");
	shl_w_pixel = getShlWithErrorDetect(shader_lc->getHandle(), "w_pixel");
	shl_color_mixing_matrix = getShlWithErrorDetect(shader_lc->getHandle(), "color_mixing_matrix");
	shl_color_mixing_matrix_inv = getShlWithErrorDetect(shader_lc->getHandle(), "color_mixing_matrix_inv");
	shl_numerator_before = getShlWithErrorDetect(shader_lc->getHandle(), "numerator_before");
	shl_denominator_before = getShlWithErrorDetect(shader_lc->getHandle(), "denominator_before");
	shl_forgetFactor = getShlWithErrorDetect(shader_lc->getHandle(), "forgetFactor");
	shl_diffThreshold = getShlWithErrorDetect(shader_lc->getHandle(), "diff_threshold");
	// ���̑��p�����[�^
	shl_nonCorection = getShlWithErrorDetect(shader_lc->getHandle(), "nonCorrection");
	glUniform1i(shl_nonCorection, (int)nonCorrection);
	shl_mask = getShlWithErrorDetect(shader_lc->getHandle(), "mask_texutre");


	//-----�Ή��}�b�v�X�V�p�V�F�[�_�̓ǂݍ���-----
	const char* feedbackNames[] = {
		"out_VBO_Data",
	};
	shader_getShiftVec->init("./shader/getShiftVector.vert", "./shader/getShiftVector.frag", sizeof(feedbackNames) / sizeof(feedbackNames[0]), feedbackNames, GL_SEPARATE_ATTRIBS);
	shader_getShiftVec->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	shl_getShift_shiftIndexTex = getShlWithErrorDetect(shader_getShiftVec->getHandle(), "shiftIndexTexture");
	shl_getShift_correntIndex = getShlWithErrorDetect(shader_getShiftVec->getHandle(), "CORRENT_INDEX");
	GLuint shl_getShift_projWidth = getShlWithErrorDetect(shader_getShiftVec->getHandle(), "PROJECTOR_SIZE");
	glUniform1i(shl_getShift_shiftIndexTex, 0);	//�e�N�X�`�����j�b�g0
	glUniform1i(shl_getShift_correntIndex, 0);	// �ϐ�
	glUniform2i(shl_getShift_projWidth, projWidth, projHeight);	// �ϐ�
	shader_getShiftVec->setEnabled(false);

	//-----�Ή��}�b�v�X�V�p�V�F�[�_�̓ǂݍ���-----
	const char* feedbackNames2[] = {
		"out_VBO_Data",
		"out_VBO_Status",
	};
	shader_mapUpdater->init("./shader/updateCoordMap.vert", "./shader/updateCoordMap.frag", sizeof(feedbackNames2) / sizeof(feedbackNames2[0]), feedbackNames2, GL_SEPARATE_ATTRIBS);
	shader_mapUpdater->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	shl_mapUpdater_statusTex = getShlWithErrorDetect(shader_mapUpdater->getHandle(), "statusTexture");
	shl_mapUpdater_resultTex = getShlWithErrorDetect(shader_mapUpdater->getHandle(), "offsetTexture");
	shl_mapUpdater_maskTex = getShlWithErrorDetect(shader_mapUpdater->getHandle(), "maskTexture");
	GLuint shl_mapUpdater_camSize = getShlWithErrorDetect(shader_mapUpdater->getHandle(), "CAMERA_SIZE");
	glUniform1i(shl_mapUpdater_statusTex, 0);	//�e�N�X�`�����j�b�g0
	glUniform1i(shl_mapUpdater_resultTex, 1);	//�e�N�X�`�����j�b�g1
	glUniform1i(shl_mapUpdater_maskTex, 2);	//�e�N�X�`�����j�b�g1
	glUniform2i(shl_mapUpdater_camSize, camWidth, camHeight);	// �ϐ�
	shader_mapUpdater->setEnabled(false);
	std::cout << "shader_ get min" << std::endl;

	//-----�Ή��}�b�v�������p�V�F�[�_�̓ǂݍ���-----
	shader_mapBlur->init("./shader/blurToCoordMap.vert", "./shader/blurToCoordMap.frag", sizeof(feedbackNames) / sizeof(feedbackNames[0]), feedbackNames, GL_SEPARATE_ATTRIBS);
	shader_mapBlur->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	shl_mapBlur_offsetTex = getShlWithErrorDetect(shader_mapBlur->getHandle(), "offsetTexture");
	shl_mapBlur_maskTex = getShlWithErrorDetect(shader_mapBlur->getHandle(), "maskTexture");
	GLuint shl_mapBlur_camSize = getShlWithErrorDetect(shader_mapBlur->getHandle(), "CamSize");
	shl_mapBlur_blurNum = getShlWithErrorDetect(shader_mapBlur->getHandle(), "BlurNum");
	glUniform1i(shl_mapBlur_offsetTex, 0);	//�e�N�X�`�����j�b�g0
	glUniform1i(shl_mapBlur_maskTex, 1);	//�e�N�X�`�����j�b�g1
	glUniform2i(shl_mapBlur_camSize, camWidth, camHeight);	//�ϐ�
	glUniform1i(shl_mapBlur_blurNum, 0);	//�ϐ�
	shader_mapBlur->setEnabled(false);

	//-----�V�F�[�_�̓ǂݍ���-----	
	const char* feedbackNames5[] = {
		"out_VBO_Data_MinV",
		"out_VBO_Data_Index",
		"out_VBO_Data_Valiance",
	};
	shader_getValiance->init("./shader/getValiance.vert", "./shader/getValiance.frag", sizeof(feedbackNames5) / sizeof(feedbackNames5[0]), feedbackNames5, GL_SEPARATE_ATTRIBS);
	shader_getValiance->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	GLuint shl_getV_k = getShlWithErrorDetect(shader_getValiance->getHandle(), "k_texture");
	GLuint shl_getV_projsize = getShlWithErrorDetect(shader_getValiance->getHandle(), "ProjSize");
	GLuint shl_getV_zero = getShlWithErrorDetect(shader_getValiance->getHandle(), "ZERO_INDEX");
	shl_getV_correntIndex = getShlWithErrorDetect(shader_getValiance->getHandle(), "CORRENT_INDEX");
	glUniform1i(shl_getV_k, 0);	//�e�N�X�`�����j�b�g
	glUniform2i(shl_getV_projsize, projWidth, projHeight);	// �ϐ�
	glUniform1i(shl_getV_zero, ZEROINDEX);	// �ϐ�
	glUniform1i(shl_getV_correntIndex, 0);	// �ϐ�
	//txo_valiance = agcTexs_RGB32F->getTextureObject();	//�e�N�X�`���̐���
	//txo_temp_diff = agcTexs_RGB32F->getTextureObject();
	shader_getValiance->setEnabled(false);

	//-----�ŏ����U�l�̌v�Z�p�V�F�[�_�̓ǂݍ���-----
	const char* feedbackNames3[] = {
		"out_VBO_Data_MinV",
		"out_VBO_Data_Index",
	};
	shader_getMinVIndex->init("./shader/getMinValianceIndex.vert", "./shader/getMinValianceIndex.frag", sizeof(feedbackNames3) / sizeof(feedbackNames3[0]), feedbackNames3, GL_SEPARATE_ATTRIBS);
	shader_getMinVIndex->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	shl_minV_correntIndex = getShlWithErrorDetect(shader_getMinVIndex->getHandle(), "CORRENT_INDEX");
	GLuint shl_minV_prj = getShlWithErrorDetect(shader_getMinVIndex->getHandle(), "projSize");
	GLuint shl_minV_zeroIndex = getShlWithErrorDetect(shader_getMinVIndex->getHandle(), "ZERO_INDEX");
	glUniform1i(shl_minV_correntIndex, 0);	//�ϐ�
	glUniform2i(shl_minV_prj, projWidth, projHeight);	// �ϐ�
	glUniform1i(shl_minV_zeroIndex, ZEROINDEX);	//�ϐ�
	shader_getMinVIndex->setEnabled(false);

	//-----�G���[����p�V�F�[�_�̓ǂݍ���-----
	const char* feedbackNames4[] = {
		"out_VBO_Data_Index",
	};
	shader_errorTest->init("./shader/errorTest.vert", "./shader/errorTest.frag", sizeof(feedbackNames4) / sizeof(feedbackNames4[0]), feedbackNames4, GL_SEPARATE_ATTRIBS);
	shader_errorTest->setEnabled(true);
	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	GLuint shl_errorTest_shiftTex = getShlWithErrorDetect(shader_errorTest->getHandle(), "indexTexture");
	GLuint shl_errorTest_projSize = getShlWithErrorDetect(shader_errorTest->getHandle(), "ProjSize");
	GLuint shl_errorTest_ZeroIndex = getShlWithErrorDetect(shader_errorTest->getHandle(), "ZeroIndex");
	glUniform1i(shl_errorTest_shiftTex, 0);	//�e�N�X�`�����j�b�g0
	glUniform2i(shl_errorTest_projSize, projWidth, projHeight);	// �ϐ�
	glUniform1i(shl_errorTest_ZeroIndex, ZEROINDEX);	// �ϐ�	
	shader_errorTest->setEnabled(false);

	//----- �摜�̓ǂݍ���------
	srcImage = imageFileList->getImage(imageIndex);

	// �}�X�N�摜�̍쐬
	// �摜�̉��𓊉e�ʌ`��ω��̌��o�����ł͂������߂̃}�X�N�摜
	whiteImage = cv::Mat(projHeight, projWidth, CV_8UC3, cv::Scalar::all(255));
	agc_maskImage = cv::Mat(projHeight, projWidth, CV_8UC3, cv::Scalar::all(255));
	int offsetPixel = 10;
	for (int y = 0; y < projHeight; ++y){
		for (int x = 0; x < projWidth; ++x){
			if (y < offsetPixel || x < offsetPixel || y > projHeight - offsetPixel || x > projWidth - offsetPixel)
				agc_maskImage.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);

		}
	}
	// ���������␳���s��
	float half = 0.5;
	for (int y = 0; y < projHeight; ++y){
		for (int x = 0; x < projWidth; ++x){
			if (projWidth * half > x)
				agc_maskImage.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
		}
	}
	//-----�I�t�X�N���[�������_�����O�p�t���[���o�b�t�@�I�u�W�F�N�g�̍쐬-----
	if (!gt_offscreen_render->init(projWidth, projHeight))
		return false;
	if (!lc_offscreen_render->init(projWidth, projHeight))
		return false;
	if (!offscreen_render_proj->init(projWidth, projHeight))
		return false;

	printInfo();

	return true;
}


/**
* @brief   �J�����n����v���W�F�N�^�n�֍��W�ϊ��s���֐�
*/
void myGL::coord_trans(GLuint texobj, GLuint proj_texobj, cv::Mat &cameraImage)
{
	if (!ps->getStatus()) {
		std::cerr << "get coordinate correspondance ...." << std::endl;
		return;
	}
	if (!cameraImage.empty()){
		// �J�����摜��GPU�ւ̓]���x�� 3~5ms ������
		camTexs_RGBA->uploadToTexture(texobj, GL_BGR, GL_UNSIGNED_BYTE, cameraImage.data);
	}

	// �u���b�N�̔w�i
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	clear();
	//fps->begin();
	// �V�F�[�_�̗L��
	shader_geom->setEnabled(true);
	camTexs_RGBA->bind(texobj, GL_TEXTURE0);
	// �I�t�X�N���[�������_�����O�J�n
	gt_offscreen_render->setEnabled(true);
	gt_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, proj_texobj);
	gt_offscreen_render->drawBuffers();

	ps->coord_trans->render_mesh();

	// �I�t�X�N���[�������_�����O�I��
	gt_offscreen_render->setEnabled(false);
	shader_geom->setEnabled(false);

	////-----�␳���ʂ�`��(debug)-----
	//glUseProgram(program_geom);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, proj_texobj);
	//m_Vertex[0]->draw();
	////// �J���[�o�b�t�@�����ւ���
	//swapBuffers();
	//glUseProgram(0);
	//std::cout << fps->getElapsed() << std::endl;
}

/**
* @brief   �P�x�␳���s���֐�
*/
void myGL::lc_projection(float targetImage_ScaleAlpha, float targetImage_ScaleBeta, float ff, float diff_th, GLuint maskTexture, GLuint texture_k_after, GLuint  texture_k_diff)
{
	// �V�F�[�_�̗L��
	shader_lc->setEnabled(true);

	//�ڕW�摜�̃X�P�[�����O�l���Z�b�g
	glUniform1f(shl_inputImage_ScalingAlpha, targetImage_ScaleAlpha);
	glUniform1f(shl_inputImage_ScalingBeta, targetImage_ScaleBeta);
	glUniform1f(shl_forgetFactor, ff);
	glUniform1ui(shl_diffThreshold, diff_th);
	glUniform1i(shl_nonCorection, nonCorrection);

	//-----�e�N�X�`�����j�b�g�Ƀe�N�X�`�����o�C���h-----
	//�ڕW�摜
	lumiTexs_RGBA->bind(txo_inputImageTexture, TEXTURE_UNIT_INPUTIMAGE_ENUM);
	//�v���W�F�N�^���e�摜
	lumiTexs_RGBA->bind(txo_projImageTexture, TEXTURE_UNIT_PRJIMAGE_ENUM);
	//�J�����摜
	lumiTexs_RGBA->bind(txo_transCamImageTexture, TEXTURE_UNIT_CAMIMAGE_ENUM);
	//�ŏ����@�p�����[�^�̕��q���ꐬ��
	lumiTexs_RGBA32F->bind(txo_numerator_before, TEXTURE_UNIT_NUME_ENUM);
	lumiTexs_RGBA32F->bind(txo_denominator_before, TEXTURE_UNIT_DENOMI_ENUM);
	//�}�X�N�摜
	lumiTexs_RGBA->bind(maskTexture, TEXTURE_UNIT_MASK_ENUM);

	//-----�t���[���o�b�t�@�I�u�W�F�N�g�̐ݒ�-----
	//�����_�����O���ʏo�͗p�e�N�X�`�����擾

	//GLuint diff_obj = lumiTexs_RGBA32F->getTextureObject();
	//�����_�����O���ʁA�ŏ����@�̕��q���ꐬ���AK�̏o�͐��ݒ�
	//(�t���O�����g�V�F�[�_��OUTPUTINDEX_*�ƈ�v�����邱��)
	lc_offscreen_render->setEnabled(true);
	lc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_projImage_after);
	lc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, txo_numerator_after);
	lc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, txo_denominator_after);
	lc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, txo_targetImageTexture);
	lc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, texture_k_after);
	lc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, texture_k_diff);
	lc_offscreen_render->drawBuffers();
	texture_vertex->render_mesh();
	//FBO�ƃV�F�[�_�𖳌���
	lc_offscreen_render->setEnabled(false);
	shader_lc->setEnabled(false);

	if (lc_noRender_flag) {
		return;
	}

	//-----�␳���ʂ�`��-----

	shader_texture->setEnabled(true);
	glViewport(0, 0, getWidth(), getHeight());
	for (int i = 0; i < 1; i++){
		lumiTexs_RGBA->bind(txo_projImage_after, GL_TEXTURE0);
		texture_vertex->render_mesh();
		swapBuffers();	// �J���[�o�b�t�@�����ւ���
	}
	shader_texture->setEnabled(false);

	//-----�㏈��-----
	//�ŏ����@�̃p�����[�^�����ւ�
	GLuint tmp;
	tmp = txo_numerator_before;
	txo_numerator_before = txo_numerator_after;
	txo_numerator_after = tmp;

	tmp = txo_denominator_before;
	txo_denominator_before = txo_denominator_after;
	txo_denominator_after = tmp;

	//�␳�摜�̓���ւ�
	tmp = txo_projImageTexture;
	txo_projImageTexture = txo_projImage_after;
	txo_projImage_after = tmp;

	return;
	//static int count = 0;
	//if (count == 10){
	//	// �ŏ���K��ێ�
	//	tmp = txo_k_after;
	//	txo_k_after = txo_first_k;
	//	txo_first_k = tmp;
	//	saveTexture(txo_first_k, to_string(count++) + "_txo_k_first", projWidth, projHeight);
	//	shader_lc->setEnabled(true);
	//	glUniform1i(shl_calc_k, 1);
	//	shader_lc->setEnabled(false);
	//}
	//count++;
}

/**
* @brief   �f�B�X�v���C�֐�(���t���[���Ă΂��)
*/
void myGL::display()
{
	if (loop_flag) {
		Sleep(delay);
		pgrCamera->getImage(capImage, 0);	// 2~4ms

		cv::flip(capImage, capImage, 0);

		// �␳��~
		if (stop_flag){

			// �J�����摜���􉽊w�I�ϊ�
			coord_trans(txo_camImageTexture, txo_transCamImageTexture, capImage);

			//-----�␳���ʂ�`��-----
			shader_texture->setEnabled(true);
			glViewport(0, 0, getWidth(), getHeight());
			for (int i = 0; i < 2; i++){
				lumiTexs_RGBA->bind(txo_projImage_after, GL_TEXTURE0);
				texture_vertex->render_mesh();
				swapBuffers();	// �J���[�o�b�t�@�����ւ���
			}
			shader_texture->setEnabled(false);

		}
		else{
			// �J�����摜���X�V (3 ~ 5ms)
			coord_trans(txo_camImageTexture, txo_transCamImageTexture, capImage);

			/* device �x���ɖ� 77.83 [ms] */
			/* �␳�����ɖ� 1.26 [ms] */

			//------�􉽑Ή�����̏C���������s��-------1~2ms(�G���[��f�ɉ����đ�����)
			if (agc_calc_flag){
				lc_noRender_flag = true;// �􉽑Ή��Y���̐��菈�����J�n
				// �����摜���쐬
				lc_projection(brightness, 0.0, fogetFactor, diff_thresh, txo_agc_maskTexture, txo_k_after, txo_diff);

				// �Ή��}�b�v���C��
				adaptiveGeom();

				// �V���ȑΉ��}�b�v�ɂ��J�����摜���X�V
				coord_trans(txo_camImageTexture, txo_transCamImageTexture);

				//agc_calc_flag = false;
				fogetFactor = 0.30;
				lc_noRender_flag = false;// �􉽑Ή��Y���̐��菈�����I��
			}

			//------�P�x�␳���s��-------
			lc_projection(brightness, 0.0, fogetFactor, diff_thresh, txo_maskTexture, txo_k_after, txo_diff);

			//------�i�]�������j�摜��]������-------
			/*
			** �v������FPS�������Ȃ��悤�ɁC�B�e�摜���͂��ׂ�GPU��ŕۑ����Ă����C�v�����I�������C�Ɍv�Z����
			*/
			if (write_flag){
				timeGetter->begin();

				static int write_num = 0;
				VBO_CAMERAIMAGES[write_num] = new glBuffer<cv::Vec4b>(GL_ARRAY_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_COPY);

				//std::cout << timeGetter->getElapsed() << std::endl;
				// �e�N�X�`����PBO�ɃR�s�[
				offscreen_render_proj->setEnabled(true);
				offscreen_render_proj->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_transCamImageTexture);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, PBO_CAMERAIMAGES->buf());
				glReadPixels(0, 0, projWidth, projHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				offscreen_render_proj->setEnabled(false);

				// PBO��VBO�ɃR�s�[
				glBindVertexArray(vao);
				bufferCopy(GL_PIXEL_PACK_BUFFER, PBO_CAMERAIMAGES->buf(), GL_ARRAY_BUFFER, VBO_CAMERAIMAGES[write_num]->buf(), VBO_CAMERAIMAGES[write_num]->size());
				glBindVertexArray(0);

				//std::cout << "��������" << timeGetter->getElapsed() << "[ms]" << std::endl;
				static double delay_copy = timeGetter->getElapsed();
				delay = DlayTime - delay_copy;

				write_num++;
				// �o�߃t���[�����w�肵���񐔂ɒB������摜��͂��J�n����
				if (write_num == 1000){
					cv::Mat targetImage, capImage, img;
					for (int i = 0; i < write_num; i++){

						// �h���J�[�e���̌v��(�E�����]����@�C��������Ď�@)
						img = glUtl::bufferChecker<cv::Vec4b>(VBO_CAMERAIMAGES[i]->buf(), vao, GL_ARRAY_BUFFER, GL_UNSIGNED_BYTE, projWidth, projHeight);
						cv::cvtColor(img, capImage, CV_RGBA2BGR);
						cv::flip(capImage, capImage, 0);// �摜�̔��](x������)
						cv::Mat targetImage = saveTexture(txo_targetImageTexture, "", projWidth, projHeight, true, false, false);
						cv::imwrite("./debugout/dynamic environmet/cameraImages/cap" + to_string(i) + ".jpg", capImage);

						// �]����@�̌v��
						cv::Mat rectOldCap(capImage, rect_old);
						cv::Mat rectOldTarget(targetImage, rect_old);
						float MSE_old = 0, PSNR_old, SSIM_old;
						PSNR_old = lc->calcPSNR<cv::Vec3b>(rectOldCap, rectOldTarget, MSE_old);
						SSIM_old = lc->calcSSIM(rectOldCap, rectOldTarget);
						cv::imwrite("./debugout/dynamic environmet/old/" + to_string(i) + "cap_rect.jpg", rectOldCap);
						cv::imwrite("./debugout/dynamic environmet/old/" + to_string(i) + "target_rect.jpg", rectOldCap);

						// ��Ď�@�̌v��
						cv::Mat rectProposalCap(capImage, rect_new);
						cv::Mat rectProposalTarget(targetImage, rect_new);
						float MSE_proposal = 0, PSNR_proposal, SSIM_proposal;
						PSNR_proposal = lc->calcPSNR<cv::Vec3b>(rectProposalCap, rectProposalTarget, MSE_proposal);
						SSIM_proposal = lc->calcSSIM(rectProposalCap, rectProposalTarget);
						cv::imwrite("./debugout/dynamic environmet/proposal/" + to_string(i) + "cap_rect.jpg", rectProposalCap);
						cv::imwrite("./debugout/dynamic environmet/proposal/" + to_string(i) + "target_rect.jpg", rectProposalCap);

						//cv::Mat capImage = saveTexture(txo_transCamImageTexture, NULL, projWidth, projHeight, true, false, false);
						//cv::Mat rectCap(capImage, rect);
						//cv::Mat rectTarget(targetImage, rect);
						//float MSE;
						//float PSNR = lc->calcPSNR<cv::Vec3b>(rectCap, rectTarget, MSE);
						//float SSIM = lc->calcSSIM(rectCap, rectTarget);

						//�t�@�C���̏�������
						std::ofstream ofsRF("./debugout/CSV/MSE/mse" + to_string(i) + ".csv");
						//�G���[����
						if (ofsRF.fail()){
							std::cout << "ERROR:�����֐�.csv���J���܂���B" << std::endl;
						}
						//ofsRF << MSE << ", " << PSNR << ", " << SSIM;
						ofsRF << MSE_old << ", " << MSE_proposal << "," << ", " << PSNR_old << ", " << PSNR_proposal << "," << ", " << SSIM_old << ", " << SSIM_proposal;
						ofsRF.close();

					}
					write_num = 0;
					write_flag = false;
					delay = DlayTime;
					//debug_textures();
				}
			}

			if (lc_counter > 15 && lc_counter % 1 == 0){
				agc_calc_flag = true;
			}
			lc_counter++;
		}
		// �R���\�[����ʂւ̕\��
		printInfo();
	}
	else{
		// �z���C�g�̔w�i
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		clear();
		// �J���[�o�b�t�@�����ւ�,�C�x���g���擾
		swapBuffers();
		//glFlush();
	}

}

/**
* @brief   ��f�Ή��̓��I�ȏC���������s���֐�(1 ~ 2ms)
*/
void myGL::adaptiveGeom()
{
	if (agc->shiftVectorMats.size() == 0)
		return;

	// �����̌v�Z����(�G���[��f)��PBO�ɃR�s�[
	offscreen_render_proj->setEnabled(true);
	offscreen_render_proj->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_diff);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, PBO_projSize->buf());
	glReadPixels(0, 0, projWidth, projHeight, GL_RGBA, GL_FLOAT, 0);
	offscreen_render_proj->setEnabled(false);
	////cv::Mat	statusMat = ps->coord_trans->bufferChecker<GLfloat>(ps->coord_trans->getVBO_STATUS(), GL_ARRAY_BUFFER, GL_FLOAT, camWidth, camHeight);
	//saveTexture(txo_diff, "adaptiveLC/diff", projWidth, projHeight);

	// �V�F�[�_�ŎQ�Ƃ��邽��VBO�փR�s�[
	glBindVertexArray(vao);
	bufferCopy(GL_PIXEL_PACK_BUFFER, PBO_projSize->buf(), GL_ARRAY_BUFFER, VBO_errorPixels->buf(), VBO_errorPixels->size());
	glBindVertexArray(0);

	// �|�C���^���w��
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_errorPixels->buf());
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(cv::Vec4f), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_minValiance_Copy->buf());
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(cv::Vec4f), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_shiftIndex_Copy->buf());
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(cv::Vec4f), (void*)0);
	glBindVertexArray(0);


	// �􉽑Ή����G�s�������ɂ��炵�A���˗��v�Z���s��
	for (int i = 0, num = 0; i < agc->shiftVectorMats.size(); ++i, ++num){
		// ��f�Ή������炷
		ps->coord_trans->setShaderLocationShiftVector(true, i);

		// �J�������W�n�ɕϊ�
		coord_trans(txo_camImageTexture, txo_transCamImageTexture);
		// ���˗����ʂ�ێ�
		//lc_projection(brightness, 0.0, 0.0, diff_thresh, txo_agc_maskTexture, txo_k_after, txo_temp_diff);
		getProjectionSurfaceReflectance(0.0, txo_agc_maskTexture, txo_k_after);

		//cv::Mat ref = saveTexture(txo_k_after, "k_tex" + to_string(i), projWidth, projHeight);

		// ���˗��̌v�Z���ʂ�PBO�ɃR�s�[
		offscreen_render_proj->setEnabled(true);
		offscreen_render_proj->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_k_after);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, PBO_projSize->buf());
		glReadPixels(0, 0, projWidth, projHeight, GL_RGBA, GL_FLOAT, 0);
		offscreen_render_proj->setEnabled(false);

		// PBO��TBO�փR�s�[
		bufferCopy(GL_PIXEL_PACK_BUFFER, PBO_projSize->buf(), GL_TEXTURE_BUFFER, TBO_projSize->buf(), PBO_projSize->size());

		// Shift Index ���R�s�[
		glBindVertexArray(vao);
		bufferCopy(GL_COPY_READ_BUFFER, VBO_shiftIndex->buf(), GL_ARRAY_BUFFER, VBO_shiftIndex_Copy->buf(), VBO_shiftIndex->size());
		glBindVertexArray(0);

		// �ŏ����U�l ���R�s�[
		glBindVertexArray(vao);
		bufferCopy(GL_COPY_READ_BUFFER, VBO_minValiance->buf(), GL_ARRAY_BUFFER, VBO_minValiance_Copy->buf(), VBO_minValiance->size());
		glBindVertexArray(0);

		/*
		**	transform feedback �ɂ��ŏ����U�l�̌v�Z
		**	�v�Z�̏ڍׂ� getValiance.vert ���Q��
		*/
		shader_getValiance->setEnabled(true);
		glUniform1i(shl_getV_correntIndex, i);	// uniform�ϐ��̒l���X�V
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnable(GL_RASTERIZER_DISCARD);	//���X�^���C�U��~���Čv�Z����
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO_minValiance->buf());
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, VBO_shiftIndex->buf());	//�C���f�b�N�X�ԍ��͏o�͐�ƍ��킹��
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, VBO_valiance->buf());		//�C���f�b�N�X�ԍ��͏o�͐�ƍ��킹��
		glActiveTexture(GL_TEXTURE0);	// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
		glBindTexture(GL_TEXTURE_BUFFER, TBO_projSize->tex());
		glBeginTransformFeedback(GL_POINTS);	// Transform Feedback �J�n
		glDrawArrays(GL_POINTS, 0, projWidth * projHeight);
		glEndTransformFeedback();	// Transform Feedback �I��
		glDisable(GL_RASTERIZER_DISCARD);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glBindVertexArray(0);
		shader_getValiance->setEnabled(false);

	}
	// ��f�Ή��V�t�g�𖳌���
	ps->coord_trans->setShaderLocationShiftVector(false);

	/*
	**	transform feedback �ɂ��ړ��ʂ̊O��l���o
	**	�v�Z�̏ڍׂ� errorTest.vert ���Q��
	*/
	// �ŏ����U�l���Z�o�����ړ��ʂ̔ԍ���TBO�փR�s�[
	glBindVertexArray(vao);
	bufferCopy(GL_ARRAY_BUFFER, VBO_shiftIndex->buf(), GL_TEXTURE_BUFFER, TBO_projSize->buf(), VBO_shiftIndex->size());
	glBindVertexArray(0);
	// �V�F�[�_�Ōv�Z
	shader_errorTest->setEnabled(true);
	//for (int i = 0; i < 2; i++){
	glBindVertexArray(vao);
	glEnable(GL_RASTERIZER_DISCARD);	//���X�^���C�U��~���Čv�Z����
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO_shiftIndex->buf());
	glActiveTexture(GL_TEXTURE0);	// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	glBindTexture(GL_TEXTURE_BUFFER, TBO_projSize->tex());
	glBeginTransformFeedback(GL_POINTS);	// Transform Feedback �J�n
	glDrawArrays(GL_POINTS, 0, projWidth * projHeight);
	glEndTransformFeedback();	// Transform Feedback �I��
	glDisable(GL_RASTERIZER_DISCARD);
	glBindVertexArray(0);
	//}
	shader_errorTest->setEnabled(false);

	/*
	**	�ŏ����U�l���Z�o�����ԍ������f�Ή��̈ړ���(pixel�l)�����߂�B
	**  ���܂����ԍ��̓v���W�F�N�^���W�ł��邪�C�ړ��ʂ̓J�������W���x�[�X�Ȃ̂ŁC
	**  �Ή��}�b�v���g���ăJ�������W�ɕϊ�����
	**	�v�Z�̏ڍׂ� getShiftVector.vert ���Q��
	*/
	// �ŏ����U�l���Z�o�����ړ��ʂ̔ԍ���TBO�փR�s�[
	glBindVertexArray(vao);
	bufferCopy(GL_ARRAY_BUFFER, VBO_shiftIndex->buf(), GL_TEXTURE_BUFFER, TBO_projSize->buf(), VBO_shiftIndex->size());
	glBindVertexArray(0);
	// �V�F�[�_�Ōv�Z
	shader_getShiftVec->setEnabled(true);
	unsigned int loop = searchSize;
	// ���_�V�F�[�_�֓��͂��钸�_�o�b�t�@�I�u�W�F�N�g��ύX
	ps->coord_trans->feedbackSetVertexPointerForGetOffsetResult(0);
	// �v�Z�����s
	for (unsigned int i = 0; i < loop; ++i)
	{
		// uniform�ϐ��̍X�V
		glUniform1i(shl_getShift_correntIndex, i);
		// ��f�Ή��̈ړ��ʂ̐ݒ�
		ps->coord_trans->setShaderLocationShiftVector(true, i);
		// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, TBO_projSize->tex());
		ps->coord_trans->feedbackCalclateForGetOffsetResult();
		// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}
	shader_getShiftVec->setEnabled(false);


	//----------���ʂ�Ή��}�b�v��offset�l�֔��f������--------------

	/* ���܂����I�t�Z�b�g�l���e�N�X�`���o�b�t�@�I�u�W�F�N�g�փR�s�[ */
	glBindVertexArray(ps->coord_trans->getVAO());
	bufferCopy(GL_ARRAY_BUFFER, ps->coord_trans->getVBO_STATUS(), GL_TEXTURE_BUFFER, TBO_vboStatus->buf(), TBO_vboStatus->size());
	glBindVertexArray(0);

	/* ���܂����I�t�Z�b�g�l���e�N�X�`���o�b�t�@�I�u�W�F�N�g�փR�s�[ */
	glBindVertexArray(ps->coord_trans->getVAO());
	bufferCopy(GL_ARRAY_BUFFER, ps->coord_trans->getVBO_OFFSET(), GL_TEXTURE_BUFFER, TBO_offsetCopy->buf(), TBO_offsetCopy->size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* �Ή��}�b�v�̃I�t�Z�b�g�l���X�V */
	shader_mapUpdater->setEnabled(true);
	// ���_�V�F�[�_�֓��͂��钸�_�o�b�t�@�I�u�W�F�N�g��ύX
	ps->coord_trans->feedbackSetVertexPointerForSetNewOffset();
	// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, TBO_vboStatus->tex());
	// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, TBO_offsetCopy->tex());
	// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, TBO_vboMask->tex());
	ps->coord_trans->feedbackCalclateForSetNewOffset();
	// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	shader_mapUpdater->setEnabled(false);

	//----------�Ή��}�b�v�𕽊���������--------------
	int loopNum = 1;
	for (int i = 0; i < loopNum; i++){

		/* �Ή��}�b�v���e�N�X�`���o�b�t�@�I�u�W�F�N�g�փR�s�[ */
		glBindVertexArray(ps->coord_trans->getVAO());
		bufferCopy(GL_ARRAY_BUFFER, ps->coord_trans->getVBO_OFFSET(), GL_TEXTURE_BUFFER, TBO_offsetCopy->buf(), TBO_offsetCopy->size());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		/* �Ή��}�b�v�̃I�t�Z�b�g�l���X�V */
		shader_mapBlur->setEnabled(true);
		// �t�B���^�̎��
		glUniform1i(shl_mapBlur_blurNum, blurNum);
		// �J�[�l���T�C�Y
		//glUniform1i(shl_mapBlur_kernelSize, 9);	//�ϐ�
		//if (i < loopNum - 1){
		//	glUniform1i(shl_mapBlur_blurNum, 0);	//�ϐ�
		//}
		//else if(i < loopNum ){
		//	glUniform1i(shl_mapBlur_blurNum, 1);	//�ϐ�
		//	glUniform1i(shl_mapBlur_kernelSize, 9);	//�ϐ�
		//}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, TBO_offsetCopy->tex());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, TBO_vboMask->tex());
		ps->coord_trans->feedbackCalclateForBlur(ps->coord_trans->getVBO_OFFSET());
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		shader_mapBlur->setEnabled(false);
	}

	// ���_�V�F�[�_�֓��͂��钸�_�o�b�t�@�I�u�W�F�N�g���f�t�H���g�ɖ߂�
	ps->coord_trans->setDefaultVertexPointer();

	//cv::Mat	statusMat = ps->coord_trans->bufferChecker<GLfloat>(ps->coord_trans->getVBO_STATUS(), GL_ARRAY_BUFFER, GL_FLOAT, camWidth, camHeight);
	//cv::Mat	offsetResultMat = ps->coord_trans->bufferChecker<cv::Vec2f>(ps->coord_trans->getVBO_OFFSETRESULT(), GL_ARRAY_BUFFER, GL_FLOAT, camWidth, camHeight);
	//cv::Mat	offsetMat = ps->coord_trans->bufferChecker<cv::Vec2f>(ps->coord_trans->getVBO_OFFSET(), GL_ARRAY_BUFFER, GL_FLOAT, camWidth, camHeight);
	//cv::Mat	maskMat = bufferChecker<GLint>(TBO_vboMask->buf(), GL_TEXTURE_BUFFER, GL_INT, camWidth, camHeight);
	////for (int y = 0; y < camHeight; y++) {
	////	for (int x = 0; x < camWidth; x++) {
	////		cv::Vec2f p = offsetResultMat.ptr<cv::Vec2f>(y)[x];
	////		if (p[1] < 0){
	////			offsetResultMat.ptr<cv::Vec2f>(y)[x][0] = abs(p[1]);
	////			offsetResultMat.ptr<cv::Vec2f>(y)[x][1] = 0;
	////		}
	////	}
	////}
	////cv::Mat tmp;
	////offsetResultMat.convertTo(tmp, CV_8UC2, 255.0);
	////cv::cvtColor(tmp, tmp, CV_BGR5652BGR);
	////cv::imwrite("./offsetMatResult.bmp", tmp);
	////cv::Mat mask = saveTexture(txo_diff, "adaptiveLC/Result/diff", projWidth, projHeight);

	//std::cout << "debug offset Mat" << std::endl;



}

void myGL::getProjectionSurfaceReflectance(float ff, GLuint maskTexture, GLuint texture_k_after)
{
	// �V�F�[�_�̗L��
	shader_lc->setEnabled(true);
	glUniform1f(shl_forgetFactor, ff);

	//-----���˗��v�Z�ɕK�v�ȕ������C�e�N�X�`�����j�b�g�Ƀe�N�X�`�����o�C���h-----
	//�v���W�F�N�^���e�摜
	lumiTexs_RGBA->bind(txo_projImageTexture, TEXTURE_UNIT_PRJIMAGE_ENUM);
	//�J�����摜
	lumiTexs_RGBA->bind(txo_transCamImageTexture, TEXTURE_UNIT_CAMIMAGE_ENUM);
	//�ŏ����@�p�����[�^�̕��q���ꐬ��
	lumiTexs_RGBA32F->bind(txo_numerator_before, TEXTURE_UNIT_NUME_ENUM);
	lumiTexs_RGBA32F->bind(txo_denominator_before, TEXTURE_UNIT_DENOMI_ENUM);
	//�}�X�N�摜
	lumiTexs_RGBA->bind(maskTexture, TEXTURE_UNIT_MASK_ENUM);

	//-----�t���[���o�b�t�@�I�u�W�F�N�g�̐ݒ�-----
	//�����_�����O���ʏo�͗p�e�N�X�`�����擾

	//GLuint diff_obj = lumiTexs_RGBA32F->getTextureObject();
	//�����_�����O���ʁA�ŏ����@�̕��q���ꐬ���AK�̏o�͐��ݒ�
	//(�t���O�����g�V�F�[�_��OUTPUTINDEX_*�ƈ�v�����邱��)
	lc_offscreen_render->setEnabled(true);
	lc_offscreen_render->attachmentTexture(GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, texture_k_after);
	lc_offscreen_render->drawBuffers();
	texture_vertex->render_mesh();
	//FBO�ƃV�F�[�_�𖳌���
	lc_offscreen_render->setEnabled(false);
	shader_lc->setEnabled(false);
}


/**
* @brief   �e��o�b�t�@�𐶐�����֐�
*/
void myGL::getGLBuffers()
{
	// �o�b�t�@�̐���
	// TBO
	TBO_offsetCopy = new glTexBuffObj<cv::Vec2f>(camWidth * camHeight, nullptr, GL_RG32F, GL_DYNAMIC_COPY);
	TBO_vboStatus = new glTexBuffObj<GLfloat>(camWidth * camHeight, nullptr, GL_R32F, GL_DYNAMIC_COPY);
	TBO_vboMask = new glTexBuffObj<GLfloat>(camWidth * camHeight, nullptr, GL_R32F, GL_DYNAMIC_COPY);
	TBO_projSize = new glTexBuffObj<cv::Vec4f>(projWidth * projHeight, nullptr, GL_RGBA32F, GL_DYNAMIC_COPY);
	//TBO_shiftIndex = new glTexBuffObj<cv::Vec4f>(projWidth * projHeight, nullptr, GL_RGBA32F, GL_DYNAMIC_COPY);
	// PBO
	PBO_projSize = new glBuffer<cv::Vec4f>(GL_PIXEL_PACK_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_DRAW);
	// VBO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	VBO_valiance = new glBuffer<cv::Vec4f>(GL_ARRAY_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_DRAW);
	VBO_minValiance = new glBuffer<cv::Vec4f>(GL_ARRAY_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_DRAW);
	VBO_minValiance_Copy = new glBuffer<cv::Vec4f>(GL_ARRAY_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_COPY);
	VBO_shiftIndex = new glBuffer<cv::Vec4f>(GL_ARRAY_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_DRAW);
	VBO_shiftIndex_Copy = new glBuffer<cv::Vec4f>(GL_ARRAY_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_COPY);
	VBO_errorPixels = new glBuffer<cv::Vec4f>(GL_ARRAY_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_COPY);
	glBindVertexArray(0);

	glGenTransformFeedbacks(1, &feedback);	// transformFeedback�p�̃I�u�W�F�N�g�𐶐�����

	PBO_CAMERAIMAGES = new glBuffer<cv::Vec4b>(GL_PIXEL_PACK_BUFFER, projWidth * projHeight, 0, GL_DYNAMIC_DRAW);


}

// �P�x�␳�̃p�����[�^���摜�����ĕۑ�
void myGL::debug_textures()
{
	//debug
	static int num = 0;
	//saveTexture(txo_numerator_before, "_txo_numerator_before" + to_string(num), projWidth, projHeight);
	//saveTexture(txo_numerator_after, "_txo_numerator_after" + to_string(num), projWidth, projHeight);
	//saveTexture(txo_denominator_before, "_txo_denominator_before" + to_string(num), projWidth, projHeight);
	//saveTexture(txo_denominator_after, "_txo_denominator_after" + to_string(num), projWidth, projHeight);
	saveTexture(txo_inputImageTexture, "_txo_inputImageTexture" + to_string(num), projWidth, projHeight);
	saveTexture(txo_projImageTexture, "_txo_projImageTexture" + to_string(num), projWidth, projHeight);
	//saveTexture(txo_camImageTexture, "_txo_camImageTexture" + to_string(num), camWidth, camHeight);
	cv::Mat	camImg = saveTexture(txo_transCamImageTexture, "_txo_transCamImageTexture" + to_string(num), projWidth, projHeight);
	cv::Mat targetImg = saveTexture(txo_targetImageTexture, "_txo_targetImageTexture", projWidth, projHeight);
	saveTexture(txo_k_after, "_txo_k_after" + to_string(num), projWidth, projHeight);
	//saveTexture(txo_diff, "_txo_differ" + to_string(num), projWidth, projHeight, true, true);
	//saveTexture(txo_maskTexture, "_txo_maskImageTexture" + to_string(num), projWidth, projHeight);
	//cv::Mat errormat = targetImg - camImg;
	//writeToExelFile(errormat, "error" + to_string(num));
	//writeToExelFile(camImg, "luminance" + to_string(num));
	//num++;
}


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
cv::Mat myGL::saveTexture(GLuint texId, const string &filename, int w, int h, bool isFlip, bool isWrite, bool isSave)
{
	cv::Mat img(h, w, CV_32FC4);

	glBindTexture(GL_TEXTURE_2D, texId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, img.data);
	glBindTexture(GL_TEXTURE_2D, 0);

	//cv::cvtColor(img, img, CV_RGBA2BGR);
	cv::cvtColor(img, img, CV_RGBA2BGR);


	if (isFlip)
		cv::flip(img, img, 0);// �摜�̔��](x������)

	if (isWrite){
		cv::Mat gray;
		cv::cvtColor(img, gray, CV_BGR2GRAY);
		ofstream ofs("./debugout/CSV/" + filename + ".csv");
		ofs << gray;
	}

	cv::Mat uc_img;
	img.convertTo(img, CV_8UC3, 255.0);
	if (isSave){
		cv::Mat rectImage(img, rect);
		cv::imwrite("./debugout/" + filename + ".bmp", img);
		//cv::imwrite("./debugout/" + filename + "_rect.bmp", rectImage);

	}

	return img;
}

cv::Mat &myGL::textureToMat(GLuint texture, int w, int h)
{
	glBuffer<GLfloat[4]> pbo(GL_PIXEL_PACK_BUFFER, w * h, nullptr, GL_DYNAMIC_COPY);
	offscreenRender fbo;
	fbo.init(w, h);
	// �e�N�X�`����PBO�փR�s�[
	fbo.setEnabled(true);
	fbo.attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo.buf());
	glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, 0);
	// Mat �փR�s�[
	cv::Mat dst(h, w, CV_32F);
	void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (ptr != NULL)
	{
		memcpy(dst.data, ptr, pbo.size());
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	fbo.setEnabled(false);

	return dst;
}
/**
* @brief   �􉽑Ή��t�����s���֐�
*
* @param   isProjection[in]		(true: ���e���s���C false: �O��̏���p����)
*/
void myGL::getCoordinateCorrespondance(bool isProjection)
{
	if (isProjection){
		// �V���b�^�[�X�s�[�h��ύX
		float measureSpeed = pgrCamera->getShutterSpeed_Measure(); //[ms]
		std::cout << "shutter speed :" << measureSpeed << "[ms]" << std::endl;
		pgrCamera->setShutterSpeed(measureSpeed);

		// �f�����e
		ps->code_projection_opengl(window);
		ps->make_thresh();
	}
	ps->getCorrespondence(isProjection);

	// �G�s�������ւ̕ψʗʂ��v�Z
	agc->init(ps->getCoordMap());

	// �ψʗʂ�VBO�Ɋi�[����
	ps->coord_trans->setShiftVector(agc->shiftVectorMats);

	// ��f�Ή��V�t�g�𖳌�
	ps->coord_trans->setShaderLocationShiftVector(false);

	// �f�o�b�O
	if (isProjection) {
		// �J�����摜��ۑ�
		cv::Mat cameraImage;
		pgrCamera->getImage(cameraImage, 0);
		cv::imwrite("./Calibration/projection.jpg", cameraImage);
		cv::Mat cameraImage_prj;
		ps->reshapeCam2Proj(cameraImage, cameraImage_prj);
		cv::imwrite("./Calibration/reshape.jpg", cameraImage_prj);
	}
	getGLBuffers();

	//cv::Mat coordmap_after;
	//ps->readPixCorrespData("pointdata_after.xml", coordmap_after);
	//cv::Mat coordmap_before;
	//ps->readPixCorrespData("pointdata.xml", coordmap_before);

	//coordmap_e = coordmap_after - coordmap_before;
	//coordmap_before.release();
	//coordmap_after.release();
}


/**
* @brief   �J���[�L�����u���[�V�������s���֐�
*
* @param   isProjection[in]		(true: ���e���s���C false: �O��̏���p����)
*/
void myGL::getColorCalibration(bool isProjection)
{
	if (isProjection){
		lc->color_projection(window);
	}

	lc->setColorMixingMatrix();

	// �P�x�␳�p�V�F�[�_�̗L��
	shader_lc->setEnabled(true);

	//-----�P�x�␳�p�p�����[�^�̐ݒ�-----
	cv::Vec3f gamma = lc->getGamma();
	cv::Vec3f cmin = lc->getCminPixel();
	cv::Vec3f cmax = lc->getCmaxPixel();
	cv::Mat cmm = cv::Mat(lc->getCMM());
	cv::Mat cmm_inv = cv::Mat(lc->getCMMinv());
	glUniform1i(shl_inputImageTexture, TEXTURE_UNIT_INPUTIMAGE);	//�e�N�X�`�����j�b�g0
	glUniform1f(shl_inputImage_ScalingAlpha, 1.0);	// �ڕW�摜�̃X�P�[�����O��
	glUniform1f(shl_inputImage_ScalingBeta, 0.0);	// �ڕW�摜�̃X�P�[�����O��
	glUniform1i(shl_prjImageTexture, TEXTURE_UNIT_PRJIMAGE);	// �␳�摜
	glUniform1i(shl_camImageTexture, TEXTURE_UNIT_CAMIMAGE);	// �J�����B�e�摜
	glUniform3f(shl_gamma, gamma[2], gamma[1], gamma[0]);	// �K���}�l
	glUniform3f(shl_b_pixel, cmin[2], cmin[1], cmin[0]);	// �ŏ��P�x�l
	glUniform3f(shl_w_pixel, cmax[2], cmax[1], cmax[0]);	// �ő�P�x�l
	glUniformMatrix3fv(shl_color_mixing_matrix, 1, GL_TRUE, (float *)cmm.data);			//�F�����s��(OpenGL�͗�D��AMat�͍s�D��Ȃ̂�transpose��GL_TRUE�ɂ���)
	glUniformMatrix3fv(shl_color_mixing_matrix_inv, 1, GL_TRUE, (float *)cmm_inv.data);	//�F�����s��̋t�s��
	glUniform1i(shl_numerator_before, TEXTURE_UNIT_NUME);		//���ꐬ��
	glUniform1i(shl_denominator_before, TEXTURE_UNIT_DENOMI);	//���q����
	glUniform1f(shl_forgetFactor, FogetFactor);		//�Y�p�W��

	glUniform1i(shl_mask, TEXTURE_UNIT_MASK);					//�}�X�N�p

	// �P�x�␳�p�V�F�[�_�̖���
	shader_lc->setEnabled(false);

	//-----�e�N�X�`���I�u�W�F�N�g�̍쐬-----
	//RGBA
	txo_inputImageTexture = lumiTexs_RGBA->getTextureObject();
	txo_targetImageTexture = lumiTexs_RGBA->getTextureObject();
	txo_projImageTexture = lumiTexs_RGBA->getTextureObject();
	txo_transCamImageTexture = lumiTexs_RGBA->getTextureObject();
	txo_projImage_after = lumiTexs_RGBA->getTextureObject();
	txo_maskTexture = lumiTexs_RGBA->getTextureObject();
	txo_agc_maskTexture = lumiTexs_RGBA->getTextureObject();
	//RGBA32F
	txo_numerator_before = lumiTexs_RGBA32F->getTextureObject();
	txo_denominator_before = lumiTexs_RGBA32F->getTextureObject();
	txo_numerator_after = lumiTexs_RGBA32F->getTextureObject();
	txo_denominator_after = lumiTexs_RGBA32F->getTextureObject();
	txo_k_after = lumiTexs_RGBA32F->getTextureObject();
	txo_diff = lumiTexs_RGBA32F->getTextureObject();
	// �v���W�F�N�^�𑜓x�̃e�N�X�`�����쐬����
	getInputTexture(imageFileList->getImage(imageIndex), txo_inputImageTexture);

	// �J�����摜��ۑ�
	cv::Mat cameraImage;
	pgrCamera->getImage(cameraImage, 0);
	//lc->getProjectorColorSpaceImage(cameraImage, cameraImage);
	// �F�����s��̋t�s���������
	cv::flip(cameraImage, cameraImage, 0);
	coord_trans(txo_camImageTexture, txo_transCamImageTexture, cameraImage);
	cv::Mat cap = saveTexture(txo_transCamImageTexture, "white_convert", projWidth, projHeight);
	// ���e�̈�̂ݐ؂�o��
	cv::Mat rectCap(cap, rect);
	cv::imwrite("./debugout/projSurface.jpg", rectCap);

	//texObjStock_RGBA32F->uploadToTexture(txo_numerator_before, GL_BGR, GL_UNSIGNED_BYTE, cap.data);

	// �����̔��˗��v���p
	getInputTexture(whiteImage, txo_projImageTexture);
	// �}�X�N�摜�p
	getInputTexture(whiteImage, txo_maskTexture);
	getInputTexture(agc_maskImage, txo_agc_maskTexture, cam_mask);

	// �}�X�N�f�[�^�̏�����
	cv::cvtColor(cam_mask, cam_mask, CV_BGR2GRAY);
	cam_mask.convertTo(cam_mask, CV_32F);
	glBindVertexArray(ps->coord_trans->getVAO());
	glBindBuffer(GL_ARRAY_BUFFER, ps->coord_trans->getVBO_MASK());
	glBufferData(GL_ARRAY_BUFFER, ps->coord_trans->getVBO_MASK_SIZE(), cam_mask.data, GL_DYNAMIC_DRAW);
	glBindVertexArray(0);

	glBindVertexArray(ps->coord_trans->getVAO());
	bufferCopy(GL_ARRAY_BUFFER, ps->coord_trans->getVBO_MASK(), GL_TEXTURE_BUFFER, TBO_vboMask->buf(), TBO_vboMask->size());
	glBindVertexArray(0);

	//float lcSpeed = pgrCamera->getShutterSpeed_LC(); //[ms]
	//std::cout << "shutter speed :" << lcSpeed << "[ms]" << std::endl;
	//pgrCamera->setShutterSpeed(lcSpeed);

	//texObjStock_RGBA32F->uploadToTexture(txo_first_k, GL_BGR, GL_UNSIGNED_BYTE, cv::imread("./Images/���i_�c��.jpg").data);
	// 1��␳�̐��x���グ��
	//texObjStock_RGBA32F->uploadToTexture(txo_numerator_before, GL_BGR, GL_UNSIGNED_BYTE, cv::imread("./debugout/1_txo_numerator_after.bmp").data);
	//texObjStock_RGBA32F->uploadToTexture(txo_denominator_before, GL_BGR, GL_UNSIGNED_BYTE, cv::imread("./debugout/3_txo_denominator_after.bmp").data);
}

/**
* @brief   ���͉摜�̍쐬
*
* @param   inputImage[in]		���摜
* @param   textureObj[in]		���摜
* @param   camGeomImage[in, dst]		���摜
*/
void myGL::getInputTexture(cv::Mat &inputImage, GLuint &textureObj, cv::Mat &camGeomImage)
{
	srcImage = inputImage.clone();
	lc_counter = 0;
	if (srcImage.empty()){
		std::cout << "�摜���ǂݍ��܂�Ă��܂���" << std::endl;
		return;
	}
	// �􉽕␳����ꍇ
	if (geom_flag){
		// �􉽕␳�p�̃e�N�X�`�����쐬
		cv::Mat srcCapImage;
		sq->adaptImageShape(srcImage, srcCapImage);
		srcCapImage.copyTo(camGeomImage);
		cv::flip(srcCapImage, srcCapImage, 0);

		// inputImage���X�V
		coord_trans(txo_camImageTexture, textureObj, srcCapImage);

		srcCapImage.release();
		return;
	}
	// ���]
	cv::flip(srcImage, srcImage, 0);
	// inputImage���X�V
	lumiTexs_RGBA->uploadToTexture(textureObj, GL_BGR, GL_UNSIGNED_BYTE, srcImage.data);

}

/**
* @brief   RLS�̌v�Z�Ɏg�p����C���ꕪ�q�����̃e�N�X�`�����폜
*/
void myGL::resetRLSParameter()
{
	// ��U�폜
	lumiTexs_RGBA32F->uploadToTexture(txo_numerator_before, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
	lumiTexs_RGBA32F->uploadToTexture(txo_denominator_before, GL_BGR, GL_UNSIGNED_BYTE, nullptr);

}

/* ���T�C�Y���̏��� */
void myGL::resize(GLFWwindow *const window, int width, int height)
{
	// �E�B���h�E�S�̂��r���[�|�[�g
	glViewport(0, 0, width, height);

	// this�|�C���^���擾
	myGL *const instance(static_cast<myGL *>(glfwGetWindowUserPointer(window)));
	if (instance != NULL)
	{
		instance->windowWidth = width;
		instance->windowHeight = height;
	}
}

void myGL::swapBuffers()
{

#if SINGLE_BUFFERING
	glFinish();
#else
	// �J���[�o�b�t�@�����ւ���
	glfwSwapBuffers(window);
#endif

	// �C�x���g�����o��
	glfwPollEvents();
}


/**
* @brief   �L�[���͎��ɌĂ΂��R�[���o�b�N�֐�
*/
void myGL::keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	cv::Mat m = cv::imread(RECTMASK_ADDRESS);
	// this�|�C���^���擾
	myGL *const instance = static_cast<myGL*>(glfwGetWindowUserPointer(window));
	if (instance != NULL)
	{
		instance->keyStatus = action;
		if (action == GLFW_RELEASE) {
			instance->keyFunc(key);
		}
	}

}
/**
* @brief   �L�[���͎��ɌĂ΂��֐�
*/
void myGL::keyFunc(int key)
{
	switch (key){
	case GLFW_KEY_0:
		getCoordinateCorrespondance(true);
		getColorCalibration(false);
		loop_flag = true;
		break;
	case GLFW_KEY_1:
		getCoordinateCorrespondance(false);
		getColorCalibration(false);
		loop_flag = true;
		break;
	case GLFW_KEY_2:
		// �􉽕␳ on off
		geom_flag = !geom_flag;
		getInputTexture(imageFileList->getImage(imageIndex), txo_inputImageTexture);
		getInputTexture(whiteImage, txo_maskTexture);
		getInputTexture(agc_maskImage, txo_agc_maskTexture);
		//resetRLSParameter();

		break;

	case GLFW_KEY_3:
		// �P�x�␳ on off
		nonCorrection = !nonCorrection;
		break;

	case GLFW_KEY_4:
		// PGR�����e�X�g
		//getCoordinateCorrespondance(false);
		getColorCalibration(true);
		//ps->code_projection_opengl(window);
		//exit(0);
		break;

	case GLFW_KEY_5:
		// �Ή��}�b�v��������
		ps->coord_trans->resetOffsetVBO();
		break;

	case GLFW_KEY_6:
	{
					   cv::Mat targetImage = saveTexture(txo_targetImageTexture, "./PSNR_target", projWidth, projHeight, true, false, true);
					   cv::Mat capImage = saveTexture(txo_transCamImageTexture, "./PSSNR_cam", projWidth, projHeight, true, false, true);
					   cv::Mat maskImg = saveTexture(txo_agc_maskTexture, "./PSSNR_mask", projWidth, projHeight, true, false, true);

					   //cv::Vec3f MSE = lc->calcMSE(targetImage, capImage, whiteImage);
					   float MSE;
					   float PSNR = lc->calcPSNR<cv::Vec3b>(targetImage, capImage, MSE);
					   float SSIM = lc->calcSSIM(targetImage, capImage);
					   std::cout << "RMSE : " << MSE << std::endl;
					   std::cout << "PSNR : " << PSNR << std::endl;
					   std::cout << "SSIM : " << SSIM << std::endl;
	}
		break;
	case GLFW_KEY_7:
		// adaptiveGeom�e�X�g
		stop_flag = true;
		break;
	case GLFW_KEY_U:
		stop_flag = false;
		//agc_calc_flag = true;
		write_flag = true;
		break;

	case GLFW_KEY_B:
		blurNum != blurNum;
		break;

	case GLFW_KEY_8:
	{
					   static int read_num = 0;
					   std::cout << "file read start" << std::endl;
					   // �f�[�^�̓ǂݍ��݂Ɠ����f�̒l���G�N�Z���֏�������
					   myUtl::getAllFiles fileGetter("./debugout/CSV/MSE", "csv");
					   std::vector<std::string> filename = fileGetter.getFileNameList();

					   //myUtl::getAllFiles fileGetter;
					   //std::vector<std::string> filename = fileGetter.getAllFileName("./debugout/CSV/MSE", "csv");

					   // �t�@�C�������\�[�g
					   std::map<int, std::string> filemap;
					   std::regex re("[0-9]");
					   std::cmatch match;
					   // �擾�����t�@�C�����̔��p�����Ō������擾���C������key�Ƃ��ĘA�z�z��Ƀt�@�C�������i�[
					   for (auto &path : filename){
						   //std::vector<std::string> result;
						   std::string result;
						   std::sregex_iterator iter(path.cbegin(), path.cend(), re);
						   std::sregex_iterator end;
						   for (; iter != end; ++iter) {
							   result += iter->str();
						   }
						   // �A�z�z��֊i�[
						   filemap[atoi(result.c_str())] = path;
					   }

					   // �t�@�C�������擾
					   std::vector<std::string> files;
					   for (auto it = filemap.begin(); it != filemap.end(); ++it){
						   std::cout << it->second << std::endl;
						   files.emplace_back(it->second);
					   }
					   cv::Point points[] =
					   {
						   cv::Point(0, 0),
						   cv::Point(1, 0),
						   cv::Point(3, 0),
						   cv::Point(4, 0),
						   cv::Point(6, 0),
						   cv::Point(7, 0),
						   //cv::Point(5, 0),
						   //cv::Point(6, 0),
						   //cv::Point(445, 612),
						   //cv::Point(395, 553),
						   //cv::Point(660, 624),
						   //cv::Point(238, 567),
						   //cv::Point(272, 240),
					   };
					   std::vector<cv::Point> pointsdata(points, std::end(points));
					   if (readWriteExelFile(files, pointsdata, "MSE/MSE_result" + to_string(read_num)))
						   std::cout << "write finish" << std::endl;
					   read_num++;

	}
		// adaptiveGeom�e�X�g
		break;
	case GLFW_KEY_9:
	{
					   //�Ă΂���FPS��������̂Œ���( bufferChecker �v���O�����v����)
					   cv::Mat	offsetResultMat = glUtl::bufferChecker<cv::Vec2f>(ps->coord_trans->getVBO_OFFSETRESULT(), ps->coord_trans->getVAO(), GL_ARRAY_BUFFER, GL_FLOAT, camWidth, camHeight);
					   cv::Mat	offsetMat = glUtl::bufferChecker<cv::Vec2f>(ps->coord_trans->getVBO_OFFSET(), ps->coord_trans->getVAO(), GL_ARRAY_BUFFER, GL_FLOAT, camWidth, camHeight);
					   cv::Mat	status = glUtl::bufferChecker<GLfloat>(ps->coord_trans->getVBO_STATUS(), ps->coord_trans->getVAO(), GL_ARRAY_BUFFER, GL_FLOAT, camWidth, camHeight);
					   cv::Mat	mask = glUtl::bufferChecker<GLint>(ps->coord_trans->getVBO_MASK(), ps->coord_trans->getVAO(), GL_ARRAY_BUFFER, GL_INT, camWidth, camHeight);
					   for (int y = 0; y < camHeight; y++) {
						   for (int x = 0; x < camWidth; x++) {
							   cv::Vec2f p = offsetResultMat.ptr<cv::Vec2f>(y)[x];
							   if (p[1] < 0){
								   offsetResultMat.ptr<cv::Vec2f>(y)[x][0] = abs(p[1]);
								   offsetResultMat.ptr<cv::Vec2f>(y)[x][1] = 0;
							   }
						   }
					   }
					   //cv::Mat offset_error(camHeight, camWidth, CV_32FC1);
					   //for (int y = 0; y < camHeight; y++) {
					   // for (int x = 0; x < camWidth; x++) {
					   //  cv::Vec2f offsets = offsetMat.ptr<cv::Vec2f>(y)[x];
					   //  cv::Vec2f error_truth = coordmap_e.ptr<cv::Vec2f>(y)[x];
					   //  float x_error = abs(abs(offsets[0]) - abs(error_truth[0]));
					   //  float y_error = abs(abs(offsets[1]) - abs(error_truth[1]));

					   //  float distance = sqrt(pow(x_error, 2.0) + pow(y_error, 2.0));
					   //  offset_error.ptr<float>(y)[x] = distance;
					   // }
					   //}
					   //cv::Mat error_truth = coordmap_e.clone();
					   //writeToExelFile(offset_error, "geom_error", cv::Point(1316, 354), 100);

					   cv::Mat camera;
					   pgrCamera->getImage(camera, 0);
					   cv::Mat diff = saveTexture(txo_diff, "adaptiveLC/Result/diff", projWidth, projHeight);
					   std::cout << "debug offset Mat" << std::endl;

					   offsetResultMat.release();
					   offsetMat.release();
					   status.release();
					   mask.release();
	}
		break;

	case GLFW_KEY_UP:
	{
						float nowSpeed = pgrCamera->getShutterSpeed(); //[ms]
						std::cout << "shutter speed :" << nowSpeed << "[ms]" << std::endl;
						pgrCamera->setShutterSpeed(nowSpeed + 0.1);
	}
		break;

	case GLFW_KEY_DOWN:
	{
						  float nowSpeed = pgrCamera->getShutterSpeed(); //[ms]
						  std::cout << "shutter speed :" << nowSpeed << "[ms]" << std::endl;
						  pgrCamera->setShutterSpeed(nowSpeed - 0.1);
	}
		break;

	case GLFW_KEY_SPACE:
		debug_textures();
		break;

	case GLFW_KEY_RIGHT:
		imageIndex++;
		if (imageIndex >= imageFileList->getFileNum()) imageIndex = 0;
		getInputTexture(imageFileList->getImage(imageIndex), txo_inputImageTexture);
		//resetRLSParameter();
		break;
	case GLFW_KEY_LEFT:
		imageIndex--;
		if (imageIndex < 0) imageIndex = imageFileList->getFileNum() - 1;
		getInputTexture(imageFileList->getImage(imageIndex), txo_inputImageTexture);
		//resetRLSParameter();
		break;

	case GLFW_KEY_Q:
		fogetFactor -= 0.05f;
		break;
	case GLFW_KEY_W:
		fogetFactor += 0.05f;
		break;
	case GLFW_KEY_A:
		brightness -= 0.05f;
		break;
	case GLFW_KEY_S:
		brightness += 0.05f;
		break;
	case GLFW_KEY_Z:
		diff_thresh -= 1;
		break;
	case GLFW_KEY_X:
		diff_thresh += 1;
		break;
	}

}

/*
*	�R���\�[����ʂ֏���\������֐�
*/
void myGL::printInfo()
{
	// FPS��\��
	fps_console->end();
	if (fps_console->show(1000) != 0){
		system("cls");
		// GPU,OpenGL���
		printf("VENDOR= %s \n", glGetString(GL_VENDOR));
		printf("GPU= %s \n", glGetString(GL_RENDERER));
		printf("OpenGL= %s \n", glGetString(GL_VERSION));
		printf("GLSL= %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		std::cout
			<< "------------------------------------" << std::endl
			<< "FPS :" << fps_console->get() << "[fps]" << std::endl
			<< "Foget Factor     :" << fogetFactor << std::endl
			<< "Target scaling   :" << brightness << std::endl
			<< "differ Threshold :" << diff_thresh << std::endl
			<< "------------------------------------" << std::endl;

		std::cout
			<< "�L�[���� : " << std::endl
			<< "0 : �R�[�h���e�����ĉf�����e" << std::endl
			<< "1 : �O��̉�f�Ή�����f�����e" << std::endl
			<< "2 : �􉽕␳��on/off" << std::endl
			<< "3 : �P�x�␳��on/off" << std::endl
			<< "5 : �Ή��}�b�v�̏�����" << std::endl
			<< "6 : �摜�]��(RMSE,SSIM)" << std::endl
			<< "7 : �␳��~" << std::endl
			<< "Q/W : �Y�p�W���̒���" << std::endl
			<< "A/S : ���邳�̒���" << std::endl
			<< "Z/X : �����l�̒���" << std::endl;

	}
	// FPS�v���J�n
	fps_console->begin();
}

bool myGL::writeToExelFile(cv::Mat &src, const std::string &str, cv::Point point, int range)
{
	int width = src.size().width;
	int height = src.size().height;

	//�t�@�C���̏�������
	std::ofstream ofsRF("./debugout/CSV/" + str + ".csv");  // �J�����o�͂��L�^����t�@�C���i�����֐��j
	//�G���[����
	if (ofsRF.fail()){
		std::cout << "ERROR:�����֐�.csv���J���܂���B" << std::endl;
		return false;
	}
	//float sum = 0.0f, ave = 0.0f;
	//int count = 0;
	//for (int y = 0; y < height; y++){
	//	for (int x = 0; x < width; x++) {
	//		if (whiteImage.at<cv::Vec3b>(y, x) == cv::Vec3b(0, 0, 0))
	//			continue;
	//		cv::Vec3f value = src.ptr<cv::Vec3f>(y)[x];
	//		float value_ave = (value[0] + value[1] + value[2]) / 3.0f;
	//		sum += value_ave;
	//		count++;
	//	}
	//}
	//ave = sum / count;
	//ofsRF << ave;
	//ofsRF.close();
	//averageVec.emplace_back(ave);
	if (point != cv::Point(0, 0)){
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++) {
				if (point.x < x && x < point.x + range && point.y < y && y < point.y + range){
					float value = src.ptr<float>(y)[x];
					//float value_ave = (fabsf(value[0]) + fabsf(value[1]) + fabsf(value[2])) / 3.0f;
					ofsRF << value << ",";
				}
			}
			if (point.y < y && y < point.y + range){
				ofsRF << "\n";
			}
		}
	}
	else
	{
		for (int y = 0; y < height; y++){
			for (int x = 0; x < width; x++) {
				cv::Vec3f value = src.ptr<cv::Vec3f>(y)[x];
				float value_ave = (fabsf(value[0]) + fabsf(value[1]) + fabsf(value[2])) / 3.0f;
				ofsRF << value_ave << ",";
			}
			ofsRF << "\n";
		}
	}

	ofsRF.close();
	return true;
}

bool myGL::readWriteExelFile(const std::vector<std::string> &fileLists, std::vector<cv::Point> pointsdata, const std::string &dstStr)
{
	// exel���璊�o�����f�[�^
	int num = pointsdata.size();
	std::cout << num << std::endl;

	std::vector<std::vector<float>> readData(num);

	for (auto &path : fileLists){

		//�t�@�C���̓ǂݍ���
		ifstream ifs(path);
		if (!ifs){
			cout << "���̓G���[";
			return false;
		}

		unsigned int rows = 0;	// �s
		unsigned int cols = 0;	// ��

		string str;
		// �s�̓ǂݍ���
		while (getline(ifs, str)){
			for (int i = 0; i < pointsdata.size(); ++i){

				if (pointsdata[i].y == rows){

					string token;
					istringstream stream(str);
					//1�s�̂����A������ƃR���}�𕪊�����(��̓ǂݍ���)
					while (getline(stream, token, ',')){
						if (pointsdata[i].x == cols){
							//���ׂĕ�����Ƃ��ēǂݍ��܂�邽�ߐ��l�͕ϊ����K�v
							float data = stof(token); //stof(string str) : string��float�ɕϊ�
							readData[i].emplace_back(data);
						}
						cols++;
					}
					cols = 0;
				}
			}
			rows++;
		}
	}

	//�t�@�C���̏�������
	std::ofstream ofsRF("./debugout/CSV/" + dstStr + ".csv");  // �J�����o�͂��L�^����t�@�C���i�����֐��j
	//�G���[����
	if (ofsRF.fail()){
		std::cout << "ERROR:�����֐�.csv���J���܂���B" << std::endl;
		return false;
	}

	for (int i = 0; i < fileLists.size(); i++){
		if (i == 0){
			for (int j = 0; j < pointsdata.size(); ++j){
				ofsRF << "(" << pointsdata[j].x << " " << pointsdata[j].y << ")" << ",";
			}
			ofsRF << "\n";
		}
		for (int j = 0; j < pointsdata.size(); ++j){
			ofsRF << readData[j][i] << ",";
		}
		ofsRF << "\n";
	}
	ofsRF.close();

	return true;
}
void myGL::getProCamDelay()
{

}

/**
* @brief   �v���W�F�N�^�n����J�����n�֍��W�ϊ��s���֐�
* �g��Ȃ�
*/
void myGL::coord_trans_inv(GLuint proj_texobj, GLuint cam_texobj, GLuint mask)
{

	////-----�t�􉽕ϊ��p�V�F�[�_�̓ǂݍ���-----
	//shader_geomInv->init("./shader/geometricTransInv.vert", "./shader/geometricTransInv.frag");
	//shader_geomInv->setEnabled(true);
	//// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	//GLuint shl_geomInvtransMat_vert = getShlWithErrorDetect(shader_geomInv->getHandle(), "transformMatrix_vertex");
	//GLuint shl_geomInvtransMat_tex = getShlWithErrorDetect(shader_geomInv->getHandle(), "transformMatrix_texture");
	//GLuint shl_tex_cam = getShlWithErrorDetect(shader_geomInv->getHandle(), "imageTexture");
	//GLuint shl_tex_mask = getShlWithErrorDetect(shader_geomInv->getHandle(), "maskTexture");
	//GLuint shl_median = getShlWithErrorDetect(shader_geomInv->getHandle(), "MEDIAN");
	//// �ϊ��s����v�Z����
	//glm::mat4 geomInvtransMat_vert = VertexTransformMatrix(camWidth, camHeight);
	//glm::mat4 geomInvtransMat_tex = TextureTransformMatrix(projWidth, projHeight);
	//// �o�[�e�b�N�X�V�F�[�_�Ŏg�p����ϊ��s���]��
	//glUniformMatrix4fv(shl_geomInvtransMat_vert, 1, GL_FALSE, &geomInvtransMat_vert[0][0]);
	//glUniformMatrix4fv(shl_geomInvtransMat_tex, 1, GL_FALSE, &geomInvtransMat_tex[0][0]);
	//// �t���O�����g�V�F�[�_�̕ϐ��̐ݒ�
	//glUniform1i(shl_tex_cam, 0);	//�e�N�X�`�����j�b�g0
	//glUniform1i(shl_tex_mask, 1);	//�e�N�X�`�����j�b�g0
	//glUniform1i(shl_median, ZEROINDEX);	//�e�N�X�`�����j�b�g0
	//shader_geomInv->setEnabled(false);

	//if (!ps->getStatus()) {
	//	std::cerr << "get coordinate correspondance ...." << std::endl;
	//	return;
	//}
	//// �V�F�[�_�̗L��
	////�u���b�N�̔w�i
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//// �E�B���h�E�S�̂��r���[�|�[�g
	//shader_geomInv->setEnabled(true);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, proj_texobj);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, mask);

	//// �I�t�X�N���[�������_�����O�J�n
	//gt_offscreen_render_cam->setEnabled(true);
	//gt_offscreen_render_cam->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cam_texobj);
	//gt_offscreen_render_cam->drawBuffers();
	//ps->coord_trans->render_point();

	////// PBO
	////glReadBuffer(GL_COLOR_ATTACHMENT0);
	////pbo->bind(GL_PIXEL_PACK_BUFFER, true);
	////glReadPixels(0, 0, camWidth, camHeight, GL_RED, GL_FLOAT, 0);

	//// �I�t�X�N���[�������_�����O�I��
	//gt_offscreen_render_cam->setEnabled(false);
	//shader_geomInv->setEnabled(false);


	////// Mat�փR�s�[
	////cv::Mat dst(camHeight, camWidth, CV_32F);
	////void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	////if (ptr != NULL)
	////{
	////	memcpy(dst.data, ptr, camWidth * camHeight * 1 * sizeof(GL_FLOAT));
	////	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	////}
	////glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	////glBindFramebuffer(GL_FRAMEBUFFER, 0);
	////std::cout << fps->getElapsed();

}


//// �摜�ۑ�
//{
//	// Mat �փR�s�[
//	cv::Mat dst(projHeight, projWidth, CV_32FC4);
//	glBindVertexArray(vao);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO_valiance->buf());
//	void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
//	if (ptr != NULL)
//	{
//		memcpy(dst.data, ptr, VBO_valiance->size());
//		glUnmapBuffer(GL_ARRAY_BUFFER);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	cv::flip(dst, dst, 0);
//	cv::Mat a1;
//	dst.copyTo(a1);
//	cv::imwrite("./debugout/adaptiveLC/Result/index.bmp", dst);
//	glBindVertexArray(0);
//}

//// �|�C���^���w��
//glBindVertexArray(vao);
//glBindBuffer(GL_ARRAY_BUFFER, VBO_valiance->buf());
//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(cv::Vec4f), (void*)0);
//glBindBuffer(GL_ARRAY_BUFFER, VBO_minValiance_Copy->buf());
//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(cv::Vec4f), (void*)0);
//glBindBuffer(GL_ARRAY_BUFFER, VBO_shiftIndex_Copy->buf());
//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(cv::Vec4f), (void*)0);
//glBindVertexArray(0);

//// ���U�l���v�Z
////getMinValianceIndex(txo_k_after, txo_diff, txo_valiance);
//saveTexture(txo_valiance, "adaptiveLC/Result/result" + to_string(i), projWidth, projHeight);

//// ���U�̌v�Z���ʂ�PBO�փR�s�[
//offscreen_render_proj->setEnabled(true);
//offscreen_render_proj->attachmentTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txo_valiance);
//glReadBuffer(GL_COLOR_ATTACHMENT0);
//glBindBuffer(GL_PIXEL_PACK_BUFFER, PBO_valiance->buf());
//glReadPixels(0, 0, projWidth, projHeight, GL_RGBA, GL_FLOAT, 0);
//offscreen_render_proj->setEnabled(false);

//// PBO��VBO�փR�s�[
//glBindVertexArray(vao);
//glBindBuffer(GL_PIXEL_PACK_BUFFER, PBO_valiance->buf());
//glBindBuffer(GL_ARRAY_BUFFER, VBO_valiance->buf());
//glCopyBufferSubData(GL_PIXEL_PACK_BUFFER, GL_ARRAY_BUFFER, 0, 0, PBO_valiance->size());
//glBindBuffer(GL_ARRAY_BUFFER, 0);
//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
//glBindVertexArray(0);

//// ��ԍŏ��͌v�Z���ʂ��ŏ����U�l�Ƃ݂Ȃ�
//if (i == 0) {
//	//  Copy To min valiance
//	glBindVertexArray(vao);
//	glBindBuffer(GL_COPY_READ_BUFFER, VBO_valiance->buf());
//	glBindBuffer(GL_ARRAY_BUFFER, VBO_minValiance->buf());
//	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, VBO_valiance->size());
//	glBindVertexArray(0);

//}

//// index Copy
//glBindVertexArray(vao);
//glBindBuffer(GL_COPY_READ_BUFFER, VBO_shiftIndex->buf());
//glBindBuffer(GL_ARRAY_BUFFER, VBO_shiftIndex_Copy->buf());
//glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, VBO_shiftIndex->size());
//glBindVertexArray(0);

//// min valiance Copy
//glBindVertexArray(vao);
//glBindBuffer(GL_COPY_READ_BUFFER, VBO_minValiance->buf());
//glBindBuffer(GL_ARRAY_BUFFER, VBO_minValiance_Copy->buf());
//glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, VBO_minValiance->size());
//glBindVertexArray(0);

//// transform feedback �ɂ��ŏ����U�l�̌v�Z
//shader_getMinVIndex->setEnabled(true);
//glUniform1i(shl_minV_correntIndex, i);	// uniform�ϐ��̒l���X�V
//glBindVertexArray(vao);
//glEnableVertexAttribArray(0);
//glEnableVertexAttribArray(1);
//glEnableVertexAttribArray(2);
//glEnable(GL_RASTERIZER_DISCARD);	//���X�^���C�U��~���Čv�Z����
//glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
//glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO_minValiance->buf());
//glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, VBO_shiftIndex->buf());	//�C���f�b�N�X�ԍ��͏o�͐�ƍ��킹��
//glBeginTransformFeedback(GL_POINTS);	// Transform Feedback �J�n
//glDrawArrays(GL_POINTS, 0, projWidth * projHeight);
//glEndTransformFeedback();	// Transform Feedback �I��
//glDisable(GL_RASTERIZER_DISCARD);
//glDisableVertexAttribArray(0);
//glDisableVertexAttribArray(1);
//glDisableVertexAttribArray(2);
//glBindVertexArray(0);
//shader_getMinVIndex->setEnabled(false);
// ��͂ɗp����e�N�X�`����ۑ�
//results.emplace_back(agc_debugTexture(txo_k[i], num));