#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <winuser.h>

#pragma comment(lib, "ws2_32.lib")
#pragma pack(push, 1)

using namespace cv;

int main() {
	WSADATA wData;
	WORD ver = MAKEWORD(2, 2);
	int wsOk = WSAStartup(ver, &wData);
	if (wsOk != 0)
	{
		std::cerr << "Error Initializing WinSock! Exiting" << std::endl;
		return -1;
	}

	// Create Socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		std::cerr << "could not open socket" << std::endl;
		return -1;
	}

	//Bind IP address to port
	sockaddr_in sockin;
	sockin.sin_family = AF_INET;
	sockin.sin_port = htons(8081);
	sockin.sin_addr.S_un.S_addr = INADDR_ANY;

	//bind socket to ip address and port
	bind(listening, (sockaddr*)&sockin, sizeof(sockin));

	//Tell winsock that socket is for listening
	listen(listening, SOMAXCONN);

	//wait for connection
	sockaddr_in clientSockAddr;
	int clientSize = sizeof(clientSockAddr);

	SOCKET clientSocket = accept(listening, (sockaddr*)&clientSockAddr, &clientSize);
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);


	bool isReceiving = true;

	//If client connects
	if (getnameinfo((sockaddr*)&clientSockAddr, sizeof(clientSockAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << " Connection established on: " << service << std::endl;
	}

	else
	{
		std::cout << "Connection could not be established." << std::endl;
	}

	//Receive the bitmap info header
	bool connected = true;
	char* buf = 0;

	BITMAPINFOHEADER bmInfo;
	char* bitmapInfo = new char[sizeof(BITMAPINFOHEADER)];
	if (int rec = recv(clientSocket, bitmapInfo, sizeof(BITMAPINFOHEADER), NULL)) {
		std::cout << "bytes received bitmap info " << rec << std::endl;
		//Copy Bitmap Info Header to Bitmap Info Header Structure
		memcpy(&bmInfo, bitmapInfo, sizeof BITMAPINFOHEADER);
		std::cout << "Incoming width/height: " << bmInfo.biWidth << " " << bmInfo.biHeight << std::endl;

	}
	//Receive bitmap bits
	buf = new char[((((bmInfo.biWidth * 32 + 31) / 32) * 4) * bmInfo.biHeight) + 1];
	ZeroMemory(buf, ((((bmInfo.biWidth * 32 + 31) / 32) * 4) * bmInfo.biHeight) + 1);
	int bitmapRec = recv(clientSocket, buf, (((((bmInfo.biWidth * 32 + 31) / 32) * 4) * bmInfo.biHeight)) + 1, NULL);
	std::cout << "recieve bitmap bytes: " << bitmapRec << " " << WSAGetLastError() << std::endl;
	HDC recHdc = GetDC(NULL);
	HBITMAP hbitTest = CreateCompatibleBitmap(recHdc, bmInfo.biWidth, bmInfo.biHeight);

	//Set bitmap bits from client to HBITMAP handle
	int diSet = SetDIBits(recHdc, hbitTest, 0,
		(UINT)bmInfo.biHeight,
		buf,
		(BITMAPINFO*)&bmInfo, DIB_RGB_COLORS);

	std::cout << buf[0] << " " << buf[1] << std::endl;


	//Create bitmap from received bitmap buffer and paste it to clipboard
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hbitTest);
	CloseClipboard();
}