#version 440

#define KERNEL_SIZE 15		// カーネルサイズ(３以上の奇数)
#define PIXEL_NUM (KERNEL_SIZE * KERNEL_SIZE)

//-----出力先-----
layout(location = 0) out vec4 out_FragData_MinV_After;	// 分散値の最小値
layout(location = 1) out vec4 out_FragData_sigma_After;// 分散値の最小値
layout(location = 2) out vec4 out_FragData_INDEX_After;	// 最小分散値を計算したときの参照番号
layout(location = 3) out vec4 out_FragData_Valiance;	// 算出した分散値

//-----頂点シェーダからの出力-----
in vec2 UV;

//-----入力テクスチャデータ
uniform sampler2D texture_k;		// 反射率画像
uniform sampler2D minV_before;		// これまでの分散値の最小値
uniform sampler2D sigma_before;		// これまでの分散値の総和
uniform sampler2D index_before;		// 最小分散値を示すインデックス番号
uniform sampler2D texutre_mask;		// マスク

uniform int CORRENT_INDEX;
uniform int LAST_INDEX;	// 探索最後
uniform int ZERO_INDEX;	// ゼロインデックス

void main (void)
{
	//----------マスク領域外なら計算しない-------
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

	//近傍画素(Kernel size)の値を取得
	int size = (KERNEL_SIZE - 1) / 2;
	int count = 0;
	for(int i = -size; i <= size; ++i){
		for(int j = -size; j <= size; ++j) {
			pixels[count] = textureOffset(texture_k, UV, ivec2(i, j)).rgb;
			count++;
		}
	}


	//----------標準偏差の計算--------------

	vec3 sum = vec3(0.0, 0.0, 0.0);
	// 合計値を計算する
	for(int i = 0; i < PIXEL_NUM; ++i) {
			sum += pixels[i];
	}
	// 平均値を計算する
	vec3 ave = sum * (1.0 / float(PIXEL_NUM));
	//vec3 ave = vec3(0.0, 0.0, 0.0);
	//for(int i = 0; i < 3; i++)
	//	ave[i] = sum[i] / float(PIXEL_NUM);

	//分散を計算する
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
	// 標準偏差を計算する
	vec3 sd = vec3(0.0, 0.0, 0.0);
	float sd_channel_ave = 0.0;
	for(int c = 0; c < 3; ++c){
		sd[c] = var[c] / (PIXEL_NUM - 1);
		sd[c] = sqrt(sd[c]);
		sd_channel_ave += sd[c];
	}
	// 各画素の平均を算出
	sd_channel_ave = sd_channel_ave / 3.0;


	//----------入れ替えの判定--------------

	// 最小標準偏差のテクスチャからサンプリング
	vec3 min_valiance = texture2D(minV_before, UV).rgb;
	float minV_channel_ave = (min_valiance[0] + min_valiance[1] + min_valiance[2]) / 3.0;

	// インデックス用テクスチャからサンプリング
	float index = float(texture2D(index_before, UV).r);

	// 分散総和計算用テクスチャからサンプリング
	float sigma_valiance = texture2D(sigma_before, UV).r;
	sigma_valiance += sd_channel_ave;
	

	// 一番最初
	if(CORRENT_INDEX == 0)
	{
		// 最小値とインデックスを更新
		out_FragData_MinV_After = vec4(sd, 1.0);
		out_FragData_sigma_After = vec4(sd_channel_ave, 0.0, 0.0, 1.0);
		out_FragData_INDEX_After = vec4(CORRENT_INDEX, 0.0, 0.0, 1.0);
	}
	// 計算した値が，今までの最小値よりも小さい値をとっている場合
	else if( sd_channel_ave < minV_channel_ave) {
		// 最小値とインデックスを更新
		out_FragData_MinV_After = vec4(sd, 1.0);
		out_FragData_sigma_After = vec4(sigma_valiance, 0.0, 0.0, 1.0);
		out_FragData_INDEX_After = vec4(CORRENT_INDEX, 0.0, 0.0, 1.0);
	}
	// 計算した値が，今までの最小値よりも大きい値をとっている場合
	else {
		// 最小値とインデックスを更新しない
		out_FragData_MinV_After = vec4(min_valiance, 1.0);
		out_FragData_sigma_After = vec4(sigma_valiance, 0.0, 0.0, 1.0);
		out_FragData_INDEX_After = vec4(index, 0.0, 0.0, 1.0);
	}

	// 探索が最後の場合
	if(CORRENT_INDEX == LAST_INDEX)
	{
		float sigma_valiance_ave = sigma_valiance / (LAST_INDEX + 1);
		out_FragData_sigma_After = vec4(sigma_valiance_ave, 0.0, 0.0, 1.0);
		// 各ピクセルの時系列における分散値が小さいとき，投影面は変化なしとみなす
		//if(sigma_valiance_ave < 1.0) {
		//	out_FragData_INDEX_After = vec4(ZERO_INDEX, 0, 0, 1);
		//	out_FragData_sigma_After = vec4(0.0, 0.0, 0.0, 1.0);
		//}
	}

	// 計算した分散値を出力
	out_FragData_Valiance = vec4(sd, 1.0);


}
