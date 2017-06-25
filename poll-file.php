<?php

// Check the dropfile for status of the running script.

require_once 'globals.inc';
require_once 'logging.inc';

function is_done()
{
    $status = get_dropfile();

    if ($status == "0" || strstr($status, 'href'))
	return true;

    return false;
}

echo '<html>';
echo '<head>';

echo '<title>Ticketmaster download status</title>';
if (!is_done())
    echo '<meta http-equiv="refresh" content="5;poll-file.php' . $params . '">';

echo '</head>';

echo '<body>';

if (is_done())
{
    echo 'Done!<br>';
    echo get_dropfile();
}
else
{
    //echo '<meta http-equiv="refresh" content="5;poll-file.php' . $params . '">';
    echo get_dropfile();
}

echo '</body>';
echo '</html>';
