#version 440
#define KERNEL_SIZE 15
#define PIXEL_NUM (KERNEL_SIZE*KERNEL_SIZE)

//-----アプリケーション側から渡される変数-------
uniform samplerBuffer indexTexture;	//移動量が入ったTBO

uniform ivec2 ProjSize;
//uniform int BlurNum;
uniform int kernelSize;
uniform int ShiftNum;	// 移動する回数
uniform int ZeroIndex;	// 移動する回数

//-----出力-------
out vec4 out_VBO_Data_Index;


void main(void)
{
	// 自身の移動量を取得
	int shiftIndex = int(texelFetch(indexTexture, gl_VertexID).r);
	
	// 自身の移動量が0ならば計算しない
	if(shiftIndex ==ZeroIndex){
		out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);
		return;
	}
	
	// 周辺ピクセル値の移動量インデックス番号を取得
	int pixels[KERNEL_SIZE][KERNEL_SIZE];

	int kernel_size = KERNEL_SIZE;
	//if(kernel_size > KERNEL_SIZE) {
	//	kernel_size = KERNEL_SIZE;
	//}
	int score = 0;
	// kernel_sizeの画素を取得し配列に格納
	int half_size = int(floor( kernel_size / 2.0 ));
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			int  point = gl_VertexID + ProjSize.x * (y - half_size) + (x - half_size);
			pixels[y][x] = int(texelFetch(indexTexture, point).r);

			if(pixels[y][x] > ZeroIndex){ score++; } 	// スコアを加算
			else if(pixels[y][x] < ZeroIndex){ score--;	}// スコアを減算	
		}
	}

	if(score > 0){ shiftIndex =  abs(shiftIndex- ZeroIndex) + ZeroIndex; }
	if(score < 0){ shiftIndex =  -abs(shiftIndex - ZeroIndex) + ZeroIndex; }


	// 画素対応マップオフセット値を更新
	out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);

}

