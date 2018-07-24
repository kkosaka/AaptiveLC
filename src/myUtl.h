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

#define USE_HIGH_PERFORMANCE_COUNTER	//高分解能パフォーマンスカウンタを使う場合はdefine


#pragma comment(lib, "winmm.lib")



namespace myUtl{

	/*
	** ファイルを一括で読み込むクラス
	*/
	class getAllFiles{

		std::vector < std::string > fileNameList;

	public:
		// コンストラクタ
		getAllFiles(){};

		// コンストラクタ2
		getAllFiles(std::string searchDir, const std::string &str);

		// ディストラクタ
		~getAllFiles();

		// ディレクトリ内のファイルを一括読込する関数
		virtual void getAllFileName(std::string searchDir, const std::string &str, std::vector<std::string> &file_list);

		// 取得されたディレクトリ内のファイル名リストの配列を取得する
		std::vector<std::string> getFileNameList(){
			std::vector<std::string> fileNames;
			std::copy(fileNameList.begin(), fileNameList.end(), back_inserter(fileNames));
			return fileNames;
		}

		// ファイル名リストを更新する関数
		void setFileNameList(std::vector< std::string> &list){
			std::copy(list.begin(), list.end(), back_inserter(fileNameList));
		}
	};

	/*
	** 画像ファイルを一括で読み込むクラス
	*/
	class getImageFiles : getAllFiles{

	public:
		// コンストラクタ
		getImageFiles(const std::string searchDir) : getAllFiles()
		{
			std::vector < std::string > file_list;
			getAllImageFileName(searchDir, file_list);
			loadInputImages(file_list, imagesArray);
		};
		// ディストラクタ
		~getImageFiles(){};

		// 画像データ配列から指定した番号の画像を取得する
		cv::Mat getImage(unsigned int index);

		// 画像データ配列の大きさを取得する
		unsigned int getFileNum(){ return imagesArray.size(); };

	protected:

		// 画像データの配列
		std::vector<cv::Mat> imagesArray;

		// 指定したディレクトリ内の画像ファイル名を一括取得する関数
		void getAllImageFileName(const std::string searchDir, std::vector<std::string> &file_list);

		// ファイル名から画像を一括取得する関数
		bool loadInputImages(std::vector <std::string> &list, cv::vector<cv::Mat> &imagesList);
	};

}


#endif
