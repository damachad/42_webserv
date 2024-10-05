import requests
from requests_toolbelt.multipart.encoder import MultipartEncoder

# Create a multipart form data payload
multipart_data = MultipartEncoder(
    fields={
        'field1': 'value1',
        'file': ('en.subject.pdf', open('en.subject.pdf', 'rb'), 'text/pdf')
    }
)

# Send the POST request using chunked encoding
response = requests.post(
    'http://localhost:8888', 
    data=multipart_data,
    headers={
        'Content-Type': multipart_data.content_type,
        'Transfer-Encoding': 'chunked'
    }
)

# Print the response from the server
print(response.status_code)
print(response.text)

