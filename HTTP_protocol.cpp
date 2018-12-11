#include "stdafx.h"

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


#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

extern string http_root_folder;


string Get_HTML(string filename, int &response_code)
{
	string response;

	string fullpath = http_root_folder + filename;
	ifstream input(fullpath);
	if (input.fail())
	{
		response = "<html><body><h1>Web Page " + filename + " not found!</h1>\r\n";
		response_code = 404;
	}
	else
	{
		stringstream sstr;
		while (input >> sstr.rdbuf());
		response = sstr.str();
		response_code = 200;
	}
	return response;
}


void response_on_get_request(int client, string pagename)
{
	int response_code;
	string html;

	printf("GET %s\n", pagename.c_str());

	if (pagename == "/")
		pagename = "/index.html";

	bool binary = true;
	string ext = pagename.substr(pagename.find_first_of(".") + 1);
	string content_type;
	if (ext == "html" || ext == "htm")
	{
		content_type = "text/html";
		binary = false;
	}
	else if (ext == "gif")
		content_type = "image/gif";
	else if (ext == "png")
		content_type = "image/png";
	else if (ext == "jpg")
		content_type = "image/jpeg";
	else if (ext == "css")
	{
		content_type = "text/css";
		binary = false;
	}
	else if (ext == "xml")
	{
		content_type = "application/xml";
		binary = false;
	}
	else if (ext == "ico")
		content_type = "image/png";
	else
	{
		content_type = "application/octet-stream";
		fprintf(stderr, "Cannot detect content type: %s\n", pagename.c_str());
	}

	html = Get_HTML(pagename, response_code);

	std::stringstream len;
	len << html.length();

	string http_response = "HTTP/1.1 " + std::to_string(response_code) + " OK\r\n";
	http_response += "Date: Wen 27 Nov 2018 17:30:15 GMT\r\n";
	http_response += "Server: MyCoolServer/0.0.2\r\n";
	http_response += "Content-Length: " + len.str() + "\r\n";
	http_response += "Content-Type: " + content_type + "\r\n";
	http_response += "Connection: Closed\r\n";
	http_response += "\r\n";

	http_response += html;
	send(client, http_response.c_str(), http_response.length(), 0);
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

// Some of HTTP session options
string host_option;
int port_option;
bool keep_alive;
string cache_control_option;
string user_agent;
string accept_encoding;
string accept_languages;
string referer;
string upgrade_insecure_requests;
vector<string> accept_format_list;

void parse_accepting_formats(string formats)
{
	accept_format_list.clear();

	vector <string> option = split(formats, ';');
	for (vector <string>::iterator item = option.begin(); item != option.end(); item++)
	{
		vector <string> subitems = split(*item, ',');
		for (vector <string>::iterator subitem = subitems.begin(); subitem != subitems.end(); subitem++)
		{
			bool is_format = subitem->find("/") != std::string::npos;
			if (is_format)
			{
				accept_format_list.push_back(*subitem);
			}
		}
	}
}


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

			//			printf("Host: '%s:%d'\n", host_option.c_str(), port_option);
		}
		else if (option[0] == "\nConnection")
		{
			if (value == "keep-alive")
			{
				printf("Keep alive connection option\n");
				keep_alive = true;
			}
			else
				printf("TODO: parse connection type: '%s'\n", value.c_str());

		}
		else if (option[0] == "\nUser-Agent")
			user_agent = value;
		else if (option[0] == "\nAccept")
			parse_accepting_formats(value);
		else if (option[0] == "\nAccept-Encoding")
			accept_encoding = value;
		else if (option[0] == "\nAccept-Language")
			accept_languages = value;
		else if (option[0] == "\nCache-Control")
			cache_control_option = value;
		else if (option[0] == "\nUpgrade-Insecure-Requests")
			upgrade_insecure_requests = value;
		else if (option[0] == "\nReferer")
		{
			referer = value;
			printf("Referer: '%s'\n", value.c_str());
		}
		else if (option[0] == "\nPragma")
			printf("Pragma: '%s'\n", value.c_str());
		else
			printf("Unknown option - %s: %s\n", option[0].c_str(), value.c_str());
	}
	puts("\n");
}

void  serve(int client)
{
	char	http_request[4096];
	int read_size;

	do
	{
		keep_alive = false;
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
			if (request_type == "GET")
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
	} while (false); // keep_alive);

#if ! defined(_WIN32)
	exit(0);
#else
	closesocket(client);
#pragma comment(user, "TODO: multithread")
#endif
}

