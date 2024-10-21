#!/usr/bin/env python3

import os
import http.cookies as Cookie
import random
import string
import tempfile
import json

# Path where session files will be stored 
SESSION_PATH = "../tmp/"

# List of user names
USER_NAMES = [
    "Ana", "Carlos", "David", "Miguel", "Tiago",
    "Fred", "Jorge", "Joana", "Fernando", "Maria"
]

# Ensure the session directory exists
if not os.path.exists(SESSION_PATH):
    os.makedirs(SESSION_PATH)

# Function to generate a session ID
def generate_session_id():
    return ''.join(random.choices(string.ascii_letters + string.digits, k=16))

# Function to load session data from a file
def load_session(session_id):
    try:
        session_file_path = os.path.join(SESSION_PATH, f"session_{session_id}.json")
        print(session_file_path)
        if os.path.exists(session_file_path):
            with open(session_file_path, 'r') as f:
                return json.load(f)  # Load the session data from the file
        else:
            return None
    except Exception as e:
        return None

# Function to save session data to a file
def save_session(session_id, session_data):
    try:
        session_file_path = os.path.join(SESSION_PATH, f"session_{session_id}.json")
        with open(session_file_path, 'w') as f:
            json.dump(session_data, f)  # Save the session data to the file
    except Exception as e:
        pass  # Handle the error if needed

# Function to create a new session
def create_new_session():
    session_id = generate_session_id()
    user_name = random.choice(USER_NAMES)
    session_data = {"user_name": user_name, "visits": 0}
    save_session(session_id, session_data)
    return session_id, session_data

# Start of the CGI response
print("Content-Type: text/html")

# Get cookies from the environment
cookie = Cookie.SimpleCookie(os.environ.get('HTTP_COOKIE', ''))
session_id = None
session_data = None

# Check if the session cookie exists
if 'session_id' in cookie:
    session_id = cookie['session_id'].value
    # Try to load the session data from the file
    session_data = load_session(session_id)

# If session data doesn't exist or session_id is invalid, create a new session
if session_data is None:
    session_id, session_data = create_new_session()
    # Set a new cookie
    print(f"Set-Cookie: session_id={session_id}; Path=/; HttpOnly")

# Update session data (e.g., increment visit count)
session_data['visits'] += 1
save_session(session_id, session_data)

print("\r\n\r\n")
print("<html><body>")
print(f"<h1>Session Management with Temporary Files</h1>")
print(f"<p>Session ID: {session_id}</p>")
print(f"<p>Session Data: {session_data}</p>")
print("</body></html>")
