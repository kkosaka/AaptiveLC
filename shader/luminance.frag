#version 450

#define OUTPUTINDEX_RESULT		0		//�␳���ʏo�͐��gl_FragData�C���f�b�N�X
#define OUTPUTINDEX_NUMERATOR	1		//����̕��q�����o�͐��gl_FragData�C���f�b�N�X
#define OUTPUTINDEX_DENOMINATOR 2		//����̕��ꐬ���o�͐��gl_FragData�C���f�b�N�X
#define OUTPUTINDEX_TARGET		3		//�ڕW�摜�̏o��
#define OUTPUTINDEX_K			4		//����̔��˗�K�o�͐��gl_FragData�C���f�b�N�X (�f�o�b�O�p)
#define OUTPUTINDEX_DIFF		5		//������gl_FragData�C���f�b�N�X (�f�o�b�O�p)

#define MINIMUM_EPSILON			0.0001	//0����h�~�p�̏������l

//-----�o�͐�-----
layout(location = OUTPUTINDEX_RESULT)		out vec4 out_FragData_result;
layout(location = OUTPUTINDEX_NUMERATOR)	out vec4 out_FragData_numerator;
layout(location = OUTPUTINDEX_DENOMINATOR)	out vec4 out_FragData_denominator;
layout(location = OUTPUTINDEX_TARGET)		out vec4 out_FragData_target;
layout(location = OUTPUTINDEX_K)			out vec4 out_FragData_k;
layout(location = OUTPUTINDEX_DIFF)			out vec4 out_FragData_diff;

//-----���_�V�F�[�_����̏o��-----
in vec2 UV;
////-----�ڕW�摜�p�����[�^-----
uniform sampler2D inputImageTexture;//�ڕW�摜�̃e�N�X�`��(RGBA)

uniform float inputImage_ScalingAlpha;//�ڕW�摜�̃X�P�[�����O�A���t�@
uniform float inputImage_ScalingBeta;//�ڕW�摜�̃X�P�[�����O�x�[�^(�ڕW��f�l = inputImageTexutre * alpha + beta)


//-----�����֐��v�Z�p�̃v���W�F�N�^���e�摜-----
uniform sampler2D prjImageTexture;	// �␳�摜�̃e�N�X�`��(RGBA)

//-----�����֐��v�Z�p�̃J�����B�e�摜-----
uniform sampler2D camImageTexture;	//�􉽕ϊ��ς݂̃J�����B�e�摜 (RGBA)


//-----�����֐��p�����[�^-----
uniform vec3 gamma;		//�K���}�l (RGB)
uniform vec3 b_pixel;	//�ŏ��P�x�lB (RGBA)
uniform vec3 w_pixel;	//�ő�P�x�lW (RGBA)
uniform mat3 color_mixing_matrix;		//�F�����s��
uniform mat3 color_mixing_matrix_inv;	//�F�����s��̋t�s��

//-----�ŏ����@�p�����[�^-----
uniform sampler2D numerator_before;//�O��̕��q���� (RGBA)
uniform sampler2D denominator_before;//�O��̕��ꐬ�� (RGBA)
uniform float forgetFactor;		//�Y�p�W��

//-----���̑��p�����[�^-----
uniform int nonCorrection;		//�␳�����邩�ǂ��� (1�ȏ�Ȃ�␳�Ȃ�)
uniform sampler2D mask_texutre;	// �v�Z�p�}�X�N�摜
uniform unsigned int diff_threshold;		// �����v�Z�ɂ�����臒l

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

//���˗�K�Ȃ��̉����֐��̋t�֐�
vec3 getProjectorInput(vec3 cam, vec3 w, vec3 b, vec3 gamma)
{
    vec3 result = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; i++)
	{
        float nume = cam[i] - b[i];
        float denomi = w[i] - b[i];
        float invval = 0.0;
        if ((denomi > 0.0) && (nume > 0.0))
		{
            invval = pow(nume / denomi, 1.0 / gamma[i]);
        }

		result[i] = min(1.0, max(0.0, invval));
    }

	return result;
}

void main (void)
{
    //vec3 cam = vec3(texture2D(camImageTexture, UV).rbg); 
	//vec3 target = vec3(texture2D(inputImageTexture, UV).rbg); 
	//out_FragData_result = vec4( (cam + target) / 3 , 1);

	// �}�X�N�̈�O�͕␳���Ȃ�
	vec3 mask = texture2D(mask_texutre, UV).rgb;
	if(mask[0] + mask[1] + mask[2] == 0)
	{
		out_FragData_result = vec4(0, 0, 0, 1);
		out_FragData_numerator = vec4(0, 0, 0, 1);
		out_FragData_denominator = vec4(0, 0, 0, 1);
		out_FragData_target = vec4(0, 0, 0, 1);
		out_FragData_k = vec4(0, 0, 0, 1);
		out_FragData_diff = vec4(0, 0, 0, 1);
		return;
	}

	// �␳���Ȃ�
	if(nonCorrection > 0){
		//�ڕW�摜���T���v�����O�A�X�P�[�����O���ăK���}�ϊ�
		vec3 inputImagePixel = texture2D(inputImageTexture, UV).rgb;
		inputImagePixel = inputImagePixel * inputImage_ScalingAlpha + vec3(inputImage_ScalingBeta, inputImage_ScalingBeta, inputImage_ScalingBeta);
		//vec3 targetImagePixel = color_mixing_matrix * getProjectorOutput(inputImagePixel, w_pixel, b_pixel, gamma);
		vec3 targetImagePixel = vec3(inputImagePixel);
		vec3 cam = texture2D(camImageTexture, UV).rgb;
		vec3 diff = vec3(0, 0, 0);
		for(int i = 0; i < 3; i++) {
			diff[i] = abs(targetImagePixel[i] - cam[i]);
		}

		out_FragData_diff = vec4(diff, 1.0);
		out_FragData_result = vec4(targetImagePixel, 1);
		out_FragData_numerator = vec4(0, 0, 0, 1);
		out_FragData_denominator = vec4(0, 0, 0, 1);
		out_FragData_target = vec4(targetImagePixel, 1);
		out_FragData_k = vec4(0, 0, 0, 1);
		out_FragData_diff = vec4(diff, 1);
		return;
	}

	//-----�����֐��̍X�V-----
	
	//�v�Z�p�̓��e�摜���Ԃ��ċ��߂�
	vec3 prj = texture2D(prjImageTexture, UV).rgb;
    //�����֐��l���v�Z
	vec3 proj_output = getProjectorOutput(prj, w_pixel, b_pixel, gamma);
    //�F�ϊ��s���K�p
	vec3 luminance = color_mixing_matrix * proj_output;
    //�O��̍ŏ����@�p�����[�^�ƃJ�����B�e��f���T���v�����O
	vec3 before_nume = texture2D(numerator_before, UV).rgb;
    vec3 before_denomi = texture2D(denominator_before, UV).rgb;
    vec3 cam = texture2D(camImageTexture, UV).rgb;
    //�ŏ����@�p�����[�^���X�V
	vec3 result_nume = vec3(0.0, 0.0, 0.0);
    vec3 result_denomi = vec3(0.0, 0.0, 0.0);
    vec3 result_k = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; i++)
	{
        result_nume[i] = max(before_nume[i] * forgetFactor + cam[i] * luminance[i], MINIMUM_EPSILON);
        result_denomi[i] = max(before_denomi[i] * forgetFactor + luminance[i] * luminance[i], MINIMUM_EPSILON);
        result_k[i] = max(result_nume[i] / result_denomi[i], MINIMUM_EPSILON);
    }
	

	//�X�V�����p�����[�^���o��
	out_FragData_numerator= vec4(result_nume, 1.0);
    out_FragData_denominator = vec4(result_denomi, 1.0);
    out_FragData_k = vec4(result_k, 1.0);

    //-----�v���W�F�N�^���e�摜�̍쐬-----
	
	//�ڕW�摜���T���v�����O�A�X�P�[�����O���ăK���}�ϊ�
	vec3 inputImagePixel = texture2D(inputImageTexture, UV).rgb;
    inputImagePixel = inputImagePixel * inputImage_ScalingAlpha + vec3(inputImage_ScalingBeta, inputImage_ScalingBeta, inputImage_ScalingBeta);
    vec3 targetImagePixel = color_mixing_matrix * getProjectorOutput(inputImagePixel, w_pixel, b_pixel, gamma);
	//vec3 targetImagePixel = vec3(inputImage_ScalingAlpha, inputImage_ScalingAlpha, inputImage_ScalingAlpha);
	//�ڕW�摜���o��
	out_FragData_target = vec4(targetImagePixel, 1.0);

    //���˗�K�ŏ��Z
	vec3 target_div_k = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; i++)
	{
        target_div_k[i] = targetImagePixel[i] / result_k[i];
    }

	//�F�ϊ��s��̋t�s���K�p
	vec3 target_div_k_cmminv = color_mixing_matrix_inv * target_div_k;
    //�����֐��̋t�֐���K�p
	vec3 result_inputImage = getProjectorInput(target_div_k_cmminv, w_pixel, b_pixel, gamma);
    //�v�Z���ʂ��o��
	out_FragData_result = vec4(result_inputImage, 1.0);

	
    //-----�ڕW�摜�ƎB�e�摜�̍�-----
	//�O���K�ƍ����K�̍������v�Z���C�����o��
	vec3 diff = vec3(0, 0, 0);

	for(int i = 0; i < 3; i++) {
		//diff[i] = abs(before[i] - result_k[i]);
		diff[i] = abs(targetImagePixel[i] - cam[i]);
	}
	float diff_ave = (diff[0] + diff[1] + diff[2]) / 3.0 ;
	if( diff_ave * 255 > diff_threshold)
		diff = vec3(1.0, 1.0, 1.0);
	else
		diff = vec3(0.0, 0.0, 0.0);

	out_FragData_diff = vec4(diff, 1.0);

	//return;
	
	//------�����ɂ��␳
	{
		////�ڕW�摜���T���v�����O�A�X�P�[�����O���ăK���}�ϊ�
		//vec3 inputImagePixel = texture2D(inputImageTexture, UV).rgb;
		//inputImagePixel = inputImagePixel * inputImage_ScalingAlpha + vec3(inputImage_ScalingBeta, inputImage_ScalingBeta, inputImage_ScalingBeta);
		//vec3 targetImagePixel = color_mixing_matrix * getProjectorOutput(inputImagePixel, w_pixel, b_pixel, gamma);
		////�ڕW�摜���o��
		//out_FragData_target = vec4(targetImagePixel, 1.0);
	
		//vec3 prj = texture2D(prjImageTexture, UV).rgb;
		//vec3 cam = texture2D(camImageTexture, UV).rgb;
		//vec3 proj_after  = vec3(0, 0, 0);
		//for(int i = 0; i < 3; i++)
		//{
		//	proj_after[i] =  max( 0.0, min(1.0 , (prj[i] + (targetImagePixel[i] - cam[i]) )) );
		//}
		//out_FragData_result = vec4(proj_after, 1.0);
	}

}





