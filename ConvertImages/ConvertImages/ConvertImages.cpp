/*===================================================================
* FILENAME :  ConvertImages.cpp
* Copyright(C)   2017-2018  Jenoptik Industrial Metrology Germany GMbh
* DESCRIPTION :
*       This software Converts the Bitmap reference Images  to Png format
* AUTHOR :    Subhasis Pradhan        START DATE :    01.04.2018
* CHANGES :
*	First release : 01.14.2018
*
===========================================================================*/
#define _AFXDLL
#include <afx.h>

#include <atlimage.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include<vector>
#include <numeric>

#include <iostream>
#include <math.h>

#include <filesystem>
#include <string>

#include <iostream>
#include <fstream>
#include <cstdint>

#include <regex>
#include <type_traits>


using namespace cv;
using namespace std;

namespace fs = std::experimental::filesystem;
wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.from_bytes(str);
}

string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes(wstr);
}

cv::Mat Load(wstring filename)
{
	cv::Mat image;
	//Read File	
	{
		FILE* fp = _wfopen(filename.c_str(), L"rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			long sz = ftell(fp);
			char* buf = new char[sz];
			fseek(fp, 0, SEEK_SET);
			long n = fread(buf, 1, sz, fp);
			_InputArray arr(buf, sz);
			image = imdecode(arr, IMREAD_UNCHANGED);

			//IMREAD_UNCHANGED  IMREAD_GRAYSCALE
			int channels = image.channels();

			delete[] buf;
			fclose(fp);

		}

	}

	return image;
}


bool Save(wstring filename, cv::Mat& image)
{

	if (!image.data)
		return false;



	fs::path filePath(filename);


	std::wstring fName = filePath.filename().wstring();


	int idx = fName.rfind('.');
	std::string extension;

	if (idx != std::string::npos)
	{
		extension = ws2s(fName.substr(idx));
		/*extension = "." + extension;*/
	}
	else
		return false;


	//Save Image
	{
		std::vector<uchar> buff;
		//cv::imencode(extension.GetBuffer(), image, buff);
		cv::imencode(extension, image, buff);

		std::ofstream outfile(filename, std::ofstream::binary);
		//std::wofstream outfile(filename, std::ofstream::binary);

		outfile.write(reinterpret_cast<const char*>(buff.data()), buff.size());
		outfile.close();
	}

	return true;
}


// list of paths of all files under the directory 'dir' when the extenstion matches the regex
// file_list<true> searches recursively into sub-directories; file_list<false> searches only the specified directory
template < bool RECURSIVE > std::vector<fs::path> file_list(fs::path dir, std::regex ext_pattern)
{
	std::vector<fs::path> result;

	using iterator = std::conditional< RECURSIVE,
		fs::recursive_directory_iterator, fs::directory_iterator >::type;

	const iterator end;
	for (iterator iter{ dir }; iter != end; ++iter)
	{
		const std::string extension = iter->path().extension().string();
		if (fs::is_regular_file(*iter) && std::regex_match(extension, ext_pattern)) result.push_back(*iter);
	}

	return result;
}

// literal '.' followed by one of "cpp", "cc", "cxx", "h", "hh", "hpp" or "hxx"
// note: ?: indicates that it is a non-capturing group
static const std::regex bmp_files("\\.(?:bmp)");


// recursive scan for c++ files: if dir is omitted, current directory is scanned 
std::vector<fs::path> rscan_bmp_files(fs::path dir = ".") { return file_list<true>(dir, bmp_files); }


void PrintHeader()
{
	std::cout << "Command For Converting Images :" <<endl<< "ConvertImage.exe <Root Folder Path of Bitmap images> <-d>(optional to delete orginal images)" << endl;	
	std::cout << endl << "ex: ConvertImage.exe D:\\Userdata -d(optional) " << endl << endl;
}

int _tmain(int argc, wchar_t* argv[])
{
	int ret = 0;
	std::wstring directorypath = _T("");
	bool fDeleteOriginal = false;

	if (argc<2)
	{
		std::cout << endl <<"Not enough  Arguments, No of Arguments : " <<argc<< endl << endl;		
		PrintHeader();
		ret = -1;
	}
	else
	{
		directorypath = std::wstring(argv[1]);		

		while ((directorypath.rbegin() != directorypath.rend() && *directorypath.rbegin() == '\\') || (directorypath.rbegin() != directorypath.rend() && *directorypath.rbegin() == '\"'))
			directorypath.pop_back();

		
		if (argc == 3)
		{
			std::wstring temp = argv[2];
			fDeleteOriginal = (wcscmp(argv[2], L"-d") == 0);
		}
	}
		
	//const fs::path win_dir = "C:\\Pradhan\\temp\\Test"; // adjust as required

	fs::path dirPath(directorypath);
	if (!directorypath.empty())
		// print files with extension ".log" in the directory "C:/Windows/Logs" (recursive)
		for (const auto& file_path : rscan_bmp_files(dirPath))
		{

			bool isFile = fs::is_regular_file(file_path);
			{

				//std::cout << file_path.parent_path().stem() << '\n';
				std::wcout << file_path.wstring() << '\n';

				fs::path path = file_path;
				std::wstring filename = path.filename().wstring();

				std::wstring DirectoryName = file_path.parent_path().wstring();
				//std::wcout << DirectoryName << '\n';


				//size_t pos = filename.find_first_of(L"_");
				size_t pos = filename.find_last_of(L"_");
				if (pos != string::npos)
				{
					//pos = filename.find_first_of(L"_", pos + 1);
					pos = filename.find_last_of(L"_", pos - 1);
					if (pos != string::npos)
					{
						wstring prefix = filename.substr(0, pos);
						wstring postfix = filename.substr(pos);

						postfix = regex_replace(postfix, std::wregex(L"bmp"), L"png");

						wstring newFilename = DirectoryName + L"\\" + prefix + L"_0" + postfix;
						std::wcout << newFilename << '\n';

						cv::Mat src = Load(file_path.wstring());
						if (!src.empty())
						{
							Save(newFilename, src);
							if (fDeleteOriginal)
								_wremove(file_path.wstring().c_str());
						}

					}
				}



			}


		}
	
	std::cout << "\n----------------------------------------------\n";

#ifdef DEBUG
	system("pause");
#endif
	return ret;
}