#version 440

#define KERNEL_SIZE 7		// �J�[�l���T�C�Y(�R�ȏ�̊)
#define PIXEL_NUM (KERNEL_SIZE * KERNEL_SIZE)

//-----�o�͐�-----
layout(location = 0) out vec4 out_FragData_Valiance;	// ���U�l�̍ŏ��l

//-----���_�V�F�[�_����̏o��-----
in vec2 UV;

//-----���̓e�N�X�`���f�[�^

uniform sampler2D texture_k;		// ���˗��摜0
uniform sampler2D texutre_mask;		// �}�X�N

void main (void)
{
	//----------�}�X�N�̈�O�Ȃ�v�Z���Ȃ�-------
	vec3 mask = texture2D(texutre_mask, UV).rgb;
	if(mask[0] + mask[1] + mask[2] == 0)
	{
		out_FragData_Valiance = vec4(0, 0, 0, 1);
		return;
	}
	
	// �ߖT�s�N�Z���l��ێ����邽�߂̔z��
	vec3 pixels[PIXEL_NUM];

	//�ߖT��f(Kernel size)�̒l���擾
	int size = (KERNEL_SIZE - 1) / 2;
	int count = 0;
	for(int i = -size; i <= size; ++i){
		for(int j = -size; j <= size; ++j) {
			// �I�t�Z�b�g�l�Ɍ��x������̂Œ���(GPU�Ɉˑ�)
			pixels[count] = textureOffset(texture_k, UV, ivec2(i, j)).rgb;
			count++;
		}
	}

	// �����������˗��摜�̕������v�Z����
	//----------�W���΍��̌v�Z--------------
	// ���v�l���v�Z����
	vec3 sum_rgb = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < PIXEL_NUM; ++i) {
			sum_rgb += pixels[i];
	}
	// ���ϒl���v�Z����
	vec3 ave = sum_rgb * (1.0 / float(PIXEL_NUM));

	//���U���v�Z����
	vec3 var_sum_rgb = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < PIXEL_NUM; ++i) {
		vec3 pixel = pixels[i];
		for(int c = 0; c < 3; ++c){
			var_sum_rgb[c] += (pixel[c] - ave[c]) * (pixel[c] - ave[c]);
		}
	}
	vec3 var_rgb = var_sum_rgb * (1.0 / (PIXEL_NUM - 1));

	// �W���΍����v�Z����
	vec3 sd_rgb;
	float sd_gray = 0.0;
	for(int c = 0; c < 3; ++c){
		sd_rgb[c] = sqrt(var_rgb[c]);
		sd_gray += sd_rgb[c];
	}
	// �e��f�̕��ς��Z�o
	float SD = sd_gray / 3.0;
	
	out_FragData_Valiance = vec4(sd_rgb, 1.0);

}
