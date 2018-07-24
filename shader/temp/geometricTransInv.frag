#version 440

//-----出力先-----
//layout(location = 0) out vec4 out_FragData;
layout(location = 0) out float out_FragData;

//-----バーテックスシェーダから渡されるテクスチャ座標------
in vec2 UV;

//-----外部から渡されるテクスチャパラメータ------
uniform sampler2D imageTexture;
uniform sampler2D maskTexture;

// 移動用がゼロのときのインデックス番号
uniform int MEDIAN;

void main (void)
{
	vec3 mask = texture2D(maskTexture, UV).rgb;
	if(mask[0] + mask[1] + mask[2] == 0)
	{
		//out_FragData = vec4(MEDIAN, 0, 0, 1);
		out_FragData = float(MEDIAN);
		return;
	}
	//out_FragData = vec4(texture2D(imageTexture, UV).r, 0, 0, 1);
	out_FragData = float(texture2D(imageTexture, UV).r);

	//vec2 size = textureSize(imageTexture, 0);
	//out_FragData = vec4(size, 0, 0);
}
