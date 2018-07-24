#ifndef MYUTL_H
#define MYUTL_H

#pragma once

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <opencv2\opencv.hpp>

#include <Windows.h>
#include <MMSystem.h>//FPS

#define USE_HIGH_PERFORMANCE_COUNTER	//������\�p�t�H�[�}���X�J�E���^���g���ꍇ��define


#pragma comment(lib, "winmm.lib")



namespace myUtl{

	/*
	** �t�@�C�����ꊇ�œǂݍ��ރN���X
	*/
	class getAllFiles{

		std::vector < std::string > fileNameList;

	public:
		// �R���X�g���N�^
		getAllFiles(){};

		// �R���X�g���N�^2
		getAllFiles(std::string searchDir, const std::string &str);

		// �f�B�X�g���N�^
		~getAllFiles();

		// �f�B���N�g�����̃t�@�C�����ꊇ�Ǎ�����֐�
		virtual void getAllFileName(std::string searchDir, const std::string &str, std::vector<std::string> &file_list);

		// �擾���ꂽ�f�B���N�g�����̃t�@�C�������X�g�̔z����擾����
		std::vector<std::string> getFileNameList(){
			std::vector<std::string> fileNames;
			std::copy(fileNameList.begin(), fileNameList.end(), back_inserter(fileNames));
			return fileNames;
		}

		// �t�@�C�������X�g���X�V����֐�
		void setFileNameList(std::vector< std::string> &list){
			std::copy(list.begin(), list.end(), back_inserter(fileNameList));
		}
	};

	/*
	** �摜�t�@�C�����ꊇ�œǂݍ��ރN���X
	*/
	class getImageFiles : getAllFiles{

	public:
		// �R���X�g���N�^
		getImageFiles(const std::string searchDir) : getAllFiles()
		{
			std::vector < std::string > file_list;
			getAllImageFileName(searchDir, file_list);
			loadInputImages(file_list, imagesArray);
		};
		// �f�B�X�g���N�^
		~getImageFiles(){};

		// �摜�f�[�^�z�񂩂�w�肵���ԍ��̉摜���擾����
		cv::Mat getImage(unsigned int index);

		// �摜�f�[�^�z��̑傫�����擾����
		unsigned int getFileNum(){ return imagesArray.size(); };

	protected:

		// �摜�f�[�^�̔z��
		std::vector<cv::Mat> imagesArray;

		// �w�肵���f�B���N�g�����̉摜�t�@�C�������ꊇ�擾����֐�
		void getAllImageFileName(const std::string searchDir, std::vector<std::string> &file_list);

		// �t�@�C��������摜���ꊇ�擾����֐�
		bool loadInputImages(std::vector <std::string> &list, cv::vector<cv::Mat> &imagesList);
	};

}


#endif
