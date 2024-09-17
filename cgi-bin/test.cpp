#include <iostream>
#include <stdlib.h>
using namespace std;

const string ENV[26] = {"AUTH_TYPE",
                        "CONTENT_LENGTH",
                        "CONTENT_TYPE",
                        "GATEWAY_INTERFACE",
                        "HTTP_ACCEPT",
                        "HTTP_ACCEPT_CHARSET",
                        "HTTP_ACCEPT_ENCODING",
                        "HTTP_ACCEPT_LANGUAGE",
                        "HTTP_FORWARDED",
                        "HTTP_HOST",
                        "HTTP_USER_AGENT",
                        "PATH_INFO",
                        "PATH_TRANSLATED",
                        "QUERY_STRING", // request specific
                        "REMOTE_ADDR",  // request specific
                        "REMOTE_HOST",
                        "REMOTE_USER",
                        "REQUEST_METHOD", // request specific
                        "SCRIPT_NAME",    // request specific
                        "SERVER_NAME",
                        "SERVER_PORT",
                        "SERVER_PROTOCOL", // request specific
                        "SERVER_SOFTWARE",
                        "HTTP_COOKIE",
                        "WEBTOP_USER",
                        "NCHOME"};

int main() {
  cout << "Content-type:text/html\r\n\r\n";
  cout << "<html>\n";
  cout << "<head>\n";
  cout << "<title>CGI Environment Variables</title>\n";
  cout << "</head>\n";
  cout << "<body>\n";
  cout << "<table border = \"0\" cellspacing = \"2\">";

  for (int i = 0; i < 24; i++) {
    cout << "<tr><td>" << ENV[i] << "</td><td>";

    // attempt to retrieve value of environment variable
    char *value = getenv(ENV[i].c_str());
    if (value != 0) {
      cout << value;
    } else {
      cout << "Environment variable does not exist.";
    }
    cout << "</td></tr>\n";
  }

  cout << "</table><\n";
  cout << "</body>\n";
  cout << "</html>\n";

  return 0;
}
