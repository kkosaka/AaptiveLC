#version 440
#define KERNEL_SIZE 11	// カーネルサイズ
#define PIXEL_NUM (KERNEL_SIZE*KERNEL_SIZE)

//-----アプリケーション側から渡される変数-------

//移動量が入ったテクスチャバッファオブジェクト
uniform samplerBuffer indexTexture;	

uniform ivec2 ProjSize;	// プロジェクタ解像度
uniform int ZeroIndex;	// 移動量が0の時のインデックス番号

//-----出力-------
out vec4 out_VBO_Data_Index;

/*
* @brief  KernelSizeの領域に含まれる画素値を参照し，移動量の方向を修正する
*/
void main(void)
{
	// 自身の移動量を取得
	int shiftIndex = int(texelFetch(indexTexture, gl_VertexID).r);
	
	// 自身の移動量が0ならば計算しない
	if(shiftIndex == ZeroIndex){
		out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);
		return;
	}
	
	// カーネル領域内の画素を取得し，正負を投票
	int score = 0;
	int half_size = int(floor( KERNEL_SIZE / 2.0 ));
	for(int y = 0; y < KERNEL_SIZE; y++) {
		for(int x = 0; x < KERNEL_SIZE; x++) {
			int  point = gl_VertexID + ProjSize.x * (y - half_size) + (x - half_size);	// 左上から走査する
			int pixels = int(texelFetch(indexTexture, point).r);

			if(pixels > ZeroIndex){ score++; } 	// 画素値が正の場合スコアを加算
			else if(pixels < ZeroIndex){ score--;	}// 画素値が負の場合スコアを減算	
		}
	}

	// 投票結果と移動量の正負を一致させる
	if(score > 0){ shiftIndex =  abs(shiftIndex- ZeroIndex) + ZeroIndex; }
	if(score < 0){ shiftIndex =  -abs(shiftIndex - ZeroIndex) + ZeroIndex; }

	// 画素対応マップオフセット値を更新
	out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);

}

