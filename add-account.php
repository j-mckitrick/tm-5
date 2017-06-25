<?php

require_once 'accounts.inc';

$username = $_REQUEST['username'];
$password = $_REQUEST['password'];

if (!empty($username) && !empty($password))
{
    if (Accounts::addAccount($username, $password, $errmsg))
    {
		header("Location: accounts.php");
    }
    else
    {
		echo 'Error adding account. ' . $errmsg;
    }
}
else
{
    echo 'No username and/or password!';
}
