<?php

// Connect to the database

$servername = "localhost";

$username = "root";

$password = "sharedbike";

$database = "sharedbike"; // Please replace this with your database name



// Create a connection

$conn = new mysqli($servername, $username, $password, $database);



// Check the connection

if ($conn->connect_error) {

    die("Connection failed: " . $conn->connect_error);

}



// Check the request method

if ($_SERVER['REQUEST_METHOD'] == 'POST' || $_SERVER['REQUEST_METHOD'] == 'GET') {

    // Retrieve data from the request

    $bike_id = isset($_REQUEST['bike_id']) ? intval($_REQUEST['bike_id']) : null;

    $time = isset($_REQUEST['time']) ? $_REQUEST['time'] : null;

    $voltage = isset($_REQUEST['voltage']) ? floatval($_REQUEST['voltage']) : null;

    $current = isset($_REQUEST['current']) ? floatval($_REQUEST['current']) : null;

    $power = isset($_REQUEST['power']) ? floatval($_REQUEST['power']) : null;



    // Check if the data is complete

    if ($bike_id !== null && $time !== null && $voltage !== null && $current !== null && $power !== null) {

        // Construct the SQL insert statement

        $stmt = $conn->prepare("INSERT INTO `charging_station` (`bike_id`, `time`, `voltage`, `current`, `power`) VALUES (?, ?, ?, ?, ?)");

        

        if ($stmt === false) {

            http_response_code(500); // Respond with HTTP status code 500 (Internal Server Error)

            echo "Statement preparation failed: " . $conn->error;

            exit();

        }



        // Bind the parameters

        if (!$stmt->bind_param("isddd", $bike_id, $time, $voltage, $current, $power)) {

            http_response_code(500); // Respond with HTTP status code 500 (Internal Server Error)

            echo "Parameter binding failed: " . $stmt->error;

            exit();

        }

        

        // Execute the insert operation

        if ($stmt->execute()) {

            // Successfully inserted

            http_response_code(201); // Respond with HTTP status code 201 (Created)

            echo "Data uploaded successfully";

        } else {

            // Insertion failed

            http_response_code(500); // Respond with HTTP status code 500 (Internal Server Error)

            echo "Data upload failed: " . $stmt->error;

        }

        

        // Close the statement

        $stmt->close();

    } else {

        // Incomplete request data

        http_response_code(400); // Respond with HTTP status code 400 (Bad Request)

        echo "Incomplete data";

    }

} else {

    // Unsupported request method

    http_response_code(405); // Respond with HTTP status code 405 (Method Not Allowed)

    echo "Unsupported request method";

}



// Close the database connection

$conn->close();

?>
