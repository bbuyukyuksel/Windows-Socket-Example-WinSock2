// client.cpp : Bu dosya 'main' işlevi içeriyor. Program yürütme orada başlayıp biter.


#include "pch.h"
#include <WinSock2.h>
#include <iostream>
#include <string>

#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable:4996)

using namespace std;

SOCKET connection;
enum Packet
{
	P_ChatMessage,
	P_Test
};

bool SendInt(int _int) {
	int ReturnCheck = send(connection, (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool GetInt(int &_int) {
	int ReturnCheck = recv(connection, (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}

bool SendPacketType(Packet pType) {
	int ReturnCheck = send(connection, (char*)&pType, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}

bool GetPacketType(Packet &pType) {
	int ReturnCheck = recv(connection, (char*)&pType, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}

bool SendString(string _string) {
	if (!SendPacketType(P_ChatMessage))
		return false;
	int bufferLength = _string.size();
	if (!SendInt(bufferLength))
		return false;

	int ReturnCheck = send(connection, _string.c_str(), bufferLength, NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;

}
bool GetString(string &_string) {
	int bufferLength;
	if (!GetInt(bufferLength))
		return false;
	char *buffer = new char[bufferLength + 1];
	buffer[bufferLength] = '\0';
	int ReturnCheck = recv(connection, buffer, bufferLength, NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;

	_string = buffer;
	delete[] buffer;
	buffer = nullptr;

	return true;

}




bool ProcessPacket(Packet pType){
	switch (pType)
	{
	case P_ChatMessage: 
	{
		string message; //string to store our message, we received
		if (!GetString(message))
			return false;
		cout << message << endl;
	}
		break;
	case P_Test:
	{
		std::cout << "You received a test packet!" << std::endl;
		break;
	}
	default:
		std::cout << "Unrecognized packet: " << pType << std::endl;
		break;
	}
	return true;
}
void ClientHandler(void) {
	Packet pType;
	while (true) {
		if (!GetPacketType(pType))
			break;
		if (!ProcessPacket(pType))
			break; //Break out of our client handler loop
	}
	cout << "Lost connection to the server." << endl;
	closesocket(connection);
	
}

int main()
{
    std::cout << "Hello Client!\n"; 
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
		//std::cin >> buffer;
		std::getline(std::cin, buffer);
		if (!SendString(buffer))
			break;

		Sleep(10);
	}
	
	system("pause");
	return 0;
}
