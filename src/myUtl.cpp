#include "myUtl.h"

// �R���X�g���N�^
myUtl::getAllFiles::getAllFiles(const std::string searchDir, const std::string &str)
{
	this->getAllFileName(searchDir, str, fileNameList);
}


// �f�B�X�g���N�^
myUtl::getAllFiles::~getAllFiles()
{
	
}


void myUtl::getAllFiles::getAllFileName(const std::string searchDir, const std::string &str, std::vector<std::string> &file_list)
{
	// �J�����g�f�B���N�g���ȉ��̃t�@�C�������擾����
	// �ċA�I�Ƀt�@�C�������擾����ꍇ�́Astd::tr2::sys::recursive_directory_iterator���g��
	for (std::tr2::sys::directory_iterator it(searchDir), end; it != end; ++it) {
		// �摜�t�@�C�������擾
		std::string ext = it->path().extension();
		if (ext == "." + str){
			file_list.emplace_back(it->path());
		}
	}
	// �擾�����t�@�C���������ׂĕ\������
	//for (auto &path : file_list) {
	//	std::cout << path << std::endl;
	//}
}

void myUtl::getImageFiles::getAllImageFileName(const std::string searchDir, std::vector<std::string> &file_list)
{
	// �J�����g�f�B���N�g���ȉ��̃t�@�C�������擾����
	// �ċA�I�Ƀt�@�C�������擾����ꍇ�́Astd::tr2::sys::recursive_directory_iterator���g��
	for (std::tr2::sys::directory_iterator it(searchDir), end; it != end; ++it) {
		// �摜�t�@�C�������擾
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
	// �ǂݍ��މ摜��������Ȃ������ꍇ
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