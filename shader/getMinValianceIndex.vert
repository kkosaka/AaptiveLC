#version 440

//-----CPUから渡されるVBO-------
layout(location = 0) in vec4 valiance;			// 今回計算した分散値
layout(location = 1) in vec4 minValiance_before;// 前回までの最小分散値
layout(location = 2) in vec4 index_before;		// 最小分散値を示したときの番号

//-----アプリケーション側から渡される変数-------
uniform ivec2 projSize;		// プロジェクタ解像度
uniform int CORRENT_INDEX;	// 現在のインデックス
uniform int ZERO_INDEX;		// 移動量が0の時のインデックス

//-----出力-------
out vec4 out_VBO_Data_MinV;		// 最小分散値を出力
out vec4 out_VBO_Data_Index;	// 最小分散値を示したときの番号を出力

void main(void)
{
	int maskOffset = 10;

	// 自身の列と行番号を計算
	int m = int(mod(float(gl_VertexID), float(projSize.x)));
	int cols = m;
	int rows = int( float(gl_VertexID - m) / float(projSize.x) ); 

	// 投影領域の縁の部分はずれ量の推定誤差が生じやすいので計算しない
	if(cols < maskOffset || projSize.x - maskOffset < cols || rows < maskOffset || projSize.y - maskOffset < rows) {
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}
	
	// 計算した分散値の平均を計算
	float valiance_ave = (valiance[0] + valiance[1] + valiance[2]) / 3.0;

	// 分散が0の場合
	if(valiance_ave == 0){
		out_VBO_Data_MinV = vec4(0.0, 0.0, 0.0, 1.0);
		out_VBO_Data_Index = vec4(ZERO_INDEX, 0, 0, 1);
		return;
	}
	
	// これまで最小値である分散値の平均を計算
	float min_valiance_ave = (minValiance_before[0] + minValiance_before[1] + minValiance_before[2]) / 3.0;
	
	// 計算した分散値が最小値以下の場合
	if(min_valiance_ave >= valiance_ave){
		out_VBO_Data_MinV = vec4(valiance);
		out_VBO_Data_Index = vec4(CORRENT_INDEX, 0, 0, 1);
	}
	// それ以外は前回の計算結果をそのまま出力
	else{
		out_VBO_Data_MinV = vec4(minValiance_before);
		out_VBO_Data_Index = vec4(index_before);
	}
}
