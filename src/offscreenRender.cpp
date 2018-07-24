#include "offscreenRender.h"

bool offscreenRender::init(int width, int height)
{
	offscreen_width = width;
	offscreen_height = height;

	// �����_�[�o�b�t�@(RGB)�̍쐬
	glGenRenderbuffers(1, &rboID[0]);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, width, height);		// �������̊m��
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// �����_�[�o�b�t�@(depth)�̍쐬
	glGenRenderbuffers(1, &rboID[1]);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);		// �������̊m��
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// �e�N�X�`���o�b�t�@(RGB)�̍쐬
	glGenTextures(1, &texID[0]);
	glBindTexture(GL_TEXTURE_2D, texID[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	// �e�N�X�`���o�b�t�@(depth)�̍쐬
	glGenTextures(1, &texID[1]);
	glBindTexture(GL_TEXTURE_2D, texID[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);


	// �t���[���o�b�t�@�̍쐬(�f�t�H���g)
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboID[0]);	// �����_�[�o�b�t�@�ƑΉ��t����
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID[1]);	// �����_�[�o�b�t�@�ƑΉ��t����
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID[0], 0);	// �e�N�X�`���o�b�t�@�ƑΉ��t����
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texID[1], 0);	// �e�N�X�`���o�b�t�@�ƑΉ��t����
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	renderTargets.emplace_back(GL_COLOR_ATTACHMENT0);
	// FBO���ł��Ă��邩�̃`�F�b�N
	if (checkFramebufferStatus() == false){
		return false;
	}

	ready = true;

	return true;
}

void offscreenRender::attachmentTexture(GLenum ATTACHMENT, GLenum target, GLenum texid)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, ATTACHMENT, target, texid, 0);

	if (!checkAttachment(ATTACHMENT))
		renderTargets.emplace_back(ATTACHMENT);
}

void offscreenRender::attachmentRenderBuffer(GLenum ATTACHMENT, GLenum target, GLenum rboid)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, ATTACHMENT, target, rboid);

	if (!checkAttachment(ATTACHMENT))
		renderTargets.emplace_back(ATTACHMENT);
}

void offscreenRender::clearAttachment()
{
	renderTargets.clear();
	renderTargets.emplace_back(GL_COLOR_ATTACHMENT0);

}

void offscreenRender::drawBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, offscreen_width, offscreen_height);
	glDrawBuffers((GLsizei)renderTargets.size(), &renderTargets[0]);
}

bool offscreenRender::checkAttachment(GLenum ATTACHMENT)
{
	bool isEnable = false;
	for (int i = 0; i < renderTargets.size(); ++i) {
		if (renderTargets[i] == ATTACHMENT)
			isEnable = true;
	}

	return isEnable;
}
/**
* @brief   �t���[���o�b�t�@�̎擾�󋵂̊m�F
*
* @returns �����������ǂ���
*/
bool offscreenRender::checkFramebufferStatus()
{
	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		std::cout << "Framebuffer complete." << std::endl;
		return true;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
		return false;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
		return false;

	default:
		std::cout << "[ERROR] Unknow error." << std::endl;
		return false;
	}
}

void offscreenRender::setEnabled(bool enabled)
{
	if (enabled && ready){
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		return;
	}

	glDrawBuffer(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}