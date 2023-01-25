#include <vector>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

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

	char* buf = new char[(((4096 * 32 + 31) / 32) * 4) * 2160];
	ZeroMemory(buf, (((4096 * 32 + 31) / 32) * 4) * 2160);
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

	//Receive the buffer
	int success = recv(clientSocket, buf, (((4096 * 32 + 31) / 32) * 4) * 2160, 0);
	std::cout << buf[0] << " " << buf[1] << std::endl;
	//Create bitmap from received bitmap buffer
	HBITMAP bitmap = { 0 };
	LONG bitRet = SetBitmapBits(bitmap, (((4096 * 32 + 31) / 32) * 4) * 2160, buf); //WORKS KIND OF
	std::cout << "Bitmap: " << bitRet;

	if (success == SOCKET_ERROR)
	{
		std::cerr << "error: " << WSAGetLastError();
		return WSAGetLastError();
	}
	else
	{
		std::cout << "Received data." << success << std::endl;
		std::cout << "Processing...." << std::endl;

		HBITMAP recvBitmap = {};

		recvBitmap = CreateBitmap(4096, 2160, 1, 32, buf);
		
		if (recvBitmap == NULL)
		{
			std::cout << "Imvalid bitmap data" << std::endl;
		}
		else
		{
			std::cout << "Successfully Created Bitmap: " << recvBitmap << std::endl;
			OpenClipboard(NULL);
			EmptyClipboard();
			SetClipboardData(CF_BITMAP, recvBitmap);
			CloseClipboard();
		}
	}
}