#version 440

#define KERNEL_SIZE 7		// カーネルサイズ(３以上の奇数)
#define PIXEL_NUM (KERNEL_SIZE * KERNEL_SIZE)

//-----出力先-----
layout(location = 0) out vec4 out_FragData_Valiance;	// 分散値の最小値

//-----頂点シェーダからの出力-----
in vec2 UV;

//-----入力テクスチャデータ

uniform sampler2D texture_k;		// 反射率画像0
uniform sampler2D texutre_mask;		// マスク

void main (void)
{
	//----------マスク領域外なら計算しない-------
	vec3 mask = texture2D(texutre_mask, UV).rgb;
	if(mask[0] + mask[1] + mask[2] == 0)
	{
		out_FragData_Valiance = vec4(0, 0, 0, 1);
		return;
	}
	
	// 近傍ピクセル値を保持するための配列
	vec3 pixels[PIXEL_NUM];

	//近傍画素(Kernel size)の値を取得
	int size = (KERNEL_SIZE - 1) / 2;
	int count = 0;
	for(int i = -size; i <= size; ++i){
		for(int j = -size; j <= size; ++j) {
			// オフセット値に限度があるので注意(GPUに依存)
			pixels[count] = textureOffset(texture_k, UV, ivec2(i, j)).rgb;
			count++;
		}
	}

	// 生成した反射率画像の分だけ計算する
	//----------標準偏差の計算--------------
	// 合計値を計算する
	vec3 sum_rgb = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < PIXEL_NUM; ++i) {
			sum_rgb += pixels[i];
	}
	// 平均値を計算する
	vec3 ave = sum_rgb * (1.0 / float(PIXEL_NUM));

	//分散を計算する
	vec3 var_sum_rgb = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < PIXEL_NUM; ++i) {
		vec3 pixel = pixels[i];
		for(int c = 0; c < 3; ++c){
			var_sum_rgb[c] += (pixel[c] - ave[c]) * (pixel[c] - ave[c]);
		}
	}
	vec3 var_rgb = var_sum_rgb * (1.0 / (PIXEL_NUM - 1));

	// 標準偏差を計算する
	vec3 sd_rgb;
	float sd_gray = 0.0;
	for(int c = 0; c < 3; ++c){
		sd_rgb[c] = sqrt(var_rgb[c]);
		sd_gray += sd_rgb[c];
	}
	// 各画素の平均を算出
	float SD = sd_gray / 3.0;
	
	out_FragData_Valiance = vec4(sd_rgb, 1.0);

}
