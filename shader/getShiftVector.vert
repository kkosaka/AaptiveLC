#version 440

//-----�A�v���P�[�V����������n�����VBO�f�[�^-------
layout(location = 0) in vec2 projPoint;			// �v���W�F�N�^���W
layout(location = 1) in vec2 camPoint;			// �����v���̃J�������W�l
layout(location = 2) in vec2 camPoint_offset;	// �ړ���̃J�������W�l
layout(location = 3) in vec2 camPoint_shift;	// ��f�Ή��C���̈ړ���
layout(location = 4) in vec2 offset_result;		// �I�t�Z�b�g�l


//-----�A�v���P�[�V����������n�����ϐ�-------
uniform samplerBuffer shiftIndexTexture;	//Texture Buffer Object(�v���W�F�N�^�𑜓x)
uniform int CORRENT_INDEX;	// ���݂̃V�t�g�C���f�b�N�X�ԍ�
uniform ivec2 PROJECTOR_SIZE;

//-----�o��-------
out vec2 out_VBO_Data;

/*
**	@brief 
**		�ŏ����U�l���Z�o�����Ƃ��̔ԍ�(i)�̈ړ���(pixel�l)�����߂�
**		�v���W�F�N�^�𑜓x�̃e�N�X�`���ɔԍ�(i)���ێ�����Ă��邪�C
**		�ړ��ʂ̓J�������W�n�ŕێ�����K�v������D
**		�Ή��}�b�v���g���΁C�J�������W�ɑΉ�����v���W�F�N�^���W��������̂ŁC
**		�Ή�����v���W�F�N�^���W�Ɋi�[����Ă���ԍ����e�N�X�`������Q�Ƃ���D
*/
void main(void)
{
	if( projPoint.x == -1.0 || projPoint.y == -1.0 ){
		out_VBO_Data = vec2(0.0, 0.0);
		return;
	}

	// ���������_�Ƃ����Ƃ��̃v���W�F�N�^��f�̔z��ԍ����擾
	ivec2 projPoint_int = ivec2(projPoint.x + 0.5,  -(int(projPoint.y+0.5)-(PROJECTOR_SIZE.y-1)) );
	// �J������f�ɑΉ�����v���W�F�N�^���W�𗣎U�����Ď擾
	int projArrayIndex = projPoint_int.y * PROJECTOR_SIZE.x + projPoint_int.x;

	// ���U�v�Z�ɂ��ނ��炵���ړ��ʂ��������ԍ����擾
	vec4 shiftIndex = vec4(texelFetch(shiftIndexTexture, projArrayIndex));
	
	// ����܂ł̌v�Z���ʂ��o��
	out_VBO_Data = offset_result;

	// ���݂̃C���f�b�N�X���ނ��炵���ړ��ʂ̃C���f�b�N�X�ƈ�v���Ă����ꍇ�C���̂Ƃ��̈ړ��ʂ��o��
	if(shiftIndex[0] == CORRENT_INDEX) {
		out_VBO_Data = camPoint_shift;
	}

}
