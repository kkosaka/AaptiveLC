#version 440

//-----アプリケーションから渡されるVBOデータ-------
layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 vertexUV;

//-----フラグメントシェーダへの出力変数-------
out vec2 UV;

//-----アプリケーション側から渡される変数-------
uniform mat4 transformMatrix_vertex;
uniform mat4 transformMatrix_texture;

void main(void)
{

 //   // 頂点座標をOpenGL座標系へ変換
	//vec4 HomogeneousPoint = vec4(vertexPosition, 0.0, 1.0);
 //   gl_Position = transformMatrix_vertex * HomogeneousPoint;
	//UV = vec2( (transformMatrix_texture * HomogeneousPoint).st );

    // 切り取られた空間での頂点の位置のアウトプット : MVP * position
    gl_Position =  vec4(vertexPosition, 0, 1);
	UV = vertexUV;
 
}
