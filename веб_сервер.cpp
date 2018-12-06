#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
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
#if true
	string fullpath = "/tmp/";
	fullpath += filename;
	FILE * page = fopen(fullpath.c_str(), "rt");
	if(page == NULL)
		response = "<html><body><h1>Page " + filename +  " not found!</h1>\r\n";
	else
	{
		while(!feof(page))
		{
			response += fgetc(page); 
		}
		fclose(page);
	}
#else	
	response = "<html><body><h1>Hello, World!</h1><br><p>Our page is working now</p></body></html>\r\n";
#endif	
	return response;
}



vector <string> split(const string &s, char delimiter)
{
	vector<string> tokens;
	string token;
	istringstream tokenStream(s);
	while(getline(tokenStream, token, delimiter)){
		tokens.push_back(token);
		}
		return tokens;
	}

void  serve(int client)
{
	
	string	http_response, html;
	char	http_request[4096];
	int read_size;
	read_size = recv(client, http_request, sizeof(http_request), 0);
	if(read_size < 0)
	{
		perror("Unable read socket");
		exit(-1);
	}
	string req=http_request;
	vector <string> words=split(req, ' ');
	puts(http_request);
	
//	html = Get_HTML("demo.html");
	html = Get_HTML(words[1]);

	std::stringstream len;
	len << html.length();

	
	http_response = "HTTP/1.1 200 OK\r\n";
	http_response += "Date: Wen 27 Nov 2018 17:30:15 GMT\r\n";
	http_response += "Server: MyCoolServer/0.0.1\r\n";
	http_response += "Content-Length: " +  len.str() + "\r\n";
	http_response += "Content-Type: text/html\r\n";
	http_response += "Connection: Closed\r\n";
	http_response += "\r\n";
	http_response += html;
	
	write(client, http_response.c_str(), http_response.length());
//	write(client, "Hello world!\r\n", 14);
	exit(0);
}

int main()
{
	int 			client, server;
	address_t 		client_addr, server_addr;
	int				size_of_client_address;
	
	server = socket(AF_INET, SOCK_STREAM, 0);
	if(server == -1)
	{
		perror("Unable create socket");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);
	
	if( bind(server, (sockaddr_t *) &server_addr, sizeof(server_addr)) < 0  )
	{
		perror("Unable bind socket");
		exit(-1);
	}
	
	listen(server, 1000);

	printf("Server ready to accept connections\n");
	
	while(true)
	{
		client = accept(server, (sockaddr_t *)  &client_addr, (socklen_t*) &size_of_client_address);
		switch( fork() )
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
	}
	
	return 0;
}


