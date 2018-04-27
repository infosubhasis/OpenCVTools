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


bool Save(wstring filename,cv::Mat& image)
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



int EncodeMask(std::wstring dirpath)
{
	std::wcout << L"Encoding Mask...."  << std::endl;
	cv::Mat encodedImage;
	
	int iMaskCode = 0;

	for (auto & p : fs::directory_iterator(dirpath))
	{
		//std::cout << p << std::endl;

		fs::path path = p;
		std::wstring filename = path.filename().wstring();

		std::wcout << "Filename : " << filename << std::endl;

		std::string temp = ws2s(filename);
		temp = path.filename().string();

		size_t lastindex = filename.find_last_of(L".");
		wstring rawname = filename.substr(0, lastindex);

		//std::wcout << "rawname : " << rawname << std::endl;
		std::wstring m = L"Mask";


		std::wstring maskNumber = L"";
		std::string::size_type found = rawname.find(m);

		if (found != std::string::npos)
			maskNumber = rawname.substr(found + m.length(), rawname.length());

		int i = -1;
		try
		{
			i = std::stoi(maskNumber);
			i = 9 - i;
			//std::wstring ws = L"456";
			//int i = std::stoi(ws); // convert to int
			//std::wstring ws2 = std::to_wstring(i); // and back to wstring
		}
		catch (...)
		{
			i = -1;
		}


		if (i >= 1 && i <= 8)
		{
			i -= 1;
			iMaskCode += (1 << i);

			Mat src = Load(path);

			int channels = src.channels();
			if (src.empty())
			{
				std::wcout << "Can't Load : " << filename << std::endl;
				return -1;
			}

			if (channels > 1)
			{
				cv::cvtColor(src, src, CV_BGRA2GRAY);
				/*std::cout << " Mask has to be Grayscale Image " << std::endl;
				return -1;*/
			}


			if (encodedImage.empty())
			{
				encodedImage = cv::Mat::zeros(src.size(), src.type());
			}


			if (src.size() != encodedImage.size() || src.type() || encodedImage.type())
			{
				std::wcout << L" Mask Type/Size incompatible" << std::endl;
				return -1;
			}

			int iVal = (1 << i);
			cv::threshold(src, src, 250, iVal, THRESH_BINARY);
			encodedImage += src;

		}


	}


	std::wstring outFile = dirpath + L"\\Mask.png";
	if (!encodedImage.empty())
	{
		encodedImage.at<uchar>(0, 0) = (uchar)iMaskCode;

		bool ret = Save(outFile, encodedImage);
	}

	return 0;
}

int DecodeMask(std::wstring fileName)
{
	std::wcout << L"Decoding Mask...." << std::endl;

	std::string::size_type found = fileName.find(L"Mask.png");
	
	std::wstring fileNameWithoutExtension = L"";
	if (found != std::string::npos)
		fileNameWithoutExtension = fileName.substr(0, found);


	std::wstring ParentPath = fs::path(fileName).parent_path().wstring();


	cv::Mat encodedImage = Load(fileName);

	int channels = encodedImage.channels();
	if (encodedImage.empty() || channels != 1 )
		return -1;

	int iMaskCode = encodedImage.at<uchar>(0, 0);


	



	for (int i(0); i < 8; i++)
	{
		//Mask bits are reversed Mask code 0 -> Bit Code 7 && Mask code 7 ->Bitcode 0
		if (iMaskCode & (1<<i))
		{
			std::wstringstream  stringStream;
			stringStream << ParentPath << L"\\Mask" <<(7-i)+1<<L".png";
			std::wstring MaskFIlePath = stringStream.str();
			std::wcout << L"File Path : " << MaskFIlePath << std::endl;
			cv::Mat decodedImage = encodedImage & (1 << i);
			cv::threshold(decodedImage, decodedImage, 0, 255, THRESH_BINARY);
			bool ret = Save(MaskFIlePath, decodedImage);
		}	

	}

	std::wstring filenameO = L".\\Mask\\Optisens\\MäskO.png";
	bool ret = Save(filenameO, encodedImage);

	return true;

}


auto ConvertCVMatToBitMapOld(cv::Mat& frame) -> HBITMAP
{
	auto convertOpenCVBitDepthToBits = [](const int32_t value)
	{
		auto regular = 0u;

		switch (value)
		{
		case CV_8U:
		case CV_8S:
			regular = 8u;
			break;

		case CV_16U:
		case CV_16S:
			regular = 16u;
			break;

		case CV_32S:
		case CV_32F:
			regular = 32u;
			break;

		case CV_64F:
			regular = 64u;
			break;

		default:
			regular = 0u;
			break;
		}

		return regular;
	};

	auto imageSize = frame.size();
	assert(imageSize.width && "invalid size provided by frame");
	assert(imageSize.height && "invalid size provided by frame");

	if (imageSize.width && imageSize.height)
	{
		auto headerInfo = BITMAPINFOHEADER{};
		ZeroMemory(&headerInfo, sizeof(headerInfo));

		headerInfo.biSize = sizeof(headerInfo);
		headerInfo.biWidth = imageSize.width;
		headerInfo.biHeight = -(imageSize.height); // negative otherwise it will be upsidedown
		headerInfo.biPlanes = 1;// must be set to 1 as per documentation frame.channels();

		const auto bits = convertOpenCVBitDepthToBits(frame.depth());
		headerInfo.biBitCount = frame.channels() * bits;


		char chunk[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
		BITMAPINFO &bitmapInfo = *(BITMAPINFO *)&chunk[0];


		//auto bitmapInfo = BITMAPINFO{};
		ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));

		bitmapInfo.bmiHeader = headerInfo;
		bitmapInfo.bmiColors->rgbBlue = 0;
		bitmapInfo.bmiColors->rgbGreen = 0;
		bitmapInfo.bmiColors->rgbRed = 0;
		bitmapInfo.bmiColors->rgbReserved = 0;

		if (headerInfo.biBitCount == 8)
		{
			RGBQUAD* palette = bitmapInfo.bmiColors;

			for (int i = 0; i < 256; i++)
			{
				palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
				palette[i].rgbReserved = 0;
			}
		}

		auto dc = GetDC(nullptr);
		assert(dc != nullptr && "Failure to get DC");

		int stepsize = (frame.cols * frame.channels() + 3) & -4; //Stepsize rerquired for DWORD allignment
		int imagesize = stepsize * frame.rows;

		uchar * data = frame.data;
		cv::Mat temp;

		uchar* tempBuffer = NULL;
		//DWORD alligning required if the image width is not multiple of 4 
		if (stepsize != frame.step)
		{
			tempBuffer = new uchar[imagesize];
			ZeroMemory(tempBuffer, imagesize);

			temp = cv::Mat(frame.rows, frame.cols, frame.type(), tempBuffer, stepsize);
			frame.copyTo(temp);

			int step1 = frame.step;
			int step2 = temp.step;
			data = temp.data;
		}


		auto bmp = CreateDIBitmap(dc,
			&headerInfo,
			CBM_INIT,
			data,
			&bitmapInfo,
			DIB_RGB_COLORS);

		if (tempBuffer)
			delete tempBuffer;

		assert(bmp != nullptr && "Failure creating bitmap from captured frame");

		return bmp;
	}
	else
	{
		return nullptr;
	}
}


auto ConvertCVMatToBitMap(cv::Mat& frame) -> HBITMAP
{
	auto convertOpenCVBitDepthToBits = [](const int32_t value)
	{
		auto regular = 0u;

		switch (value)
		{
		case CV_8U:
		case CV_8S:
			regular = 8u;
			break;

		case CV_16U:
		case CV_16S:
			regular = 16u;
			break;

		case CV_32S:
		case CV_32F:
			regular = 32u;
			break;

		case CV_64F:
			regular = 64u;
			break;

		default:
			regular = 0u;
			break;
		}

		return regular;
	};

	auto imageSize = frame.size();
	assert(imageSize.width && "invalid size provided by frame");
	assert(imageSize.height && "invalid size provided by frame");

	if (imageSize.width && imageSize.height)
	{
		auto headerInfo = BITMAPINFOHEADER{};
		ZeroMemory(&headerInfo, sizeof(headerInfo));

		headerInfo.biSize = sizeof(headerInfo);
		headerInfo.biWidth = imageSize.width;
		headerInfo.biHeight = -(imageSize.height); // negative otherwise it will be upsidedown
		headerInfo.biPlanes = 1;// must be set to 1 as per documentation frame.channels();

		const auto bits = convertOpenCVBitDepthToBits(frame.depth());
		headerInfo.biBitCount = frame.channels() * bits;

		char chunk[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
		BITMAPINFO &bitmapInfo = *(BITMAPINFO *)&chunk[0];


		//auto bitmapInfo = BITMAPINFO{};
		ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));

		bitmapInfo.bmiHeader = headerInfo;
		bitmapInfo.bmiColors->rgbBlue = 0;
		bitmapInfo.bmiColors->rgbGreen = 0;
		bitmapInfo.bmiColors->rgbRed = 0;
		bitmapInfo.bmiColors->rgbReserved = 0;


		if (headerInfo.biBitCount == 8)
		{
			RGBQUAD* palette = bitmapInfo.bmiColors;

			for (int i = 0; i < 256; i++)
			{
				palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
				palette[i].rgbReserved = 0;
			}
		}


		auto dc = GetDC(nullptr);
		assert(dc != nullptr && "Failure to get DC");


		int stepsize = (frame.cols * frame.channels() + 3) & -4;


		void* dst_ptr = 0;
		auto bmp = CreateDIBSection(dc, &bitmapInfo, DIB_RGB_COLORS, &dst_ptr, 0, 0);

		int imagesize = stepsize * frame.rows;

		uchar * data = frame.data;

		//DWORD alligning required if the image width is not multiple of 4 
		if (stepsize != frame.step)
		{
			cv::Mat temp(frame.rows, frame.cols, frame.type(), dst_ptr, stepsize);

			frame.copyTo(temp);

			int step1 = frame.step;
			int step2 = temp.step;
			data = temp.data;
		}
		memcpy(dst_ptr, data, imagesize);

		assert(bmp != nullptr && "Failure creating bitmap from captured frame");

		return bmp;
	}
	else
	{
		return nullptr;
	}
}

void SplitMergeChannels(cv::Mat& frame)
{
	if (frame.channels() != 4)
		return;
	//Mat bgrA[4];   //destination array
	vector<Mat> bgrA;
	split(frame, bgrA);//split source  

						//Note: OpenCV uses BGR color order
	imwrite(".\\Mask\\Optisens\\split\\blue.png", bgrA[0]); //blue channel
	imwrite(".\\Mask\\Optisens\\split\\green.png", bgrA[1]); //green channel
	imwrite(".\\Mask\\Optisens\\split\\red.png", bgrA[2]); //red channel
	imwrite(".\\Mask\\Optisens\\split\\alpha.png", bgrA[3]); //Alpha channel


	bgrA[3] -= 150;
	cv::Mat finalImg;
	cv::merge(bgrA, finalImg);
	imwrite(".\\Mask\\Optisens\\split\\Final.png", finalImg); //Alpha channel
	
	cv::Mat newImg;
	cvtColor(finalImg, newImg, COLOR_BGRA2RGBA);
	imwrite(".\\Mask\\Optisens\\split\\FinalNew.png", newImg); //Alpha channel
	cv::flip(finalImg, finalImg, 0);
	auto bmp = ConvertCVMatToBitMap(finalImg);

	if (bmp)
	{
		CImage img;
		img.Attach(bmp);
		int k = img.GetBPP();

		img.SetHasAlphaChannel(true);
		
		int l = img.GetBPP();
		img.Save(_T(".\\Mask\\Optisens\\split\\FinalNewCImage.png"));


		CImage test;


 	}

}

void MixImageChannels(cv::Mat& rgba)
{
	if (rgba.channels() != 4)
		return;

	Mat bgr(rgba.rows, rgba.cols, CV_8UC3);
	Mat alpha(rgba.rows, rgba.cols, CV_8UC1);

	Mat out[] = { bgr, alpha };


}

void RGBtoHSV(cv::Scalar rgb,cv::Scalar& hsv ) {
	
	/*float fR = rgb.x / 255.0;
	float fG = rgb.y / 255.0;
	float fB = rgb.z / 255.0;*/

	float fR = rgb[0] / 255.0;
	float fG = rgb[1] / 255.0;
	float fB = rgb[2] / 255.0;

	float fCMax = std::max({ fR, fG, fB });

	float fCMin = std::min({ fR, fG, fB });

	/*float fCMax = 0;

	float fCMin = 0;*/

	float fDelta = fCMax - fCMin;

	float fH = 0, fS = 0, fV = 0;

	if (fDelta > 0)
	{
		if (fCMax == fR) {
			fH = 60 * (fmod(((fG - fB) / fDelta), 6));
		}
		else if (fCMax == fG) {
			fH = 60 * (((fB - fR) / fDelta) + 2);
		}
		else if (fCMax == fB) {
			fH = 60 * (((fR - fG) / fDelta) + 4);
		}

		if (fCMax > 0) {
			fS = fDelta / fCMax;
		}
		else {
			fS = 0;
		}

		fV = fCMax;
	}
	else 
	{
		fH = 0;
		fS = 0;
		fV = fCMax;
	}

	if (fH < 0) {
		fH = 360 + fH;
	}



	hsv[0] = std::floor(fH / 2 + 0.5);
	hsv[1] = std::floor(fS * 255 + 0.5);
	hsv[2] = std::floor(fV * 255 + 0.5);

	
}


void CreateImage()
{
	cv::Scalar hsvVal;
	RGBtoHSV(cv::Scalar(0, 0, 255), hsvVal);

	std::cout << hsvVal << std::endl;


	cv::Mat bgra = cv::Mat::zeros(400, 400, CV_8UC4);
	Mat rgb(bgra.rows, bgra.cols, CV_8UC3);
	Mat alpha= cv::Mat::zeros(bgra.rows, bgra.cols, CV_8UC1);
	




	//// forming an array of matrices is a quite efficient operation,
	//// because the matrix data is not copied, only the headers
	//Mat out[] = { bgr, alpha };
	//// rgba[0] -> bgr[2], rgba[1] -> bgr[1],
	//// rgba[2] -> bgr[0], rgba[3] -> alpha[0]
	//int from_to[] = { 0,2, 1,1, 2,0, 3,3 };
	//cv::mixChannels(&rgba, 1, out, 2, from_to, 4);

	
	
	//bgra(cv::Rect(cv::Point2i(100, 100), cv::Point2i(150, 300))).setTo(cv::Scalar(255, 0, 0, 255));
	//bgra(cv::Rect(cv::Point2i(150, 100), cv::Point2i(200, 300))).setTo(cv::Scalar(0, 255, 0, 255));
	//bgra(cv::Rect(cv::Point2i(200, 100), cv::Point2i(250, 300))).setTo(cv::Scalar(0, 0, 255, 255));     //Cv::Scalar format is always BGRA even if RGB or BGR image
	imwrite(".\\Mask\\Optisens\\split\\bgra.png", bgra); //Alpha channel
	//imshow("bgr", bgra);

	Mat out[] = { rgb, alpha };
	int from_to[] = { 0,2, 1,1, 2,0, 3,3 };

	cv::mixChannels(&bgra, 1, out, 2, from_to, 4);




	alpha = 255;
	cv::Mat hsv;
	cvtColor(rgb, hsv, COLOR_RGB2HSV);
	vector<Mat> vHSV;
	split(hsv, vHSV);

	//===============================
	//Color1 (66,164,244) (207,
	//Color2 (249,249,17)
	//Color3 (244,76,4)
	//Color4 (244,4,144)
	//Color5 (4,52,244)
	//Color6 (4,244,168)
	//Color7 (201,204,242)
	//Color8 (121,209,144)



	//====================================

	vHSV[0] = 22*1;
	vHSV[1] = 255;
	vHSV[2] = 220;


	merge(vHSV, hsv);
	cvtColor(hsv, rgb, COLOR_HSV2BGR);


	cv::Mat rgba = cv::Mat::zeros(400, 400, CV_8UC4);
	int from_to2[] = { 0,0, 1,1, 2,2, 3,3 };
	cv::mixChannels(out, 2, &rgba, 1, from_to2, 4);

	imwrite(".\\Mask\\Optisens\\split\\rgba.png", rgba); //Alpha channel
	//imshow("rgb", rgba);

	//cv::waitKey();
}


void PrintHeader()
{
	std::cout << "Command For Encoding Mask" << "CodeMask.exe -e ""Folder Path with number coded Mask images"" " << endl;
	std::cout << "Command For Decoding Mask" << "CodeMask.exe -d ""Endcoded Mask File Name"" " << endl;
}
int _tmain(int argc, wchar_t* argv[])
{	
	int ret = 0;
	bool encode = true;
	std::wstring directorypath = _T(""), MaskFileName = _T("");

	if(argc<3)
	{
		std::cout << "Not enough  Arguements" << endl;
		PrintHeader();
		ret = -1;
	}
	else
	{
		for (int i(0); i < argc-1; i++)
		{
			std::wstring tem(argv[i]);
			std::wcout << tem << endl;
			if (wcscmp(argv[i], L"-e")==0)
			{
				directorypath = std::wstring(argv[i + 1]);			

				std::wcout << "dir Path : " <<directorypath << endl;
				fs::path dirPath(directorypath);		
				
				bool isdir = fs::is_directory(directorypath);
				if (!isdir)
				{
					std::cout << "Directory Doesn't exists" << endl;
					ret = -1;
					break;
				}
				else
					encode = true;

			}
			else if (wcscmp(argv[i], L"-d")==0)
			{
				MaskFileName = argv[i + 1];
				std::wcout << "Mask filename to decocde : " << MaskFileName << endl;
				fs::path filePath(MaskFileName);
				bool isFile = fs::is_regular_file(MaskFileName);
				if (!isFile)
				{
					std::cout << "File Doesn't exists" << endl;
					ret = -1;
					break;
				}
				else
					encode = false;

			}
		}

	}
	
	if(ret == 0)
	{
		if(encode)
			EncodeMask(directorypath);
		else
			DecodeMask(MaskFileName);

		//CreateImage();


		

		//std::vector<string> filenames;

		////std::wstring dirpath = L".\\Mask\\optisens\\";

		//std::wstring dirpath = L".\\Mask\\temp\\";

		//bool MaskFound = false;

		//EncodeMask(dirpath);

		//std::wstring fileName = dirpath + L"\\Mask.png";
		//DecodeMask(fileName);


		//cv::Mat ColorMask = cv::Mat::zeros(cv::Size(512,512), CV_8UC4);


		/*std::wstring fileName = dirpath + L"\\test.png";
		cv::Mat imgPng = Load(fileName);
		*/



		//SplitMergeChannels(imgPng);






		//int chns = imgPng.channels();

		//Save(L".\\Mask\\Optisens\\test2.png", imgPng);
		////cv::imwrite(".\\Mask\\Optisens\\test.png", imgPng);

	}
	
	if (ret != 0)
		std::cout << "Operation not successful" << endl;
	std::cout << endl;
	
#ifdef DEBUG
	system("pause");
#endif

}