#version 440

// ���_�f�[�^���C���v�b�g����B
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;	// �����v���̃J�������W�l
layout(location = 2) in vec2 offset;	// �ړ���̃J�������W�l
layout(location = 3) in vec2 shiftUV;	// ��f�Ή��C���̈ړ���

// �t���O�����g�V�F�[�_�֏o��
out vec2 UV;
// ���_�o�b�t�@�I�u�W�F�N�g�֏o��
out vec2 out_VBO_Data;

uniform ivec2 movePixel;

void main(void)
{
    // �؂���ꂽ��Ԃł̒��_�̈ʒu�̃A�E�g�v�b�g : MVP * position
    gl_Position = vec4(vertexPosition,1);
	vec2 transrate = vec2( shiftUV[0] / (1920.0 - 1.0), shiftUV[1] / (1200.0 - 1.0) );
	vec2 offset_gl = vec2( offset[0] / (1920.0 - 1.0), offset[1] / (1200.0 - 1.0) );
	UV = vertexUV + offset_gl + transrate;

	out_VBO_Data = offset;
}
