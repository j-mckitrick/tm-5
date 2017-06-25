<?php

  // Bare-bones db connection setup for TM app.

if (isset($_SERVER['WINDIR']))
    $db_pass = "";
else
    $db_pass = "root";

$success = mysql_pconnect('localhost', 'root', $db_pass);
if (!$success)
    die ("<b>Cannot connect to database, check username, password and host.</b>");

$db_name = "tm";

$success = mysql_select_db($db_name);
if (!$success)
{
    print "<b>Cannot choose database, check if database name is correct.";
    die();
}
