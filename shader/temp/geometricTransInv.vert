#version 440

// ���_�f�[�^���C���v�b�g����B
layout(location = 0) in vec2 projPoint;			// �v���W�F�N�^���W
layout(location = 1) in vec2 camPoint;			// �����v���̃J�������W�l
layout(location = 2) in vec2 camPoint_offset;	// �ړ���̃J�������W�l
layout(location = 3) in vec2 camPoint_shift;	// ��f�Ή��C���̈ړ���

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform mat4 transformMatrix_vertex;
uniform mat4 transformMatrix_texture;

//-----�t���O�����g�V�F�[�_�ւ̏o�͕ϐ�-------
out vec2 UV;

void main(void)
{
    // ���_���W��OpenGL���W�n�֕ϊ�
	vec4 translatedCamPoint = vec4(camPoint + camPoint_offset, 0.0, 1.0);
    gl_Position = transformMatrix_vertex * translatedCamPoint;
 
	// �e�N�X�`�����W��OpenGL���W�n�֕ϊ�
	vec4 HomogeneousProjPoint = vec4(projPoint, 0.0, 1.0);
	UV = vec2( (transformMatrix_texture * HomogeneousProjPoint).st );
}
