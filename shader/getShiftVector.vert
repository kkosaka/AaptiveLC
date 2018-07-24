#version 440

//-----アプリケーション側から渡されるVBOデータ-------
layout(location = 0) in vec2 projPoint;			// プロジェクタ座標
layout(location = 1) in vec2 camPoint;			// 初期計測のカメラ座標値
layout(location = 2) in vec2 camPoint_offset;	// 移動後のカメラ座標値
layout(location = 3) in vec2 camPoint_shift;	// 画素対応修正の移動量
layout(location = 4) in vec2 offset_result;		// オフセット値


//-----アプリケーション側から渡される変数-------
uniform samplerBuffer shiftIndexTexture;	//Texture Buffer Object(プロジェクタ解像度)
uniform int CORRENT_INDEX;	// 現在のシフトインデックス番号
uniform ivec2 PROJECTOR_SIZE;

//-----出力-------
out vec2 out_VBO_Data;

/*
**	@brief 
**		最小分散値を算出したときの番号(i)の移動量(pixel値)を求める
**		プロジェクタ解像度のテクスチャに番号(i)が保持されているが，
**		移動量はカメラ座標系で保持する必要がある．
**		対応マップを使えば，カメラ座標に対応するプロジェクタ座標が分かるので，
**		対応するプロジェクタ座標に格納されている番号をテクスチャから参照する．
*/
void main(void)
{
	if( projPoint.x == -1.0 || projPoint.y == -1.0 ){
		out_VBO_Data = vec2(0.0, 0.0);
		return;
	}

	// 左下を原点としたときのプロジェクタ画素の配列番号を取得
	ivec2 projPoint_int = ivec2(projPoint.x + 0.5,  -(int(projPoint.y+0.5)-(PROJECTOR_SIZE.y-1)) );
	// カメラ画素に対応するプロジェクタ座標を離散化して取得
	int projArrayIndex = projPoint_int.y * PROJECTOR_SIZE.x + projPoint_int.x;

	// 分散計算により尤もらしい移動量を示した番号を取得
	vec4 shiftIndex = vec4(texelFetch(shiftIndexTexture, projArrayIndex));
	
	// これまでの計算結果を出力
	out_VBO_Data = offset_result;

	// 現在のインデックスが尤もらしい移動量のインデックスと一致していた場合，そのときの移動量を出力
	if(shiftIndex[0] == CORRENT_INDEX) {
		out_VBO_Data = camPoint_shift;
	}

}
