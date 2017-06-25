<?php

define("SQL_HOST",		"localhost");
define("SQL_USER",		"root");
define("SQL_DATABASE",	"tm");

if (isset($_SERVER['WINDIR']))
	define("SQL_PASS",	"");
else
	define("SQL_PASS",	"root");

class SQL
{
	function connect()
	{
	    // format is: database@host|user|pass

	    if (isset($_SERVER['WINDIR']))
			return(SQL_DATABASE."@".SQL_HOST."|".SQL_USER);
	    else
			return(SQL_DATABASE."@".SQL_HOST."|".SQL_USER."|".SQL_PASS);
	}

	function deConnect($str,&$d,&$h,&$u,&$p)
	{
	    // deconstruct our connect string
	    $arr = preg_split("/@|\|/",$str);
	    $d = $arr[0];
	    $h = $arr[1];
	    $u = $arr[2];
	    $p = $arr[3];

	    return true;
	}

	function getLastId($connect, $table, &$id, &$errMsg)
	{
	    $sql = "SELECT last_insert_id() AS id FROM $table LIMIT 1";

	    if(!SQL::getResults($connect,$sql,$r,$errMsg))
			return false;

	    $id = $r[0]["id"];

	    return true;
	}

	function getResults($connect, $sql, &$results, &$errMsg)
	{
	    $results = array();

	    SQL::deConnect($connect,$db,$host,$user,$pass);

	    if (($m = mysql_pconnect($host, $user, $pass)) === false)
	    {
			$errMsg = "Could not connect: " . mysql_error();

			return false;
	    }

	    if (mysql_select_db($db) === false)
	    {
			$errMsg = "Could not select database";

			return false;
	    }

	    // Performing SQL query
	    if (($result = mysql_query($sql,$m)) === false)
	    {
			$errMsg = "Query failed: " . mysql_error();

			return false;
	    }

	    $n = 0;

	    while ($line = mysql_fetch_array($result, MYSQL_ASSOC))
	    {
			while(list($key,$value) = each($line))
				$results[$n][$key] = $value;

			$n++;
	    }

	    // Free resultset
	    mysql_free_result($result);

	    // Closing connection
//	    mysql_close($m);

	    return true;
	}

	function doUpdate($connect, $sql, &$lastID, &$errMsg, $status=false)
	{
	    SQL::deConnect($connect,$db,$host,$user,$pass);

	    if (($m = mysql_pconnect($host, $user, $pass)) === false)
	    {
			$errMsg = "Could not connect: " . mysql_error($m);

			return false;
	    }

	    if (mysql_select_db($db) === false)
	    {
			$errMsg = "Could not select database";

			return false;
	    }

	    if (($result = mysql_query($sql,$m)) === false)
	    {
			$errMsg = "Query failed: " . mysql_error($m);

			return false;
	    }

	    if (preg_match("/INSERT|UPDATE|DELETE/i",$sql)) // ins, upd, delete
	    {
			if (mysql_affected_rows($m) == -1)
			{
				$errMsg = "Affected rows: " . mysql_error($m);

				return false;
			}

			if ($status !== false) // then they're looking for some answers
			{
				$status = "Affected rows: " . mysql_affected_rows($m);
			}

			$lastID = mysql_insert_id($m);
	    }

	    @mysql_free_result($result);

	    // Closing connection
	    mysql_close($m);

	    return true;
	}

	// Returns the current date/time in mySQL query format.
	function SQLDateTime() { return(date("Y-m-d H:i:s")); }
	
	// Returns the current date in mySQL query format.
	function SQLDate() { return(date("Y-m-d")); }	

	// Replace single quote with SQL-friendly 2 single quotes.
	function quote($t) { return(preg_replace("/'/","''",$t)); }

}
