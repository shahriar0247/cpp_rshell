#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;


int main()
{
	int buffer3 = 256;

	// initialze winsock

	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);

	if (wsOk != 0) {
		cerr << "Cant initilize winsock, quitting" << endl;
		return 1;
	}

	// create a socket

	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET) {
		cerr << "Cant create socket, quitting" << endl;
		WSACleanup();
		return 1;
	}

	// bind socket

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(4422);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// tell winsock the socket is for listening

	listen(listening, SOMAXCONN);

	// wait for a connection

	sockaddr_in client;

	int clientsize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientsize);

	char host[NI_MAXHOST]; // client's remote name
	char service[NI_MAXSERV]; // port client is connected to

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " conneted on port " << service;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}

	// close listening socket

	closesocket(listening);


	// while loop: accpet and echo message back to client

	char buf[4096];
	char buf2[409600];

	while (true) {
	

		while (true) {
			char buf[100];
			cin.getline(buf, sizeof(buf));
			cout << endl;

	
			

			send(clientSocket, buf, 100, 0);


			char buffer[8];
			recv(clientSocket, buffer, 8, 0);
			send(clientSocket, "K", 8, 0);

			int buffer2 = atoi(buffer) + 1;
		

			int bytesRecv = recv(clientSocket, buf2, buffer2, 0);


			if (bytesRecv == SOCKET_ERROR) {
				cerr << "Error in recv(), Quitting" << endl;
				break;
			}

			if (bytesRecv == 0) {
				cout << "Client disconnected" << endl;
				break;
			}
			
			cout << buf2;





			// echo message back to client
		}

		// close socket

		closesocket(clientSocket);

		// cleanup winsock

		WSACleanup();
	}
}

