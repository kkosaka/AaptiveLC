#ifndef GLUTL_H
#define GLUTL_H

#pragma once

#include <string>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// glfw��pragma��
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glew��pragma��
#pragma comment(lib,"glew32.lib")

namespace glUtl{

	/*
	** �o�b�t�@�I�u�W�F�N�g
	**
	**    ���_�^�C���f�b�N�X�o�b�t�@�I�u�W�F�N�g�̊��N���X
	*/
	template <typename T> class glBuffer
	{
		// �o�b�t�@�I�u�W�F�N�g
		GLuint buffer;

		// �f�[�^��
		unsigned int number;

		// �f�[�^�̃T�C�Y
		unsigned int data_size;

		// �^�[�Q�b�g
		GLenum buff_target;
	public:

		// �f�X�g���N�^
		virtual ~glBuffer<T>(void)
		{
			glDeleteBuffers(1, &buffer);
		}

		// �R���X�g���N�^
		glBuffer<T>(void)
			: number(0)
		{
			glGenBuffers(1, &buffer);
		}

		// �R���X�g���N�^
		glBuffer<T>(GLenum target, unsigned int num, const T *data, GLenum usage = GL_STATIC_DRAW)
		{
			glGenBuffers(1, &buffer);
			load(target, num, data, usage);
		}

		// �R�s�[�R���X�g���N�^
		glBuffer<T>(const glBuffer<T> &o)
		{
			buffer = o.buffer;
			number = o.number;
			data_size = o.data_size;
		}

		// ������Z�q
		glBuffer<T> &operator=(const glBuffer<T> &o)
		{
			if (this != &o)
			{
				glDeleteBuffers(1, &buffer);
				buffer = o.buffer;
				number = o.number;
				data_size = o.data_size;
			}

			return *this;
		}

		// �f�[�^�̎擾
		void load(GLenum target, unsigned int num, const T *data, GLenum usage = GL_STATIC_DRAW)
		{
			this->number = num;
			this->data_size = num * sizeof(T);
			this->buff_target = target;
			glBindBuffer(target, buffer);
			glBufferData(target, sizeof (T)* num, data, usage);
			glBindBuffer(target, 0);
		}

		// �o�b�t�@�I�u�W�F�N�g���𓾂�
		GLuint buf(void) const
		{
			return buffer;
		}

		// �f�[�^�̐��𓾂�
		unsigned int num(void) const
		{
			return number;
		}
		// �f�[�^�̐��𓾂�
		unsigned int size(void) const
		{
			return data_size;
		}
		GLenum target(void) const
		{
			return this->buff_target;
		}
	};

	/*
	** �e�N�X�`��
	**
	**    ���_�^�C���f�b�N�X�o�b�t�@�I�u�W�F�N�g�̊��N���X
	*/
	class glTexture
	{
		// �e�N�X�`����
		GLuint texture;
		// �e�N�X�`���̃o�C���h���̃^�[�Q�b�g
		GLenum target;
		// �e�N�X�`���t�H�[�}�b�g
		GLenum internalFormat;

		int imageWidth;
		int imageHeight;

		// �f�[�^�T�C�Y
		int dataSize;

	public:

		// �f�X�g���N�^
		virtual ~glTexture(void)
		{
			glDeleteTextures(1, &texture);
		}

		// �R���X�g���N�^
		glTexture(void)
			:dataSize(0)
		{
			glGenTextures(1, &texture);
		}

		// �R���X�g���N�^(�f�[�^����e�N�X�`�����쐬����)
		glTexture(
			cv::Mat &src,						// �f�[�^
			GLenum _target = GL_TEXTURE_2D,		// �e�N�X�`���̃^�C�v(GL_TEXTURE2D�Ȃ�)
			GLenum _textureformat = GL_RGB,     // �e�N�X�`���̃t�H�[�}�b�g (GL_RGB/GL_RGBA/GL_RGBA32F�Ȃ�)
			GLenum _matFormat = GL_BGR,			// �摜�t�@�C���̃t�H�[�}�b�g(GL_BGR)
			GLenum _matType = GL_UNSIGNED_BYTE	// �摜�t�@�C���̃f�[�^�^�C�v(gl_unsigned_byte, gl_float, gl_int)
			)
		{
			glGenTextures(1, &texture);
			loadFromMat(src, _target, _textureformat, _matFormat, _matType);
		}

		// �R���X�g���N�^(�t�@�C������ǂݍ��݃e�N�X�`�����쐬����)
		glTexture(
			const std::string& FileName,               // �摜�t�@�C�����i3/4 �`���l���� RAW �摜�j
			GLenum _target = GL_TEXTURE_2D,
			GLenum _format = GL_RGBA
			)
		{
			glGenTextures(1, &texture);
			load(FileName, _target, _format);
		}


		// �R�s�[�R���X�g���N�^
		glTexture(const glTexture &o)
			: texture(o.texture) {}

		// ������Z�q
		glTexture &operator=(const glTexture &o)
		{
			if (this != &o)
			{
				glDeleteTextures(1, &texture);
				texture = o.texture;
			}

			return *this;
		}

		//    filename: �t�@�C����, width, height: ���ƍ��� (2^n), format: GL_RGB �� GL_RGBA
		void load(const std::string& FileName, GLenum target = GL_TEXTURE_2D, GLenum internalFormat = GL_RGBA);
		void loadFromMat(cv::Mat &src, GLenum target = GL_TEXTURE_2D, GLenum format = GL_RGB, GLenum matFormat = GL_BGR, GLenum matType = GL_UNSIGNED_BYTE);
		void upload(const GLvoid *data, GLenum dataFormat = GL_BGR, GLenum dataType = GL_UNSIGNED_BYTE);

		// �f�[�^����̃e�N�X�`�����쐬
		// getTexture<�f�[�^�̌^>(width�Cheight, �o�C���h��̃^�[�Q�b�g, �^�ɑΉ�����GL�̃t�H�[�}�b�g)
		template <typename T> void getTexture(int width, int height, GLenum _target, GLenum _internalFormat, GLenum dataFomat = GL_BGR, GLenum dataType = GL_UNSIGNED_BYTE, const void *data = 0)
		{
			imageWidth = width;
			imageHeight = height;
			target = _target;
			internalFormat = _internalFormat;
			dataSize = width * height *  sizeof(T);

			// �e�N�X�`�������蓖�Ă�
			glBindTexture(_target, tex());
			glTexImage2D(_target, 0, internalFormat, width, height, 0, dataFomat, dataType, data);
			// �o�C���j�A�i�~�b�v�}�b�v�Ȃ��j�C�G�b�W�ŃN�����v
			glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glBindTexture(_target, 0);
		}

		// �e�N�X�`���I�u�W�F�N�g���g�p����
		void bind(bool isBind, GLenum TextureUnit = GL_TEXTURE0)
		{
			if (isBind){
				glActiveTexture(TextureUnit);
				glBindTexture(target, texture);
			}
			else{
				glBindTexture(target, 0);
				glActiveTexture(0);
			}
		}

		// �e�N�X�`���I�u�W�F�N�g�����擾����
		GLuint tex(void) const{ return texture; }
		GLuint num(void) const{ return imageWidth * imageHeight; }
		GLuint size(void) const{ return dataSize; }
		GLuint format(void) const{ return internalFormat; }
	};

	template <typename T> class glTexBuffObj
	{
		glBuffer<T> *textureBuff;
		glTexture *texture;

	public:

		// �f�X�g���N�^
		virtual ~glTexBuffObj<T>(void)
		{
			delete textureBuff;
			delete texture;
		}

		// �R���X�g���N�^
		glTexBuffObj<T>(unsigned int num, const T *data, GLenum internalFormat, GLenum usage = GL_STATIC_DRAW)
		{
			textureBuff = new glBuffer<T>(GL_TEXTURE_BUFFER, num, data, usage);
			texture = new glTexture;
			// �e�N�X�`���o�b�t�@�I�u�W�F�N�g���Q�Ƃ���e�N�X�`�����쐬����
			glBindTexture(GL_TEXTURE_BUFFER, texture->tex());
			glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, textureBuff->buf());
			glBindTexture(GL_TEXTURE_BUFFER, 0);
		}
		GLuint buf() const {
			return textureBuff->buf();
		}
		GLuint tex() const {
			return texture->tex();
		}
		GLint size() const {
			return textureBuff->size();
		}
		// �e�N�X�`���I�u�W�F�N�g���g�p����
		void bind(bool isBind, GLenum TextureUnit = GL_TEXTURE0) const
		{
			if (isBind){
				glActiveTexture(TextureUnit);
				glBindTexture(GL_TEXTURE_BUFFER, texture->tex());
			}
			else{
				glBindTexture(GL_TEXTURE_BUFFER, 0);
				glActiveTexture(0);
			}
		}

	};
	//template <typename T> class glTexture{};

	/*
	** �V�F�[�_
	**
	**    �V�F�[�_�̊��N���X
	*/
	class glShader{

	private:

		// �v���O������
		GLuint program_handle;

		bool ready_init;

		bool _readShaderCompile(GLuint shader, const char *file);
		bool _shaderLink(GLuint prog);

	public:
		// �R���X�g���N�^
		glShader(){
			ready_init = false;
			program_handle = 0;
		}

		/***** GLSL�̃Z�b�g�A�b�v(���_�V�F�[�_, �t���O�����g�V�F�[�_) *****/
		bool init(const char *vertexFile, const char *fragmentFile);
		bool init(const char *vertexFile, const char *fragmentFile, int n, const char **feedbackNames, GLenum mode);

		// �V�F�[�_��L��/��������֐�
		bool setEnabled(bool enabled)
		{
			if (ready_init)
			{
				if (enabled)
					glUseProgram(program_handle);
				else
					glUseProgram(0);

				return true;
			}
			return false;
		};
		// �V�F�[�_�̃v���O���������擾����֐�
		GLuint getHandle(){
			if (ready_init)
				return program_handle;
			else
				std::cout << "�����N���ꂽ�v���O����������܂���" << std::endl;

			return 0;
		}

	};

	/*
	**  �V�F�[�_�R���p�C�����̃G���[���o�֐�
	*/
	inline GLint getShlWithErrorDetect(GLuint program, const GLchar *name)
	{
		GLint location = glGetUniformLocation(program, name);

		if (location < 0)
		{
			std::cerr << "Error: glGetUniformLocation(" << program << ", " << name << ") == " << location << std::endl;
		}

		return location;
	}

	/*
	**  �o�b�t�@�̃R�s�[�֐�
	**	in
	**	out
	*/
	inline void bufferCopy(GLenum src_target, GLuint src_buff, GLenum dst_target, GLuint dst_buff, unsigned int size)
	{
		glBindBuffer(src_target, src_buff);
		glBindBuffer(dst_target, dst_buff);
		glCopyBufferSubData(src_target, dst_target, 0, 0, size);
		glBindBuffer(src_target, 0);
		glBindBuffer(dst_target, 0);
	}

	/* VBO�̒��g��Mat�ɃR�s�[���� */
	template <typename T> cv::Mat bufferChecker(GLuint buf_obj, GLuint vao, GLenum bufferType, GLenum dataType, unsigned int w, unsigned int h){
		glBindVertexArray(vao);

		std::vector<T> vec;
		vec.resize(w*h);
		/* ���e��ҏW */
		glBindBuffer(bufferType, buf_obj);
		T *ptr = (T*)glMapBuffer(bufferType, GL_READ_ONLY);
		if (ptr != NULL){
			memcpy(&vec[0], ptr, w * h * sizeof(T));
			glUnmapBuffer(bufferType);
		}
		glBindBuffer(bufferType, 0);

		glBindVertexArray(0);

		cv::Mat dst;
		switch (dataType){
		case GL_UNSIGNED_BYTE:
			if (sizeof(T) / sizeof(GLubyte) == 1)
				dst = cv::Mat(h, w, CV_8UC1);
			if (sizeof(T) / sizeof(GLubyte) == 2)
				dst = cv::Mat(h, w, CV_8UC2);
			if (sizeof(T) / sizeof(GLubyte) == 3)
				dst = cv::Mat(h, w, CV_8UC3);
			if (sizeof(T) / sizeof(GLubyte) == 4)
				dst = cv::Mat(h, w, CV_8UC4);
			break;
		case GL_INT:
			if (sizeof(T) / sizeof(GLint) == 1)
				dst = cv::Mat(h, w, CV_32SC1);
			if (sizeof(T) / sizeof(GLint) == 2)
				dst = cv::Mat(h, w, CV_32SC2);
			if (sizeof(T) / sizeof(GLint) == 3)
				dst = cv::Mat(h, w, CV_32SC3);
			if (sizeof(T) / sizeof(GLint) == 4)
				dst = cv::Mat(h, w, CV_32SC4);
			break;
		case GL_FLOAT:
			if (sizeof(T) / sizeof(GLfloat) == 1)
				dst = cv::Mat(h, w, CV_32FC1);
			if (sizeof(T) / sizeof(GLfloat) == 2)
				dst = cv::Mat(h, w, CV_32FC2);
			if (sizeof(T) / sizeof(GLfloat) == 3)
				dst = cv::Mat(h, w, CV_32FC3);
			if (sizeof(T) / sizeof(GLfloat) == 4)
				dst = cv::Mat(h, w, CV_32FC4);
			break;
		}
		cv::MatIterator_<T> *it = &dst.begin<T>();
		for (int i = 0; i < vec.size(); ++i){
			(*it)[i] = vec[i];
		}
		return dst;
	}
}

#endif

