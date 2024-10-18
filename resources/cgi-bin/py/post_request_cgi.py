#!/usr/bin/env python3

import cgi
import cgitb
import json
import os

cgitb.enable()  # Enable debugging

# Path to the JSON file
json_dir_path = "../json"  # Directory path
json_file_path = os.path.join(json_dir_path, "data.json")  # Full file path

# Get form data
form = cgi.FieldStorage()

# Properly format the HTTP response header
print("Content-Type: text/html")
print("\r\n\r\n")  # End of headers

# Retrieve form fields
name = form.getvalue("name", "No name provided")
email = form.getvalue("email", "No email provided")
message = form.getvalue("message", "No message provided")

# Create a dictionary with the form data
form_data = {
    "name": name,
    "email": email,
    "message": message
}

# Ensure the directory for the JSON file exists
if not os.path.exists(json_dir_path):
    os.makedirs(json_dir_path)  # Create the directory if it doesn't exist

# Read the existing JSON file (if it exists)
if os.path.exists(json_file_path):
    with open(json_file_path, "r") as json_file:
        try:
            data = json.load(json_file)
        except json.JSONDecodeError:
            data = []  # If file is corrupted or empty, initialize as empty list
else:
    data = []  # Initialize an empty list if the file doesn't exist

# Append the new form data to the list
data.append(form_data)

# Write the updated data back to the JSON file
with open(json_file_path, "w") as json_file:
    json.dump(data, json_file, indent=4)

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
