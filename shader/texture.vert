#version 440

//-----�A�v���P�[�V��������n�����VBO�f�[�^-------
layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 vertexUV;

//-----�t���O�����g�V�F�[�_�ւ̏o�͕ϐ�-------
out vec2 UV;

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform mat4 transformMatrix_vertex;
uniform mat4 transformMatrix_texture;

void main(void)
{

 //   // ���_���W��OpenGL���W�n�֕ϊ�
	//vec4 HomogeneousPoint = vec4(vertexPosition, 0.0, 1.0);
 //   gl_Position = transformMatrix_vertex * HomogeneousPoint;
	//UV = vec2( (transformMatrix_texture * HomogeneousPoint).st );

    // �؂���ꂽ��Ԃł̒��_�̈ʒu�̃A�E�g�v�b�g : MVP * position
    gl_Position =  vec4(vertexPosition, 0, 1);
	UV = vertexUV;
 
}
