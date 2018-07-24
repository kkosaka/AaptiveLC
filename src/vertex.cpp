#include "vertex.h"

// �e�N�X�`���`��p�̃t���[���T�C�Y�̃|���S����p��
void Vertex::init()
{
	vertecies.clear();
	indecies.clear();

	// �s�N�Z���l�����炷���߂̕ϐ�
	//texel_x = 0.0f;
	//texel_y = 0.0f;

	// ���_���W�̐ݒ�
	GLfloat g_vertex_buffer_data[][2] = {
		{ -1.0f, 1.0f},	// ���� 0
		{ -1.0f, -1.0f },	// ���� 1
		{ 1.0f, 1.0f },	// �E�� 2
		{ 1.0f, -1.0f },	// �E�� 3
	};
	//GLfloat g_vertex_buffer_data[][2] = {
	//	{ 0.0f, 0.0f },	// ���� 1
	//	{ 0.0f, (float)windowHeight },// ���� 0
	//	{ (float)windowWidth, 0.0f },// �E�� 3
	//	{ (float)windowWidth, (float)windowHeight },// �E�� 2
	//};
	int vertex_num = sizeof(g_vertex_buffer_data) / sizeof(g_vertex_buffer_data[0]);
	// �e���_���W�̒l�ƁC����ɑΉ�����e�N�X�`�����W��z��Ɋi�[
	for (int i = 0; i < vertex_num; ++i) {
		//// ���_���W
		//cv::Vec2f pos = cv::Vec2f(g_vertex_buffer_data[i]);
		//vertecies.emplace_back(pos);
		cv::Vec2f pos = cv::Vec2f(g_vertex_buffer_data[i]);
		// �s�N�Z���ƃe�N�Z���̈Ⴂ���l�����C���W�l�����炷
		cv::Vec2f pos_texel = cv::Vec2f(pos[0] - texel_x, pos[1] - texel_y);
		// ���_���W�ɑΉ�����e�N�X�`�����W���v�Z
		cv::Vec2f tex = cv::Vec2f(pos[0] * 0.5f + 0.5f, pos[1] * 0.5f + 0.5f);
		// �z��Ɋi�[
		vertecies.emplace_back(pos_texel, tex);
	}

	// ���_���W�ɑ΂���`�揇��ݒ�(�O�p���b�V���ŕ\��)
	GLuint g_index_buffer_data_triangles[][3] = {
		{ 0, 1, 2 },
		{ 3, 2, 1 },
	};
	int num = sizeof(g_index_buffer_data_triangles) / sizeof(g_index_buffer_data_triangles[0]);
	for (int i = 0; i < num; ++i) {
		indecies.emplace_back(
			MyIndex_TRIANGLES(
			g_index_buffer_data_triangles[i][0],
			g_index_buffer_data_triangles[i][1],
			g_index_buffer_data_triangles[i][2]
			)
			);
	}

	this->genVAO();
}

void Vertex::genVAO()
{
	if (ready){
		// ���_�z��I�u�W�F�N�g���폜����
		glDeleteVertexArrays(1, &vao);
		// ���_�o�b�t�@�I�u�W�F�N�g���폜����
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &indexbuffer);
	}

	// ���_�z��I�u�W�F�N�g�̐���
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// ���_�o�b�t�@�I�u�W�F�N�g�̐���
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MyVertex)* vertecies.size(), &vertecies[0], GL_DYNAMIC_DRAW);

	// �o�[�e�b�N�X�V�F�[�_�̂O�Ԗڂ�attribute�ϐ������o��(location = 0)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)0); // location�ԍ�, �T�C�Y, �^, ���K��, �X�g���C�h, �o�b�t�@�I�t�Z�b�g
	// �o�[�e�b�N�X�V�F�[�_�̂P�Ԗڂ�attribute�ϐ������o��(location = 1)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MyVertex), BUFFER_OFFSET(sizeof(cv::Vec2f)));

	// �C���f�b�N�X�o�b�t�@�I�u�W�F�N�g�̐���
	index_num = (int)(indecies.size() * sizeof(MyIndex_TRIANGLES) / sizeof(GLuint));
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indecies.size() * sizeof(MyIndex_TRIANGLES), &indecies[0], GL_STATIC_DRAW);

	// ���̒��_�z��I�u�W�F�N�g�̌�������������
	glBindVertexArray(0);

	ready = true;

}

void Vertex::render_mesh()
{
	if (!ready){
		std::cout << "vertex.cpp -> draw() : error" << std::endl;
		return;
	}

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawElements(GL_TRIANGLES, index_num, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}


void Vertex::close()
{
	// ���_�z��I�u�W�F�N�g���폜����
	glDeleteVertexArrays(1, &vao);

	// ���_�o�b�t�@�I�u�W�F�N�g���폜����
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &indexbuffer);
}

