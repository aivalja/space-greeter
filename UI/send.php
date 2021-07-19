<?php 
$server = "localhost";
$user = "root";
$pass = "password";
$db = "recog";

$conn = new mysqli($server, $user, $pass, $db);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

if(isset($_POST['alreadyhasaccount']) && 
   $_POST['alreadyhasaccount'] == 'Yes') 
{
    $nick = $_POST["nick"];
    $sql = "UPDATE users SET guesses=guesses+1, answers=answers+1 WHERE nick= '".$nick. "'";
}
else{

    $sql = "INSERT INTO users (nick)
    VALUES ('$_POST[nick]')";   

}
if ($conn->query($sql) === TRUE) {
    echo "New record created successfully";

} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}
$conn->close();
$conn = new mysqli($server, $user, $pass, $db);
$sql = "SELECT id FROM users ORDER BY id DESC LIMIT 1";
$id = $conn->query($sql);
if ($id->num_rows > 0) {
    while($row = mysqli_fetch_assoc($id))
        $user_id = $row["id"]; 

    utf8_encode($user_id);
    echo json_encode($user_id);
}

$sql = "INSERT into status (id, person_id, confidence, status)
VALUES (1, '".$user_id."', NULL, 0)";

if ($conn->query($sql) === TRUE) {
    echo "New user added!";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
header("Location: http://localhost/face_recog_html/not_recognized.html"); 

 ?>
</body>
</html>
