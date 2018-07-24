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
	// 最新の画像と入れ替え
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
	{
		if (imageBuffers.size() == bufferSize)
			imageBuffers.pop_back();	//	最も過去フレームを削除
		if (!cap.empty()){
			// 最新フレームを挿入
			imageBuffers.emplace_front(cap);
			return true;
		}
		return false;
	}
}


/**
* @brief   画像を取得
*
* @param	dst[in,out]		画像
*
* @param	num[in]	取得したいバッファ番号(最新画像 0)
*/
bool PGRCamera::getImage(cv::Mat &dst, unsigned int num)
{
	// バッファ情報をすべて読み込み
	boost::shared_lock<boost::shared_mutex> read_lock(mutex);
	if (num < imageBuffers.size()){
		dst = imageBuffers[num].clone();
		return true;
	}


	return false;
}

/**
* @brief   画像を取得
*
* @param	dst[in,out]		画像
*
* @param	num[in]	取得したいバッファ番号(最新画像 0)
*/
bool PGRCamera::getLatencyImage(cv::Mat &dst, unsigned int num)
{
	// バッファ情報をすべて読み込み
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
* @brief   バッファの画像をすべて取得
*
* @param	dst[in,out]		画像
*/
bool PGRCamera::getAllImages(std::vector<cv::Mat> &dst)
{
	ready = false;

	boost::shared_lock<boost::shared_mutex> read_lock(mutex);
	// dequeバッファの情報をすべてコピーする
	//for (unsigned int i = 0; i < (unsigned int)imageBuffers.size(); ++i) {
	//	dst.emplace_back(imageBuffers[i].clone());
	//}

	std::copy(imageBuffers.begin(), imageBuffers.end(), std::back_inserter(dst));
	ready = true;

	return true;
}

/**
* @brief   撮影にかかった時間をバッファに保持
*
* @param	time[in]		時刻
*/
bool PGRCamera::setCaptureTime(double time, bool isEmpty)
{
	// 最新の画像と入れ替え
	boost::upgrade_lock<boost::shared_mutex> up_lock(mutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
	{
		if (!isEmpty){
			if (timeBuffers.size() == bufferSize)
				timeBuffers.pop_back();	//	最も過去フレームを削除
			// 最新フレームを挿入
			timeBuffers.emplace_front(time);
			return true;
		}
		return false;
	}
}
/**
* @brief   バッファの時間をすべて取得
*
* @param	dst[in,out]		画像
*/
bool PGRCamera::getAllTime(std::vector<double> &dst)
{
	ready = false;

	boost::shared_lock<boost::shared_mutex> read_lock(mutex);
	// dequeバッファの情報をすべてコピーする
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
		// 新たに設定したバッファサイズが，以前より小さい場合余分なメモリは削除
		if (size < bufferSize ){
			imageBuffers.resize(size);// キューをリサイズ
			timeBuffers.resize(size);	// キューをリサイズ
			imageBuffers.shrink_to_fit();	// 余分なメモリを削除
			timeBuffers.shrink_to_fit();	// 余分なメモリを削除
		}
		bufferSize = size;
		std::cout << "resize ___ data size :" << bufferSize;
		std::cout << "data num :" << imageBuffers.size() << std::endl;
		return true;
	}
	return false;
}

