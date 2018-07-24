#include "myUtl.h"

// コンストラクタ
myUtl::getAllFiles::getAllFiles(const std::string searchDir, const std::string &str)
{
	this->getAllFileName(searchDir, str, fileNameList);
}


// ディストラクタ
myUtl::getAllFiles::~getAllFiles()
{
	
}


void myUtl::getAllFiles::getAllFileName(const std::string searchDir, const std::string &str, std::vector<std::string> &file_list)
{
	// カレントディレクトリ以下のファイル名を取得する
	// 再帰的にファイル名を取得する場合は、std::tr2::sys::recursive_directory_iteratorを使う
	for (std::tr2::sys::directory_iterator it(searchDir), end; it != end; ++it) {
		// 画像ファイルだけ取得
		std::string ext = it->path().extension();
		if (ext == "." + str){
			file_list.emplace_back(it->path());
		}
	}
	// 取得したファイル名をすべて表示する
	//for (auto &path : file_list) {
	//	std::cout << path << std::endl;
	//}
}

void myUtl::getImageFiles::getAllImageFileName(const std::string searchDir, std::vector<std::string> &file_list)
{
	// カレントディレクトリ以下のファイル名を取得する
	// 再帰的にファイル名を取得する場合は、std::tr2::sys::recursive_directory_iteratorを使う
	for (std::tr2::sys::directory_iterator it(searchDir), end; it != end; ++it) {
		// 画像ファイルだけ取得
		std::string ext = it->path().extension();
		if (ext == ".jpg" ||
			ext == ".JPG" ||
			ext == ".png" ||
			ext == ".PNG" ||
			ext == ".bmp" ||
			ext == ".BMP"
			){
			file_list.emplace_back(it->path());
		}
	}
}


bool myUtl::getImageFiles::loadInputImages(std::vector <std::string> &list, cv::vector<cv::Mat> &imagesList)
{
	unsigned int fileNum = (unsigned int)list.size();
	// 読み込む画像が見つからなかった場合
	if (fileNum == 0){
		std::cout << "no image" << std::endl;
		return false;
	}
	for (unsigned int i = 0; i < fileNum; i++){
		imagesList.emplace_back(cv::imread(list[i].c_str()));
	}
	return true;
}

cv::Mat myUtl::getImageFiles::getImage(unsigned int index)
{
	if (imagesArray.size() != 0 && index < (unsigned int)imagesArray.size())
		return imagesArray[index];

	cv::Mat img;
	return img;
}