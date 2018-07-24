#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include "myParam.h"
/**
 * @brief   OpenGL�̃E�B���h�E�֘A�̏���
 *
 * @note	��Ƀ��C�������̃N���X�Ōp�����Ďg��
 */
class Window
{

protected:
	// �E�B���h�E�̃n���h��
	GLFWwindow	*const	window;

	// �E�B���h�E�̍����ƕ�
	int windowWidth, windowHeight;

	// �L�[�{�[�h�̏��
	int keyStatus;

public:
	// �R���X�g���N�^
	Window(int width = 640,	int	height = 480, const char *title = "Window")
				: window(glfwCreateWindow(width, height, title, NULL, NULL))
				, windowWidth(width)
				, windowHeight(height)
				, keyStatus(GLFW_RELEASE)
	{
		if (window == NULL)
		{
			// �E�B���h�E���쐬�ł��Ȃ�����
			std::cerr << "Can't create GLFW window." << std::endl;
			exit(1);
		}
		// ���݂̃E�B���h�E�������Ώۂɂ���
		glfwMakeContextCurrent(window);

		// �쐬�����E�B���h�E�ɑ΂���ݒ�
		glfwSwapInterval(1);
	}
	
	// �f�X�g���N�^
	virtual	~Window()
	{
		glfwDestroyWindow(window);
	}
	
	// �E�B���h�E�����ׂ����𔻒�
	virtual int	shouldClose()
	{
		return	glfwWindowShouldClose(window) | glfwGetKey(window, GLFW_KEY_ESCAPE);
	}
	

	// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
	virtual void swapBuffers()
	{
#if SINGLE_BUFFERING
		glFinish();
#else
		// �J���[�o�b�t�@�����ւ���
		glfwSwapBuffers(window);
#endif
		// �C�x���g�����o��
		if (keyStatus == GLFW_RELEASE)
			glfwWaitEvents();
		else
			glfwPollEvents();
	}

	// �E�B���h�E�̃n���h���̎擾
	GLFWwindow *const getWindowID() { return window; }


	// ��ʃN���A �}�`�`��̑O�ɌĂяo��
	virtual void clear()
	{
		glViewport(0, 0, windowWidth, windowHeight);

		// �J���[�o�b�t�@�ƃf�v�X�o�b�t�@����������
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// ���̎擾
	int getWidth() { return windowWidth; }
	// �����̎擾
	int getHeight() { return windowHeight; }
};


#endif