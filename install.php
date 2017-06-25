<?php

require_once 'conn.inc';

error_reporting(E_ALL);

function dosql($sql)
{
    echo 'Executing: ' . $sql . '<br>';
    mysql_query($sql);

    if (mysql_errno() > 0)
    {
	print "Error: ";
	print mysql_error();
	print "<br>\n";
    }
}

function setupLic($clean=true)
{
    if ($clean)
    {
	$sql = 'DROP TABLE install';
	dosql($sql);

	$sql  = 'CREATE TABLE install';
	$sql .= ' (user varchar(40), lic varchar(80), dig varchar(80))';
	dosql($sql);
    }
    else
    {
	$sql = 'TRUNCATE TABLE install';
	dosql($sql);
    }
}

function setupTickets($clean=true)
{
    if ($clean)
    {
	$sql = 'DROP TABLE tickets';
	dosql($sql);

	$sql  = 'CREATE TABLE tickets ';
	$sql .= '(';
	$sql .= 'compare_status text, ';
	$sql .= 'received_printed boolean, ';
	$sql .= 'listed boolean, ';
	$sql .= 'comments text, ';
	$sql .= 'order_date date, ';
	$sql .= 'order_num varchar(40), ';
	$sql .= 'order_page tinyint, ';
	$sql .= 'event_name tinytext, ';
	$sql .= 'event_date date, ';
	$sql .= 'event_time varchar(40), ';
	$sql .= 'venue tinytext, ';
	$sql .= 'city varchar(40), ';
	$sql .= 'state varchar(10), ';
	$sql .= 'currency varchar(10), ';
	$sql .= 'amount decimal(6,2), ';
	$sql .= 'seat_group tinyint, ';
	$sql .= 'seats_from varchar(10), ';
	$sql .= 'seats_thru varchar(10), ';
	$sql .= 'section varchar(10), ';
	$sql .= 'row varchar(10), ';
	$sql .= 'ticket_type varchar(10), ';
	$sql .= 'ticketfast boolean, ';
	$sql .= 'price decimal(6,2), ';
	$sql .= 'bld_chg decimal(6,2), ';
	$sql .= 'conv_chg decimal(6,2), ';
	$sql .= 'description tinytext, ';
	$sql .= 'quantity tinyint, ';
	$sql .= 'total_charges decimal(6,2), ';
	$sql .= 'card_owner varchar(40), ';
	$sql .= 'card_type varchar(10), ';
	$sql .= 'card_last4 smallint, ';
	$sql .= 'billing_addr tinytext, ';
	$sql .= 'account_name tinytext, ';
	$sql .= 'got_tix_today varchar(10), ';
	$sql .= 'fetch_date date, ';
	$sql .= 'tm_status text,';
	$sql .= 'tm_status_p text,';
	$sql .= 'tm_status_o text,';
	$sql .= 'dummy text';
	$sql .= ')';
	dosql($sql);
    }
    else
    {
	$sql = 'TRUNCATE TABLE tickets';
	dosql($sql);
    }
}

setupLic();
setupTickets();

?>

<h3>Installation complete</h3>
<a href="index.html">Main Menu</a><br/>
