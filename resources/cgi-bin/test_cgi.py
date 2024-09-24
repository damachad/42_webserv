#!/usr/bin/env python3

import cgi
import os

def main():
    # Set the content type for the response
    print("Content-Type: text/html")    
    print()  # End of headers

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
    form = cgi.FieldStorage()
    # while True:
    #     pass

    if os.environ.get('REQUEST_METHOD') == 'POST':
        print("<h3>POST Data</h3>")
        if form:
            print("<ul>")
            for key in form.keys():
                value = form.getvalue(key)
                print(f"<li><strong>{key}:</strong> {value}</li>")
            print("</ul>")
    else:
        print("<h3>GET Data</h3>")
        query_string = os.environ.get('QUERY_STRING', '')
        if query_string:
            print("<ul>")
            params = cgi.parse_qs(query_string)
            for key, values in params.items():
                for value in values:
                    print(f"<li><strong>{key}:</strong> {value}</li>")
            print("</ul>")

    # Print HTML end
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
