#version 440

//-----�o�͐�-----
out vec4 out_Frag;

//-----���_�V�F�[�_����̏o��-----
in vec2 UV;

////-----�ڕW�摜�p�����[�^-----
uniform sampler2D inputImageTexture;//�ڕW�摜�̃e�N�X�`��(RGBA)

uniform float inputImage_ScalingAlpha;//�ڕW�摜�̃X�P�[�����O�A���t�@
uniform float inputImage_ScalingBeta;//�ڕW�摜�̃X�P�[�����O�x�[�^(�ڕW��f�l = inputImageTexutre * alpha + beta)

//-----�����֐��v�Z�p�̃J�����B�e�摜-----
uniform sampler2D camImageTexture;	//�􉽕ϊ��ς݂̃J�����B�e�摜 (RGBA)

//-----���̑��p�����[�^-----
uniform sampler2D mask_texutre;	// �v�Z�p�}�X�N�摜
uniform unsigned int diff_threshold;		// �����v�Z�ɂ�����臒l

//-----�����֐��p�����[�^-----
uniform vec3 gamma;		//�K���}�l (RGB)
uniform vec3 b_pixel;	//�ŏ��P�x�lB (RGBA)
uniform vec3 w_pixel;	//�ő�P�x�lW (RGBA)
uniform mat3 color_mixing_matrix;		//�F�����s��

//���˗�K�Ȃ��̉����֐�
vec3 getProjectorOutput(vec3 id, vec3 w, vec3 b, vec3 gamma)
{
    vec3 result = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; i++)
	{
        float idval = min(1.0, max(0.0, id[i]));
        float responseval = (w[i] - b[i]) * pow(id[i], gamma[i]) + b[i];
        result[i] = min(1.0, max(0.0, responseval));
    }

	return result;
}

void main (void)
{
	vec3 mask = texture2D(mask_texutre, UV).rgb;
	if(mask[0] + mask[1] + mask[2] == 0)
	{
		out_FragData_result = vec4(0, 0, 0, 1);
		out_FragData_numerator = vec4(0, 0, 0, 1);
		out_FragData_denominator = vec4(0, 0, 0, 1);
		out_FragData_target = vec4(0, 0, 0, 1);
		out_FragData_k = vec4(0, 0, 0, 1);
		return;
	}
	
	//�ڕW�摜���T���v�����O�A�X�P�[�����O���ăK���}�ϊ�
	vec3 inputImagePixel = texture2D(inputImageTexture, UV).rgb;
    inputImagePixel = inputImagePixel * inputImage_ScalingAlpha + vec3(inputImage_ScalingBeta, inputImage_ScalingBeta, inputImage_ScalingBeta);
    vec3 targetImagePixel = color_mixing_matrix * getProjectorOutput(inputImagePixel, w_pixel, b_pixel, gamma);
	
	// �J�����摜���T���v�����O
    vec3 cam = texture2D(camImageTexture, UV).rgb;

	//�O���K�ƍ����K�̍������v�Z���C�����o��
	vec3 diff = vec3(0, 0, 0);

	for(int i = 0; i < 3; i++) {
		diff[i] = abs(targetImagePixel[i] - cam[i]);
	}
	float diff_ave = (diff[0] + diff[1] + diff[2]) / 3.0 ;
	if( diff_ave * 255 > diff_threshold)
		diff = vec3(1.0, 1.0, 1.0);
	else
		diff = vec3(0.0, 0.0, 0.0);

	out_Frag = vec4(diff, 1.0);

}
