<?php

// Entry point for TM system web interface (adv-login-tm.php).

require_once 'accounts.inc';
require_once 'tmdata.inc';
require_once 'util.inc';

echo '<html>';
echo '<head>';

echo '<title>Ticketmaster downloader</title>';

echo '</head>';

echo '<body>';

$accounts = array();

$sel = $_REQUEST['sel_accounts'];
foreach ($sel as $id)
{
    //echo 'Account id: ' . $id . '<br>';

    if (Accounts::getAccount($id, $results, $errmsg))
    {
		$accounts[] = $results[0];
	
		//echo 'Username: ' . $results[0]['username'] . '<br>';
		//echo 'Password: ' . $results[0]['password'] . '<br>';
    }
    else
    {
		echo 'Could not get account.<br>';
		echo $errmsg . '<br>';
    }
}

$tm = new TMData();

$query_method = Util::getRequestValue('query_method');

if ($query_method == 'pages')
{
    $tm->queryPages = true;

    // Page constraints.
    $temp = Util::getRequestValue('tm_start_page');
    if ($temp != '')
		$tm->startPage = $temp;

    $temp = Util::getRequestValue('tm_max_page');
    if ($temp != '')
		$tm->maxPages = $temp;
}
else if (strstr($query_method, 'dates'))
{
    $tm->queryDates = true;

    // Date constraints.
    $tm->startMonth = Util::getRequestValue('month_start');
    $tm->startDay = Util::getRequestValue('day_start');
    $tm->startYear = Util::getRequestValue('year_start');
    $tm->startDate = mktime(0, 0, 0, $tm->startMonth, $tm->startDay, $tm->startYear);

    $tm->endMonth = Util::getRequestValue('month_end');
    $tm->endDay = Util::getRequestValue('day_end');
    $tm->endYear = Util::getRequestValue('year_end');
    $tm->endDate = mktime(0, 0, 0, $tm->endMonth, $tm->endDay, $tm->endYear);
    
    if ($query_method == 'eventdates')
    {
		$tm->eventDates = true;
    }
    else if ($query_method == 'orderdates')
    {
		$tm->orderDates = true;
    }
}

if (Util::getRequestValue('save_tickets') == '1')
    $tm->saveTickets = true;

if (Util::getRequestValue('old_tickets') == '1')
    $tm->saveOldTickets = true;

if (Util::getRequestValue('old_tickets') == '1')
    $tm->skipFailed = true;

$tm->startRun();

foreach ($accounts as $account)
{
    // Collect form data.
    //$username = Util::getRequestValue('tm_user');
    //$password = Util::getRequestValue('tm_pass');
    $username = $account['username'];
    $password = $account['password'];

    $tm->user = $username;
    $tm->pass = $password;

    // Do the work.
    $tm->Main();
}

$tm->endRun();

// Show the result.
echo "<br><a href=\"dl-data.php?file=tm-data.csv\">Download data</a><br>";
update_dropfile("<a href=\"dl-data.php?file=tm-data.csv\">Download data</a><br>\n");

echo '</body>';
echo '</html>';
