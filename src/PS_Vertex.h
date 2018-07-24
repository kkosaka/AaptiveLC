#pragma once
#include "vertex.h"
#include "glUtl.h"

#define LOCATION_PROJECTOR			0
#define LOCATION_CAMERA				1
#define LOCATION_CAMERAOFFSET		2
#define LOCATION_CAMERASHIFT		3
#define LOCATION_OFFSETRESULT		4

class PS_Vertex :
	public Vertex
{
public:
	PS_Vertex(unsigned int window_w, unsigned int window_h);
	~PS_Vertex();

	// ���_���̒��_���W�ƃe�N�X�`�����W���܂Ƃ߂邽�߂̍\����
	virtual struct MyVertex{
		MyVertex(cv::Vec2f proj = cv::Vec2f(0.0f, 0.0f), cv::Vec2f cam = cv::Vec2f(0.0f, 0.0f))
		: Proj(proj), Cam(cam){};
		cv::Vec2f Proj;
		cv::Vec2f Cam;
	};

	// (�I�[�o�[���C�h)
	void render_mesh();
	void render_point();
	
	// (�I�[�o�[���C�h)�I�����ɌĂ΂��֐�
	void close();

	// �O���Ōv�Z����index-vertex buffer���C���g�̃����o�ϐ���index-vertex buffer�ɃZ�b�g����֐�
	void setBuffers(std::vector<MyVertex> _vertecies, std::vector<MyIndex_TRIANGLES> _indecies);
	void setCameraIndex(std::vector<float> _cameraRefer);
	void setShiftZeroIndex(int zeroIndex){ shift_zeroIndex = zeroIndex; };

	// �ړ��ʂ��i�[�����x�N�^�[
	void setShiftVector(std::vector<cv::Mat> &shiftVectors);

	// layout = 2�ɃZ�b�g����VBO��ݒ�
	void setShaderLocationShiftVector(bool isEnable, GLuint num = 0);

	void setDefaultVertexPointer();

	void feedbackCalclateForGetOffsetResult();
	void feedbackSetVertexPointerForGetOffsetResult(unsigned int num);

	// ��f�Ή��̈ړ��ʂ��X�V����֐�
	void feedbackSetVertexPointerForSetNewOffset();
	void feedbackCalclateForSetNewOffset();
	// ��f�Ή��̈ړ��ʂɑ΂��C�����������{���֐�
	void feedbackSetVertexPointerForBlur();
	void feedbackCalclateForBlur(GLuint vbo);

	// �Ή��}�b�v���f�t�H���g�ɖ߂��֐�
	void resetOffsetVBO();

	GLuint getVAO(){ return vao; }
	GLuint* getVBO_SHIFT(){ return vbo_shift; };
	GLuint getVBO_OFFSET(){ return vbo_offset; };
	//GLuint getVBO_CAMERAINDEX(){ return vbo_camIndex; };
	GLuint getVBO_OFFSETRESULT(){ return vbo_offset_result; };
	GLuint getVBO_STATUS(){ return vbo_status; };
	GLuint getVBO_FEEDBACK(){ return feedback; };
	GLuint getVBO_MASK(){ return VBO_mask_camsize->buf(); }
	GLuint getVBO_MASK_SIZE(){ return VBO_mask_camsize->size(); }
	int getVertexNum(){ return vertex_num; };
	int getShiftNum(){ return shift_num; };

private:
	unsigned int windowWidth;
	unsigned int windowHeight;
	// ���_�z��I�u�W�F�N�g(0�F�􉽕␳�A1:�􉽕␳�Ȃ�)
	GLuint vao;

	// ���_�o�b�t�@�I�u�W�F�N�g
	GLuint vbo;			// �����v�����̊􉽑Ή�
	GLuint vbo_offset;	// �I�t�Z�b�g
	GLuint *vbo_shift;	// �ړ��p�o�b�t�@�I�u�W�F�N�g

	// �I�t�Z�b�g�����߂�����
	GLuint vbo_offset_result;

	// �R�s�[�p���_�o�b�t�@�I�u�W�F�N�g
	GLuint vbo_offset_copy;

	glUtl::glBuffer<GLfloat> *VBO_mask_camsize;

	// �e�N�X�`���o�b�t�@�I�u�W�F�N�g�Q�Ɨp�̃o�b�t�@�I�u�W�F�N�g
	//GLuint vbo_camIndex;
	GLuint vbo_status;

	// �C���f�b�N�X�o�b�t�@�I�u�W�F�N�g
	GLuint indexbuffer;

	// feedback
	GLuint feedback;

	int vertex_num;
	int shift_num;
	int shift_zeroIndex;

	// �`�悷��C���f�b�N�X��
	int index_num;

	// �`�揀�����ł��Ă��邩�ǂ���
	bool ready;

};

