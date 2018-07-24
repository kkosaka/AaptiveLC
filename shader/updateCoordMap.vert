#version 440

//-----CPUから渡されるVBO-------
layout(location = 0) in vec2 projPoint;			// プロジェクタ座標
layout(location = 1) in vec2 camPoint;			// 初期計測のカメラ座標値
layout(location = 2) in vec2 camPoint_offset;	// 過去のオフセット値
layout(location = 3) in vec2 camPoint_shift;	// 画素対応修正の移動量
layout(location = 4) in vec2 offset_result;		// オフセット値

//-----アプリケーション側から渡される変数-------
uniform samplerBuffer statusTexture;	//状態管理のTBO
uniform samplerBuffer offsetTexture;	//状態管理のTBO
uniform samplerBuffer maskTexture;
uniform ivec2 CAMERA_SIZE;

//-----出力-------
out vec2 out_VBO_Data;
out float out_VBO_Status;

void main(void)
{
	// 状態を保持
	float status = float(texelFetch(statusTexture, int(gl_VertexID)));

	// 移動量が0の場合は正しく補正できているものとみなす
	if(offset_result.x == 0 && offset_result.y == 0){
		// 画素対応マップオフセット値を更新
		out_VBO_Data = camPoint_offset + offset_result;
		// 状態を初期化
		out_VBO_Status = 0;
		return;
	}
	
	// 状態管理用VBOを更新
	out_VBO_Status = status + 1;

	// 計算したオフセット値
	vec2 newOffset = camPoint_offset + offset_result;
	
	//if(abs(newOffset[0]) > 20 || abs(newOffset[1]) > 20)
	//	newOffset = vec2(0.0, 0.0);

	// 画素対応マップオフセット値を更新
	out_VBO_Data = newOffset;
	
	// 状態が常にエラーの場合
	if(3 < status && status < 105) {
		
		// 自身の列と行番号を計算
		int m = int(mod(float(gl_VertexID), float(CAMERA_SIZE.x)));
		int cols = m;	
		int rows = int( float(gl_VertexID - m) / float(CAMERA_SIZE.x) );
		 
		// 周辺の補正できている領域から補間する

		vec2 interpolation = vec2(0.0, 0.0);
		int counter = 0;
		// 徐々に半径を広げていく
		for(int radius = 1; radius < 30; radius++){
			for(int y = -radius; y <= radius; y++) {
				for(int x = -radius; x <= radius; x++) {
					// 探索は外縁部だけでよい
					if(abs(y) != radius && abs(x) != radius){
						continue;
					}
					// １次元配列上での並びに変換
					int point = int( (rows + radius) * CAMERA_SIZE.x + (cols + radius));
					if( float(texelFetch(maskTexture, point)) == 0 )
					{
						continue;
					}
					float pix_status = float(texelFetch(statusTexture, int(point)));	// ステータス状況を取得
					// 正常状態の画素値を加算
					if(pix_status == 0) {
						interpolation += vec2(texelFetch(offsetTexture, int(point)));
						counter++;
					}
				}
			}
			// 値が取得できた場合，その平均値を新たなoffset値とする
			if(counter > 0){
				interpolation *= (1.0 / counter);
				break;
			}
			radius++;
			counter = 0;
		}

		// 画素対応マップオフセット値を更新
		out_VBO_Data = interpolation;
		// 状態管理用VBOを更新
		//out_VBO_Status = status + 100;

		return;
	}
	//// 常にエラーでかつ保管してもダメな場合は初期化
	//if(105 <= status) {
	//	// 画素対応マップオフセット値を更新
	//	out_VBO_Data = -camPoint_offset - offset_result;
	//	// 状態を初期化
	//	out_VBO_Status = 0;
	//}

}
