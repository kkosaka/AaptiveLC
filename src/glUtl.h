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

// glfwのpragma文
#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
// glewのpragma文
#pragma comment(lib,"glew32.lib")

namespace glUtl{

	/*
	** バッファオブジェクト
	**
	**    頂点／インデックスバッファオブジェクトの基底クラス
	*/
	template <typename T> class glBuffer
	{
		// バッファオブジェクト
		GLuint buffer;

		// データ数
		unsigned int number;

		// データのサイズ
		unsigned int data_size;

		// ターゲット
		GLenum buff_target;
	public:

		// デストラクタ
		virtual ~glBuffer<T>(void)
		{
			glDeleteBuffers(1, &buffer);
		}

		// コンストラクタ
		glBuffer<T>(void)
			: number(0)
		{
			glGenBuffers(1, &buffer);
		}

		// コンストラクタ
		glBuffer<T>(GLenum target, unsigned int num, const T *data, GLenum usage = GL_STATIC_DRAW)
		{
			glGenBuffers(1, &buffer);
			load(target, num, data, usage);
		}

		// コピーコンストラクタ
		glBuffer<T>(const glBuffer<T> &o)
		{
			buffer = o.buffer;
			number = o.number;
			data_size = o.data_size;
		}

		// 代入演算子
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

		// データの取得
		void load(GLenum target, unsigned int num, const T *data, GLenum usage = GL_STATIC_DRAW)
		{
			this->number = num;
			this->data_size = num * sizeof(T);
			this->buff_target = target;
			glBindBuffer(target, buffer);
			glBufferData(target, sizeof (T)* num, data, usage);
			glBindBuffer(target, 0);
		}

		// バッファオブジェクト名を得る
		GLuint buf(void) const
		{
			return buffer;
		}

		// データの数を得る
		unsigned int num(void) const
		{
			return number;
		}
		// データの数を得る
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
	** テクスチャ
	**
	**    頂点／インデックスバッファオブジェクトの基底クラス
	*/
	class glTexture
	{
		// テクスチャ名
		GLuint texture;
		// テクスチャのバインド時のターゲット
		GLenum target;
		// テクスチャフォーマット
		GLenum internalFormat;

		int imageWidth;
		int imageHeight;

		// データサイズ
		int dataSize;

	public:

		// デストラクタ
		virtual ~glTexture(void)
		{
			glDeleteTextures(1, &texture);
		}

		// コンストラクタ
		glTexture(void)
			:dataSize(0)
		{
			glGenTextures(1, &texture);
		}

		// コンストラクタ(データからテクスチャを作成する)
		glTexture(
			cv::Mat &src,						// データ
			GLenum _target = GL_TEXTURE_2D,		// テクスチャのタイプ(GL_TEXTURE2Dなど)
			GLenum _textureformat = GL_RGB,     // テクスチャのフォーマット (GL_RGB/GL_RGBA/GL_RGBA32Fなど)
			GLenum _matFormat = GL_BGR,			// 画像ファイルのフォーマット(GL_BGR)
			GLenum _matType = GL_UNSIGNED_BYTE	// 画像ファイルのデータタイプ(gl_unsigned_byte, gl_float, gl_int)
			)
		{
			glGenTextures(1, &texture);
			loadFromMat(src, _target, _textureformat, _matFormat, _matType);
		}

		// コンストラクタ(ファイルから読み込みテクスチャを作成する)
		glTexture(
			const std::string& FileName,               // 画像ファイル名（3/4 チャネルの RAW 画像）
			GLenum _target = GL_TEXTURE_2D,
			GLenum _format = GL_RGBA
			)
		{
			glGenTextures(1, &texture);
			load(FileName, _target, _format);
		}


		// コピーコンストラクタ
		glTexture(const glTexture &o)
			: texture(o.texture) {}

		// 代入演算子
		glTexture &operator=(const glTexture &o)
		{
			if (this != &o)
			{
				glDeleteTextures(1, &texture);
				texture = o.texture;
			}

			return *this;
		}

		//    filename: ファイル名, width, height: 幅と高さ (2^n), format: GL_RGB か GL_RGBA
		void load(const std::string& FileName, GLenum target = GL_TEXTURE_2D, GLenum internalFormat = GL_RGBA);
		void loadFromMat(cv::Mat &src, GLenum target = GL_TEXTURE_2D, GLenum format = GL_RGB, GLenum matFormat = GL_BGR, GLenum matType = GL_UNSIGNED_BYTE);
		void upload(const GLvoid *data, GLenum dataFormat = GL_BGR, GLenum dataType = GL_UNSIGNED_BYTE);

		// データが空のテクスチャを作成
		// getTexture<データの型>(width，height, バインド先のターゲット, 型に対応するGLのフォーマット)
		template <typename T> void getTexture(int width, int height, GLenum _target, GLenum _internalFormat, GLenum dataFomat = GL_BGR, GLenum dataType = GL_UNSIGNED_BYTE, const void *data = 0)
		{
			imageWidth = width;
			imageHeight = height;
			target = _target;
			internalFormat = _internalFormat;
			dataSize = width * height *  sizeof(T);

			// テクスチャを割り当てる
			glBindTexture(_target, tex());
			glTexImage2D(_target, 0, internalFormat, width, height, 0, dataFomat, dataType, data);
			// バイリニア（ミップマップなし），エッジでクランプ
			glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glBindTexture(_target, 0);
		}

		// テクスチャオブジェクトを使用する
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

		// テクスチャオブジェクト名を取得する
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

		// デストラクタ
		virtual ~glTexBuffObj<T>(void)
		{
			delete textureBuff;
			delete texture;
		}

		// コンストラクタ
		glTexBuffObj<T>(unsigned int num, const T *data, GLenum internalFormat, GLenum usage = GL_STATIC_DRAW)
		{
			textureBuff = new glBuffer<T>(GL_TEXTURE_BUFFER, num, data, usage);
			texture = new glTexture;
			// テクスチャバッファオブジェクトを参照するテクスチャを作成する
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
		// テクスチャオブジェクトを使用する
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
	** シェーダ
	**
	**    シェーダの基底クラス
	*/
	class glShader{

	private:

		// プログラム名
		GLuint program_handle;

		bool ready_init;

		bool _readShaderCompile(GLuint shader, const char *file);
		bool _shaderLink(GLuint prog);

	public:
		// コンストラクタ
		glShader(){
			ready_init = false;
			program_handle = 0;
		}

		/***** GLSLのセットアップ(頂点シェーダ, フラグメントシェーダ) *****/
		bool init(const char *vertexFile, const char *fragmentFile);
		bool init(const char *vertexFile, const char *fragmentFile, int n, const char **feedbackNames, GLenum mode);

		// シェーダを有効/無効する関数
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
		// シェーダのプログラム名を取得する関数
		GLuint getHandle(){
			if (ready_init)
				return program_handle;
			else
				std::cout << "リンクされたプログラムがありません" << std::endl;

			return 0;
		}

	};

	/*
	**  シェーダコンパイル時のエラー検出関数
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
	**  バッファのコピー関数
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

	/* VBOの中身をMatにコピーする */
	template <typename T> cv::Mat bufferChecker(GLuint buf_obj, GLuint vao, GLenum bufferType, GLenum dataType, unsigned int w, unsigned int h){
		glBindVertexArray(vao);

		std::vector<T> vec;
		vec.resize(w*h);
		/* 内容を編集 */
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

