#!/usr/bin/env python3
import time
import cgi
import os
import urllib.parse

# Allocate more than 10 MB of memory
def allocate_memory():
    # Allocate a list with ~500,000 integers (each integer is ~24 bytes in Python)
    size_in_mb = 999
    num_elements = (size_in_mb * 1024 * 1024) // 24
    
    # Allocate a large list
    large_list = [0] * num_elements
    
    print(f"Allocated {size_in_mb} MB of memory.")
    
    # Keep the program running for a while to test memory limits
    time.sleep(2)

def main():
    # Set the content type for the response
    print("Content-Type: text/html")    
    print("\r\n\r\n")  # End of headers

    # Print the HTML start
    print("<html>")
    print("<head><title>CGI Test Script</title></head>")
    print("<body>")
    allocate_memory()
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()



