#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <winuser.h>
#include <wingdi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma pack(push, 1)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int getTaskBarHeight();

HBITMAP hbitTest = { 0 };
BITMAPINFOHEADER bmInfo;
HWND hwnd;
char* buf = 0;
int newWidth = 0;
int newHeight = 0;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

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
	std::cout << "receive bitmap bytes: " << bitmapRec << " " << WSAGetLastError() << std::endl;

	HDC recHdc = GetDC(NULL);
	hbitTest = CreateCompatibleBitmap(recHdc, bmInfo.biWidth, bmInfo.biHeight);



	// Register window class
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = L"BitmapWindowClass";
	RegisterClass(&wc);

	// Create the window
	hwnd = CreateWindow(L"BitmapWindowClass", L"Client Screen", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, bmInfo.biWidth, bmInfo.biHeight, NULL, NULL, hInstance, NULL);

	//Set bitmap bits from client to HBITMAP handle
	/*int diSet = SetDIBits(recHdc, hbitTest, 0,
		(UINT)bmInfo.biHeight,
		buf,
		(BITMAPINFO*)&bmInfo, DIB_RGB_COLORS);*/

	std::cout << buf[0] << " " << buf[1] << std::endl;

	//Create bitmap from received bitmap buffer and paste it to clipboard
	/*OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hbitTest);
	CloseClipboard();*/
	

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	// Main message loop
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// Create a compatible DC for the bitmap
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hbitTest);

		// Draw the bitmap to the window
		//BitBlt(hdc, 0, 0, bmInfo.biWidth, bmInfo.biHeight, hdcMem, 0, 0, SRCCOPY);

		//Get TaskBar height
		int taskHeight = getTaskBarHeight();

		//Set bits
		/*int diSet = SetDIBits(hdc, hbitTest, 0,
			(UINT)bmInfo.biHeight - taskHeight,
			buf,
			(BITMAPINFO*)&bmInfo, DIB_RGB_COLORS);*/

			//Get Task Bar height
		int taskBar = getTaskBarHeight();

		//Stretch image
		if (newWidth && newHeight == 0) {
			StretchDIBits(hdc, 0, 0, bmInfo.biWidth, bmInfo.biHeight - taskBar, 0, 0, bmInfo.biWidth,
				bmInfo.biHeight, buf, (BITMAPINFO*)&bmInfo, DIB_RGB_COLORS, SRCCOPY);
		}
		else {
			StretchDIBits(hdc, 0, 0, newWidth, newHeight - taskBar, 0, 0, bmInfo.biWidth,
				bmInfo.biHeight, buf, (BITMAPINFO*)&bmInfo, DIB_RGB_COLORS, SRCCOPY);
		}
		// Clean up
		SelectObject(hdcMem, hOldBitmap);
		DeleteDC(hdcMem);

		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_SIZE:
	{
		// Update the bitmap size based on the new window size
		newWidth = LOWORD(lParam);
		newHeight = HIWORD(lParam);

		// Invalidate the window to trigger a repaint
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
		return 0;
	}

	case WM_CLOSE:
		DestroyWindow(hwnd);
		WSACleanup();
		return 0;

	case WM_DESTROY:
		DeleteObject(hbitTest);
		PostQuitMessage(0);
		WSACleanup();
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}

int getTaskBarHeight()
{
	/*RECT rect;
	HWND taskBar = FindWindow(L"Shell_TrayWnd", NULL);
	if (taskBar && GetWindowRect(taskBar, &rect)) {
		return rect.bottom - rect.top;
	}*/
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	return bmInfo.biHeight - rect.bottom;
}