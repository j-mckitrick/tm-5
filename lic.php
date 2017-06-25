<?php

require_once 'conn.inc';
require_once 'tmcurl.inc';

error_reporting(E_ALL);

function dosql($sql)
{
    mysql_query($sql);

    if (mysql_errno() > 0)
    {
	print "Error: ";
	print mysql_error();
	print "<br>\n";
    }
}

$user = $_POST['user'];
$reg = $_POST['reg'];

/*
 // Force registration default
$user = 'Jonathon McKitrick';
$reg = 'a-20-0001';
*/

/*
 // COMMAND LINE: Generate license text
php -r 'echo sha1("a-20-0001:" . date("d/m/y")) . "\n";' > dig.txt
OR
php hash.php > dig.txt
THEN
scp dig.txt jcm@mynode:/var/www/apache2-default/jcm/dig.txt
*/

$lic = $reg . ':' . date('d/m/y');
$dig = sha1($lic);
$rdig = trim(TMCurl::getPageSimple('http://67.18.89.168/apache2-default/jcm/dig.txt'));

/*
echo 'user: ' . $user . "<br>";
echo 'reg: ' . $reg . "<br>";
echo 'lic: ' . $lic . "<br>";
echo 'dig: -' . $dig . "-<br>";
echo 'rdig: -' . $rdig . "-<br>";
*/

/*
if ($dig == $rdig)
    echo 'Pass<br>';
else
    echo 'Fail<br>';
*/

if ($dig == $rdig)
{
    $sql = "TRUNCATE install";
    dosql($sql);

    $sql = "INSERT into install (user, lic, dig) VALUES ('$user', '$lic', '$dig')";
    dosql($sql);
}
else
{
    die('Registration failed.<br>');
}

?>

<h3>Registration complete</h3>
<a href="index.html">Main Menu</a><br/>
