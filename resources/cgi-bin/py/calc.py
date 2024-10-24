#!/usr/bin/env python3

import cgi
import cgitb
cgitb.enable()  # Enable CGI error reporting

def generate_html(result=None, error=None):
    """Generates HTML content dynamically."""
    html = """
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="icon" href="../../assets/favicon.ico" type="image/x-icon">
    	<link rel="stylesheet" href="../../assets/css/style.css">
        <title>CGI Calculator</title>
    </head>
    <body>
        <h2>Simple CGI Calculator</h2>
        <p>This calculator will add both numbers you enter below:</p>
        <form action="/cgi-bin/py/calc.py" method="post">
            <label for="num1">First Number:</label>
            <input type="text" id="num1" name="num1"><br><br>
            <label for="num2">Second Number:</label>
            <input type="text" id="num2" name="num2"><br><br>
            <input type="submit" value="Calculate">
        </form>
        <br>
    	<a href="index.html">Back to Index</a>
    """
    
    if error:
        html += f"<p style='color: red;'>{error}</p>"
    elif result is not None:
        html += f"<p>Result: {result}</p>"

    html += "</body></html>"
    return html


def main():
    # Print necessary headers first
    print("Content-Type: text/html\n")
    print("\r\n\r\n")
    
    # Fetch user inputs
    form = cgi.FieldStorage()
    
    try:
        num1 = float(form.getvalue('num1'))
        num2 = float(form.getvalue('num2'))
        result = num1 + num2
        
        # Generate and print HTML with the result
        print(generate_html(result=result))
    
    except (TypeError, ValueError):
        # Handle missing or invalid inputs
        print(generate_html(error="Please enter valid numbers."))

if __name__ == "__main__":
    main()
