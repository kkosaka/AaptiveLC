#version 440

// 頂点データをインプットする。
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;	// 初期計測のカメラ座標値
layout(location = 2) in vec2 offset;	// 移動後のカメラ座標値
layout(location = 3) in vec2 shiftUV;	// 画素対応修正の移動量

// フラグメントシェーダへ出力
out vec2 UV;
// 頂点バッファオブジェクトへ出力
out vec2 out_VBO_Data;

uniform ivec2 movePixel;

void main(void)
{
    // 切り取られた空間での頂点の位置のアウトプット : MVP * position
    gl_Position = vec4(vertexPosition,1);
	vec2 transrate = vec2( shiftUV[0] / (1920.0 - 1.0), shiftUV[1] / (1200.0 - 1.0) );
	vec2 offset_gl = vec2( offset[0] / (1920.0 - 1.0), offset[1] / (1200.0 - 1.0) );
	UV = vertexUV + offset_gl + transrate;

	out_VBO_Data = offset;
}
