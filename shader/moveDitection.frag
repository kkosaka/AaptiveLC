#version 440

//-----出力先-----
out vec4 out_Frag;

//-----頂点シェーダからの出力-----
in vec2 UV;

////-----目標画像パラメータ-----
uniform sampler2D inputImageTexture;//目標画像のテクスチャ(RGBA)

uniform float inputImage_ScalingAlpha;//目標画像のスケーリングアルファ
uniform float inputImage_ScalingBeta;//目標画像のスケーリングベータ(目標画素値 = inputImageTexutre * alpha + beta)

//-----応答関数計算用のカメラ撮影画像-----
uniform sampler2D camImageTexture;	//幾何変換済みのカメラ撮影画像 (RGBA)

//-----その他パラメータ-----
uniform sampler2D mask_texutre;	// 計算用マスク画像
uniform unsigned int diff_threshold;		// 差分計算における閾値

//-----応答関数パラメータ-----
uniform vec3 gamma;		//ガンマ値 (RGB)
uniform vec3 b_pixel;	//最小輝度値B (RGBA)
uniform vec3 w_pixel;	//最大輝度値W (RGBA)
uniform mat3 color_mixing_matrix;		//色混合行列

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
	
	//目標画像をサンプリング、スケーリングしてガンマ変換
	vec3 inputImagePixel = texture2D(inputImageTexture, UV).rgb;
    inputImagePixel = inputImagePixel * inputImage_ScalingAlpha + vec3(inputImage_ScalingBeta, inputImage_ScalingBeta, inputImage_ScalingBeta);
    vec3 targetImagePixel = color_mixing_matrix * getProjectorOutput(inputImagePixel, w_pixel, b_pixel, gamma);
	
	// カメラ画像をサンプリング
    vec3 cam = texture2D(camImageTexture, UV).rgb;

	//前回のKと今回のKの差分を計算し，差を出力
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
