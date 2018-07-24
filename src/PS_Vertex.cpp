#include "PS_Vertex.h"

// constractor
PS_Vertex::PS_Vertex(unsigned int window_w, unsigned int window_h) : Vertex(window_w, window_h)
{
	windowWidth = window_w;
	windowHeight = window_h;
}


PS_Vertex::~PS_Vertex()
{
}

void PS_Vertex::setBuffers(std::vector<MyVertex> _vertecies, std::vector<MyIndex_TRIANGLES> _indecies)
{

	if (ready){
		// ���_�z��I�u�W�F�N�g���폜����
		glDeleteVertexArrays(1, &vao);
		// ���_�o�b�t�@�I�u�W�F�N�g���폜����
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &indexbuffer);
		glDeleteBuffers(1, &vbo_offset_copy);
	}

	vertex_num = _vertecies.size();

	std::vector<cv::Vec2f> offsetVec;
	std::vector<GLfloat> status;
	for (int i = 0; i < vertex_num; ++i){
		offsetVec.emplace_back(cv::Vec2f(0.0f, 0.0f));
		status.emplace_back(255.0f);
	}

	// ���_�z��I�u�W�F�N�g�̐���
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// ���_�o�b�t�@�I�u�W�F�N�g�̐���
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(MyVertex), &_vertecies[0], GL_DYNAMIC_DRAW);

	// �I�t�Z�b�g�p���_�o�b�t�@�I�u�W�F�N�g�̐���
	glGenBuffers(1, &vbo_offset);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(cv::Vec2f), &offsetVec[0], GL_DYNAMIC_DRAW);

	// �I�t�Z�b�g�̌��ʗp���_�o�b�t�@�I�u�W�F�N�g(�R�s�[�p)�̐���
	glGenBuffers(1, &vbo_offset_result);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_result);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(cv::Vec2f), nullptr, GL_DYNAMIC_COPY);

	// �R�s�[�p���_�o�b�t�@�I�u�W�F�N�g�̐���
	glGenBuffers(1, &vbo_offset_copy);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(cv::Vec2f), nullptr, GL_DYNAMIC_COPY);

	// ��ԊǗ��p�̒��_�o�b�t�@�I�u�W�F�N�g�̐���
	glGenBuffers(1, &vbo_status);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_status);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);

	// �C���f�b�N�X�o�b�t�@�I�u�W�F�N�g�̐���
	GLuint size;
	index_num = (int)(_indecies.size() * sizeof(MyIndex_TRIANGLES) / sizeof(GLuint));
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indecies.size() * sizeof(MyIndex_TRIANGLES), &_indecies[0], GL_STATIC_DRAW);

	// �}�X�N�p
	VBO_mask_camsize = new glUtl::glBuffer<GLfloat>(GL_ARRAY_BUFFER, vertex_num, 0, GL_DYNAMIC_COPY);
	
	// ���̒��_�z��I�u�W�F�N�g�̌�������������
	glBindVertexArray(0);

	// transformFeedback�p�̃I�u�W�F�N�g�𐶐�����
	glGenTransformFeedbacks(1, &feedback);

	
	setDefaultVertexPointer();

	ready = true;
}
void PS_Vertex::setCameraIndex(std::vector<float> _cameraRefer)
{
	if (!ready) {
		std::cerr << "error vbo for camera index is not generate" << std::endl;
		return;
	}

	//glBindVertexArray(vao);
	//// ���_�o�b�t�@�I�u�W�F�N�g�̐���
	//glGenBuffers(1, &vbo_camIndex);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo_camIndex);
	//glBufferData(GL_ARRAY_BUFFER, vertex_num * sizeof(float), &_cameraRefer[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
}

// �`�揈��
void PS_Vertex::render_mesh()
{
	if (!ready){
		std::cout << "PS_Vertex.cpp -> draw() : error" << std::endl;
		return;
	}

	glBindVertexArray(vao);
	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glEnableVertexAttribArray(LOCATION_CAMERA);
	glEnableVertexAttribArray(LOCATION_CAMERAOFFSET);

	glDrawElements(GL_TRIANGLES, index_num, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(LOCATION_PROJECTOR);
	glDisableVertexAttribArray(LOCATION_CAMERA);
	glDisableVertexAttribArray(LOCATION_CAMERAOFFSET);
	glBindVertexArray(0);

}
void PS_Vertex::render_point()
{
	if (!ready){
		std::cout << "PS_Vertex.cpp -> draw() : error" << std::endl;
		return;
	}

	glBindVertexArray(vao);
	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glEnableVertexAttribArray(LOCATION_CAMERA);
	glEnableVertexAttribArray(LOCATION_CAMERAOFFSET);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	glDisableVertexAttribArray(LOCATION_PROJECTOR);
	glDisableVertexAttribArray(LOCATION_CAMERA);
	glDisableVertexAttribArray(LOCATION_CAMERAOFFSET);
}

// �I������
void PS_Vertex::close()
{
	// ���_�z��I�u�W�F�N�g���폜����
	glDeleteVertexArrays(1, &vao);

	// ���_�o�b�t�@�I�u�W�F�N�g���폜����
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &vbo_offset);
	glDeleteBuffers(1, &indexbuffer);
	glDeleteBuffers(shift_num, vbo_shift);
}


//void PS_Vertex::copyVertexBuffer()
//{
//	// �C�����钸�_�o�b�t�@���o�C���h
//	glBindVertexArray(vao);
//	GLint size = 0;
//	glBindBuffer(GL_COPY_READ_BUFFER, vbo);
//	glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);
//
//	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_offset_copy);
//	glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
//
//	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);
//
//	glBindVertexArray(0);
//}

// �G�s�|�[�����ւ̕ψʗʂ�VBO�Ɋi�[����
void PS_Vertex::setShiftVector(std::vector<cv::Mat> &_shiftMatVectors)
{
	//std::cout << "modify vbo" << std::endl;
	shift_num = _shiftMatVectors.size();
	if (ready != true)
		return;

	std::cout << _shiftMatVectors[0].size().area() << ", " << vertex_num << std::endl;
	if (_shiftMatVectors[0].size().area() != vertex_num)
		return;

	// ���_�z��I�u�W�F�N�g���o�C���h
	glBindVertexArray(vao);

	// ���_�o�b�t�@�I�u�W�F�N�g�̐���
	vbo_shift = new GLuint[shift_num];
	// �ړ��ʂ�VBO�𐶐�
	for (int i = 0; i < shift_num; ++i){
		glGenBuffers(1, &vbo_shift[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_shift[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cv::Vec2f) * vertex_num, _shiftMatVectors[i].data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// ���̒��_�z��I�u�W�F�N�g�̌�������������
	glBindVertexArray(0);
}

// ��f�Ή��𐔃s�N�Z���G�s�|�[���������ɃV�t�g������
void PS_Vertex::setShaderLocationShiftVector(bool isEnabled, GLuint num)
{
	if (ready != true)
		return;

	if (num >= shift_num)
		return;

	// ���_�z��I�u�W�F�N�g���o�C���h
	glBindVertexArray(vao);

	// ��f�Ή����ړ�������ꍇ
	if (isEnabled){
		// ���_�o�b�t�@�I�u�W�F�N�g���o�C���h
		glBindBuffer(GL_ARRAY_BUFFER, vbo_shift[num]);
		// �o�[�e�b�N�X�V�F�[�_�̃��P�[�V�����ԍ�3�ɐݒ�
		glVertexAttribPointer(LOCATION_CAMERASHIFT, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��(�@��), �X�g���C�h, �o�b�t�@�I�t�Z�b�g
		// ���_�o�b�t�@�I�u�W�F�N�g���o�C���h����������
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(LOCATION_CAMERASHIFT);
	}
	else{
		glDisableVertexAttribArray(LOCATION_CAMERASHIFT);
	}

	// ���̒��_�z��I�u�W�F�N�g�̌�������������
	glBindVertexArray(0);
}

void PS_Vertex::setDefaultVertexPointer()
{
	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(vao);

	// �􉽕ϊ��p�̒��_�o�b�t�@�I�u�W�F�N�g������
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// �o�[�e�b�N�X�V�F�[�_��0�Ԗڂ�attribute�ϐ������o��(location = 0)
	glVertexAttribPointer(LOCATION_PROJECTOR, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��, �X�g���C�h, �o�b�t�@�I�t�Z�b�g
	// �o�[�e�b�N�X�V�F�[�_��1�Ԗڂ�attribute�ϐ������o��(location = 1)
	glVertexAttribPointer(LOCATION_CAMERA, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), BUFFER_OFFSET(sizeof(cv::Vec2f)));

	// �I�t�Z�b�g�p�̒��_�o�b�t�@�I�u�W�F�N�g������
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset);
	// �o�[�e�b�N�X�V�F�[�_��2�Ԗڂ�attribute�ϐ������o��(location = 2)
	glVertexAttribPointer(LOCATION_CAMERAOFFSET, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��, �X�g���C�h, �o�b�t�@�I�t�Z�b�g

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(0);
}

void PS_Vertex::feedbackCalclateForGetOffsetResult()
{
	//-----�o�b�t�@�̃R�s�[-------

	// �I�t�Z�b�g�p�̒��_�o�b�t�@�I�u�W�F�N�g���R�s�[����
	glBindVertexArray(vao);
	glBindBuffer(GL_COPY_READ_BUFFER, vbo_offset_result);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, vertex_num * sizeof(cv::Vec2f));
	glBindVertexArray(0);

	//-----�o�b�t�@�̌v�Z-------

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(vao);

	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glEnableVertexAttribArray(LOCATION_CAMERASHIFT);
	glEnableVertexAttribArray(LOCATION_OFFSETRESULT);

	// ���X�^���C�U���~�߂Čv�Z�����s����
	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
	// ���܂����I�t�Z�b�g�l���i�[���邽�߂̒��_�o�b�t�@�I�u�W�F�N�g���o�̓^�[�Q�b�g�Ƃ��Ďw�肷��
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_offset_result);
	// Transform Feedback �J�n
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	// Transform Feedback �I��
	glEndTransformFeedback();

	//// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	//glBindTexture(GL_TEXTURE_BUFFER, 0);
	glDisable(GL_RASTERIZER_DISCARD);


	glEnableVertexAttribArray(LOCATION_PROJECTOR);
	glDisableVertexAttribArray(LOCATION_CAMERASHIFT);
	glEnableVertexAttribArray(LOCATION_OFFSETRESULT);

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(0);

}

void PS_Vertex::feedbackSetVertexPointerForGetOffsetResult(unsigned int num)
{
	//-----�o�[�e�b�N�X�|�C���^�[�̐ݒ�-------

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(vao);

	// �I�t�Z�b�g�̃R�s�[�̒��_�o�b�t�@�I�u�W�F�N�g������
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	// �o�[�e�b�N�X�V�F�[�_��1�Ԗڂ�attribute�ϐ������o��(location = 1)
	glVertexAttribPointer(LOCATION_OFFSETRESULT, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��, �X�g���C�h, �o�b�t�@�I�t�Z�b�g

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(0);

}

void PS_Vertex::feedbackSetVertexPointerForSetNewOffset()
{
	//-----�o�[�e�b�N�X�|�C���^�[�̐ݒ�-------

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(vao);

	// ���߂��I�t�Z�b�g�l�̒��_�o�b�t�@�I�u�W�F�N�g������
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	// �o�[�e�b�N�X�V�F�[�_��0�Ԗڂ�attribute�ϐ������o��(location = 0)
	glVertexAttribPointer(LOCATION_CAMERAOFFSET, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��, �X�g���C�h, �o�b�t�@�I�t�Z�b�g

	// ���߂��I�t�Z�b�g�l�̒��_�o�b�t�@�I�u�W�F�N�g������
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_result);
	// �o�[�e�b�N�X�V�F�[�_��0�Ԗڂ�attribute�ϐ������o��(location = 0)
	glVertexAttribPointer(LOCATION_OFFSETRESULT, 2, GL_FLOAT, GL_FALSE, sizeof(cv::Vec2f), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��, �X�g���C�h, �o�b�t�@�I�t�Z�b�g

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(0);

}

void PS_Vertex::feedbackCalclateForSetNewOffset()
{

	//-----�o�b�t�@�̃R�s�[-------

	// �I�t�Z�b�g�p�̒��_�o�b�t�@�I�u�W�F�N�g���R�s�[����
	glBindVertexArray(vao);
	glBindBuffer(GL_COPY_READ_BUFFER, vbo_offset);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset_copy);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, vertex_num * sizeof(cv::Vec2f));
	glBindVertexArray(0);

	//-----�o�b�t�@�̌v�Z-------

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(vao);

	glEnableVertexAttribArray(LOCATION_CAMERAOFFSET);
	glEnableVertexAttribArray(LOCATION_OFFSETRESULT);

	// ���X�^���C�U���~�߂Čv�Z�����s����
	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
	// �I�t�Z�b�g�l�̒��_�o�b�t�@�I�u�W�F�N�g���o�̓^�[�Q�b�g�Ƃ��Ďw�肷��
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo_offset);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, vbo_status);
	// Transform Feedback �J�n
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	// Transform Feedback �I��
	glEndTransformFeedback();

	//// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	glDisable(GL_RASTERIZER_DISCARD);

	glDisableVertexAttribArray(LOCATION_CAMERAOFFSET);
	glDisableVertexAttribArray(LOCATION_OFFSETRESULT);

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(0);
}


void PS_Vertex::feedbackCalclateForBlur(GLuint vbo)
{

	//-----�o�[�e�b�N�X�|�C���^�[�̐ݒ�-------

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(vao);

	// ���߂��I�t�Z�b�g�l�̒��_�o�b�t�@�I�u�W�F�N�g������
	glBindBuffer(GL_ARRAY_BUFFER, VBO_mask_camsize->buf());
	// �o�[�e�b�N�X�V�F�[�_��0�Ԗڂ�attribute�ϐ������o��(location = 0)
	glVertexAttribPointer(0, 1, GL_INT, GL_FALSE, sizeof(GLint), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��, �X�g���C�h, �o�b�t�@�I�t�Z�b�g

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(0);

	//-----�o�b�t�@�̌v�Z-------

	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(vao);

	// ���X�^���C�U���~�߂Čv�Z�����s����
	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback);
	// �I�t�Z�b�g�l�̒��_�o�b�t�@�I�u�W�F�N�g���o�̓^�[�Q�b�g�Ƃ��Ďw�肷��
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);
	// Transform Feedback �J�n
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, vertex_num);

	// Transform Feedback �I��
	glEndTransformFeedback();

	//// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�̃e�N�X�`������������
	glDisable(GL_RASTERIZER_DISCARD);


	// ���_�z��I�u�W�F�N�g������
	glBindVertexArray(0);
}

/*
*	�Ή��}�b�v������������֐�
*/
void PS_Vertex::resetOffsetVBO()
{
	std::vector<cv::Vec2f> offsetVec;
	for (int i = 0; i < vertex_num; ++i){
		offsetVec.emplace_back(cv::Vec2f(0.0f, 0.0f));
	}
	// �I�t�Z�b�g�p�̒��_�o�b�t�@�I�u�W�F�N�g���R�s�[����
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_offset);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_num * sizeof(cv::Vec2f), &offsetVec[0]);
	glBindVertexArray(0);
}