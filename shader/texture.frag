#version 440

out vec4 out_Frag;

// ���_�V�F�[�_����̓���
in vec2 UV;

uniform sampler2D projection_texture;

void main (void)
{
	out_Frag = vec4(texture2D(projection_texture, UV).rgb, 1.0);
}
