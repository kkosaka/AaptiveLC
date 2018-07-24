#include "glUtl.h"

/*
** �e�N�X�`���FRGB/RGBA �`���� RAW �摜�t�@�C���̓ǂݍ���
*/
void glUtl::glTexture::load(const std::string& FileName, GLenum _target, GLenum format)
{
	// �e�N�X�`���̓ǂݍ��ݐ�

	// OpenCV�ɂ��摜�ǂݍ���
	cv::Ptr<IplImage> iplimg = cvLoadImage(FileName.c_str(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cv::Mat fileImage = cv::cvarrToMat(iplimg);
	cv::flip(fileImage, fileImage, 0);

	if (fileImage.empty()) {
		std::cout << "Error loading texture '" << FileName.c_str() << std::endl;
		return;
	}
	else{

		// 4�`�����l���ɕϊ�
		if (fileImage.channels() == 1)
			cv::cvtColor(fileImage, fileImage, CV_GRAY2BGRA);

		if (fileImage.channels() == 3)
			cv::cvtColor(fileImage, fileImage, CV_BGR2BGRA);

		// �e�N�X�`�������蓖�Ă�
		glBindTexture(_target, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(_target, 0, GL_RGBA, fileImage.size().width, fileImage.size().height, 0, GL_BGRA, GL_UNSIGNED_BYTE, fileImage.data);
		glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//�}�b�s���O���̊g��
		glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//�}�b�s���O���̏k��
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
		// 4�`�����l���ɕϊ�
		if (src.channels() == 1)
			cv::cvtColor(src, tmpImage, CV_GRAY2BGRA);

		if (src.channels() == 3)
			cv::cvtColor(src, tmpImage, CV_BGR2BGRA);

		if (src.channels() == 4)
			src.copyTo(tmpImage);

		cv::flip(tmpImage, tmpImage, 0);

		// �e�N�X�`�������蓖�Ă�
		glBindTexture(target, tex());
		glTexImage2D(target, 0, internalFormat, imageWidth, imageHeight, 0, matFormat, matType, tmpImage.data);

		// �o�C���j�A�i�~�b�v�}�b�v�Ȃ��j�C�G�b�W�ŃN�����v
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
* @brief   Shader�̃Z�b�g�A�b�v(���_�V�F�[�_, �t���O�����g�V�F�[�_)
*
* @param   program[in,out]			�V�F�[�_�v���O����
* @param   vertexFile[in]			���_�V�F�[�_�̃t�@�C����
* @param   fragmentFile[in]		�t���O�����g�V�F�[�_�̃t�@�C����
*/
bool glUtl::glShader::init(const char *vertexFile, const char *fragmentFile)
{
	GLuint program;

	// �V�F�[�_�I�u�W�F�N�g�̍쐬
	GLuint vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// �V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	if (!_readShaderCompile(vertexShader, vertexFile))
		return false;
	if (!_readShaderCompile(fragmentShader, fragmentFile))
		return false;

	// �V�F�[�_�v���O�����̍쐬 
	program = glCreateProgram();

	// �V�F�[�_�I�u�W�F�N�g���V�F�[�_�v���O�����Ɋ֘A�t����
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// �V�F�[�_�I�u�W�F�N�g�̍폜 
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// �V�F�[�_�v���O�����̃����N 
	_shaderLink(program);

	ready_init = true;
	program_handle = program;

	return true;
}
bool glUtl::glShader::init(const char *vertexFile, const char *fragmentFile, int n, const char **feedbackNames, GLenum mode)
{
	GLuint program;

	// �V�F�[�_�I�u�W�F�N�g�̍쐬
	GLuint vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// �V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	if (!_readShaderCompile(vertexShader, vertexFile))
		return false;
	if (!_readShaderCompile(fragmentShader, fragmentFile))
		return false;

	// �V�F�[�_�v���O�����̍쐬 
	program = glCreateProgram();

	// �V�F�[�_�I�u�W�F�N�g���V�F�[�_�v���O�����Ɋ֘A�t����
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// �V�F�[�_�I�u�W�F�N�g�̍폜 
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// �V�F�[�_�v���O�����̃����N 
	glTransformFeedbackVaryings(program, n, feedbackNames, mode);

	GLsizei	size, len;
	GLint linked;
	char *infoLog;

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (linked == GL_FALSE)
	{
		printf("�����N�ł��܂���ł���!! \n");

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
* @brief   �V�F�[�_�t�@�C����ǂݍ��݃R���p�C��
*
* @param   shader[in]		�V�F�[�_�I�u�W�F�N�g
* @param   file[in]		�V�F�[�_�t�@�C����
*/
bool glUtl::glShader::_readShaderCompile(GLuint shader, const char *file)
{
	FILE *fp;
	char *buf;
	GLsizei size, len;
	GLint compiled;

	// �t�@�C�����J��
	if (fopen_s(&fp, file, "rb") != 0)
	{
		printf("�t�@�C�����J�����Ƃ��ł��܂��� %s\n", file);
		return false;

	}

	// �t�@�C���̖����Ɉړ������݈ʒu�𓾂� 
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);		// �t�@�C���T�C�Y���擾

	// �t�@�C���T�C�Y�̃��������m�� 
	buf = (GLchar *)malloc(size);
	if (buf == NULL) {
		printf("���������m�ۂł��܂���ł��� \n");
		return false;
	}

	// �t�@�C����擪����ǂݍ��� 
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, size, fp);

	// �V�F�[�_�I�u�W�F�N�g�Ƀv���O�������Z�b�g 
	glShaderSource(shader, 1, (const GLchar **)&buf, &size);

	// �V�F�[�_�ǂݍ��ݗ̈�̉�� 
	free(buf);
	fclose(fp);

	// �V�F�[�_�̃R���p�C�� 
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_FALSE)
	{
		printf("�R���p�C���ł��܂���ł���!!: %s \n ", file);
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
* @brief   �V�F�[�_�v���O�����̃����N
*
* @param   prog[in]		�V�F�[�_�v���O����
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
		printf("�����N�ł��܂���ł���!! \n");

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


