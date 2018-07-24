#version 450

#define OUTPUTINDEX_RESULT		0		//補正結果出力先のgl_FragDataインデックス
#define OUTPUTINDEX_NUMERATOR	1		//今回の分子成分出力先のgl_FragDataインデックス
#define OUTPUTINDEX_DENOMINATOR 2		//今回の分母成分出力先のgl_FragDataインデックス
#define OUTPUTINDEX_TARGET		3		//目標画像の出力
#define OUTPUTINDEX_K			4		//今回の反射率K出力先のgl_FragDataインデックス (デバッグ用)
#define OUTPUTINDEX_DIFF		5		//差分のgl_FragDataインデックス (デバッグ用)

#define MINIMUM_EPSILON			0.0001	//0割り防止用の小さい値

//-----出力先-----
layout(location = OUTPUTINDEX_RESULT)		out vec4 out_FragData_result;
layout(location = OUTPUTINDEX_NUMERATOR)	out vec4 out_FragData_numerator;
layout(location = OUTPUTINDEX_DENOMINATOR)	out vec4 out_FragData_denominator;
layout(location = OUTPUTINDEX_TARGET)		out vec4 out_FragData_target;
layout(location = OUTPUTINDEX_K)			out vec4 out_FragData_k;
layout(location = OUTPUTINDEX_DIFF)			out vec4 out_FragData_diff;

//-----頂点シェーダからの出力-----
in vec2 UV;
////-----目標画像パラメータ-----
uniform sampler2D inputImageTexture;//目標画像のテクスチャ(RGBA)

uniform float inputImage_ScalingAlpha;//目標画像のスケーリングアルファ
uniform float inputImage_ScalingBeta;//目標画像のスケーリングベータ(目標画素値 = inputImageTexutre * alpha + beta)


//-----応答関数計算用のプロジェクタ投影画像-----
uniform sampler2D prjImageTexture;	// 補正画像のテクスチャ(RGBA)

//-----応答関数計算用のカメラ撮影画像-----
uniform sampler2D camImageTexture;	//幾何変換済みのカメラ撮影画像 (RGBA)


//-----応答関数パラメータ-----
uniform vec3 gamma;		//ガンマ値 (RGB)
uniform vec3 b_pixel;	//最小輝度値B (RGBA)
uniform vec3 w_pixel;	//最大輝度値W (RGBA)
uniform mat3 color_mixing_matrix;		//色混合行列
uniform mat3 color_mixing_matrix_inv;	//色混合行列の逆行列

//-----最小二乗法パラメータ-----
uniform sampler2D numerator_before;//前回の分子成分 (RGBA)
uniform sampler2D denominator_before;//前回の分母成分 (RGBA)
uniform float forgetFactor;		//忘却係数

//-----その他パラメータ-----
uniform int nonCorrection;		//補正をするかどうか (1以上なら補正なし)
uniform sampler2D mask_texutre;	// 計算用マスク画像
uniform unsigned int diff_threshold;		// 差分計算における閾値

//反射率Kなしの応答関数
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

//反射率Kなしの応答関数の逆関数
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

	// マスク領域外は補正しない
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

	// 補正しない
	if(nonCorrection > 0){
		//目標画像をサンプリング、スケーリングしてガンマ変換
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

	//-----応答関数の更新-----
	
	//計算用の投影画像を補間して求める
	vec3 prj = texture2D(prjImageTexture, UV).rgb;
    //応答関数値を計算
	vec3 proj_output = getProjectorOutput(prj, w_pixel, b_pixel, gamma);
    //色変換行列を適用
	vec3 luminance = color_mixing_matrix * proj_output;
    //前回の最小二乗法パラメータとカメラ撮影画素をサンプリング
	vec3 before_nume = texture2D(numerator_before, UV).rgb;
    vec3 before_denomi = texture2D(denominator_before, UV).rgb;
    vec3 cam = texture2D(camImageTexture, UV).rgb;
    //最小二乗法パラメータを更新
	vec3 result_nume = vec3(0.0, 0.0, 0.0);
    vec3 result_denomi = vec3(0.0, 0.0, 0.0);
    vec3 result_k = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; i++)
	{
        result_nume[i] = max(before_nume[i] * forgetFactor + cam[i] * luminance[i], MINIMUM_EPSILON);
        result_denomi[i] = max(before_denomi[i] * forgetFactor + luminance[i] * luminance[i], MINIMUM_EPSILON);
        result_k[i] = max(result_nume[i] / result_denomi[i], MINIMUM_EPSILON);
    }
	

	//更新したパラメータを出力
	out_FragData_numerator= vec4(result_nume, 1.0);
    out_FragData_denominator = vec4(result_denomi, 1.0);
    out_FragData_k = vec4(result_k, 1.0);

    //-----プロジェクタ投影画像の作成-----
	
	//目標画像をサンプリング、スケーリングしてガンマ変換
	vec3 inputImagePixel = texture2D(inputImageTexture, UV).rgb;
    inputImagePixel = inputImagePixel * inputImage_ScalingAlpha + vec3(inputImage_ScalingBeta, inputImage_ScalingBeta, inputImage_ScalingBeta);
    vec3 targetImagePixel = color_mixing_matrix * getProjectorOutput(inputImagePixel, w_pixel, b_pixel, gamma);
	//vec3 targetImagePixel = vec3(inputImage_ScalingAlpha, inputImage_ScalingAlpha, inputImage_ScalingAlpha);
	//目標画像を出力
	out_FragData_target = vec4(targetImagePixel, 1.0);

    //反射率Kで除算
	vec3 target_div_k = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 3; i++)
	{
        target_div_k[i] = targetImagePixel[i] / result_k[i];
    }

	//色変換行列の逆行列を適用
	vec3 target_div_k_cmminv = color_mixing_matrix_inv * target_div_k;
    //応答関数の逆関数を適用
	vec3 result_inputImage = getProjectorInput(target_div_k_cmminv, w_pixel, b_pixel, gamma);
    //計算結果を出力
	out_FragData_result = vec4(result_inputImage, 1.0);

	
    //-----目標画像と撮影画像の差-----
	//前回のKと今回のKの差分を計算し，差を出力
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
	
	//------差分による補正
	{
		////目標画像をサンプリング、スケーリングしてガンマ変換
		//vec3 inputImagePixel = texture2D(inputImageTexture, UV).rgb;
		//inputImagePixel = inputImagePixel * inputImage_ScalingAlpha + vec3(inputImage_ScalingBeta, inputImage_ScalingBeta, inputImage_ScalingBeta);
		//vec3 targetImagePixel = color_mixing_matrix * getProjectorOutput(inputImagePixel, w_pixel, b_pixel, gamma);
		////目標画像を出力
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





