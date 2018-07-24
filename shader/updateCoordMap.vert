#version 440

//-----CPU����n�����VBO-------
layout(location = 0) in vec2 projPoint;			// �v���W�F�N�^���W
layout(location = 1) in vec2 camPoint;			// �����v���̃J�������W�l
layout(location = 2) in vec2 camPoint_offset;	// �ߋ��̃I�t�Z�b�g�l
layout(location = 3) in vec2 camPoint_shift;	// ��f�Ή��C���̈ړ���
layout(location = 4) in vec2 offset_result;		// �I�t�Z�b�g�l

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform samplerBuffer statusTexture;	//��ԊǗ���TBO
uniform samplerBuffer offsetTexture;	//��ԊǗ���TBO
uniform samplerBuffer maskTexture;
uniform ivec2 CAMERA_SIZE;

//-----�o��-------
out vec2 out_VBO_Data;
out float out_VBO_Status;

void main(void)
{
	// ��Ԃ�ێ�
	float status = float(texelFetch(statusTexture, int(gl_VertexID)));

	// �ړ��ʂ�0�̏ꍇ�͐������␳�ł��Ă�����̂Ƃ݂Ȃ�
	if(offset_result.x == 0 && offset_result.y == 0){
		// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
		out_VBO_Data = camPoint_offset + offset_result;
		// ��Ԃ�������
		out_VBO_Status = 0;
		return;
	}
	
	// ��ԊǗ��pVBO���X�V
	out_VBO_Status = status + 1;

	// �v�Z�����I�t�Z�b�g�l
	vec2 newOffset = camPoint_offset + offset_result;
	
	//if(abs(newOffset[0]) > 20 || abs(newOffset[1]) > 20)
	//	newOffset = vec2(0.0, 0.0);

	// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
	out_VBO_Data = newOffset;
	
	// ��Ԃ���ɃG���[�̏ꍇ
	if(3 < status && status < 105) {
		
		// ���g�̗�ƍs�ԍ����v�Z
		int m = int(mod(float(gl_VertexID), float(CAMERA_SIZE.x)));
		int cols = m;	
		int rows = int( float(gl_VertexID - m) / float(CAMERA_SIZE.x) );
		 
		// ���ӂ̕␳�ł��Ă���̈悩���Ԃ���

		vec2 interpolation = vec2(0.0, 0.0);
		int counter = 0;
		// ���X�ɔ��a���L���Ă���
		for(int radius = 1; radius < 30; radius++){
			for(int y = -radius; y <= radius; y++) {
				for(int x = -radius; x <= radius; x++) {
					// �T���͊O���������ł悢
					if(abs(y) != radius && abs(x) != radius){
						continue;
					}
					// �P�����z���ł̕��тɕϊ�
					int point = int( (rows + radius) * CAMERA_SIZE.x + (cols + radius));
					if( float(texelFetch(maskTexture, point)) == 0 )
					{
						continue;
					}
					float pix_status = float(texelFetch(statusTexture, int(point)));	// �X�e�[�^�X�󋵂��擾
					// �����Ԃ̉�f�l�����Z
					if(pix_status == 0) {
						interpolation += vec2(texelFetch(offsetTexture, int(point)));
						counter++;
					}
				}
			}
			// �l���擾�ł����ꍇ�C���̕��ϒl��V����offset�l�Ƃ���
			if(counter > 0){
				interpolation *= (1.0 / counter);
				break;
			}
			radius++;
			counter = 0;
		}

		// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
		out_VBO_Data = interpolation;
		// ��ԊǗ��pVBO���X�V
		//out_VBO_Status = status + 100;

		return;
	}
	//// ��ɃG���[�ł��ۊǂ��Ă��_���ȏꍇ�͏�����
	//if(105 <= status) {
	//	// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
	//	out_VBO_Data = -camPoint_offset - offset_result;
	//	// ��Ԃ�������
	//	out_VBO_Status = 0;
	//}

}
