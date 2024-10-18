#!/usr/bin/env python3

import cgi
import cgitb
import sys

cgitb.enable()  # Enable debugging

# Get form data
form = cgi.FieldStorage()

# Log the form data to stderr (which is often logged by the server)
print("Content-Type: text/html")  # Properly format the HTTP response header
print("\r\n\r\n")  # End of headers

# Check if the form data is received

# Retrieve form fields
name = form.getvalue("name", "No name provided")
email = form.getvalue("email", "No email provided")
message = form.getvalue("message", "No message provided")

# Display the user's input in an HTML format
print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Form Submission</title>
</head>
<body>
    <h1>Form Submission Details</h1>
    <p><strong>Name:</strong> {name}</p>
    <p><strong>Email:</strong> {email}</p>
    <p><strong>Message:</strong> {message}</p>
</body>
</html>
""")
