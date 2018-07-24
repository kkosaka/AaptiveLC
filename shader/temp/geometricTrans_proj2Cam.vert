#version 440

// ���_�f�[�^���C���v�b�g����B
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec2 offsetUV;

// �t���O�����g�V�F�[�_�֏o��
out vec2 UV;

void main(void)
{
    // �؂���ꂽ��Ԃł̒��_�̈ʒu�̃A�E�g�v�b�g : MVP * position
    gl_Position = vec4(vertexPosition,1);
	UV = vertexUV;
 
}
