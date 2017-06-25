<?php

// Query db order data, render as table, and export to csv.

require_once 'conn.inc';

class Orders
{
	static $progColumnNames = array('Select',
									'Ticket status',
									'Comparison status');

	static $dataColumnNames = array('Received/Printed',
									'Listed',
									'Comments',
									'Order date',
									'Order #',
									'Page #',
									'Event',
									'Event date',
									'Event time',
									'Venue',
									'City',
									'State',
									'Quantity',
									'Section',
									'Row',
									'Seats from',
									'Seats thru',
									'Description',
									'Type of tickets',
									'Most recent TM status',
									'Previous TM status',
									'Oldest previous TM status',
									'Downloaded this run',
									'Download date',
									'Account',
									'Currency',
									'Cost of ticket',
									'Amount',
									'Price',
									'Bld Chg',
									'Conv Chg',
									'Total charges',
									'Type',
									'Card owner',
									'Card type',
									'Last 4 digits',
									'Billing address');

	static $compareStatusOptions = array('-' => 'Blank',
										 'NEW' => 'New',
										 'UNCHANGED' => 'Unchanged',
										 'CHANGED' => 'Changed',
										 'PAST' => 'Past');

	static $ticketStatusOptions = array('done' => 'Done',
										'waiting' => 'Waiting',
										'canceled' => 'Canceled');

	static $yesNoOptions = array('1' => 'Yes', '0' => 'No');

	function getColumnNames()
	{
		return array_merge(Orders::$progColumnNames, Orders::$dataColumnNames);
	}

	function getOptionString($optionsArray, $sel='')
	{
		$optionString = '';

		foreach ($optionsArray as $val => $opt)
		{
			$selected = $sel == $val ? 'selected="selected"' : '';
			$optionString .= "<option value='$val' $selected>$opt</option>";
		}

		return $optionString;
	}

	function getCompareStatusOptions($sel='')
	{
		return Orders::getOptionString(Orders::$compareStatusOptions, $sel);
	}

	function getTicketStatusOptions($sel='')
	{
		return Orders::getOptionString(Orders::$ticketStatusOptions, $sel);
	}

	function getYesNoOptions($sel='')
	{
		return Orders::getOptionString(Orders::$yesNoOptions, $sel);
	}

	function getAllOrders($query=false)
	{
		$sql  = "SELECT ";
		$sql .= "* ";
		$sql .= "FROM tickets ";

		if ($query)
			$sql .= $query;

		$rows = mysql_query($sql);

		if (mysql_errno() > 0)
			echo "Error: " . mysql_error();

		return $rows;
	}

	function getOrder($orderNum, $seatGroup=false)
	{
//	    echo "Getting order: " . $orderNum . "\n";

		$sql  = "SELECT " .
			"compare_status, received_printed, listed, comments, " .
			"order_date, order_num, order_page, " .
			"event_name, event_date, event_time, " .
			"venue, city, state, " .
			"quantity, section, row, seats_from, seats_thru," .
			"currency, description, ticket_type, " .
			"tm_status, tm_status_p, tm_status_o, " .
			"got_tix_today, fetch_date, " .
			"account_name, price, bld_chg, conv_chg, total_charges, " .
			"ticket_type, card_owner, card_type, card_last4, billing_addr " .
			"FROM tickets " .
			"WHERE order_num LIKE '$orderNum' ";
		if ($seatGroup)
			$sql .= "AND seat_group = $seatGroup ";

		$rows = mysql_query($sql);

//	    echo "SQL: $sql\n";
		if (mysql_errno() > 0)
			echo "Error: " . mysql_error();

		return $rows;
	}

	// ---------------------------------------------------------------------
	// Check if an order is in the database.
	// NB: We do not check seat_group, so there may be more than one.
	// ---------------------------------------------------------------------
	function checkExisting($orderNum)
	{
		$rows = Orders::getOrder($orderNum);

//	    assert(mysql_num_rows($rows) <= 1);
		if (mysql_num_rows($rows) > 1)
			echo 'XXX: ' . $orderNum . ': ' . mysql_num_rows($rows) . "\n";

		if (mysql_num_rows($rows) > 0)
			return true;
		else
			return false;
	}

	// ---------------------------------------------------------------------
	// Build array of status items for an order row.
	// ---------------------------------------------------------------------
	function getOrderRowStatus($row)
	{
		$details = array();

		if (stristr($row['tm_status'], 'cancel'))
			$details[] = 'Cancelled';
		else if ($row['received_printed'] && $row['listed'])
			$details[] = 'DONE';
		else
			$details[] = 'Waiting';

		$details[] = $row['compare_status'];
		$details[] = $row['received_printed'] ? 'Yes' : 'No';
		$details[] = $row['listed'] ? 'Yes' : 'No';

		return $details;
	}

	// ---------------------------------------------------------------------
	// Build array of details for an order row.
	// ---------------------------------------------------------------------
	function getOrderRowDetail($row, $csv=false)
	{
		$details = array();

		$unixtimeOrder = strtotime($row['order_date']);
		$isotimeOrder = strftime('%m/%d/%Y', $unixtimeOrder);
		$unixtimeEvent = strtotime($row['event_date']);
		$isotimeEvent = strftime('%m/%d/%Y', $unixtimeEvent);

		$details[] = $row['comments'];
		$details[] = $isotimeOrder;
		$details[] = $row['order_num'];
		$details[] = $row['order_page'];

		$details[] = $row['event_name'];
		$details[] = $isotimeEvent;
		$details[] = $row['event_time'];

		$details[] = $row['venue'];
		$details[] = $row['city'];
		$details[] = $row['state'];

		$quantity = $row['quantity'];

		$details[] = $quantity;
		$details[] = $row['section'];
		$details[] = $row['row'];
		$details[] = $row['seats_from'];
		$details[] = $row['seats_thru'];

		$totalCharges = $row['total_charges'];

		$details[] = $row['description'];
		$details[] = $row['ticketfast'] ? 'TicketFast' : 'Hard tickets';

		$details[] = $row['tm_status'];
		$details[] = $row['tm_status_p'];
		$details[] = $row['tm_status_o'];

		$details[] = $row['got_tix_today'];

		$unixDate = strtotime($row['fetch_date']);
		$isoDate = strftime('%m/%d/%Y', $unixDate);
		$fetchDate = $isoDate;

		if ($row['fetch_date'] != '0000-00-00')
			$details[] = $fetchDate;
		else
			$details[] = '';

		$details[] = $row['account_name'];

		$details[] = $row['currency'];

		if ($quantity > 0)
			$details[] = round($totalCharges / $quantity, 2);
		else
			$details[] = 'N/A';

		$details[] = $totalCharges;

		$details[] = $row['price'];
		$details[] = $row['bld_chg'];
		$details[] = $row['conv_chg'];
		$details[] = $totalCharges;

		$details[] = $row['ticket_type'];
		$details[] = $row['card_owner'];
		$details[] = $row['card_type'];
		$details[] = $row['card_last4'];
		$details[] = $row['billing_addr'];

		return $details;
	}

	function getOrderDetail($orderNum, $seatGroup=false)
	{
		$rows = Orders::getOrder($orderNum, $seatGroup);

		assert(mysql_num_rows($rows) > 0);
		if (mysql_num_rows($rows) == 0)
			echo 'XXX: ' . $orderNum . ': ' . mysql_num_rows($rows) . "\n";

		return mysql_fetch_assoc($rows);
	}

	// ---------------------------------------------------------------------
	// Mark this order as having fetched the pdf ticket.
	// ---------------------------------------------------------------------
	function gotTicketPDF($orderNum)
	{
		$sql  =
			"UPDATE tickets SET " .
			"fetch_date = curdate(), " .
			"got_tix_today = 'Y' " .
			"WHERE order_num = '$orderNum'";

		$rows = mysql_query($sql);
	}

	// ---------------------------------------------------------------------
	// Check if we have a pdf ticket for this order.
	// ---------------------------------------------------------------------
	function haveTicketPDF($orderNum)
	{
		$sql  =
			"SELECT date_format(fetch_date, '%m/%d/%Y') FROM tickets " .
			"WHERE order_num LIKE '%$orderNum%'";

		$rows = mysql_query($sql);
		$row = mysql_fetch_row($rows);

		// XXX not sure this is correct.  Might have empty result set.
		assert(count($row) > 0);
		if ($row[0] != "00/00/0000")
			return true;
		else
			return false;
	}

	// ---------------------------------------------------------------------
	// Write spreadsheet column headings to CSV file.
	// ---------------------------------------------------------------------
	function writeCSVHeader($handle)
	{
		$cols = Orders::getColumnNames();
		array_shift($cols);	// drop the checkbox column
		$header = implode(", ", $cols) . "\n";
		fwrite($handle, $header);
	}

	// ---------------------------------------------------------------------
	// Export an order row to csv.
	// ---------------------------------------------------------------------
	function export($row, $csvFile)
	{
		$orderStatus = Orders::getOrderRowStatus($row);
		foreach($orderStatus as $status)
			fwrite($csvFile, "$status,");

		$orderDetails = Orders::getOrderRowDetail($row);
		foreach($orderDetails as $column => $detail)
		{
			if ($column == 'comments')
				fwrite($csvFile, '"');

			fwrite($csvFile, "$detail");

			if ($column == 'comments')
				fwrite($csvFile, '"');

			fwrite($csvFile, ",");
		}

		fwrite($csvFile, "\n");
	}
}
