#version 440

// 頂点データをインプットする。
layout(location = 0) in vec2 projPoint;			// プロジェクタ座標
layout(location = 1) in vec2 camPoint;			// 初期計測のカメラ座標値
layout(location = 2) in vec2 camPoint_offset;	// 移動後のカメラ座標値
layout(location = 3) in vec2 camPoint_shift;	// 画素対応修正の移動量

//-----アプリケーション側から渡される変数-------
uniform mat4 transformMatrix_vertex;
uniform mat4 transformMatrix_texture;

//-----フラグメントシェーダへの出力変数-------
out vec2 UV;

void main(void)
{
    // 頂点座標をOpenGL座標系へ変換
	vec4 translatedCamPoint = vec4(camPoint + camPoint_offset, 0.0, 1.0);
    gl_Position = transformMatrix_vertex * translatedCamPoint;
 
	// テクスチャ座標をOpenGL座標系へ変換
	vec4 HomogeneousProjPoint = vec4(projPoint, 0.0, 1.0);
	UV = vec2( (transformMatrix_texture * HomogeneousProjPoint).st );
}
