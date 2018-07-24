#version 440

//-----�o�͐�-----
layout(location = 0) out vec4 out_FragData;

//-----�o�[�e�b�N�X�V�F�[�_����n�����e�N�X�`�����W------
in vec2 UV;

//-----�O������n�����e�N�X�`���p�����[�^------
uniform sampler2D imageTexture;

void main (void)
{
	//out_FragData = vec4(textureOffset(imageTexture, UV, movePixel).rgb, 1);
	out_FragData = vec4(texture2D(imageTexture, UV).rgb, 1);
}
