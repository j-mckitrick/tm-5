<?php

// Simple logging functions.

// -------------------------------------------------------------------------
// Create log file and optionally rename the old one.
// -------------------------------------------------------------------------
function init_log()
{
	$logfilename = $GLOBALS['tmpdir'] . 'tm-log.txt';

	if ($GLOBALS['rotate_logs'])
	{
	    $backuplogfilename = $GLOBALS['tmpdir'].'tm-log-'.date('Y-m-d-His').'.txt';

	    rename($logfilename, $backuplogfilename);
	}

	$GLOBALS['logfile'] = fopen($logfilename, 'wt');

	tm_log('Log started ' . date('Y-m-d-H:i:s'));

	$sql = "SELECT lic, dig FROM install";
	$rows = mysql_query($sql);
	$row = mysql_fetch_row($rows);
	return $row;
}

function close_log()
{
	tm_log('Done logging.');
	
	fclose($GLOBALS['logfile']);
}

// ---------------------------------------------------------------------
// Log a message.
// If $V is true, this is a verbose message that may be filtered.
// ---------------------------------------------------------------------
function tm_log($msg, $v=false)
{
	if (isset($GLOBALS['show_log']))
	    echo $msg . '<br>';

	if (($v === true && $GLOBALS['verbose_logs'] === true) || $v === false)
	    fwrite($GLOBALS['logfile'], $msg . "\n");
}

// -------------------------------------------------------------------------
// The dropfile is a one line file updated with a status message which is
// loaded on refresh by the status page as a sort of status line display.
// -------------------------------------------------------------------------
function get_dropfile()
{
	$dropfilename = $GLOBALS['tmpdir'] . $GLOBALS['filedrop'];

	return file_get_contents($dropfilename);
}

// ---------------------------------------------------------------------
// Update the dropfile with a message reflecting current status.
// ---------------------------------------------------------------------
function update_dropfile($msg)
{
	$dropfilename = $GLOBALS['tmpdir'] . $GLOBALS['filedrop'];

	file_put_contents($dropfilename, $msg);
}
