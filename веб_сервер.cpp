#include "stdafx.h"
#if ! defined(_WIN32)
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>
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

using namespace std;

typedef struct sockaddr_in  address_t;
typedef struct sockaddr		sockaddr_t;


string Get_HTML(string filename)
{
	string response;
#if false
	string fullpath = "/tmp/";
	fullpath += filename;
	FILE * page = fopen(fullpath.c_str(), "rt");
	if (page == NULL)
		response = "<html><body><h1>Page " + filename + " not found!</h1>\r\n";
	else
	{
		while (!feof(page))
		{
			response += fgetc(page);
		}
		fclose(page);
	}
#else	
	response = "<html><body><h1>Hello, World!</h1><br><p>This is hardcoded static HTML page</p></body></html>\r\n";
#endif	
	return response;
}



vector <string> split(const string &s, char delimiter)
{
	vector<string> tokens;
	string token;
	istringstream tokenStream(s);
	while (getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}


string host_option;
int port_option;
bool keep_alive;

void parse_request_options(vector <string> lines)
{
	for (int i = 1; i < lines.size(); i++)
	{
		vector <string> option = split(lines[i], ':');
		if (option.size() < 2)
			break;

		string value = option[1];
		if (value[0] == ' ')
			value = value.erase(0, 1);

		if (option[0] == "\nHost")
		{
			host_option = value;
			if (option.size() > 2)
				port_option = std::atoi(option[2].c_str());

			printf("Host: '%s:%d'\n", host_option.c_str(), port_option);
		}
		else if (option[0] == "\nConnection")
		{
			if (value == "keep-alive")
			{
				keep_alive = true;
			}
			else
				printf("TODO: parse connection type: '%s'\n", value.c_str());

		}
		else if (option[0] == "\nUser-Agent")
			printf("User-Agent: '%s'\n", value.c_str());
		else if (option[0] == "\nAccept")
			printf("Accept: '%s'\n", value.c_str());
		else if (option[0] == "\nAccept-Encoding")
			printf("Accept-Encoding: '%s'\n", value.c_str());
		else if (option[0] == "\nAccept-Language")
			printf("Accept-Language: '%s'\n", value.c_str());
		else if (option[0] == "\nCache-Control")
			printf("Cache-Control: '%s'\n", value.c_str());
		else if (option[0] == "\nUpgrade-Insecure-Requests")
			printf("Upgrade-Insecure-Requests: '%s'\n", value.c_str());
		else if (option[0] == "\nReferer")
			printf("Referer: '%s'\n", value.c_str());
		else if (option[0] == "\nPragma")
			printf("Pragma: '%s'\n", value.c_str());
		else
			printf("Unknown option - %s: %s\n", option[0].c_str(), value.c_str());
	}
	puts("\n");
}

void response_on_get_request(int client, string pagename)
{
	if (pagename == "/")
		pagename = "/index.html";

	string ext = pagename.substr(pagename.find_first_of(".") + 1);
	string content_type;
	if (ext == "html" || ext == "htm")
		content_type = "text/html";
	else if (ext == "gif")
		content_type = "image/gif";
	else if (ext == "png")
		content_type = "image/png";
	else if (ext == "jpg")
		content_type = "image/jpeg";
	else if (ext == "css")
		content_type = "text/css";
	else if (ext == "xml")
		content_type = "application/xml";
	else
		fprintf(stderr, "Cannot detect content type: %s\n", pagename.c_str());

	string html = Get_HTML(pagename);

	std::stringstream len;
	len << html.length();

	string http_response = "HTTP/1.1 200 OK\r\n";
	http_response += "Date: Wen 27 Nov 2018 17:30:15 GMT\r\n";
	http_response += "Server: MyCoolServer/0.0.1\r\n";
	http_response += "Content-Length: " + len.str() + "\r\n";
	http_response += "Content-Type: " + content_type + "\r\n";
	http_response += "Connection: Closed\r\n";
	http_response += "\r\n";
	http_response += html;

	send(client, http_response.c_str(), http_response.length(), 0);
}

void  serve(int client)
{
	char	http_request[4096];
	int read_size;
	read_size = recv(client, http_request, sizeof(http_request), 0);
	if (read_size < 0)
	{
		perror("Unable read socket");
		exit(-1);
	}
	http_request[read_size] = 0;

	vector<string> lines = split(http_request, '\r');
	parse_request_options(lines);

	if (lines.size() > 0)
	{
		vector <string> request_header = split(lines[0], ' ');
		string request_type = request_header[0];
		if ( request_type == "GET")
		{
			response_on_get_request(client, request_header[1]);
		}
		else
		{
			printf("Request '%s' not supported\n", request_type.c_str());
		}

	}
	else
	{
		fprintf(stdout, "Got empty request\n");
	}
#if ! defined(_WIN32)
	exit(0);
#else
	closesocket(client);
#pragma comment(user, "TODO: multithread")
#endif
}

int main()
{
	int 			client, server;
	address_t 		client_addr, server_addr;
	int				size_of_client_address;

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
	server_addr.sin_port = htons(8080);

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

	printf("Server ready to accept connections\n");

	while (true)
	{
		size_of_client_address = sizeof(client_addr);
		client = accept(server, (sockaddr_t *)&client_addr, (socklen_t*)&size_of_client_address);
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
			printf("Connection from %d.%d.%d.%d\n",
				((client_addr.sin_addr.s_addr) & 0x00000ff),
				((client_addr.sin_addr.s_addr >> 8) & 0x00000ff),
				((client_addr.sin_addr.s_addr >> 16) & 0x00000ff),
				(client_addr.sin_addr.s_addr >> 24)
				);

			close(client);
			break;
		}
#else
		if (client < 0)
		{
			fprintf(stderr, "Unable accept connection: %d\n", WSAGetLastError());
			exit(-1);
		}

		serve(client);
#endif
	}

	return 0;
}


