/*
Comments in this code written in russian language for students
*/

#include "stdafx.h"

const int	LISTEN_PORT = 8080;

#if ! defined(_WIN32)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#else
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
#include <io.h>
#define write _write
#define close _close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

typedef struct sockaddr_in  address_t;
typedef struct sockaddr		sockaddr_t;

string programm_path;
string http_root_folder;

extern void  serve(int client);

// 
// Функция возвращает строку с IP-адресом WEB-браузера, соединившегося с сервером.
// IP-адрес "вычисляется" на основе соединённого соекта
//
string get_peer_address(int sock, int &port)
{
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];

	len = sizeof addr;
	getpeername(sock, (struct sockaddr*)&addr, &len);

	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	}
	else { // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}

	return string(ipstr);
}

// Печать в файл информации о входящем TCP соединении
// It takes an IP address of interface which accepted connection
void log_AcceptTcpConnection(
	FILE		*	file,
	address_t 		client_addr,
	int				sock)
{
	int port;
	string peer = get_peer_address(sock, port);

	char ipstr[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, ipstr, sizeof ipstr);

	fprintf(file, "TCP Connection from %s:%d to %s:%d\n",
		ipstr,
		htons(client_addr.sin_port),
		peer.c_str(),
		LISTEN_PORT
		);

	// You can modify this for writing connection info to database
}

// Under Windows HTTP_ROOT is directory where sever's executable file is located
// Unnder Linux it take pages from user's home directory
// TODO: Set HTTP_ROOT by config or/and by command line parameter
void Prepare_HTTP_ROOT()
{
    char * env_root = getenv("HTTP_ROOT");
    if(env_root != NULL)
    {
	http_root_folder = env_root;
	return;
    }
#if defined (_WIN32)
	http_root_folder = programm_path.substr(0, programm_path.find_last_of("\\") + 1);
#else
	// We use it in classroom
	http_root_folder = "/home";
	struct passwd *pw = getpwuid(getuid());
	if (pw)
		http_root_folder = pw->pw_dir;
	else
		http_root_folder = "/tmp";

#endif
}

// all bits set - allow access, otherwise deny any access
unsigned int CheckRemoteRights(
	address_t 		client_addr,
	address_t 		server_addr,
	int				sock)
{
	// Full access for everybody. You can implement access rules here
	return (unsigned int)-1;
}

//
// Главная функция сервера. Вход в программу - функция main()
//
int main(int argc, char *argv[])
{
	int 			client, server;
	address_t 		client_addr, server_addr;
	int				size_of_client_address;

	programm_path = argv[0];
	Prepare_HTTP_ROOT();

#if defined(WIN32)
	WSADATA ws;
	if (0 > WSAStartup(MAKEWORD(2, 2), &ws))
	{
		//error = WSAGetLastError();
		perror("Unable initializw WinSock library");
		exit(1);
	}

#endif

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == -1)
	{
		perror("Unable create socket");
		exit(-1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(LISTEN_PORT);

	if (bind(server, (sockaddr_t *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Unable bind socket");
		exit(-1);
	}

	if (0 > listen(server, 1000))
	{
		perror("Unable accept connection");
		exit(-1);
	}

	fprintf(stdout, "Server ready to accept HTTP requests\n");

	while (true)
	{
		size_of_client_address = sizeof(client_addr);
		client = accept(server, (sockaddr_t *)&client_addr, (socklen_t*)&size_of_client_address);
		if (CheckRemoteRights(client_addr, server_addr, client) != (unsigned int)-1)
		{
			log_AcceptTcpConnection(stdout, client_addr, client);
			close(client);
			continue;
		}
#if ! defined(_WIN32)
		if (client < 0)
		{
			perror("Unable accept connection");
			exit(-1);
		}
		switch (fork())
		{
		case -1:
			perror("Unable fork()");
			break;

		case 0:
			serve(client);
			break;

		default:
			log_AcceptTcpConnection(stdout, client_addr, client);
			close(client);
			break;
		}
#else
		if (client < 0)
		{
			fprintf(stderr, "Unable accept connection: %d\n", WSAGetLastError());
			exit(-1);
		}

		log_AcceptTcpConnection(stdout, client_addr, client);

		serve(client);
#endif
	}

	return 0;
}
