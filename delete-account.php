<?php

require_once 'accounts.inc';
$id = $_REQUEST['id'];

if (!empty($id))
{
    if (Accounts::deleteAccount($id, $errmsg))
    {
	header("Location: accounts.php");
    }
    else
    {
	echo 'Error deleting account. ' . $errmsg;
    }
}
else
{
    echo 'No id!';
}
