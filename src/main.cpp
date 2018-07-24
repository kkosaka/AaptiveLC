#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "PGRCameraThread.h"
#include "LC_main.h"
#include "monitor.h"

// glfw��pragma��
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glew��pragma��
#pragma comment(lib,"glew32.lib")

// �ŏ������Ă΂��֐��i�T�u�X���b�h��run�̒��ŌĂԁj
int initCallFunc()
{
	// GLFW�̏�����
	if (!glfwInit())
	{
		std::cerr << "Can't initilize GLFW" << std::endl;
		return -1;
	}

	// OpenGL�̃o�[�W�����̑I��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if SINGLE_BUFFERING
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
#endif

	// �J�����X���b�h�̊J�n
	PGRCamera *pgrCamera = new PGRCamera(FlyCapture2::PIXEL_FORMAT_BGR, FlyCapture2::HQ_LINEAR);

#if FULLSCREEN 

	// ���j�^�[�����o
	Monitor::SearchDisplay();
	if (PROJECT_MONITOR_NUMBER > Monitor::Disps_Prop.size() - 1)
	{
		std::cerr << "�f�B�X�v���C" << PROJECT_MONITOR_NUMBER << "��������܂���" << std::endl;
		exit(0);
	}

	Monitor::Disp_Prop dispProp = Monitor::Disps_Prop[PROJECT_MONITOR_NUMBER];		// �f�B�X�v���C���w��

	// �g�����`��
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	myGL mygl(pgrCamera, dispProp.width, dispProp.height);
	glfwSetWindowPos(mygl.getWindowID(), dispProp.x, dispProp.y);	// �E�B���h�E�ʒu
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
#else
	myGL mygl(pgrCamera);
	glfwSetWindowPos(mygl.getWindowID(), 2000, 200);	// �E�B���h�E�ʒu
#endif


	// GPU,OpenGL���
	printf("VENDOR= %s \n", glGetString(GL_VENDOR));
	printf("GPU= %s \n", glGetString(GL_RENDERER));
	printf("OpenGL= %s \n", glGetString(GL_VERSION));
	printf("GLSL= %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));


	if (!mygl.init())
		return 0;

	while (mygl.shouldClose() == GL_FALSE)
	{
		// �`��
		mygl.display();

	}

	glfwTerminate();
	return 0;
}

int main()
{
	initCallFunc();

	return 0;
}
