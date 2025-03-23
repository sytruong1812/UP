#include <string>
#include <iostream>
#include "cjson.h"
#include "utils.h"

#include "curl.h"
#include "easy.h"
#include "urldata.h"

void TEST_GET();
void TEST_POST();
void TEST_HTTPS();
void TEST_GET_HEADER();
void TEST_DOWNLOAD_FILE();
void TEST_WEBSOCKET();
void TEST_FTP();

int main()
{
	int number = 0;
	while (true)
	{
		std::cout << "\n============[ List action ]=============" << std::endl;
		std::cout << " [1] HTTP_GET" << std::endl;
		std::cout << " [2] HTTP_POST" << std::endl;
		std::cout << " [3] HTTPS" << std::endl;
		std::cout << " [4] HTTP_HEADER" << std::endl;
		std::cout << " [5] DOWNLOAD_FILE" << std::endl;
		std::cout << " [6] WEBSOCKET" << std::endl;
		std::cout << " [7] FTP" << std::endl;
		std::cout << " [E] Exit program!" << std::endl;
		std::cout << "=> Input a number: ";

		std::string input;
		std::getline(std::cin, input);
		if (strcmp(input.c_str(), "e") == 0 || strcmp(input.c_str(), "E") == 0)
		{
			return 0;
		}
		else if (Utils::is_number(input))
		{
			number = atoi(input.c_str());
			switch (number)
			{
				case 1:
					std::cout << "\n==========[ HTTP_GET ]=============\n\n";
					TEST_GET();
					break;
				case 2:
					std::cout << "\n==========[ HTTP_POST ]============\n\n";
					TEST_POST();
					break;
				case 3:
					std::cout << "\n=============[ HTTPS ]==============\n\n";
					TEST_HTTPS();
					break;
				case 4:
					std::cout << "\n==========[ HTTP_HEADER ]============\n\n";
					TEST_GET_HEADER();
					break;
				case 5:
					std::cout << "\n==========[ DOWNLOAD_FILE ]=============\n\n";
					TEST_DOWNLOAD_FILE();
					break;
				case 6:
					std::cout << "\n============[ WEBSOCKET ]==============\n\n";
					TEST_WEBSOCKET();
					break;
				case 7:
					std::cout << "\n===============[ FTP ]=================\n\n";
					TEST_FTP();
					break;
				default:
					std::cout << "=> Invalid input!" << std::endl;
					break;
			}
		}
		else
		{
			continue;
		}
	}
	return 0;
}

struct text {
	char* ptr;
	size_t len;
};
static void init_text(struct text* s)
{
	s->len = 0;
	s->ptr = (char*)malloc(s->len + 1);
	if (s->ptr == NULL)
	{
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}
static size_t write_func(void* ptr, size_t size, size_t nmemb, struct text* s)
{
	size_t new_len = s->len + size * nmemb;
	s->ptr = (char*)realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL)
	{
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;
	return size * nmemb;
}

void TEST_GET()
{
	char url[] = "http://localhost:8080/json";
	CURL* curl = curl_easy_init();
	if (curl)
	{
		struct text s;
		init_text(&s);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "HTTP 1.0");
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);

		/* Perform the request, res gets the return code */
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			printf("\nResponse: %s\n", s.ptr);
			free(s.ptr);
		}
		curl_easy_cleanup(curl);
	}
}

void TEST_POST() {
	CURL* curl;
	CURLcode res;

	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "user_name", "An");
	cJSON_AddStringToObject(root, "password", "Key37");
	char* post_str = cJSON_Print(root);
	cJSON_Delete(root);

	curl = curl_easy_init();
	if (curl)
	{
		struct text s;
		init_text(&s);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "HTTP 1.0");

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post_str));

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			printf("\nResponse: %s\n", s.ptr);
			free(s.ptr);
		}
		curl_easy_cleanup(curl);
	}
}

void TEST_HTTPS()
{
	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl)
	{
		struct text s;
		init_text(&s);

		curl_easy_setopt(curl, CURLOPT_URL, "https://desktop-im3unol:443/");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); /* redirects */

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			printf("\nResponse: %s\n", s.ptr);
			free(s.ptr);
		}
		curl_easy_cleanup(curl);
	}

}

void TEST_GET_HEADER()
{
	char url[] = "http://localhost:8080/";
	CURL* curl = curl_easy_init();
	if (curl)
	{
		struct text s;
		init_text(&s);

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &s);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_func);

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			printf("\nResponse: %s\n", s.ptr);
			free(s.ptr);
		}
		curl_easy_cleanup(curl);
	}
}

void TEST_DOWNLOAD_FILE()
{
	FILE* fp;
	CURLcode res;
	char url[] = "http://localhost:8080/aaa.txt";
	char fout[] = "T:\\Me\\SE26\\server_ps1\\source\\out\\aaa.txt";

	CURL* curl = curl_easy_init();
	if (curl)
	{
		fopen_s(&fp, fout, "wb");

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 1024);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);		//pass headers to the data stream
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		if (fp) fclose(fp);
		curl_easy_cleanup(curl);
	}
}

void TEST_WEBSOCKET()
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, "ws://127.0.0.1:8081");
		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L); /* WebSocket style */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		/* Perform the request, res gets the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else
		{
			/* Send data */
			size_t sent = 0;
			std::string message = "Hello World!";
			CURLcode result = curl_ws_send(curl, message.c_str(), message.size(), &sent);

			/* connected and ready */
			size_t rlen;
			char buffer[256];
			curl_ws_recv(curl, buffer, sizeof(buffer), &rlen);
			printf("\nResponse: %s\n", buffer);

			/* close the connection */
			result = curl_ws_send(curl, "", 0, &sent);
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
}

struct FtpFile {
	const char* filename;
	FILE* stream;
};
static size_t my_fwrite(void* buffer, size_t size, size_t nmemb, void* stream)
{
	struct FtpFile* out = (struct FtpFile*)stream;
	if (!out->stream)
	{
		/* open file for writing */
		fopen_s(&out->stream, out->filename, "wb");
		if (!out->stream)
		{
			return 0; /* failure, cannot open file to write */
		}
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

void TEST_FTP()
{
	CURL* curl;
	CURLcode res;
	struct FtpFile ftpfile = { "data.txt", NULL };

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl)
	{
		/* You better replace the URL with one that works! */
		curl_easy_setopt(curl, CURLOPT_URL, "ftp://localhost:21/file.txt");
		/* Define our callback to get called when there is data to be written */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
		/* Set a pointer to our struct to pass to the callback */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
		/* Switch on full protocol/debug output */
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);
		if (CURLE_OK != res)
		{
			fprintf(stderr, "curl told us %d\n", res);
		}
		curl_easy_cleanup(curl);
	}

	if (ftpfile.stream)
	{
		fclose(ftpfile.stream);
	}

	curl_global_cleanup();
}
