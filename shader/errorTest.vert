#version 440
#define KERNEL_SIZE 11	// �J�[�l���T�C�Y
#define PIXEL_NUM (KERNEL_SIZE*KERNEL_SIZE)

//-----�A�v���P�[�V����������n�����ϐ�-------

//�ړ��ʂ��������e�N�X�`���o�b�t�@�I�u�W�F�N�g
uniform samplerBuffer indexTexture;	

uniform ivec2 ProjSize;	// �v���W�F�N�^�𑜓x
uniform int ZeroIndex;	// �ړ��ʂ�0�̎��̃C���f�b�N�X�ԍ�

//-----�o��-------
out vec4 out_VBO_Data_Index;

/*
* @brief  KernelSize�̗̈�Ɋ܂܂���f�l���Q�Ƃ��C�ړ��ʂ̕������C������
*/
void main(void)
{
	// ���g�̈ړ��ʂ��擾
	int shiftIndex = int(texelFetch(indexTexture, gl_VertexID).r);
	
	// ���g�̈ړ��ʂ�0�Ȃ�Όv�Z���Ȃ�
	if(shiftIndex == ZeroIndex){
		out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);
		return;
	}
	
	// �J�[�l���̈���̉�f���擾���C�����𓊕[
	int score = 0;
	int half_size = int(floor( KERNEL_SIZE / 2.0 ));
	for(int y = 0; y < KERNEL_SIZE; y++) {
		for(int x = 0; x < KERNEL_SIZE; x++) {
			int  point = gl_VertexID + ProjSize.x * (y - half_size) + (x - half_size);	// ���ォ�瑖������
			int pixels = int(texelFetch(indexTexture, point).r);

			if(pixels > ZeroIndex){ score++; } 	// ��f�l�����̏ꍇ�X�R�A�����Z
			else if(pixels < ZeroIndex){ score--;	}// ��f�l�����̏ꍇ�X�R�A�����Z	
		}
	}

	// ���[���ʂƈړ��ʂ̐�������v������
	if(score > 0){ shiftIndex =  abs(shiftIndex- ZeroIndex) + ZeroIndex; }
	if(score < 0){ shiftIndex =  -abs(shiftIndex - ZeroIndex) + ZeroIndex; }

	// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
	out_VBO_Data_Index = vec4(shiftIndex, 0.0, 0.0, 1.0);

}

