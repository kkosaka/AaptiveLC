#version 440
#define KERNEL_SIZE 11
#define PIXEL_NUM (KERNEL_SIZE*KERNEL_SIZE)

layout(location = 0) in float mask ;			// �J�������W

//-----�A�v���P�[�V����������n�����ϐ�-------
uniform samplerBuffer offsetTexture;	//offset�l��������TBO
uniform samplerBuffer maskTexture;	//mask��������TBO

uniform ivec2 CamSize;
uniform int BlurNum;

//-----�o��-------
out vec2 out_VBO_Data;

double gauss_filter3[3][3] = {
	{1.0/16.0, 2.0/16.0, 1.0/16.0},
	{2.0/16.0, 4.0/16.0, 2.0/16.0},
	{1.0/16.0, 2.0/16.0, 1.0/16.0},
};

double gauss_filter5[5][5] = {
	{1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
	{4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
	{6.0/256.0, 24.0/256.0, 36.0/256.0, 24.0/256.0, 6.0/256.0},
	{4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
	{1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
};

double gauss_filter7[7][7] = {
	{1.0/4096.0, 6.0/4096.0, 15.0/4096.0, 20.0/4096.0, 15.0/4096.0, 6.0/4096.0, 1.0/4096.0},
	{6.0/4096.0, 36.0/4096.0, 90.0/4096.0, 120.0/4096.0, 90.0/4096.0, 36.0/4096.0, 6.0/4096.0},
	{15.0/4096.0, 90.0/4096.0, 225.0/4096.0, 300.0/4096.0, 225.0/4096.0, 90.0/4096.0, 15.0/4096.0},
	{20.0/4096.0, 120.0/4096.0, 300.0/4096.0, 400.0/4096.0, 300.0/4096.0, 120.0/4096.0, 20.0/4096.0},
	{15.0/4096.0, 90.0/4096.0, 225.0/4096.0, 300.0/4096.0, 225.0/4096.0, 90.0/4096.0, 15.0/4096.0},
	{6.0/4096.0, 36.0/4096.0, 90.0/4096.0, 120.0/4096.0, 90.0/4096.0, 36.0/4096.0, 6.0/4096.0},
	{1.0/4096.0, 6.0/4096.0, 15.0/4096.0, 20.0/4096.0, 15.0/4096.0, 6.0/4096.0, 1.0/4096.0},
};

// �ړ����σt�B���^
vec2 movingAverageFilter(vec2 src[KERNEL_SIZE][KERNEL_SIZE], int kernel)
{
	vec2 dst = vec2(0.0, 0.0);
	for(int y = 0; y < kernel; y++) {
		for(int x = 0; x < kernel; x++) {
			dst += src[y][x] ;
		}
	}

	float pixelNum = float(kernel * kernel);
	float div = 1.0 / pixelNum;
	dst *= div;

	return dst;
}

// �K�E�X�t�B���^
vec2 gaussFilter(vec2 src[KERNEL_SIZE][KERNEL_SIZE], int kernel)
{
	vec2 dst = vec2(0.0, 0.0);
	if(kernel == 3) {
		for(int y = 0; y < kernel; y++) {
			for(int x = 0; x < kernel; x++) {
				dst += vec2(src[y][x] * gauss_filter3[y][x]);
			}
		}
	}
	else if(kernel == 5) {
		for(int y = 0; y < kernel; y++) {
			for(int x = 0; x < kernel; x++) {
				dst += vec2(src[y][x] * gauss_filter5[y][x]);
			}
		}
	}
	else {
		for(int y = 0; y < kernel; y++) {
			for(int x = 0; x < kernel; x++) {
				dst += vec2(src[y][x] * gauss_filter7[y][x]);
			}
		}
	}
	return dst;
}

/* �o�u���\�[�g
* a     : �\�[�g����z��
*/
float bubblesort(float a[PIXEL_NUM], int size)
{
	for(int i = 0; i < size -1; ++i) {
		for(int j = size - 1 ; j > i; j--) {
			if(a[j - 1] > a[j]) { /*�O�̗v�f���傫�����������*/
				float tmp = a[j];
				a[j] = a[j - 1];
				a[j - 1] = tmp;
			}
		}
	}
	int middle = int(ceil(KERNEL_SIZE / 2.0) - 1);
	return a[middle];
}

// ���f�B�A���t�B���^
vec2 medianFilter(vec2 src[KERNEL_SIZE][KERNEL_SIZE], int kernel)
{
	vec2 dst = vec2(0.0, 0.0);
	// x, y�ŏ����𕪂���
	float array[2][PIXEL_NUM]; 
	for(int y = 0; y < kernel; y++) {
		for(int x = 0; x < kernel; x++) {
			array[0][y * kernel + x] = src[y][x].x;
			array[1][y * kernel + x] = src[y][x].y;
		}
	}
	// �\�[�g���Ē��Ԓl���擾����
	dst[0] = bubblesort(array[0], PIXEL_NUM);
	dst[1] = bubblesort(array[1], PIXEL_NUM);

	return dst;
}

void main(void)
{
	float mask_val = float(texelFetch(maskTexture, gl_VertexID));
	// ���������f
	if(mask_val == 0) {
		out_VBO_Data = vec2(0.0, 0.0);
		return;
	}

	vec2 pixels[KERNEL_SIZE][KERNEL_SIZE];


	// kernel_size�̉�f���擾���z��Ɋi�[
	int half_size = int(floor( KERNEL_SIZE / 2.0 ));
	
	// ���Ӄs�N�Z���l���擾
	for(int y = 0; y < KERNEL_SIZE; y++) {
		for(int x = 0; x < KERNEL_SIZE; x++) {
			int  point = gl_VertexID + CamSize.x * (y - half_size) + (x - half_size);
			if( float(texelFetch(maskTexture, point)) == 0 ) {
				pixels[y][x] = vec2(texelFetch(offsetTexture, gl_VertexID));
			}
			else{
				pixels[y][x] = vec2(texelFetch(offsetTexture, point));
			}
		}
	}
	// blur��������
	vec2 newOffset = vec2(0.0, 0.0); //gl_VertexID�͎��g�̃C���f�b�N�X�ԍ�������

	switch(BlurNum){
	case 0:
		{
			int kernel_size = KERNEL_SIZE;
			if(kernel_size > 7)
				kernel_size = 7;
			newOffset = gaussFilter(pixels, kernel_size);
			break;
		}
	case 1:
		{
			newOffset = movingAverageFilter(pixels, KERNEL_SIZE);
			break;
		}
	case 2:
		{
			newOffset = medianFilter(pixels, KERNEL_SIZE);
		}
	default:
		{
			break;
		}
	}
	

	if(abs(newOffset[0]) > 9 || abs(newOffset[1]) > 9)
		newOffset = vec2(0.0, 0.0);
	
	//if(abs(newOffset[0]) < 0.2 || abs(newOffset[1]) < 0.2)
	//	newOffset = vec2(0.0, 0.0);

	// ��f�Ή��}�b�v�I�t�Z�b�g�l���X�V
	out_VBO_Data = newOffset;

}

