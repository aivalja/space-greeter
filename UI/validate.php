<?php 
$server = "localhost";
$user = "root";
$pass = "password";
$db = "recog";

$conn = new mysqli($server, $user, $pass, $db);


if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$id = $_POST["user_id"];

if(isset($_POST['yes']))
{
    $sql = "UPDATE users SET guesses=guesses+1, answers=answers+1,corrects=corrects+1 WHERE id= '".$id. "'";
}
else if (isset($_POST['no']))
{
  $sql = "UPDATE users SET guesses=guesses+1, answers=answers+1 WHERE id= '".$id. "'";
}


if ($conn->query($sql) === TRUE) {
    echo "Updated";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$sql = "DELETE FROM status WHERE id=2";

if ($conn->query($sql) === TRUE) {
    echo "Removed";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();

header("Location: http://localhost/face_recog_html/not_recognized.html");


 ?>
