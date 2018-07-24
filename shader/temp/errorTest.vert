#version 440
#define KERNEL_SIZE 13
#define PIXEL_NUM (KERNEL_SIZE*KERNEL_SIZE)

//-----アプリケーション側から渡される変数-------
uniform samplerBuffer shiftTexture;	//移動量が入ったTBO

uniform ivec2 CamSize;
//uniform int BlurNum;
uniform int kernelSize;

//-----出力-------
out vec2 out_VBO_Data_shift;


void main(void)
{
	// 自身の移動量を取得
	vec2 shift = vec2(texelFetch(shiftTexture, gl_VertexID));
	
	// 自身の移動量が0ならば計算しない
	if(shift.x ==0 && shift.y == 0){
		out_VBO_Data_shift = shift;
		return;
	}
	
	// 周辺ピクセル値の移動量を取得
	vec2 pixels[KERNEL_SIZE][KERNEL_SIZE];

	int kernel_size = KERNEL_SIZE;
	//if(kernel_size > KERNEL_SIZE) {
	//	kernel_size = KERNEL_SIZE;
	//}
	ivec2 score = ivec2(0, 0);
	// kernel_sizeの画素を取得し配列に格納
	int half_size = int(floor( kernel_size / 2.0 ));
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			int  point = gl_VertexID + CamSize.x * (y - half_size) + (x - half_size);
			pixels[y][x] = vec2(texelFetch(shiftTexture, point));

			if(pixels[y][x].x > 0){ score.x++; } 	// スコアを加算
			else if(pixels[y][x].x < 0){ score.x--;	}// スコアを減算	
			// y
			if(pixels[y][x].y > 0){	score.y++; }	// スコアを加算
			else if(pixels[y][x].y < 0){ score.y--;	}// スコアを減算
		}
	}

	if(score.x > 0){ shift.x =  abs(pixels[half_size][half_size].x); }
	else if(score.x < 0) { shift.x =  -abs(pixels[half_size][half_size].x); }

	if(score.y > 0){ shift.y =  abs(pixels[half_size][half_size].y); }
	else if(score.y < 0) { shift.y =  -abs(pixels[half_size][half_size].y); }

	// 画素対応マップオフセット値を更新
	out_VBO_Data_shift = shift;

}

