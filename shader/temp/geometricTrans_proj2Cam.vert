#version 440

// 頂点データをインプットする。
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec2 offsetUV;

// フラグメントシェーダへ出力
out vec2 UV;

void main(void)
{
    // 切り取られた空間での頂点の位置のアウトプット : MVP * position
    gl_Position = vec4(vertexPosition,1);
	UV = vertexUV;
 
}
