<?php
$servername = "localhost";
$username = "root";
$password = "sharedbike";
$dbname = "sharedbike";

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Database connection failed: " . $conn->connect_error);
}

$bike_id = $_GET['bike_id'];

// Fetch all data for the given bike_id, ordered by time
$sql_data = "SELECT voltage, current, time FROM charging_station WHERE bike_id = $bike_id ORDER BY time ASC";
$result_data = $conn->query($sql_data);

$energy = 0; // Initialize total energy
$previous_time = null; // To store the previous time
$previous_current = null; // To store the previous current value

while($row = $result_data->fetch_assoc()) {
    $current_time = new DateTime($row['time']);
    $current_current = $row['current'];

    if ($previous_time !== null) {
        // Calculate the time difference in seconds
        $interval = $previous_time->diff($current_time);
        $seconds_diff = $interval->s + ($interval->i * 60) + ($interval->h * 3600);

        // If the time difference is more than 1 second, add energy for every missing second
        if ($seconds_diff > 1) {
            // Accumulate energy using the previous current for the missing seconds
            $energy += $previous_current * ($seconds_diff - 1);
        }
    }

    // Add energy for the current second
    $energy += $current_current;

    // Update previous time and current
    $previous_time = $current_time;
    $previous_current = $current_current;
}

// Fetch latest data (for display purposes)
$sql_latest = "SELECT voltage, current, time FROM charging_station WHERE bike_id = $bike_id ORDER BY time DESC LIMIT 1";
$result_latest = $conn->query($sql_latest);
$data_latest = $result_latest->fetch_assoc();

// Calculate discharge time
$sql_times = "SELECT MIN(time) AS min_time, MAX(time) AS max_time FROM charging_station WHERE bike_id = $bike_id";
$result_times = $conn->query($sql_times);
$data_times = $result_times->fetch_assoc();

$min_time = new DateTime($data_times['min_time']);
$max_time = new DateTime($data_times['max_time']);
$interval = $min_time->diff($max_time);

if ($interval->i > 10) {
    $min_time = $max_time; // Reset if gap is more than 10 minutes
}

$discharge_time = $interval->format('%i');

// Calculate service time
$today = new DateTime();
$service_time_interval = $min_time->diff($today);
$service_time = $service_time_interval->format('%a'); // Days difference

$response = array(
    "voltage" => round($data_latest['voltage'], 2),
    "current" => round($data_latest['current'], 2),
    "time" => $discharge_time,
    "energy" => round($energy, 2), // Return the accumulated energy
    "service_time" => $service_time,
    "latest_time" => $data_latest['time']
);

echo json_encode($response);

$conn->close();
?>
