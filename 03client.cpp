// client.cpp : Bu dosya 'main' işlevi içeriyor. Program yürütme orada başlayıp biter.


#include "pch.h"
#include <WinSock2.h>
#include <iostream>
#include <string>

#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable:4996)

SOCKET connection;

int ClientHandler(void) {
	int bufferlength;
	while (true) {
		recv(connection, (char*)&bufferlength, sizeof(int), NULL);
		char *buffer = new char[bufferlength+1];
		buffer[bufferlength] = '\0';
		recv(connection, buffer, bufferlength, NULL);
		std::cout << "From Stranger:" << buffer << std::endl;
		delete buffer;
		buffer = nullptr;
	}
}

int main()
{
    std::cout << "Hello World!\n"; 
	// WinSock start up
	WSADATA wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //if wsa returns anything other
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	SOCKADDR_IN addr; //Adress that we will bind our listening socket to
	int addrlen = sizeof(addr); //length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Broadcast locally
	addr.sin_port = htons(1111); //port
	addr.sin_family = AF_INET; //IPv4 Socket

	connection = socket(AF_INET, SOCK_STREAM, NULL); //Set connection socket
	if (connect(connection, (SOCKADDR*)&addr, addrlen) != 0) {
		MessageBoxA(NULL, "Failed to connect", "Error", MB_OK | MB_ICONERROR);
		return 0; //Failed to connect 
	}
	
	//Start Handler Thread
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	std::string buffer;
	while (true) {
		std::cin >> buffer;
		int bufferlenght = buffer.size();
	 	send(connection, (char*)&bufferlenght, sizeof(int), NULL);
		send(connection, buffer.c_str(), bufferlenght, NULL);
		Sleep(10);
	}
	
	system("pause");
	return 0;
}
