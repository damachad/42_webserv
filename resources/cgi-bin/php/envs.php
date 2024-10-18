#!/usr/bin/env php

<?php
// Set the content type for the response
echo("Content-Type: text/html");
echo("\r\n\r\n");

// Print the HTML start
echo "<html>";
echo "<head><title>CGI Test Script</title></head>";
echo "<body>";
echo "<h1>CGI Environment Variables</h1>";

// Display environment variables
echo "<h2>Environment Variables</h2>";
echo "<ul>";
foreach ($_SERVER as $key => $value) {
    echo "<li><strong>$key:</strong> ";
    if (is_array($value)) {
        echo implode(', ', $value);  // Convert array to a comma-separated string
    } else {
        echo $value;
    }
    echo "</li>";
}
echo "</ul>";

// Handle GET and POST data
echo "<h2>Form Data</h2>";

// Check if the request is a POST or GET request
$request_method = $_SERVER['REQUEST_METHOD'];

if ($request_method == 'POST') {
    echo "<h3>POST Data</h3>";
    // Read the POST data
    $post_data = $_POST;

    if (!empty($post_data)) {
        echo "<ul>";
        foreach ($post_data as $key => $value) {
            echo "<li><strong>$key:</strong> $value</li>";
        }
        echo "</ul>";
    }
} else {
    echo "<h3>GET Data</h3>";
    $query_string = $_SERVER['QUERY_STRING'];
    if (!empty($query_string)) {
        echo "<ul>";
        parse_str($query_string, $params);
        foreach ($params as $key => $values) {
            foreach ((array)$values as $value) {
                echo "<li><strong>$key:</strong> $value</li>";
            }
        }
        echo "</ul>";
    }
}

// Print HTML end
echo "</body>";
echo "</html>";
?>
