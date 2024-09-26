#!/usr/bin/env python3

import cgi
import os
import urllib.parse

def main():
    # Set the content type for the response
    print("Content-Type: text/html")    
    print("\r\n\r\n")  # End of headers

    # Print the HTML start
    print("<html>")
    print("<head><title>Webserv</title></head>")
    print("<body>")
    print("<h1>File successfully uploaded!</h1>")
    print("<h2>You legend :D</h2>")
    print('<p><a href="/">Go to Homepage</a></p>')

    # Print HTML end
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()