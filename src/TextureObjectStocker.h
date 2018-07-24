#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>

using namespace std;

//�e�N�X�`���I�u�W�F�N�g���쐬�A�Ǘ�����N���X
class TextureObjectStocker
{
protected:
	int texWidth, texHeight;		//�e�N�X�`���̕��ƍ���
	GLint internalFmt;					//�e�N�X�`���̓����t�H�[�}�b�g

	vector<GLuint> usingTextures;		//�g�p���̃e�N�X�`���I�u�W�F�N�g
	vector<GLuint> unusingTextures;		//���g�p�̃e�N�X�`���I�u�W�F�N�g


public:
	//�R���X�g���N�^
	//width, height: �e�N�X�`���̕��ƍ���
	//internalFormat: �e�N�X�`���̓����t�H�[�}�b�g(GL_RGB, GL_RGBA32F�Ȃ�)
	TextureObjectStocker(int width, int height, GLint internalFormat);
	~TextureObjectStocker();

	//�e�N�X�`���I�u�W�F�N�g���擾����
	//(���g�p�̃e�N�X�`���I�u�W�F�N�g������΂����Ԃ��A������ΐV���ɍ쐬����)
	GLuint getTextureObject();

	//�g�p���Ȃ��Ȃ����e�N�X�`���I�u�W�F�N�g���A��
	//(���g�p���X�g�ɓo�^�����getTextureObject()���Ăяo�����ۂɍė��p�����)
	void releaseTextureObject(GLuint texobj);

	//�e�N�X�`���ɉ摜���A�b�v���[�h����
	//(���̃N���X�ŊǗ����Ă���e�N�X�`���I�u�W�F�N�g�݂̂ɑ΂��ėL��)
	//texobj: �e�N�X�`���I�u�W�F�N�g
	//format: pixels�̃�������̉摜�`�� (GL_RGB, GL_BGR, GL_RGBA�Ȃ�)
	//type: pixels�̗v�f�̃f�[�^�^ (GL_UNSIGNED_BYTE, GL_FLOAT�Ȃ�)
	void uploadToTexture(GLuint texobj, GLenum format, GLenum type, const GLvoid *pixels);

	void bind(GLuint texobj, GLenum TextureUnit);
	void debug();

};