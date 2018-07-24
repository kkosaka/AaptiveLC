#version 440
#define KERNEL_SIZE 13
#define PIXEL_NUM (KERNEL_SIZE*KERNEL_SIZE)

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform samplerBuffer shiftTexture;	//�ړ��ʂ�������TBO

uniform ivec2 CamSize;
//uniform int BlurNum;
uniform int kernelSize;

//-----�o��-------
out vec2 out_VBO_Data_shift;


void main(void)
{
	// ���g�̈ړ��ʂ��擾
	vec2 shift = vec2(texelFetch(shiftTexture, gl_VertexID));
	
	// ���g�̈ړ��ʂ�0�Ȃ�Όv�Z���Ȃ�
	if(shift.x ==0 && shift.y == 0){
		out_VBO_Data_shift = shift;
		return;
	}
	
	// ���Ӄs�N�Z���l�̈ړ��ʂ��擾
	vec2 pixels[KERNEL_SIZE][KERNEL_SIZE];

	int kernel_size = KERNEL_SIZE;
	//if(kernel_size > KERNEL_SIZE) {
	//	kernel_size = KERNEL_SIZE;
	//}
	ivec2 score = ivec2(0, 0);
	// kernel_size�̉�f���擾���z��Ɋi�[
	int half_size = int(floor( kernel_size / 2.0 ));
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			int  point = gl_VertexID + CamSize.x * (y - half_size) + (x - half_size);
			pixels[y][x] = vec2(texelFetch(shiftTexture, point));

			if(pixels[y][x].x > 0){ score.x++; } 	// �X�R�A�����Z
			else if(pixels[y][x].x < 0){ score.x--;	}// �X�R�A�����Z	
			// y
			if(pixels[y][x].y > 0){	score.y++; }	// �X�R�A�����Z
			else if(pixels[y][x].y < 0){ score.y--;	}// �X�R�A�����Z
		}
	}

	if(score.x > 0){ shift.x =  abs(pixels[half_size][half_size].x); }
	else if(score.x < 0) { shift.x =  -abs(pixels[half_size][half_size].x); }

	if(score.y > 0){ shift.y =  abs(pixels[half_size][half_size].y); }
	else if(score.y < 0) { shift.y =  -abs(pixels[half_size][half_size].y); }

	// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
	out_VBO_Data_shift = shift;

}

