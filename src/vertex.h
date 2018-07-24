#ifndef VERTEX_H
#define VERTEX_H
#pragma once

#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

// glfw��pragma��
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glew��pragma��
#pragma comment(lib,"glew32.lib")

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define BUFFER_OFFSET(bytes) ((GLubyte *)NULL + (bytes))

#include "FPSMajor.h"

class Vertex {

public:
	Vertex(unsigned int width, unsigned int height)
	{
		windowWidth = width;
		windowHeight = height;

		texel_x = 0.5f / (((float)windowWidth - 1.0f) * 2.0f);
		texel_y = 0.5f / (((float)windowHeight - 1.0f) * 2.0f);

		fps = new myUtl::FPSMajor(1);

		ready = false;
	};

	// ���_���̒��_���W�ƃe�N�X�`�����W���܂Ƃ߂邽�߂̍\����
	virtual struct MyVertex{
		MyVertex(cv::Vec2f pos = cv::Vec2f(0, 0), cv::Vec2f tex = cv::Vec2f(0, 0))
		: Pos(pos), Tex(tex){};
		cv::Vec2f Pos;
		cv::Vec2f Tex;
	};

	// �O�p���b�V���̒��_�C���f�b�N�X��ێ�����\����
	struct MyIndex_TRIANGLES{
		MyIndex_TRIANGLES(GLuint v1, GLuint v2, GLuint v3)
		{
			index[0] = v1;
			index[1] = v2;
			index[2] = v3;
		};
		GLuint index[3];
	};

	// ������
	// glew�����������Ă���g�p���邱��
	virtual void init();

	// �����o�ϐ���vertex-index buffer ����VBO�𐶐�����
	virtual void genVAO();

	// display�֐����ŌĂ΂��֐�
	virtual void render_mesh();

	// �I�����ɌĂ΂��֐�
	virtual void close();

	virtual cv::Point2f getTexel(){ return texel_x, texel_y; };

	GLuint getVAO(){ return vao; };
	GLuint getVBO(){ return vertexbuffer; };
	GLuint getIBO(){ return indexbuffer; };
	bool getReady(){ return ready; }
	int  getVertexSize(){ return vertecies.size(); };


	myUtl::FPSMajor *fps;

private:
	unsigned int windowWidth;
	unsigned int windowHeight;

	// �e�N�X�`���}�b�s���O���̃s�N�Z���ƃe�N�Z���̂����
	float texel_x;
	float texel_y;

	// ���_�z��I�u�W�F�N�g(0�F�􉽕␳�A1:�􉽕␳�Ȃ�)
	GLuint vao;
	// ���_�o�b�t�@�I�u�W�F�N�g
	GLuint vertexbuffer;
	// �C���f�b�N�X�o�b�t�@�I�u�W�F�N�g
	GLuint indexbuffer;
	//std::vector<GLuint> shift_vbo;

	// ���_���
	std::vector<MyVertex> vertecies;
	std::vector<MyIndex_TRIANGLES> indecies;

	// �`�悷��C���f�b�N�X��
	int index_num;

	// �`�揀�����ł��Ă��邩�ǂ���
	bool ready;

};

#endif