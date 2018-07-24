#version 440

//-----CPU����n�����VBO-------
layout(location = 0) in vec4 valiance;			// ����v�Z�������U�l
layout(location = 1) in vec4 minValiance_before;// �O��܂ł̍ŏ����U�l
layout(location = 2) in vec4 index_before;		// �ŏ����U�l���������Ƃ��̔ԍ�

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform ivec2 projSize;		// �v���W�F�N�^�𑜓x
uniform int CORRENT_INDEX;	// ���݂̃C���f�b�N�X
uniform int ZERO_INDEX;		// �ړ��ʂ�0�̎��̃C���f�b�N�X

//-----�o��-------
out vec4 out_VBO_Data_MinV;		// �ŏ����U�l���o��
out vec4 out_VBO_Data_Index;	// �ŏ����U�l���������Ƃ��̔ԍ����o��

void main(void)
{
	int maskOffset = 10;

	// ���g�̗�ƍs�ԍ����v�Z
	int m = int(mod(float(gl_VertexID), float(projSize.x)));
	int cols = m;
	int rows = int( float(gl_VertexID - m) / float(projSize.x) ); 

	// ���e�̈�̉��̕����͂���ʂ̐���덷�������₷���̂Ōv�Z���Ȃ�
	if(cols < maskOffset || projSize.x - maskOffset < cols || rows < maskOffset || projSize.y - maskOffset < rows) {
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}
	
	// �v�Z�������U�l�̕��ς��v�Z
	float valiance_ave = (valiance[0] + valiance[1] + valiance[2]) / 3.0;

	// ���U��0�̏ꍇ
	if(valiance_ave == 0){
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}
	
	// ����܂ōŏ��l�ł��镪�U�l�̕��ς��v�Z
	float min_valiance_ave = (minValiance_before[0] + minValiance_before[1] + minValiance_before[2]) / 3.0;
	
	// �v�Z�������U�l���ŏ��l�ȉ��̏ꍇ
	if(min_valiance_ave >= valiance_ave){
		out_VBO_Data_MinV = vec4(valiance);
		out_VBO_Data_Index = vec4(CORRENT_INDEX, 0, 0, 1);
	}
	// ����ȊO�͑O��̌v�Z���ʂ����̂܂܏o��
	else{
		out_VBO_Data_MinV = vec4(minValiance_before);
		out_VBO_Data_Index = vec4(index_before);
	}
}
