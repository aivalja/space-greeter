<?php 
$server = "localhost";
$user = "root";
$pass = "password";
$db = "recog";

$conn = new mysqli($server, $user, $pass, $db);


if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT * FROM status WHERE id=2";
$result = $conn->query($sql);
if ($result->num_rows > 0) {
    while($row = mysqli_fetch_assoc($result))
        $user_id = $row["id"];
        $status = $row["status"];
    utf8_encode($user_id);
    if ($user_id == 2) {
        $sql = "SELECT person_id FROM status WHERE id=2";
        $result = $conn->query($sql);
        if ($result->num_rows > 0) {
        while($row = mysqli_fetch_assoc($result))
               $person_id = $row["person_id"];
           utf8_encode($person_id);
           echo json_encode($person_id);
    }
}else{
    echo json_encode(0);
}
}

$conn->close();


 ?>

