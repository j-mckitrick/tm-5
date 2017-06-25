<?php

require_once 'sql.inc';

class Accounts
{
	function getAccounts(&$results, &$errmsg)
	{
	    $sql = 'SELECT * from accounts ORDER BY id;';

	    return SQL::getResults(SQL::connect(), $sql, $results, $errmsg);
	}

	function getAccount($id, &$results, &$errmsg)
	{
	    $sql = "SELECT * FROM accounts WHERE id = '$id';";

	    return SQL::getResults(SQL::connect(), $sql, $results, $errmsg);
	}

	function addAccount($username, $password, &$errmsg)
	{
	    $sql =  "INSERT INTO accounts (username, password) ";
	    $sql .= "VALUES ('$username', '$password');";

	    return SQL::doUpdate(SQL::connect(), $sql, $lastid, $errmsg);
	}

	function deleteAccount($id, &$errmsg)
	{
	    $sql = "DELETE FROM accounts WHERE id = '$id';";

	    return SQL::doUpdate(SQL::connect(), $sql, $lastid, $errmsg);
	}
}
