#version 440

#define KERNEL_SIZE 13		// �J�[�l���T�C�Y(�R�ȏ�̊)
#define PIXEL_NUM (KERNEL_SIZE * KERNEL_SIZE)

//-----�A�v���P�[�V����������n�����VBO�f�[�^-------
layout(location = 0) in vec4 maskData;			// �}�X�N�f�[�^(�G���[��f)
layout(location = 1) in vec4 minValiance_before;// �O��܂ł̍ŏ����U�l
layout(location = 2) in vec4 index_before;		// �ŏ����U�l���������Ƃ��̔ԍ�

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform samplerBuffer k_texture;	//Texture Buffer Object(�v���W�F�N�^�𑜓x)

uniform ivec2 ProjSize;		// �v���W�F�N�^�𑜓x
uniform int CORRENT_INDEX;	// ���݂̃C���f�b�N�X
uniform int ZERO_INDEX;		// �ړ��ʂ�0�̎��̃C���f�b�N�X

//-----�o��-------
out vec4 out_VBO_Data_MinV;		// �ŏ����U�l���o��
out vec4 out_VBO_Data_Index;	// �ŏ����U�l���������Ƃ��̔ԍ����o��
out vec4 out_VBO_Data_Valiance;	// ����̕��U�l���o��

void main(void)
{

	/*�o�[�e�b�N�X�V�F�[�_�Ōv�Z����ꍇ*/
	
	//----------�}�X�N�̈�O�Ȃ�v�Z���Ȃ�-------
	if(maskData[0] + maskData[1] + maskData[2] == 0)
	{
		out_VBO_Data_Valiance = vec4(0, 0, 0, 1);
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}
	
	// ���g�̗�ƍs�ԍ����v�Z
	int maskOffset = 10;
	int m = int(mod(float(gl_VertexID), float(ProjSize.x)));
	int cols = m;
	int rows = int( float(gl_VertexID - m) / float(ProjSize.x) ); 

	// ���e�̈�̉��̕����͂���ʂ̐���덷�������₷���̂Ōv�Z���Ȃ�
	if(cols < maskOffset || ProjSize.x - maskOffset < cols || rows < maskOffset || ProjSize.y - maskOffset < rows) {
		out_VBO_Data_Valiance = vec4(0, 0, 0, 1);
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}

	// �ߖT�s�N�Z���l��ێ����邽�߂̔z��
	vec3 pixels[KERNEL_SIZE][KERNEL_SIZE];
	// �J�[�l���T�C�Y
	int kernel_size = KERNEL_SIZE;
	// �̈�̒T�����a
	int half_size = int(floor( kernel_size / 2.0 ));

	// ���v�l���v�Z����
	vec3 sum_rgb = vec3(0.0, 0.0, 0.0);	
 	// ���Ӄs�N�Z���l���擾
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			int  point = gl_VertexID + ProjSize.x * (y - half_size) + (x - half_size);
			pixels[y][x] = vec3(texelFetch(k_texture, point));
			// ���v�l���v�Z
			sum_rgb += pixels[y][x];
		}
	}
	
	// ���ϒl���v�Z����
	vec3 ave = sum_rgb * (1.0 / float(PIXEL_NUM));
	
	//----------���U�̌v�Z--------------
	vec3 var_sum_rgb = vec3(0.0, 0.0, 0.0);
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			var_sum_rgb += ( (pixels[y][x] - ave) * (pixels[y][x] - ave) );
		}
	}
	vec3 var_rgb = var_sum_rgb * (1.0 / (PIXEL_NUM - 1));
	
	////----------�W���΍��̌v�Z--------------
	//vec3 sd_rgb;
	//float sd_gray = 0.0;
	//for(int c = 0; c < 3; ++c){
	//	sd_rgb[c] = sqrt(var_rgb[c]);
	//	sd_gray += sd_rgb[c];
	//}
	//// �e��f�̕��ς��Z�o
	//float SD = sd_gray / 3.0;

	// �v�Z�������U�l�̕��ς��v�Z
	float valiance_ave = (var_rgb[0] + var_rgb[1] + var_rgb[2]) / 3.0;
	
	// ���U��0�̏ꍇ�C�[���ɂ���
	if(valiance_ave == 0){
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}

	// ��ԍŏ���, �ŏ����U�����g�̒l�ɐݒ�
	if(CORRENT_INDEX == 0){
		out_VBO_Data_MinV = vec4(var_rgb, 1.0);
		out_VBO_Data_Index = vec4(CORRENT_INDEX, 0, 0, 1);
		return;
	}
	
	// ����܂ōŏ��l�ł��镪�U�l�̕��ς��v�Z
	float min_valiance_ave = (minValiance_before[0] + minValiance_before[1] + minValiance_before[2]) / 3.0;

	// �v�Z�������U�l���ŏ��l�ȉ��̏ꍇ
	if(min_valiance_ave >= valiance_ave){
		out_VBO_Data_MinV = vec4(var_rgb, 1.0);
		out_VBO_Data_Index = vec4(CORRENT_INDEX, 0, 0, 1);
	}
	// ����ȊO�͑O��̌v�Z���ʂ����̂܂܏o��
	else{
		out_VBO_Data_MinV = vec4(minValiance_before);
		out_VBO_Data_Index = vec4(index_before);
	}
	
	// VBO�֏o��
	out_VBO_Data_Valiance = vec4(var_rgb, 1.0);
	return;
}
