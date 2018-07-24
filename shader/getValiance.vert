#version 440

#define KERNEL_SIZE 13		// カーネルサイズ(３以上の奇数)
#define PIXEL_NUM (KERNEL_SIZE * KERNEL_SIZE)

//-----アプリケーション側から渡されるVBOデータ-------
layout(location = 0) in vec4 maskData;			// マスクデータ(エラー画素)
layout(location = 1) in vec4 minValiance_before;// 前回までの最小分散値
layout(location = 2) in vec4 index_before;		// 最小分散値を示したときの番号

//-----アプリケーション側から渡される変数-------
uniform samplerBuffer k_texture;	//Texture Buffer Object(プロジェクタ解像度)

uniform ivec2 ProjSize;		// プロジェクタ解像度
uniform int CORRENT_INDEX;	// 現在のインデックス
uniform int ZERO_INDEX;		// 移動量が0の時のインデックス

//-----出力-------
out vec4 out_VBO_Data_MinV;		// 最小分散値を出力
out vec4 out_VBO_Data_Index;	// 最小分散値を示したときの番号を出力
out vec4 out_VBO_Data_Valiance;	// 今回の分散値を出力

void main(void)
{

	/*バーテックスシェーダで計算する場合*/
	
	//----------マスク領域外なら計算しない-------
	if(maskData[0] + maskData[1] + maskData[2] == 0)
	{
		out_VBO_Data_Valiance = vec4(0, 0, 0, 1);
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}
	
	// 自身の列と行番号を計算
	int maskOffset = 10;
	int m = int(mod(float(gl_VertexID), float(ProjSize.x)));
	int cols = m;
	int rows = int( float(gl_VertexID - m) / float(ProjSize.x) ); 

	// 投影領域の縁の部分はずれ量の推定誤差が生じやすいので計算しない
	if(cols < maskOffset || ProjSize.x - maskOffset < cols || rows < maskOffset || ProjSize.y - maskOffset < rows) {
		out_VBO_Data_Valiance = vec4(0, 0, 0, 1);
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}

	// 近傍ピクセル値を保持するための配列
	vec3 pixels[KERNEL_SIZE][KERNEL_SIZE];
	// カーネルサイズ
	int kernel_size = KERNEL_SIZE;
	// 領域の探索半径
	int half_size = int(floor( kernel_size / 2.0 ));

	// 合計値を計算する
	vec3 sum_rgb = vec3(0.0, 0.0, 0.0);	
 	// 周辺ピクセル値を取得
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			int  point = gl_VertexID + ProjSize.x * (y - half_size) + (x - half_size);
			pixels[y][x] = vec3(texelFetch(k_texture, point));
			// 合計値を計算
			sum_rgb += pixels[y][x];
		}
	}
	
	// 平均値を計算する
	vec3 ave = sum_rgb * (1.0 / float(PIXEL_NUM));
	
	//----------分散の計算--------------
	vec3 var_sum_rgb = vec3(0.0, 0.0, 0.0);
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			var_sum_rgb += ( (pixels[y][x] - ave) * (pixels[y][x] - ave) );
		}
	}
	vec3 var_rgb = var_sum_rgb * (1.0 / (PIXEL_NUM - 1));
	
	////----------標準偏差の計算--------------
	//vec3 sd_rgb;
	//float sd_gray = 0.0;
	//for(int c = 0; c < 3; ++c){
	//	sd_rgb[c] = sqrt(var_rgb[c]);
	//	sd_gray += sd_rgb[c];
	//}
	//// 各画素の平均を算出
	//float SD = sd_gray / 3.0;

	// 計算した分散値の平均を計算
	float valiance_ave = (var_rgb[0] + var_rgb[1] + var_rgb[2]) / 3.0;
	
	// 分散が0の場合，ゼロにする
	if(valiance_ave == 0){
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}

	// 一番最初は, 最小分散を自身の値に設定
	if(CORRENT_INDEX == 0){
		out_VBO_Data_MinV = vec4(var_rgb, 1.0);
		out_VBO_Data_Index = vec4(CORRENT_INDEX, 0, 0, 1);
		return;
	}
	
	// これまで最小値である分散値の平均を計算
	float min_valiance_ave = (minValiance_before[0] + minValiance_before[1] + minValiance_before[2]) / 3.0;

	// 計算した分散値が最小値以下の場合
	if(min_valiance_ave >= valiance_ave){
		out_VBO_Data_MinV = vec4(var_rgb, 1.0);
		out_VBO_Data_Index = vec4(CORRENT_INDEX, 0, 0, 1);
	}
	// それ以外は前回の計算結果をそのまま出力
	else{
		out_VBO_Data_MinV = vec4(minValiance_before);
		out_VBO_Data_Index = vec4(index_before);
	}
	
	// VBOへ出力
	out_VBO_Data_Valiance = vec4(var_rgb, 1.0);
	return;
}
