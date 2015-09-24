/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

#include <algorithm>
#include <string.h>
#include <string>
#include <stdio.h>
#include <sstream>
#include <regex>
using namespace std;

#include "fcgio.h"

#include <curl/curl.h>

void htmlTop();
void htmlBot();

int curl(string);

struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	
	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL){
		cout << "realloc returned NULL\n";
		return 0;
	}
	
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	
	return realsize;
}


int main(void)
{
	streambuf * cin_streambuf = cin.rdbuf();
	streambuf * cout_streambuf = cout.rdbuf();
	streambuf * cerr_streambuf = cerr.rdbuf();

	FCGX_Request request;
	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);
	string queryString;
	
	while(FCGX_Accept_r(&request) == 0){
		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);
		
		cin.rdbuf(&cin_fcgi_streambuf);
		cout.rdbuf(&cout_fcgi_streambuf);
		cerr.rdbuf(&cerr_fcgi_streambuf);
		
		cout << "Content-type: text/html\r\n\r\n";
		queryString = FCGX_GetParam("QUERY_STRING", request.envp);
		

		if(		regex_match(queryString, regex("(email=)(.*)"))
			&&  !(regex_match(queryString, regex("(email=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&cardN=)(.*)"))
			&&  !(regex_match(queryString, regex("(cardN=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&mo=)(.*)"))
			&&  !(regex_match(queryString, regex("(mo=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&year=)(.*)"))
			&&  !(regex_match(queryString, regex("(year=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&cv=)(.*)"))
			&&  !(regex_match(queryString, regex("(cv=&)(.*)")))
			&&  regex_match(queryString, regex("(.*)(&cur=)(.*)"))
			&&  !(regex_match(queryString, regex("(cur=&)(.*)")))
			&&  regex_match(queryString, regex("(.*)(&amount=)(.*)"))
			&&  !(regex_match(queryString, regex("(amount=&)(.*)"))))
				curl(queryString);
		else
			cout << "\n\n404 - Something went wrong...\n\n";
		
	}
	
	
	cin.rdbuf(cin_streambuf);
	cout.rdbuf(cout_streambuf);
	cerr.rdbuf(cerr_streambuf);

	return EXIT_SUCCESS;
}


int curl(string qs)
{

	
	size_t index = 0;

	//commas
	while(true){
		index = qs.find("%40", index);
		if (index == string::npos) break;
		qs.replace(index, 3, "@");
		index++;
	}
	
	unsigned short qsLen = qs.length();
	char * input = new char[qsLen];
	qs.copy(input, qsLen);
	
	string dumpEM = strtok(input, "=");
	string email = strtok(NULL, "&");


	string dumpCN = strtok(NULL, "=");
	string cardNum = strtok(NULL, "&");
	
	
	string dumpMO = strtok(NULL, "=");
	string month = strtok(NULL, "&");
	
	
	string dumpYR = strtok(NULL, "=");
	string year = strtok(NULL, "&");
	
	
	string dumpCV = strtok(NULL, "=");
	string cvc = strtok(NULL, "&");
	
	string dumpCR = strtok(NULL, "=");
	string currency = strtok(NULL, "&");
	
	string dumpAM = strtok(NULL, "=");
	string amount = strtok(NULL, "+");
	
	string curlString = "currency=" + currency + "&amount=" + amount + 
	"00&description=Donation for computermouth.com &source[object]=card&source[number]="
						+ cardNum + "&source[exp_month]=" + month +
						"&source[exp_year]=" + year + "&source[cvc]=" 
						+ cvc + "&receipt_email=" + email;
	
	CURLcode ret;
	CURL *hnd;
	
	struct MemoryStruct chunk;
	
	chunk.memory = (char*)malloc(1);
	chunk.size = 0;
	
	curl_global_init(CURL_GLOBAL_ALL);

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_URL, "https://api.stripe.com/v1/charges");
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERPWD, "YOUR_STRIPE_SECRET_KEY:");
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, curlString.c_str());
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)curlString.length());
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.38.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);
	

	ret = curl_easy_perform(hnd);

	htmlTop();
	
	string chunkReturn = chunk.memory;
	string suc = "succeeded";
	
	size_t found = chunkReturn.find(suc);
	
	if(ret == CURLE_OK && found!= string::npos){
		cout << "<p><img src=\"../faenza/success.svg\" class=\"img-responsive\" alt=\"Error Icon\"></p>";
		
		cout << "<h1 id=\"type\">Transmission completed:</h1>";
		
		cout << "\
					<div class=\"row\">\
						<div class=\"col-md-12\">";
				
		cout << "			<pre>" << chunk.memory;
		cout << "			</pre>\
						</div>\
					</div>\
				</p>";
	}else{
		cout << "<p><img src=\"../faenza/error.svg\" class=\"img-responsive\" alt=\"Error Icon\"></p>";
		
		cout << "<h1 id=\"type\">Transmission failed:</h1>";
		
		cout << "\
					<div class=\"row\">\
						<div class=\"col-md-12\">";
				
		cout << "			<pre>" << chunk.memory;
		cout << "			</pre>\
						</div>\
					</div>\
				</p>";
	}
	
	htmlBot();

	curl_easy_cleanup(hnd);
	hnd = NULL;

	curl_global_cleanup();

	return 0;
}


void htmlTop(){
	cout << " \
\
<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"utf-8\">\
    <title>computermouth</title>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />\
    <link rel=\"stylesheet\" href=\"../bootstrap/css/bootstrap.min.css\" media=\"screen\">\
 \
	<link rel=\"apple-touch-icon\" sizes=\"57x57\" href=\"../favicons/apple-touch-icon-57x57.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"60x60\" href=\"../favicons/apple-touch-icon-60x60.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"72x72\" href=\"../favicons/apple-touch-icon-72x72.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"76x76\" href=\"../favicons/apple-touch-icon-76x76.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"114x114\" href=\"../favicons/apple-touch-icon-114x114.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"120x120\" href=\"../favicons/apple-touch-icon-120x120.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"144x144\" href=\"../favicons/apple-touch-icon-144x144.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"152x152\" href=\"../favicons/apple-touch-icon-152x152.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"../favicons/apple-touch-icon-180x180.png\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/favicon-32x32.png\" sizes=\"32x32\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/android-chrome-192x192.png\" sizes=\"192x192\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/favicon-96x96.png\" sizes=\"96x96\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/favicon-16x16.png\" sizes=\"16x16\">\
	<link rel=\"manifest\" href=\"../favicons/manifest.json\">\
	<link rel=\"shortcut icon\" href=\"../favicons/favicon.ico\">\
	<meta name=\"msapplication-TileColor\" content=\"#00aba9\">\
	<meta name=\"msapplication-TileImage\" content=\"../favicons/mstile-144x144.png\">\
	<meta name=\"msapplication-config\" content=\"../favicons/browserconfig.xml\">\
	<meta name=\"theme-color\" content=\"#ffffff\">\
  </head>\
\
  <body>\
\
    <nav class=\"navbar navbar-default navbar-fixed-top\">\
      <div class=\"container\">\
        <div class=\"navbar-header\">\
          <button type=\"button\" class=\"navbar-toggle collapsed\" data-toggle=\"collapse\" data-target=\"#navbar\" aria-expanded=\"false\" aria-controls=\"navbar\">\
            <span class=\"sr-only\">Toggle navigation</span>\
            <span class=\"icon-bar\"></span>\
            <span class=\"icon-bar\"></span>\
            <span class=\"icon-bar\"></span>\
          </button>\
          <a class=\"navbar-brand\" href=\"index.html\">computermouth</a>\
        </div>\
        <div id=\"navbar\" class=\"navbar-collapse collapse\">\
          <ul class=\"nav navbar-nav navbar-right\">\
			<li>\
				<a href=\"../index.html\">HOME</a>\
			</li>\
			<li>\
				<a href=\"../projects.html\">PROJECTS</a>\
			</li>\
			<li>\
				<a href=\"../tutorials.html\">TUTORIALS</a>\
			</li>\
			<li class=\"active\">\
				<a href=\"../donate.html\">DONATE</a>\
			</li>\
			<li>\
				<a href=\"../whois.html\">WHOIS</a>\
			</li>\
		  </ul>\
        </div><!--/.navbar-collapse -->\
      </div>\
    </nav>\
\
    <!-- Main jumbotron for a primary marketing message or call to action -->\
    <div class=\"jumbotron\">\
      <div class=\"container\">\
        <h1>./response</h1>\
      </div>\
    </div>\
\
    <div class=\"container\">\
		<p>\
	\
	";
	
}


void htmlBot(){
	
	cout << "\
      <hr>\
\
      <footer>\
        <p class=\"pull-left\"><a href=\"http://www.zlib.net/zlib_license.html\">zlib</a> Ben Young 2015</p>\
        <div class=\"pull-right\">\
			<ul class=\"list-inline\">\
				<li>Thanks to:</li>\
				<li><a href=\"https://www.debian.org\">Debian</a></li>\
				<li><a href=\"https://www.digitalocean.com/?refcode=fa0d7b3cdac5\">Digital Ocean</a></li>\
			</ul>\
		</div>\
      </footer>\
    </div> <!-- /container -->\
\
\
    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>\
    <script src=\"../bootstrap/js/bootstrap.min.js\"></script>\
  </body>\
</html>\
	\
	";
}
/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

#include <algorithm>
#include <string.h>
#include <string>
#include <stdio.h>
#include <sstream>
#include <regex>
using namespace std;

#include "fcgio.h"

#include <curl/curl.h>

void htmlTop();
void htmlBot();

int curl(string);

struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	
	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL){
		cout << "realloc returned NULL\n";
		return 0;
	}
	
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	
	return realsize;
}


int main(void)
{
	streambuf * cin_streambuf = cin.rdbuf();
	streambuf * cout_streambuf = cout.rdbuf();
	streambuf * cerr_streambuf = cerr.rdbuf();

	FCGX_Request request;
	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);
	string queryString;
	
	while(FCGX_Accept_r(&request) == 0){
		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);
		
		cin.rdbuf(&cin_fcgi_streambuf);
		cout.rdbuf(&cout_fcgi_streambuf);
		cerr.rdbuf(&cerr_fcgi_streambuf);
		
		cout << "Content-type: text/html\r\n\r\n";
		queryString = FCGX_GetParam("QUERY_STRING", request.envp);
		

		if(		regex_match(queryString, regex("(email=)(.*)"))
			&&  !(regex_match(queryString, regex("(email=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&cardN=)(.*)"))
			&&  !(regex_match(queryString, regex("(cardN=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&mo=)(.*)"))
			&&  !(regex_match(queryString, regex("(mo=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&year=)(.*)"))
			&&  !(regex_match(queryString, regex("(year=&)(.*)")))
			&&	regex_match(queryString, regex("(.*)(&cv=)(.*)"))
			&&  !(regex_match(queryString, regex("(cv=&)(.*)")))
			&&  regex_match(queryString, regex("(.*)(&cur=)(.*)"))
			&&  !(regex_match(queryString, regex("(cur=&)(.*)")))
			&&  regex_match(queryString, regex("(.*)(&amount=)(.*)"))
			&&  !(regex_match(queryString, regex("(amount=&)(.*)"))))
				curl(queryString);
		else
			cout << "\n\n404 - Something went wrong...\n\n";
		
	}
	
	
	cin.rdbuf(cin_streambuf);
	cout.rdbuf(cout_streambuf);
	cerr.rdbuf(cerr_streambuf);

	return EXIT_SUCCESS;
}


int curl(string qs)
{

	
	size_t index = 0;

	//commas
	while(true){
		index = qs.find("%40", index);
		if (index == string::npos) break;
		qs.replace(index, 3, "@");
		index++;
	}
	
	unsigned short qsLen = qs.length();
	char * input = new char[qsLen];
	qs.copy(input, qsLen);
	
	string dumpEM = strtok(input, "=");
	string email = strtok(NULL, "&");


	string dumpCN = strtok(NULL, "=");
	string cardNum = strtok(NULL, "&");
	
	
	string dumpMO = strtok(NULL, "=");
	string month = strtok(NULL, "&");
	
	
	string dumpYR = strtok(NULL, "=");
	string year = strtok(NULL, "&");
	
	
	string dumpCV = strtok(NULL, "=");
	string cvc = strtok(NULL, "&");
	
	string dumpCR = strtok(NULL, "=");
	string currency = strtok(NULL, "&");
	
	string dumpAM = strtok(NULL, "=");
	string amount = strtok(NULL, "+");
	
	string curlString = "currency=" + currency + "&amount=" + amount + 
	"00&description=Donation for computermouth.com &source[object]=card&source[number]="
						+ cardNum + "&source[exp_month]=" + month +
						"&source[exp_year]=" + year + "&source[cvc]=" 
						+ cvc + "&receipt_email=" + email;
	
	CURLcode ret;
	CURL *hnd;
	
	struct MemoryStruct chunk;
	
	chunk.memory = (char*)malloc(1);
	chunk.size = 0;
	
	curl_global_init(CURL_GLOBAL_ALL);

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_URL, "https://api.stripe.com/v1/charges");
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERPWD, "YOUR_STRIPE_SECRET_KEY:");
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, curlString.c_str());
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)curlString.length());
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.38.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);
	

	ret = curl_easy_perform(hnd);

	htmlTop();
	
	string chunkReturn = chunk.memory;
	string suc = "succeeded";
	
	size_t found = chunkReturn.find(suc);
	
	if(ret == CURLE_OK && found!= string::npos){
		cout << "<p><img src=\"../faenza/success.svg\" class=\"img-responsive\" alt=\"Error Icon\"></p>";
		
		cout << "<h1 id=\"type\">Transmission completed:</h1>";
		
		cout << "\
					<div class=\"row\">\
						<div class=\"col-md-12\">";
				
		cout << "			<pre>" << chunk.memory;
		cout << "			</pre>\
						</div>\
					</div>\
				</p>";
	}else{
		cout << "<p><img src=\"../faenza/error.svg\" class=\"img-responsive\" alt=\"Error Icon\"></p>";
		
		cout << "<h1 id=\"type\">Transmission failed:</h1>";
		
		cout << "\
					<div class=\"row\">\
						<div class=\"col-md-12\">";
				
		cout << "			<pre>" << chunk.memory;
		cout << "			</pre>\
						</div>\
					</div>\
				</p>";
	}
	
	htmlBot();

	curl_easy_cleanup(hnd);
	hnd = NULL;

	curl_global_cleanup();

	return 0;
}


void htmlTop(){
	cout << " \
\
<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"utf-8\">\
    <title>computermouth</title>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />\
    <link rel=\"stylesheet\" href=\"../bootstrap/css/bootstrap.min.css\" media=\"screen\">\
 \
	<link rel=\"apple-touch-icon\" sizes=\"57x57\" href=\"../favicons/apple-touch-icon-57x57.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"60x60\" href=\"../favicons/apple-touch-icon-60x60.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"72x72\" href=\"../favicons/apple-touch-icon-72x72.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"76x76\" href=\"../favicons/apple-touch-icon-76x76.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"114x114\" href=\"../favicons/apple-touch-icon-114x114.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"120x120\" href=\"../favicons/apple-touch-icon-120x120.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"144x144\" href=\"../favicons/apple-touch-icon-144x144.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"152x152\" href=\"../favicons/apple-touch-icon-152x152.png\">\
	<link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"../favicons/apple-touch-icon-180x180.png\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/favicon-32x32.png\" sizes=\"32x32\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/android-chrome-192x192.png\" sizes=\"192x192\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/favicon-96x96.png\" sizes=\"96x96\">\
	<link rel=\"icon\" type=\"image/png\" href=\"../favicons/favicon-16x16.png\" sizes=\"16x16\">\
	<link rel=\"manifest\" href=\"../favicons/manifest.json\">\
	<link rel=\"shortcut icon\" href=\"../favicons/favicon.ico\">\
	<meta name=\"msapplication-TileColor\" content=\"#00aba9\">\
	<meta name=\"msapplication-TileImage\" content=\"../favicons/mstile-144x144.png\">\
	<meta name=\"msapplication-config\" content=\"../favicons/browserconfig.xml\">\
	<meta name=\"theme-color\" content=\"#ffffff\">\
  </head>\
\
  <body>\
\
    <nav class=\"navbar navbar-default navbar-fixed-top\">\
      <div class=\"container\">\
        <div class=\"navbar-header\">\
          <button type=\"button\" class=\"navbar-toggle collapsed\" data-toggle=\"collapse\" data-target=\"#navbar\" aria-expanded=\"false\" aria-controls=\"navbar\">\
            <span class=\"sr-only\">Toggle navigation</span>\
            <span class=\"icon-bar\"></span>\
            <span class=\"icon-bar\"></span>\
            <span class=\"icon-bar\"></span>\
          </button>\
          <a class=\"navbar-brand\" href=\"index.html\">computermouth</a>\
        </div>\
        <div id=\"navbar\" class=\"navbar-collapse collapse\">\
          <ul class=\"nav navbar-nav navbar-right\">\
			<li>\
				<a href=\"index.html\">HOME</a>\
			</li>\
			<li>\
				<a href=\"projects.html\">PROJECTS</a>\
			</li>\
			<li>\
				<a href=\"tutorials.html\">TUTORIALS</a>\
			</li>\
			<li class=\"active\">\
				<a href=\"about.html\">ABOUT</a>\
			</li>\
			<li>\
				<a href=\"contact.html\">CONTACT</a>\
			</li>\
		  </ul>\
        </div><!--/.navbar-collapse -->\
      </div>\
    </nav>\
\
    <!-- Main jumbotron for a primary marketing message or call to action -->\
    <div class=\"jumbotron\">\
      <div class=\"container\">\
        <h1>./response</h1>\
      </div>\
    </div>\
\
    <div class=\"container\">\
		<p>\
	\
	";
	
}


void htmlBot(){
	
	cout << "\
      <hr>\
\
      <footer>\
        <p class=\"pull-left\"><a href=\"http://www.zlib.net/zlib_license.html\">zlib</a> Ben Young 2015</p>\
        <div class=\"pull-right\">\
			<ul class=\"list-inline\">\
				<li>Thanks to:</li>\
				<li><a href=\"https://www.debian.org\">Debian</a></li>\
				<li><a href=\"https://www.digitalocean.com/?refcode=fa0d7b3cdac5\">Digital Ocean</a></li>\
			</ul>\
		</div>\
      </footer>\
    </div> <!-- /container -->\
\
\
    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>\
    <script src=\"../bootstrap/js/bootstrap.min.js\"></script>\
  </body>\
</html>\
	\
	";
}
