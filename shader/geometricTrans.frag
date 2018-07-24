#version 440

//-----出力先-----
layout(location = 0) out vec4 out_FragData;

//-----バーテックスシェーダから渡されるテクスチャ座標------
in vec2 UV;

//-----外部から渡されるテクスチャパラメータ------
uniform sampler2D imageTexture;

void main (void)
{
	//out_FragData = vec4(textureOffset(imageTexture, UV, movePixel).rgb, 1);
	out_FragData = vec4(texture2D(imageTexture, UV).rgb, 1);
}
