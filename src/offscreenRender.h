#ifndef OFFSCREENRENDER_H
#define OFFSCREENRENDER_H

#pragma once

#include <iostream>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#pragma comment (lib,"glew32.lib")

class offscreenRender
{
public:
	offscreenRender()
	{
		ready = false;
	};
	~offscreenRender()
	{
		glDeleteFramebuffers(1, &fboID);
		glDeleteTextures(2, texID);
		glDeleteRenderbuffers(2, rboID);
	};

	bool init(int width, int height);

	// �I�t�X�N���[�������_�����O��on/off
	void setEnabled(bool enabled);

	// �w��̃A�^�b�`�����g��Ƀe�N�X�`����o�^����
	void attachmentTexture(GLenum ATTACHMENT, GLenum target, GLenum id);
	
	// �w��̃A�^�b�`�����g��Ƀ����_�[�o�b�t�@�o�^����
	void attachmentRenderBuffer(GLenum ATTACHMENT, GLenum target, GLenum id);
	
	void clearAttachment();

	// �`�悷��
	void drawBuffers();

	int getWidth(){ return offscreen_width; };
	int getHeight(){ return offscreen_height; };
	GLuint getFbo(){ return fboID; };
	
private:
	// �����o�ϐ�
	GLuint fboID;
	GLuint rboID[2];
	GLuint texID[2];

	// �I�t�X�N���[����̃X�N���[���T�C�Y
	int offscreen_width;
	int offscreen_height;

	bool ready;

	// �t���[���o�b�t�@�̃X�e�[�^�X���`�F�b�N����֐�
	bool checkFramebufferStatus();

	// �A�^�b�`�����g�����łɓo�^����Ă��邩���ׂ�֐�
	bool checkAttachment(GLenum ATTACHMENT);

	// �����_�����O���o�^����x�N�^�[
	std::vector<GLenum> renderTargets;

};

#endif
