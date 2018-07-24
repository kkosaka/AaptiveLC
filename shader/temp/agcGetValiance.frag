#version 440

#define KERNEL_SIZE 15		// �J�[�l���T�C�Y(�R�ȏ�̊)
#define PIXEL_NUM (KERNEL_SIZE * KERNEL_SIZE)

//-----�o�͐�-----
layout(location = 0) out vec4 out_FragData_MinV_After;	// ���U�l�̍ŏ��l
layout(location = 1) out vec4 out_FragData_sigma_After;// ���U�l�̍ŏ��l
layout(location = 2) out vec4 out_FragData_INDEX_After;	// �ŏ����U�l���v�Z�����Ƃ��̎Q�Ɣԍ�
layout(location = 3) out vec4 out_FragData_Valiance;	// �Z�o�������U�l

//-----���_�V�F�[�_����̏o��-----
in vec2 UV;

//-----���̓e�N�X�`���f�[�^
uniform sampler2D texture_k;		// ���˗��摜
uniform sampler2D minV_before;		// ����܂ł̕��U�l�̍ŏ��l
uniform sampler2D sigma_before;		// ����܂ł̕��U�l�̑��a
uniform sampler2D index_before;		// �ŏ����U�l�������C���f�b�N�X�ԍ�
uniform sampler2D texutre_mask;		// �}�X�N

uniform int CORRENT_INDEX;
uniform int LAST_INDEX;	// �T���Ō�
uniform int ZERO_INDEX;	// �[���C���f�b�N�X

void main (void)
{
	//----------�}�X�N�̈�O�Ȃ�v�Z���Ȃ�-------
	vec3 mask = texture2D(texutre_mask, UV).rgb;
	if(mask[0] + mask[1] + mask[2] == 0)
	{
		int shift_zero = int(LAST_INDEX / 2);
		out_FragData_MinV_After  = vec4(0, 0, 0, 1);
		out_FragData_sigma_After = vec4(0, 0, 0, 1);
		out_FragData_INDEX_After = vec4(shift_zero, 0, 0, 1);
		out_FragData_Valiance    = vec4(0, 0, 0, 1);
		return;
	}

	vec3 pixels[PIXEL_NUM];
	int pixel_num = KERNEL_SIZE * KERNEL_SIZE;

	//�ߖT��f(Kernel size)�̒l���擾
	int size = (KERNEL_SIZE - 1) / 2;
	int count = 0;
	for(int i = -size; i <= size; ++i){
		for(int j = -size; j <= size; ++j) {
			pixels[count] = textureOffset(texture_k, UV, ivec2(i, j)).rgb;
			count++;
		}
	}


	//----------�W���΍��̌v�Z--------------

	vec3 sum = vec3(0.0, 0.0, 0.0);
	// ���v�l���v�Z����
	for(int i = 0; i < PIXEL_NUM; ++i) {
			sum += pixels[i];
	}
	// ���ϒl���v�Z����
	vec3 ave = sum * (1.0 / float(PIXEL_NUM));
	//vec3 ave = vec3(0.0, 0.0, 0.0);
	//for(int i = 0; i < 3; i++)
	//	ave[i] = sum[i] / float(PIXEL_NUM);

	//���U���v�Z����
	vec3 var = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < PIXEL_NUM; ++i) {
		vec3 pixel = pixels[i];
		for(int c = 0; c < 3; ++c){
			var[c] += (pixel[c] - ave[c]) * (pixel[c] - ave[c]);
		}
	}
	//for(int i = -size; i < size; ++i){
	//	for(int j = -size; j < size; ++j) {
	//		for(int c = 0; c < 3; ++c){
	//			vec3 pixel = textureOffset(texture_k, UV, ivec2(i, j)).rgb;
	//			var[c] += (pixel[c] - ave[c]) * (pixel[c] - ave[c]);
	//		}
			
	//	}
	//}
	// �W���΍����v�Z����
	vec3 sd = vec3(0.0, 0.0, 0.0);
	float sd_channel_ave = 0.0;
	for(int c = 0; c < 3; ++c){
		sd[c] = var[c] / (PIXEL_NUM - 1);
		sd[c] = sqrt(sd[c]);
		sd_channel_ave += sd[c];
	}
	// �e��f�̕��ς��Z�o
	sd_channel_ave = sd_channel_ave / 3.0;


	//----------����ւ��̔���--------------

	// �ŏ��W���΍��̃e�N�X�`������T���v�����O
	vec3 min_valiance = texture2D(minV_before, UV).rgb;
	float minV_channel_ave = (min_valiance[0] + min_valiance[1] + min_valiance[2]) / 3.0;

	// �C���f�b�N�X�p�e�N�X�`������T���v�����O
	float index = float(texture2D(index_before, UV).r);

	// ���U���a�v�Z�p�e�N�X�`������T���v�����O
	float sigma_valiance = texture2D(sigma_before, UV).r;
	sigma_valiance += sd_channel_ave;
	

	// ��ԍŏ�
	if(CORRENT_INDEX == 0)
	{
		// �ŏ��l�ƃC���f�b�N�X���X�V
		out_FragData_MinV_After = vec4(sd, 1.0);
		out_FragData_sigma_After = vec4(sd_channel_ave, 0.0, 0.0, 1.0);
		out_FragData_INDEX_After = vec4(CORRENT_INDEX, 0.0, 0.0, 1.0);
	}
	// �v�Z�����l���C���܂ł̍ŏ��l�����������l���Ƃ��Ă���ꍇ
	else if( sd_channel_ave < minV_channel_ave) {
		// �ŏ��l�ƃC���f�b�N�X���X�V
		out_FragData_MinV_After = vec4(sd, 1.0);
		out_FragData_sigma_After = vec4(sigma_valiance, 0.0, 0.0, 1.0);
		out_FragData_INDEX_After = vec4(CORRENT_INDEX, 0.0, 0.0, 1.0);
	}
	// �v�Z�����l���C���܂ł̍ŏ��l�����傫���l���Ƃ��Ă���ꍇ
	else {
		// �ŏ��l�ƃC���f�b�N�X���X�V���Ȃ�
		out_FragData_MinV_After = vec4(min_valiance, 1.0);
		out_FragData_sigma_After = vec4(sigma_valiance, 0.0, 0.0, 1.0);
		out_FragData_INDEX_After = vec4(index, 0.0, 0.0, 1.0);
	}

	// �T�����Ō�̏ꍇ
	if(CORRENT_INDEX == LAST_INDEX)
	{
		float sigma_valiance_ave = sigma_valiance / (LAST_INDEX + 1);
		out_FragData_sigma_After = vec4(sigma_valiance_ave, 0.0, 0.0, 1.0);
		// �e�s�N�Z���̎��n��ɂ����镪�U�l���������Ƃ��C���e�ʂ͕ω��Ȃ��Ƃ݂Ȃ�
		//if(sigma_valiance_ave < 1.0) {
		//	out_FragData_INDEX_After = vec4(ZERO_INDEX, 0, 0, 1);
		//	out_FragData_sigma_After = vec4(0.0, 0.0, 0.0, 1.0);
		//}
	}

	// �v�Z�������U�l���o��
	out_FragData_Valiance = vec4(sd, 1.0);


}
