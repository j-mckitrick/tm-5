<?php

// Query/update orders from web form.

require_once 'globals.inc';
require_once 'orders.inc';
require_once 'tmrip.inc';

class OrdersQuery
{
	var $perform;

	var $q_ticketstatus;
	var $q_comparestatus;
	var $q_receivedprinted;
	var $q_listed;
	var $q_comment;
	var $q_orderdate;
	var $q_eventdate;
	var $q_eventtime;
	var $q_venue;
	var $q_city;
	var $q_state;
	var $q_tmstatus;
	var $q_tmstatus_p;
	var $q_tmstatus_o;
	var $q_gottixtoday;

	var $sql;
	var $exportFile;
	var $exportReady = false;

	function OrdersQuery()
	{
	    $this->exportFile = $GLOBALS['exportfile'];
	}

	function makeSQLDate($date)
	{
	    if ($date)
	    {
			$unix_time = strtotime($date);
			return strftime('%Y-%m-%d', $unix_time);
	    }
	}

	// ---------------------------------------------------------------------
	// Grab a bunch of parameters and query the orders table.
	// ---------------------------------------------------------------------
	function query()
	{
	    $this->q_ticketstatus = $_POST['queryticketstatus'];
	    $this->q_comparestatus = $_POST['querycomparestatus'];
	    $this->q_receivedprinted = $_POST['queryreceivedprinted'];
	    $this->q_listed = $_POST['querylisted'];
	    $this->q_comment = $_POST['querycomment'];
	    $this->q_orderdate = $this->makeSQLDate($_POST['queryorderdate']);
	    $this->q_orderdate_to = $this->makeSQLDate($_POST['queryorderdate_to']);
	    $this->q_ordernum = $_POST['queryordernum'];
	    $this->q_event = $_POST['queryevent'];
	    $this->q_eventdate = $this->makeSQLDate($_POST['queryeventdate']);
	    $this->q_eventdate_to = $this->makeSQLDate($_POST['queryeventdate_to']);
	    $this->q_eventtime = $_POST['queryeventtime'];
	    $this->q_venue = $_POST['queryvenue'];
	    $this->q_city = $_POST['querycity'];
	    $this->q_state = $_POST['querystate'];
	    $this->q_ticketfast = $_POST['queryticketfast'];
	    $this->q_tmstatus = $_POST['querytmstatus'];
	    $this->q_tmstatus_p = $_POST['querytmstatus_p'];
	    $this->q_tmstatus_o = $_POST['querytmstatus_o'];
	    $this->q_gottixtoday = $_POST['querygottixtoday'];
	    $this->q_account = $_POST['queryaccount'];

	    $this->sql = ' WHERE order_num != -1 ';

	    switch ($this->q_ticketstatus)
	    {
	    case 'done':
			$this->sql .= "AND received_printed = TRUE AND listed = TRUE ";
			break;
	    case 'canceled':
			$this->sql .= "AND tm_status LIKE '%cancel%' ";
			break;
	    case 'waiting':
			$this->sql .= "AND NOT (received_printed = TRUE AND listed = TRUE) ";
			$this->sql .= "AND tm_status NOT LIKE '%cancel%' ";
			break;
	    default:
			break;
	    }

	    if ($this->q_comparestatus != -1)
			$this->sql .= "AND compare_status = '$this->q_comparestatus' ";

	    if ($this->q_receivedprinted == 1)
			$this->sql .= "AND received_printed = TRUE ";
	    else if ($this->q_receivedprinted == 0)
			$this->sql .= "AND received_printed = FALSE ";

	    if ($this->q_listed == 1)
			$this->sql .= "AND listed = TRUE ";
	    else if ($this->q_listed == 0)
			$this->sql .= "AND listed = FALSE ";

	    if ($this->q_comment)
			$this->sql .= "AND comments LIKE '%$this->q_comment%' ";

	    if ($this->q_orderdate)
			$this->sql .= "AND order_date >= DATE '$this->q_orderdate' ";
	    if ($this->q_orderdate_to)
			$this->sql .= "AND order_date <= DATE '$this->q_orderdate_to' ";

	    if ($this->q_ordernum)
			$this->sql .= "AND order_num LIKE '%$this->q_ordernum%' ";

	    if ($this->q_event)
			$this->sql .= "AND event_name LIKE '%$this->q_event%' ";

	    if ($this->q_eventdate)
			$this->sql .= "AND event_date >= DATE '$this->q_eventdate' ";
	    if ($this->q_eventdate_to)
			$this->sql .= "AND event_date_to <= DATE '$this->q_eventdate_to' ";

	    if ($this->q_eventtime)
			$this->sql .= "AND event_time LIKE '%$this->q_eventtime%'";

	    if ($this->q_venue)
			$this->sql .= "AND venue LIKE '%$this->q_venue%' ";

	    if ($this->q_city)
			$this->sql .= "AND city LIKE '%$this->q_city%' ";

	    if ($this->q_state)
			$this->sql .= "AND state LIKE '%$this->q_state%' ";

	    // ticketfast query here?

	    if ($this->q_tmstatus)
			$this->sql .= "AND tm_status LIKE '%$this->q_tmstatus%' ";
	    if ($this->q_tmstatus_p)
			$this->sql .= "AND tm_status_p LIKE '%$this->q_tmstatus_p%' ";
	    if ($this->q_tmstatus_o)
			$this->sql .= "AND tm_status_o LIKE '%$this->q_tmstatus_o%' ";

	    if ($this->q_gottixtoday == 1)
			$this->sql .= "AND got_tix_today LIKE 'Y%'";
	    else if ($this->q_gottixtoday == 0)
			$this->sql .= "AND got_tix_today = ''";

	    if ($this->q_account)
			$this->sql .= "AND account_name LIKE '%$this->q_account%' ";
	}

	// ---------------------------------------------------------------------
	// Update user-editable data for the orders table from parameters.
	// ---------------------------------------------------------------------
	function update()
	{
	    $u_receivedprinted = $_POST['updatereceivedprinted'];
	    $u_listed = $_POST['updatelisted'];
	    $u_comment = $_POST['updatecomment'];
	    $u_orders = $_POST['orders'];

	    // Nothing to do.
	    if (count($u_orders) == 0)
			return;

	    $this->sql = "UPDATE tickets SET ";

	    if ($u_receivedprinted == 1)
			$this->sql .= "received_printed = TRUE, ";
	    else if ($u_receivedprinted == 0)
			$this->sql .= "received_printed = FALSE, ";

	    if ($u_listed == 1)
			$this->sql .= "listed = TRUE, ";
	    else if ($u_listed == 0)
			$this->sql .= "listed = FALSE, ";

	    if ($u_comment != '')
			$this->sql .= "comments = concat(comments, '$u_comment', ' - '), ";

	    // Nothing to do.
	    if (!strstr($this->sql, '='))
			return;

	    // Make sure SQL does not have comma before WHERE.
	    $this->sql = preg_replace('/,\s*$/', ' ', $this->sql);

/*	    
	    $this->sql .= "WHERE order_num IN (''";
	    foreach ($u_orders as $orderNum)
		$this->sql .= ", '$orderNum'";
	    $this->sql .= ')';
*/

	    $this->sql .= "WHERE order_num IN (";
	    foreach ($u_orders as $orderNum)
			$this->sql .= "'$orderNum',";
	    $this->sql = preg_replace('/,\s*$/', ' ', $this->sql);
	    $this->sql .= ')';

	    $rows = mysql_query($this->sql);
	    if (mysql_errno() > 0)
			echo "Error: " . mysql_error();
	}

	// ---------------------------------------------------------------------
	// Export selected orders to csv.
	// ---------------------------------------------------------------------
	function export()
	{
	    $query = '';

	    // NB: This WHERE clause is added only if there are orders selected.
	    // Default is ALL orders are exported.  To change this behavior,
	    // create the WHERE outside this IF statement.
	    if (count($_POST['orders']) > 0)
	    {
			$query = " WHERE order_num IN (''";

			foreach ($_POST['orders'] as $orderNum)
			{
				$query .= ", '$orderNum'";
			}
			$query .= ')';
	    }

	    $rows = Orders::getAllOrders($query);

	    $csvFile = fopen($GLOBALS['tmpdir'] . $this->exportFile, 'wt');
	    Orders::writeCSVHeader($csvFile);
	    while ($row = mysql_fetch_assoc($rows))
			Orders::export($row, $csvFile);
	    fclose($csvFile);

	    $this->exportReady = true;
	}

	// ---------------------------------------------------------------------
	// Handle the action for submitted web form.
	// ---------------------------------------------------------------------
	function doPerform()
	{
	    $perform = $_POST['perform'];

	    switch ($perform)
	    {
	    case 'update':
			$this->update();
			// FALLTHROUGH
	    case 'query':
			$this->query();
			break;
	    case 'export':
			$this->export();
			break;
	    default:
			$this->q_ticketstatus = -1;
			$this->q_comparestatus = -1;
			$this->q_receivedprinted = -1;
			$this->q_listed = -1;
			$this->q_comment = '';
			$this->q_gottixtoday = -1;
			break;
	    }
	}
}
