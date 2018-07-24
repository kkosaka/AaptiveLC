#include "glUtl.h"

/*
** テクスチャ：RGB/RGBA 形式の RAW 画像ファイルの読み込み
*/
void glUtl::glTexture::load(const std::string& FileName, GLenum _target, GLenum format)
{
	// テクスチャの読み込み先

	// OpenCVによる画像読み込み
	cv::Ptr<IplImage> iplimg = cvLoadImage(FileName.c_str(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cv::Mat fileImage = cv::cvarrToMat(iplimg);
	cv::flip(fileImage, fileImage, 0);

	if (fileImage.empty()) {
		std::cout << "Error loading texture '" << FileName.c_str() << std::endl;
		return;
	}
	else{

		// 4チャンネルに変換
		if (fileImage.channels() == 1)
			cv::cvtColor(fileImage, fileImage, CV_GRAY2BGRA);

		if (fileImage.channels() == 3)
			cv::cvtColor(fileImage, fileImage, CV_BGR2BGRA);

		// テクスチャを割り当てる
		glBindTexture(_target, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(_target, 0, GL_RGBA, fileImage.size().width, fileImage.size().height, 0, GL_BGRA, GL_UNSIGNED_BYTE, fileImage.data);
		glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//マッピング時の拡大
		glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//マッピング時の縮小
		glBindTexture(_target, 0);

		imageWidth = fileImage.size().width;
		imageHeight = fileImage.size().height;
		internalFormat = format;
		target = _target;
		dataSize = sizeof(fileImage.data) * imageWidth * imageHeight;
	}

	fileImage.release();
}

void glUtl::glTexture::loadFromMat(cv::Mat &src, GLenum _target, GLenum _format, GLenum matFormat, GLenum matType)
{
	if (src.empty()) {
		std::cout << "Error loading texture from Mat'" << std::endl;
		return;
	}
	else{
		imageWidth = src.cols;
		imageHeight = src.rows;
		target = _target;
		internalFormat = _format;

		cv::Mat tmpImage;
		// 4チャンネルに変換
		if (src.channels() == 1)
			cv::cvtColor(src, tmpImage, CV_GRAY2BGRA);

		if (src.channels() == 3)
			cv::cvtColor(src, tmpImage, CV_BGR2BGRA);

		if (src.channels() == 4)
			src.copyTo(tmpImage);

		cv::flip(tmpImage, tmpImage, 0);

		// テクスチャを割り当てる
		glBindTexture(target, tex());
		glTexImage2D(target, 0, internalFormat, imageWidth, imageHeight, 0, matFormat, matType, tmpImage.data);

		// バイリニア（ミップマップなし），エッジでクランプ
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		dataSize = sizeof(tmpImage.data);
	}

}

void glUtl::glTexture::upload(const GLvoid *data, GLenum dataFormat, GLenum dataType)
{
	if (dataSize != 0 && dataSize == sizeof(data)){
		glBindTexture(target, texture);
		glTexSubImage2D(target, 0, 0, 0, imageWidth, imageHeight, dataFormat, dataType, data);
		glBindTexture(target, 0);

		dataSize = sizeof(data);
	}
	else
		std::cerr << "get texture memory or data size is different" << std::endl;
}

/**
* @brief   Shaderのセットアップ(頂点シェーダ, フラグメントシェーダ)
*
* @param   program[in,out]			シェーダプログラム
* @param   vertexFile[in]			頂点シェーダのファイル名
* @param   fragmentFile[in]		フラグメントシェーダのファイル名
*/
bool glUtl::glShader::init(const char *vertexFile, const char *fragmentFile)
{
	GLuint program;

	// シェーダオブジェクトの作成
	GLuint vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// シェーダーの読み込みとコンパイル
	if (!_readShaderCompile(vertexShader, vertexFile))
		return false;
	if (!_readShaderCompile(fragmentShader, fragmentFile))
		return false;

	// シェーダプログラムの作成 
	program = glCreateProgram();

	// シェーダオブジェクトをシェーダプログラムに関連付ける
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// シェーダオブジェクトの削除 
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// シェーダプログラムのリンク 
	_shaderLink(program);

	ready_init = true;
	program_handle = program;

	return true;
}
bool glUtl::glShader::init(const char *vertexFile, const char *fragmentFile, int n, const char **feedbackNames, GLenum mode)
{
	GLuint program;

	// シェーダオブジェクトの作成
	GLuint vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// シェーダーの読み込みとコンパイル
	if (!_readShaderCompile(vertexShader, vertexFile))
		return false;
	if (!_readShaderCompile(fragmentShader, fragmentFile))
		return false;

	// シェーダプログラムの作成 
	program = glCreateProgram();

	// シェーダオブジェクトをシェーダプログラムに関連付ける
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// シェーダオブジェクトの削除 
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// シェーダプログラムのリンク 
	glTransformFeedbackVaryings(program, n, feedbackNames, mode);

	GLsizei	size, len;
	GLint linked;
	char *infoLog;

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (linked == GL_FALSE)
	{
		printf("リンクできませんでした!! \n");

		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
		if (size > 0)
		{
			infoLog = (char *)malloc(size);
			glGetProgramInfoLog(program, size, &len, infoLog);
			printf(infoLog);
			free(infoLog);
		}
		return false;
	}

	ready_init = true;
	program_handle = program;

	return true;
}

/**
* @brief   シェーダファイルを読み込みコンパイル
*
* @param   shader[in]		シェーダオブジェクト
* @param   file[in]		シェーダファイル名
*/
bool glUtl::glShader::_readShaderCompile(GLuint shader, const char *file)
{
	FILE *fp;
	char *buf;
	GLsizei size, len;
	GLint compiled;

	// ファイルを開く
	if (fopen_s(&fp, file, "rb") != 0)
	{
		printf("ファイルを開くことができません %s\n", file);
		return false;

	}

	// ファイルの末尾に移動し現在位置を得る 
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);		// ファイルサイズを取得

	// ファイルサイズのメモリを確保 
	buf = (GLchar *)malloc(size);
	if (buf == NULL) {
		printf("メモリが確保できませんでした \n");
		return false;
	}

	// ファイルを先頭から読み込む 
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, size, fp);

	// シェーダオブジェクトにプログラムをセット 
	glShaderSource(shader, 1, (const GLchar **)&buf, &size);

	// シェーダ読み込み領域の解放 
	free(buf);
	fclose(fp);

	// シェーダのコンパイル 
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_FALSE)
	{
		printf("コンパイルできませんでした!!: %s \n ", file);
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &size);
		if (size > 0)
		{
			buf = (char *)malloc(size);
			glGetShaderInfoLog(shader, size, &len, buf);
			printf(buf);
			free(buf);
		}
		return false;
	}
	return true;
}

/**
* @brief   シェーダプログラムのリンク
*
* @param   prog[in]		シェーダプログラム
*/
bool glUtl::glShader::_shaderLink(GLuint prog)
{
	GLsizei	size, len;
	GLint linked;
	char *infoLog;

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &linked);

	if (linked == GL_FALSE)
	{
		printf("リンクできませんでした!! \n");

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		if (size > 0)
		{
			infoLog = (char *)malloc(size);
			glGetProgramInfoLog(prog, size, &len, infoLog);
			printf(infoLog);
			free(infoLog);
		}
		return false;
	}
	return true;
}


