<?php

  // Batch-run version of TM data collection.

require_once 'accounts.inc';
require_once 'tmdata.inc';
require_once 'util.inc';

Accounts::getAccounts($accounts, $errmsg);

$tm = new TMData();

// Defaults
$tm->queryPages = true;
$tm->startPage = 0;
$tm->maxPages = 1;
$tm->saveTickets = false;
$tm->cliMode = true;

// Overload from ini
$conf = parse_ini_file('batch_conf.ini');
//print_r($conf);

$do_all_accounts = $conf['do_all_accounts'];

$tm->queryPages = $conf['query_pages'];
$tm->startPage = $conf['start_page'];
$tm->maxPages = $conf['max_pages'];
$tm->maxOrders = $conf['max_orders'];
$tm->saveTickets = $conf['tix_save'];

$tm->startRun();

foreach ($accounts as $account)
{
    if ($do_all_accounts || strstr($conf['use_account_names'], $account['username']))
    {
	$username = $account['username'];
	$password = $account['password'];

	$tm->user = $username;
	$tm->pass = $password;

	// Do the work.
	$tm->Main();
    }
}

$tm->endRun();
