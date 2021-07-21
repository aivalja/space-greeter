<?php 
$server = "localhost";
$user = "root";
$pass = "password";
$db = "recog";

$conn = new mysqli($server, $user, $pass, $db);


if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$which = $_POST['result'];
$sql = "SELECT nick FROM users WHERE id = '".$which. "'";
$nick = $conn->query($sql);

if ($nick->num_rows > 0) {
    while($row = mysqli_fetch_assoc($nick))
        $name = $row["nick"]; 

    utf8_encode($name);
    echo json_encode($name);
}else{
    echo json_encode("Not found");
}

$conn->close();


 ?>
