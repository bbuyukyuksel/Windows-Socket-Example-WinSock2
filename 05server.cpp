// server.cpp : Bu dosya 'main' işlevi içeriyor. Program yürütme orada başlayıp biter.

#include "pch.h"
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable:4996)

using namespace std;

SOCKET connections[100];
int connectionCounter = 0;

enum Packet {
	P_ChatMessage,
	P_Test
};

bool SendInt(int ID, int _int) {
	int ReturnCheck = send(connections[ID], (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool GetInt(int ID, int &_int) {
	int ReturnCheck = recv(connections[ID], (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}

bool SendPacketType(int ID, Packet pType) {
	int ReturnCheck = send(connections[ID], (char*)&pType, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}

bool GetPacketType(int ID, Packet &pType) {
	int ReturnCheck = recv(connections[ID], (char*)&pType, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}

bool SendString(int ID, string _string) {
	if (!SendPacketType(ID, P_ChatMessage))
		return false;
	int bufferLength = _string.size();
	if (!SendInt(ID ,bufferLength))
		return false;

	int ReturnCheck = send(connections[ID], _string.c_str(), bufferLength, NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;

}
bool GetString(int ID, string &_string) {
	int bufferLength;
	if (!GetInt(ID, bufferLength))
		return false;
	char *buffer = new char[bufferLength + 1];
	buffer[bufferLength] = '\0';
	int ReturnCheck = recv(connections[ID], buffer, bufferLength, NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;

	_string = buffer;
	delete[] buffer;
	buffer = nullptr;

	return true;

}


bool ProcessPacket(int ID, Packet pType) {
	switch (pType)
	{
	case P_ChatMessage: {
		string message;
		if (!GetString(ID, message))
			return false;

		cout << "# Got Message From Client ID:" << ID << endl;
		cout << "# Message:" << message.c_str() << endl;
		
		for (int i = 0; i < connectionCounter; i++) {
			if (i == ID)
				continue;
			if (!SendString(i, message))
				cout << "Failed to send message from client ID: " << ID << " to client ID:" << i << endl;
		}
		cout << "Processed chat message packet from user ID:" << ID << endl;
		break;
	}


	default:
		std::cout << "Unrecognized packet type: " << pType << std::endl;
		break;
	}
	return true;
}

int ClientHandlerThread(int ID) {
	Packet pType;
	while (true) {
		if (!GetPacketType(ID, pType))
			break;
		if (!ProcessPacket(ID, pType))
			break; //break out of our client handler loop
	}
	cout << "Close connection to the client ID: " << ID << endl;
	closesocket(connections[ID]);
	return 0;
}

int main()
{
	char SERVER_STARTUP[]{ "Server is running.." };

    std::cout << SERVER_STARTUP << "\t[Size:" << sizeof(SERVER_STARTUP) << "]" << std::endl; 
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


	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //Create socket to listen for new connections
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //Bind the address to the socket
	listen(sListen, SOMAXCONN); //Places sListen socket in a state in which it is listening for an incoming connections. Note: SOMAXCONN = Socket Oustanding 

	SOCKET newConnection; //Socket to hold the client's connection
	for (int i = 0; i < 100; i++)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen); //Accept a new connection
		if (newConnection == 0) {
			MessageBoxA(NULL, "Failed to accept the client's connection", "Error", MB_OK | MB_ICONERROR);
		}
		else //If client connection properly accepted! 
		{
			connections[i] = newConnection;
			connectionCounter++;
			std::cout << "[" << i<<"]"<<" Client Connected!" << std::endl;
			string MOTD = "Welcome, this is message of the Day!	";
			if (!SendString(i, MOTD)) {
				cout << "From Server: Failed to Client ID:" << i << endl;
			}
		}
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(i), NULL, NULL); //Create thread to handle this client. 

		
		
	}
	system("pause"); 
	return 0;
}
