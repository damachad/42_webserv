#!/usr/bin/env python3

import os
import urllib.parse

def main():
    # Set the content type for the response
    print("Content-Type: text/html")    
    print("\r\n\r\n")  # End of headers

    # Print the HTML start
    print("<html>")
    print("<head><title>CGI Test Script</title></head>")
    print("<body>")
    print("<h1>CGI Environment Variables</h1>")

    # Display environment variables
    print("<h2>Environment Variables</h2>")
    print("<ul>")
    for key, value in os.environ.items():
        print(f"<li><strong>{key}:</strong> {value}</li>")
    print("</ul>")

    # Handle GET and POST data
    print("<h2>Form Data</h2>")

    # Check if the request is a POST or GET request
    request_method = os.environ.get('REQUEST_METHOD', 'GET')

    if request_method == 'POST':
        print("<h3>POST Data</h3>")
        # Read the POST data from stdin
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        post_data = os.stdin.read(content_length) if content_length > 0 else ''
        
        # Parse the POST data using urllib
        if post_data:
            params = urllib.parse.parse_qs(post_data)
            print("<ul>")
            for key, values in params.items():
                for value in values:
                    print(f"<li><strong>{key}:</strong> {value}</li>")
            print("</ul>")
    else:
        print("<h3>GET Data</h3>")
        query_string = os.environ.get('QUERY_STRING', '')
        if query_string:
            print("<ul>")
            # Parse the GET data using urllib
            params = urllib.parse.parse_qs(query_string)
            for key, values in params.items():
                for value in values:
                    print(f"<li><strong>{key}:</strong> {value}</li>")
            print("</ul>")

    # Print HTML end
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
