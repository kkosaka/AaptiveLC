#version 440
#define KERNEL_SIZE 15
#define PIXEL_NUM (KERNEL_SIZE*KERNEL_SIZE)

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform samplerBuffer indexTexture;	//�ړ��ʂ�������TBO

uniform ivec2 ProjSize;
//uniform int BlurNum;
uniform int kernelSize;
uniform int ShiftNum;	// �ړ������
uniform int ZeroIndex;	// �ړ������

//-----�o��-------
out vec4 out_VBO_Data_Index;


void main(void)
{
	// ���g�̈ړ��ʂ��擾
	int shiftIndex = int(texelFetch(indexTexture, gl_VertexID).r);
	
	// ���g�̈ړ��ʂ�0�Ȃ�Όv�Z���Ȃ�
	if(shiftIndex ==ZeroIndex){
		out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);
		return;
	}
	
	// ���Ӄs�N�Z���l�̈ړ��ʃC���f�b�N�X�ԍ����擾
	int pixels[KERNEL_SIZE][KERNEL_SIZE];

	int kernel_size = KERNEL_SIZE;
	//if(kernel_size > KERNEL_SIZE) {
	//	kernel_size = KERNEL_SIZE;
	//}
	int score = 0;
	// kernel_size�̉�f���擾���z��Ɋi�[
	int half_size = int(floor( kernel_size / 2.0 ));
	for(int y = 0; y < kernel_size; y++) {
		for(int x = 0; x < kernel_size; x++) {
			int  point = gl_VertexID + ProjSize.x * (y - half_size) + (x - half_size);
			pixels[y][x] = int(texelFetch(indexTexture, point).r);

			if(pixels[y][x] > ZeroIndex){ score++; } 	// �X�R�A�����Z
			else if(pixels[y][x] < ZeroIndex){ score--;	}// �X�R�A�����Z	
		}
	}

	if(score > 0){ shiftIndex =  abs(shiftIndex- ZeroIndex) + ZeroIndex; }
	if(score < 0){ shiftIndex =  -abs(shiftIndex - ZeroIndex) + ZeroIndex; }


	// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
	out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);

}

