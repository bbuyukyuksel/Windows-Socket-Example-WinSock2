// server.cpp : Bu dosya 'main' işlevi içeriyor. Program yürütme orada başlayıp biter.

#include "pch.h"
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable:4996)

SOCKET connections[100];
int connectionCounter = 0;

enum Packet {
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(int ID, Packet pType) {
	switch (pType)
	{
	case P_ChatMessage: {
		int bufferlength;
		char *buffer;
		recv(connections[ID], (char*)&bufferlength, sizeof(int), NULL); //get buffer lenght
		buffer = new char[bufferlength + 1]; //Buffer to hold received message
		buffer[bufferlength] = '\0';
		recv(connections[ID], buffer, bufferlength, NULL); //Receive message from client
		std::cout << "From " << "[" << ID << "]" << "  Message Len:" << bufferlength << "  Message:" << buffer << "Packet Type:"<< "Chat Message" << std::endl;
		for (int i = 0; i < connectionCounter; i++) {
			if (i == ID)
				continue;
			Packet chatMessagePacket = P_ChatMessage;
			send(connections[i], (char*)& chatMessagePacket, sizeof(Packet), NULL);
			send(connections[i], (char*)& bufferlength, sizeof(int), NULL);
			send(connections[i], buffer, bufferlength, NULL);
		}
		delete buffer;
		buffer = nullptr;

	}
		break;

	default:
		std::cout << "Unrecognized packet type: " << pType << std::endl;
		break;
	}
	return true;
}

int ClientHandlerThread(int ID) {

	Packet pType;
	while (true) {
		recv(connections[ID], (char*)&pType, sizeof(pType), NULL); //Receive packet type from client
		if (!ProcessPacket(ID, pType))
			break; //break out of our client handler loop
	}
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
			std::cout << "[" << i<<"]"<<" Client Connected!" << std::endl;
			char MOTD[]{ "Welcome, this is message of the Day!	" };
			int lenMOTD = sizeof(MOTD);

			Packet chatMessagePacket = P_ChatMessage;
			send(newConnection, (char*)& chatMessagePacket, sizeof(Packet), NULL);
			send(newConnection, (char*)&lenMOTD, sizeof(int), NULL);
			send(newConnection, (char*)&MOTD, sizeof(MOTD), NULL);	//Send MOTD buffer
		
			Packet testPacket = P_Test;
			send(newConnection, (char*)& testPacket, sizeof(Packet), NULL);

		}
		connections[i] = newConnection;
		connectionCounter++;
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(i), NULL, NULL); //Create thread to handle this client. 

	}
	system("pause"); 
	return 0;
}
