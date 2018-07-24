#include "PGRCameraThread.h"


void PGRCamera::run()
{
	while (!threadStopFlag){
		cv::Mat cap;
		if (ready)
		{
			TPGROpenCV::CameraCapture(cap);
			// write
			//fps->begin();
			setImage(cap);
			//std::cout << fps->getElapsed() << std::endl;

			setCaptureTime(fps->getElapsed(), cap.empty());
			capCount++;
			//TPGROpenCV::showCapImg(cap);
		}
		else{
			std::cout << "stop capture" << std::endl;
		}
		//fps->end();
		//fps->show(500);
	}
}

bool PGRCamera::setImage(cv::Mat &cap)
{
	// �ŐV�̉摜�Ɠ���ւ�
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
	{
		if (imageBuffers.size() == bufferSize)
			imageBuffers.pop_back();	//	�ł��ߋ��t���[�����폜
		if (!cap.empty()){
			// �ŐV�t���[����}��
			imageBuffers.emplace_front(cap);
			return true;
		}
		return false;
	}
}


/**
* @brief   �摜���擾
*
* @param	dst[in,out]		�摜
*
* @param	num[in]	�擾�������o�b�t�@�ԍ�(�ŐV�摜 0)
*/
bool PGRCamera::getImage(cv::Mat &dst, unsigned int num)
{
	// �o�b�t�@�������ׂēǂݍ���
	boost::shared_lock<boost::shared_mutex> read_lock(mutex);
	if (num < imageBuffers.size()){
		dst = imageBuffers[num].clone();
		return true;
	}


	return false;
}

/**
* @brief   �摜���擾
*
* @param	dst[in,out]		�摜
*
* @param	num[in]	�擾�������o�b�t�@�ԍ�(�ŐV�摜 0)
*/
bool PGRCamera::getLatencyImage(cv::Mat &dst, unsigned int num)
{
	// �o�b�t�@�������ׂēǂݍ���
	boost::shared_lock<boost::shared_mutex> read_lock(mutex);
	int capBeforeNum = capCount;
	int capnum = 0;
	bool stopFlag = false;
	while (capnum < num){
		capnum = capCount - capBeforeNum;
		std::cout << "counter camera thread : " << capCount << std::endl;
		Sleep(30);
	}
	if (num < imageBuffers.size()){
		dst = imageBuffers[0].clone();
		return true;
	}

	return false;
}
/**
* @brief   �o�b�t�@�̉摜�����ׂĎ擾
*
* @param	dst[in,out]		�摜
*/
bool PGRCamera::getAllImages(std::vector<cv::Mat> &dst)
{
	ready = false;

	boost::shared_lock<boost::shared_mutex> read_lock(mutex);
	// deque�o�b�t�@�̏������ׂăR�s�[����
	//for (unsigned int i = 0; i < (unsigned int)imageBuffers.size(); ++i) {
	//	dst.emplace_back(imageBuffers[i].clone());
	//}

	std::copy(imageBuffers.begin(), imageBuffers.end(), std::back_inserter(dst));
	ready = true;

	return true;
}

/**
* @brief   �B�e�ɂ����������Ԃ��o�b�t�@�ɕێ�
*
* @param	time[in]		����
*/
bool PGRCamera::setCaptureTime(double time, bool isEmpty)
{
	// �ŐV�̉摜�Ɠ���ւ�
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
	{
		if (!isEmpty){
			if (timeBuffers.size() == bufferSize)
				timeBuffers.pop_back();	//	�ł��ߋ��t���[�����폜
			// �ŐV�t���[����}��
			timeBuffers.emplace_front(time);
			return true;
		}
		return false;
	}
}
/**
* @brief   �o�b�t�@�̎��Ԃ����ׂĎ擾
*
* @param	dst[in,out]		�摜
*/
bool PGRCamera::getAllTime(std::vector<double> &dst)
{
	ready = false;

	boost::shared_lock<boost::shared_mutex> read_lock(mutex);
	// deque�o�b�t�@�̏������ׂăR�s�[����
	//for (unsigned int i = 0; i < (unsigned int)timeBuffers.size(); ++i) {
	//	dst.emplace_back(timeBuffers[i]);
	//}
	std::copy(timeBuffers.begin(), timeBuffers.end(), std::back_inserter(dst));
	ready = true;

	return true;
}


bool PGRCamera::clear()
{
	ready = false;
	Sleep(20);
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
	imageBuffers.clear();
	timeBuffers.clear();
	//std::cout << "clear ____ data num :" << imageBuffers.size() << std::endl;
	ready = true;

	return true;

}

bool PGRCamera::resize(const unsigned int size)
{

	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
	Sleep(20);
	if (size > 0){
		// �V���ɐݒ肵���o�b�t�@�T�C�Y���C�ȑO��菬�����ꍇ�]���ȃ������͍폜
		if (size < bufferSize ){
			imageBuffers.resize(size);// �L���[�����T�C�Y
			timeBuffers.resize(size);	// �L���[�����T�C�Y
			imageBuffers.shrink_to_fit();	// �]���ȃ��������폜
			timeBuffers.shrink_to_fit();	// �]���ȃ��������폜
		}
		bufferSize = size;
		std::cout << "resize ___ data size :" << bufferSize;
		std::cout << "data num :" << imageBuffers.size() << std::endl;
		return true;
	}
	return false;
}

