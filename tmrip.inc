<?php

// Rip data from ticketmaster web pages and export as csv.

require_once 'globals.inc';
require_once 'orders.inc';

define('TMRIP_TABLE_NONE',		0);
define('TMRIP_TABLE_PURCHASE',	1);
define('TMRIP_TABLE_EVENT',		2);
define('TMRIP_TABLE_TICKETS',	3);
define('TMRIP_TABLE_CHARGE',	4);
define('TMRIP_TABLE_BILLING',	5);

class TMRip
{
	var $fileHandle;

	var $tfBits;
	var $ticketFast = false;
	var $readyForPickup;
	var $ticketsFetchDate = '';

	var $orderDate;
	var $orderNumber;
	var $orderCurrency;
	var $orderAmount;
	var $eventName;
	var $eventDetails;
	var $eventDate;
	var $eventTime;
	var $venue;
	var $city;
	var $state;

	var $ticketDetails = array();
	var $seats = array();

	var $customerName;
	var $cardType;
	var $totalCharges = 0;
	var $last4Digits = 'xxx';
	var $billingAddress;

	var $ticketQuantity = 0;
	var $totalTickets = 0;
	var $orderStatus;

	// --------------------------------------------------------------------
	// Constructor takes handle for csv output.
	// --------------------------------------------------------------------
	function TMRip($handle)
	{
		$this->fileHandle = $handle;
	}

	// --------------------------------------------------------------------
	// Save a page of html with a given suffix
	// so it can be stripped of CR/LF and
	// viewed later if needed.
	// --------------------------------------------------------------------
	function savePage($page, $suffix=false)
	{
		$suffix = $suffix ? $suffix : 'tm_page';
		$filename = 'tm_' . $suffix . '.html';
		$tmp_filename = $GLOBALS['tmpdir'] . $filename;

		file_put_contents($tmp_filename, $page);

		return $tmp_filename;
	}

	// --------------------------------------------------------------------
	// Save a page of html with a given suffix
	// so it can be stripped of CR/LF and
	// viewed later if needed.
	// --------------------------------------------------------------------
	function savePDF($pdf_data, $suffix=false)
	{
		$suffix = $suffix ? $suffix : 'tm_';
		$filename = $suffix . '.pdf';
		$tmp_filename = $GLOBALS['tmpdir'] . $filename;

		file_put_contents($tmp_filename, $pdf_data);

		return $tmp_filename;
	}

	// --------------------------------------------------------------------
	// Strip an html file of CR/LF.
	// We might not need this later once
	// regexes are correctly tweaked.
	// --------------------------------------------------------------------
	function stripHTMLFile($filename)
	{
		if ($html_source = @fopen($filename, 'r'))
		{
			while(!(feof($html_source)))
			{
				$ch = fgetc($html_source);

				if (ord($ch) == 13)
				{
					$ch = "";
				}
				elseif (ord($ch) == 10)
				{
					$ch = "";
				}

				$line .= $ch;    
			}

			fclose($html_source);

			return $line;
		}
		else
		{
			echo 'HTML file to strip not found: ' . $filename . '<br>';
		}
	}

	// ---------------------------------------------------------------------
	// Get login seed from hidden form input named 'v'.
	// ---------------------------------------------------------------------
	function ripSeed($page)
	{
		// Get all 'input' elements.
		$re_input_attrs = '/<input([^>]*)\/>/i';

		if (preg_match_all($re_input_attrs, $page, $all_matches))
		{
			//tm_log('Matched input element.', true);

			// We only want the attribute list groups @ offset 1.
			foreach ($all_matches[1] as $match)
			{
				//tm_log('Checking input: ' . $match, true);

				// Find (hidden) element where attribute 'name' equals 'v'.
				$re_name_v = '/name=[\'\"]v[\'\"]/i';
				if (preg_match($re_name_v, $match))
				{
					//tm_log('Matched input name.', true);

					// Return the contents of the 'value' attribute.
					$re_value_v = '/value=.([^\'\"]*)./i';
					if (preg_match($re_value_v, $match, $matches))
					{
						tm_log('Matched input value.', true);

						return $matches[1];
					}
				}
			}
		}
		else
		{
			tm_log('No matches for form input element.');
			echo 'No matches for form input element.';
		}

		tm_log('getSeed failed, checking for messages.');
		TMRip::ripAnnoyingMessages($page);

		// If no 'v' attribute found, we're dead in the water.
		return false;
	}

	// --------------------------------------------------------------------
	// Rip a specific table from html.
	// --------------------------------------------------------------------
	function ripTable($line, $tablespec=array())
	{
		// $tAttr: attribute name desired
		// $tVal: attribute value desired
		// $tNum: ORDINAL number of table desired,
		// if more than 1 match the tablespec.
		list($tAttr, $tVal, $tNum) = $tablespec;

		// Find all tables.
		$re_tables = '/<table([^>]*)>(.*?)<\/table>/i';

		if (preg_match_all($re_tables, $line, $all_matches))
		{
			// Get the whole table, tags and all -
			// we need the attributes and table contents.
			$tables = $all_matches[0];

			foreach ($tables as $table)
			{
				// Find a table that matches the attribute/value specified.
				$re_table_attr_value = '/' . $tAttr . '="' . $tVal . '"/i';

				if (preg_match($re_table_attr_value, $table))
				{
					$tNum--;

					// Count to the desired table.
					if ($tNum == 0)
					{
						return $table;
					}
				}
			}
		}
		else
		{
			tm_log('No table matches TABLESPEC');

			//die('No table matches TABLESPEC');
		}
		return false;
	}

	// --------------------------------------------------------------------
	// Rip a specific div from html.
	// --------------------------------------------------------------------
	function ripDiv($line, $divspec=array())
	{
		// $tAttr: attribute name desired
		// $tVal: attribute value desired
		// $tNum: ORDINAL number of div desired,
		// if more than 1 match the divspec.
		list($tAttr, $tVal, $tNum) = $divspec;

		// Find all divs.
		$re_divs = '/<div([^>]*)>(.*?)<\/div>/i';

		if (preg_match_all($re_divs, $line, $all_matches))
		{
			// Get the whole div, tags and all -
			// we need the attributes and div contents.
			$divs = $all_matches[0];

			foreach ($divs as $div)
			{
				// Find a div that matches the attribute/value specified.
				$re_div_attr_value = '/' . $tAttr . '=.' . $tVal . '/i';

				if (preg_match($re_div_attr_value, $div))
				{
					$tNum--;

					// Count to the desired div.
					if ($tNum == 0)
					{
						return $div;
					}
				}
			}
		}
		else
		{
			tm_log('No div matches DIVSPEC');

			//die('No div matches DIVSPEC');
		}
		return false;
	}

	function ripDivContents($div)
	{
		$re = '/<div(?:[^>]*)>(.*?)<\/div>/';
		if (preg_match($re, $div, $matches))
			return $matches[1];
		else
			return false;
	}

	function ripShipDivs($page)
	{
		$seatArray = array();

		// Rip divs where class=shipValueRight

		// XXX Still need ticketQuantity, ticketFast

		$details = array('Section' => '',
						 'Row' => '', // not in labels
//							 'Seats' => null, // not in labels
						 'Type' => '',
						 'Ticket Price' => 0, // These are in reverse order for matching.
						 'Price' => 0,
						 'Facility Charge' => 0,
						 'Convenience Charge' => 0,
						 'SUBTOTAL' => 0,
						 'Tickets/Items' => 0,
						 'Order Processing Fee' => 0,
						 'TicketFast Delivery' => 0,
						 'TOTAL CHARGES' => 0,
			);

		// Seating Info
		// Section is before first <br/>
		// Row is before second <br/>
		// Seats are inside span where class=messageText
		$divspec = array('class', 'shipValueRight', 1);
		$div = TMRip::ripDiv($page, $divspec);
		$d = TMRip::ripDivContents($div);
		$re = '/(.*?)<br \/>(.*?)<br \/>(.*?)/';
		$re = '/(.*?)<br \/>(.*?)<br \/>\s*<span(?:[^>]*)>(.*?)<\/span>/';
		if (preg_match($re, $d, $matches))
		{
			$details['Section'] = $matches[1];
			$details['Row'] = $matches[2];
			$details['Type'] = $matches[3];

			$seats = explode('-', $matches[3]);
			$seatArray['from'] = trim($seats[0]);
			$seatArray['thru'] = trim($seats[1]);

			$this->seats[0] = $seatArray;
		}

		for ($i = 2, $div = true; $div; $i++)
		{
			// XXX Use a nested div ripped for some of these???
			$divspec = array('class', 'shipLabelLeft', $i);
			$div = TMRip::ripDiv($page, $divspec);
//				$b = TMRip::ripDivContents($div);
//				$label = $b;

			if ($div)
			{
				foreach ($details as $k => $v)
				{
					if (strstr($div, $k))
					{
						$b = $k;
						$label = $k;
						break;
					}
				}

				$divspec = array('class', 'shipValueRight', $i);
				$div = TMRip::ripDiv($page, $divspec);
				$d = TMRip::ripDivContents($div);
				$value = $d;

				switch ($label)
				{
				case 'Section':
				case 'Row':
				case 'Seats':
				case 'Type':
					$scrubbed = $value;
				break;
				case 'TicketFast Delivery':
					$this->ticketFast = true;
				case 'Price':
				case 'Ticket Price':
				case 'Facility Charge':
				case 'Convenience Charge':
				case 'Tickets/Items':
				case 'Order Processing Fee':
					list ($scrubbed, $c) = $this->ripCurrency($this->ripPerTicketValue($value));
				break;

				case 'SUBTOTAL':
				case 'TOTAL CHARGES':
					list ($scrubbed, $c) = $this->ripCurrency($value);
				default:
					break;
				}

				$details[$label] = $scrubbed;
			}
		}

		$this->ticketDetails[0] = $details;

	}

	// ---------------------------------------------------------------------
	// Extract all links from a page.
	// Each link is an array of href, text.
	// ---------------------------------------------------------------------
	function ripLinks($page)
	{
		$re = '/<a\s+.*?href=[\"\']?([^\"\' >]*)[\"\']?[^>]*>(.*?)<\/a>/i';
		$a = array();

		if (preg_match_all($re, $page, $matches, PREG_SET_ORDER))
		{
			foreach ($matches as $match)
			{
				// Make sure they are the right kind of link.
				// NB: This has been modified to ignore 'Print My Tickets' links.
				// This should be up-to-date to only accept valid order links.
				// (Positive match versus negative filtering)
				if (strstr($match[1], 'member') &&
					!strstr($match[0], 'Print My Tickets'))
					array_push($a, array($match[1], $match[2]));
			}
		}
		return $a;
	}

	// --------------------------------------------------------------------
	// Find a button-like div with the link for printing tickets.
	// --------------------------------------------------------------------
	function ripPrintButtonDivLink($page)
	{
		$re = '/<div\s+class=\'button\'([^>]*)>(.*?)<\/div>/i';

		if (preg_match_all($re, $page, $all_matches))
		{
			tm_log('Matched print div elements.', true);

			// We only want the attribute lists.
			foreach ($all_matches[1] as $match)
			{
				tm_log('Checking div: ' . $match, true);

				// Find (hidden) element where attribute 'name' equals 'v'.
				$re = '/onclick=[\'\"]window.open\([\'\"]([^\'\"\)]*)[\'\"]/i';
				if (preg_match($re, $match, $matches))
				{
					tm_log('Matched onclick handler.', true);
					tm_log($matches[1], true);

					return $matches[1];
				}
			}
		}
		else
		{
			tm_log('No matches for print button div.');
			echo 'No matches for print button div.';
		}

		tm_log('ripPrintButtonDivLink failed, checking for messages.');
		TMRip::ripAnnoyingMessages($page);

		return false;
	}

	// ---------------------------------------------------------------------
	// Show all links in an array.
	// Each link is an array of href, text.
	// ---------------------------------------------------------------------
	function showLinks($links)
	{
		foreach ($links as $link)
		{
			echo 'Event code: ' . $link[1] . '<br>';
		}
	}

	// ---------------------------------------------------------------------
	// Rip order info (including links) from table.
	// ---------------------------------------------------------------------
	function ripOrders($table)
	{
		$rows = TMRip::ripTableItems($table, 'tr');

		if (!$rows)
			return false;

		// Each row represents one order,
		// and contains a link to the order details page.
		$orders = array();
		foreach ($rows as $row)
		{
			if (preg_match('/<th([^>]*)>.*?<\/th>/i', $row))
			{
				// Skip bold heading row.

				//echo 'found header row<br>';
			}
			else
			{
				//echo 'Order row: ' . $row . '<br>';
				//tm_log('Order row: ' . $row);

				$orderLink = false;

				$orderDate  = TMRip::ripTableItem($row, 'td', 0);
				$orderCell  = TMRip::ripTableItem($row, 'td', 1);
				$eventCell  = TMRip::ripTableItem($row, 'td', 2);
				$eventDate  = TMRip::ripTableItem($row, 'td', 3);

				$re = '/<a\s+.*?href=[\"\']?([^\"\' >]*)[\"\']?[^>]*>(.*?)<\/a>/i';
				if (preg_match($re, $orderCell, $matches))
					$orderLink = $matches[1];

				if ($orderLink)
				{
					$orderNum   = TMRip::ripTableItem($orderCell, 'a', 0);
					$eventName  = TMRip::ripTableItem($eventCell, 'strong', 0);

					array_push($orders, array($orderLink,
											  $orderNum,
											  $orderDate,
											  $eventDate,
											  $eventName));
				}
			}
		}

		return $orders;
	}

	// ---------------------------------------------------------------------
	// Check if we get messages that could interfere with our evil purposes.
	// Return true if nothing major happened, but false if there was a
	// non-trivial message found in the page.
	// ---------------------------------------------------------------------
	function ripAnnoyingMessages($page, $obj=false)
	{
		// TRUE - condition recognized, continue, we'll retry.
		// FALSE - unhandled or fatal error.

		// 
		// First check the 'message text' span in the page.
		// This is the message that would appear to a user.
		// 
		$message_text = TMRip::rip_message_text($page);

		$re = '/(cannot\s+print|unavailable)/i';

		if (preg_match($re, $message_text, $matches))
		{
			tm_log('Annoying message text: "' . $message_text . '"');

			return false;
		}

		$re = '/(cannot\s+print|sorry|problem|processing|unavailable)/i';

		if (preg_match($re, $message_text, $matches))
		{
			tm_log('Annoying message text: "' . $message_text . '"');

			return true;
		}

		// This is usually an indicator we have been kicked off.
		$re = '/(please note)/i';

		if (preg_match($re, $message_text, $matches))
		{
			tm_log('RUDE Annoying message text: "' . $message_text . '"');
		}
		else
		{
			tm_log('Nothing annoying in message text.', true);
		}

		// 
		// Now we check the entire page content for other text.
		// 
		$re = '/<meta(?:[^>]*)content=.system.error/i';

		if (preg_match($re, $page, $matches))
		{
			tm_log('Annoying message: "' . $matches[1] . '"');

			return true;
		}

		$re = '/(problem|unable)/i';

		if (preg_match($re, $page, $matches))
		{
			tm_log('Annoying message: "' . $matches[1] . '"');

			return true;
		}

		$re = '/(maintenance)/i';

		if (preg_match($re, $page, $matches))
		{
			tm_log('Ticketmaster in maintenance mode.');
			die('Ticketmaster in maintenance mode.');

			return false;
		}

		// Need to pick most unique words for this match.
		$re = '/(web page requests|fair access)/i';

		if (preg_match($re, $page, $matches))
		{
			tm_log('Annoying message: "' . $matches[1] . '"');

			tm_log('Ticketmaster has disabled us for a bit.  Try again later.');
			die('Ticketmaster has disabled us for a bit.  Try again later.');

			return false;
		}

		//$re = '/<h1>login<\/h1>/i';
		$re = '/create\s+account/i';

		if (preg_match($re, $page, $matches))
		{
			tm_log('Annoying message: "' . $matches[0] . '"');
			tm_log('Re-login forced.');

			// Let's try ignoring this and re-getting the page.
			//return true;

			//tm_log('Need to finish re-login function.');
			//die('Need to finish re-login function.');

			if ($obj && $obj->loggedIn)
			{
//					$obj->logout();

				$obj->loggedIn = false;
				$obj->login($obj->user, $obj->pass, $page, '/member/order_history');

				return true;
			}

			return false;
		}

		tm_log('No matching annoying message?');

		return false;
	}

	// --------------------------------------------------------------------
	// This error code might be of interest later.
	// 3: Problem: contact customer service
	// 14: Processing
	// --------------------------------------------------------------------
	function ripTfBits($page)
	{
		// <!-- Ticketfast Error Bits: N -->
		$re = '/<!--\s+Ticketfast\s+Error\s+Bits:\s+(\d*)\s+-->/';

		if (preg_match($re, $page, $matches))
		{
			$this->tfBits = $matches[1];
			tm_log('Ticketfast bits: ' . $this->tbBits);

			return true;
		}

		return false;
	}

	// ---------------------------------------------------------------------
	// Pending orders are shown as 'messageText' class,
	// so extract them if they are present.
	// ---------------------------------------------------------------------
	function rip_message_text($page)
	{
		// Modified to handle non-stripped html content
		// because if we expect pdf content, it is not stripped.
		$re = '/<(?:div|span)\s+class="messagetext">(.*?)<\/(?:div|span)>/i';

		if (preg_match($re, $page, $matches))
		{
			//tm_log($matches[0]);

			return $matches[1];
		}

		return false;
	}

	// ---------------------------------------------------------------------
	// Extract message text and save it for later CSV export.
	// ---------------------------------------------------------------------
	function ripMessageText($page)
	{
		$this->status = TMRip::scrubCSVText(TMRip::rip_message_text($page));
	}

	// ---------------------------------------------------------------------
	// Check if tickets are ready for pickup.
	// ---------------------------------------------------------------------
	function ripReady($page)
	{
		$re = '/<h4>([^<]*)ready for pick up([^<]*)<\/h4>/';

		if ($this->readyForPickup = preg_match($re, $page))
			$this->status = 'Ready for pickup!';

		return $this->readyForPickup;
	}

	// ---------------------------------------------------------------------
	// Rip nth element $NUM of type $TAG inside $CONTAINER.
	// ---------------------------------------------------------------------
	function ripTableItem($container, $tag, $num)
	{
		$re = '/<' . $tag . '(?:[^>]*)>(.*?)<\/' . $tag . '>/i';

		if (preg_match_all($re, $container, $all_matches))
		{
			return $all_matches[1][$num];
		}
		else
		{
			return false;
		}
	}

	// ---------------------------------------------------------------------
	// Rip all elements of type $TAG inside $CONTAINER.
	// ---------------------------------------------------------------------
	function ripTableItems($container, $tag)
	{
		$re = '/<' . $tag . '(?:[^>]*)>(.*?)<\/' . $tag . '>/i';

		if (preg_match_all($re, $container, $all_matches))
		{
			return $all_matches[1];
		}
		else
		{
			return false;
		}
	}

	// ---------------------------------------------------------------------
	// Strip non-csv chars from status text.
	// ---------------------------------------------------------------------
	function scrubCSVText($text)
	{
		$re = '/[,\'"]/';
		return preg_replace($re, '', $text);
	}

	// ---------------------------------------------------------------------
	// Extend date year to four digits.
	// ---------------------------------------------------------------------
	function extendYear($date)
	{
		$re = '/(\d+\/\d+\/)(\d+)/';
		return preg_replace($re, '${1}20${2}', $date);
	}

	// ---------------------------------------------------------------------
	// Write the column names for csv data.
	// ---------------------------------------------------------------------
	function writeCSVHeader($handle)
	{
		$header = 
			'Order date, ' .
			'Order #, ' .
			'Order page, ' .
			'Event name, ' .
			'Event date, ' .
			'Event time, ' .
			'Venue, ' .
			'City, ' .
			'State, ' .
			'Quantity, ' .
			'Section, Row, Type, Price, Bld Chg, Conv Chg, Description, ' .
			'Seats from, ' .
			'Seats thru, ' .
			'Currency, ' .
			'Amount, ' .
			'Total charges, ' .
			'Cost of ticket, ' .
			'Card holder, ' .
			'Card type, Last 4 digits, Billing address, ' .
			'Account, ' .
			'Got tix today, ' .
			'Tix fetched, ' .
			'Tix type, ' .
			'Status' .
			"\n";

		fwrite($handle, $header);
	}

	// ---------------------------------------------------------------------
	// Save a new order.  Use separate entries for each group of seats.
	// ---------------------------------------------------------------------
	function insertOrder($account, $pagenum, $seatGroup)
	{
		tm_log("XXX Inserting Order for: " .
			   "$account, {$this->orderNumber}, $seatGroup");

		$details = $this->ticketDetails[$seatGroup];

		$unixtimeOrder = strtotime($this->orderDate);
		$isotimeOrder = strftime('%Y-%m-%d', $unixtimeOrder);
		$unixtimeEvent = strtotime($this->eventDate);
		$isotimeEvent = strftime('%Y-%m-%d', $unixtimeEvent);

		if (0)
		{
			tm_log("ORDER DATE: " . $riptm->orderDate . "\n");
			tm_log("ORDER DATE TIMESTAMP: " . $unixtime . "\n");
			tm_log("ORDER DATE ISO: " . $isotimeOrder . "\n");
		}

		$sql  =
			"INSERT into tickets " .
			"(" .
			"compare_status," .
			"received_printed," .
			"listed," .
			"comments," .
			"order_date," .
			"order_num," .
			"order_page," .
			"event_name," .
			"event_date," .
			"event_time," .
			"venue," .
			"city," .
			'state,' .
			'currency, ' .
			'amount, ' .
			'seat_group, ' .
			'seats_from, ' .
			'seats_thru, ' .
			'section, ' .
			'row, ' .
			'ticket_type, ' .
			'ticketfast, ' .
			'price, ' .
			'bld_chg , ' .
			'conv_chg , ' .
			'description , ' .
			'quantity , ' .
			'total_charges, ' .
			'card_owner , ' .
			'card_type , ' .
			'card_last4, ' .
			'billing_addr, ' .
			'account_name, ' .
			'got_tix_today, ' .
			'fetch_date , ' .
			'tm_status' .
			") VALUES (" .
			"'NEW', " .	// compare_status
			"FALSE, FALSE, " .	// ticket status = received_printed, listed
			"'- ', " .
			"'$isotimeOrder'," .
			"'$this->orderNumber'," .
			"$pagenum," .
			"'$this->eventName'," .
			"'$isotimeEvent'," .
			"'$this->eventTime'," .
			"'$this->venue'," .
			"'$this->city'," .
			"'$this->state'," .
			"'$this->orderCurrency'," .
			"$this->orderAmount," .
			"$seatGroup," .
			"'{$this->seats[$seatGroup]['from']}'," .
			"'{$this->seats[$seatGroup]['thru']}'," .
			"'{$details['Section']}'," .
			"'{$details['Row']}'," .
			"'{$details['Type']}'," .
			($this->ticketFast ? 'TRUE, ' : 'FALSE, ') .
			"{$details['Ticket Price']}," .
			"{$details['Facility Charge']}," .
			"{$details['Convenience Charge']}," .
			"'{$details['description']}'," .
			"'{$this->ticketQuantity}'," .
			"{$this->totalCharges}," .
			"'{$this->customerName}'," .
			"'{$this->cardType}'," .
			"{$this->last4Digits}," .
			"'{$this->billingAddress}'," .
			"'$account'," .
			"''," .
			"'{$this->ticketsFetchDate}'," .
			"'{$this->status}'" .
			")";

		tm_log("SQL here: $sql\n");
		$rows = mysql_query($sql);

		if ($rows === false)
			tm_log("Possible error?\n");

		if (mysql_errno() > 0)
			tm_log("Error: " . mysql_error());
	}

	// ---------------------------------------------------------------------
	// Update saved order status and $PAGENUM.
	// ---------------------------------------------------------------------
	function updateOrder($account, $pagenum, $seatGroup)
	{
		tm_log("XXX Updating Order for: " .
			   "$account, {$this->orderNumber}, $seatGroup");

		tm_log('Checking order for updates: ' . $this->orderNumber);

		// Compare existing order info, and flag 'Changed' if needed.
		// This should only apply to tm_status and total_charges.
		$orderDetail = Orders::getOrderDetail($this->orderNumber, $seatGroup);
		tm_log('Current status: ' . $this->status);
		tm_log('Saved status: ' . $orderDetail['tm_status']);
		tm_log('Current total charges: ' . $this->totalCharges);
		tm_log('Saved total charges: ' . $orderDetail['total_charges']);
/*
  foreach ($orderDetail as $key => $value)
  tm_log("Order detail: '$key' - '$value'");
*/

		$orderChanged = false;

		if ($this->status != $orderDetail['tm_status'] ||
			$this->totalCharges != $orderDetail['total_charges'])
		{
			$orderChanged = true;
			tm_log('Changed!');
		}
		else
		{
			tm_log('No Change.');
		}

		// As new orders are added, the page number where this order was found
		// could change, so update it for easier location later on.
		$sql  =
			"UPDATE tickets SET " .
			"order_page = $pagenum, ";

		if ($orderChanged === true)
		{
			// Rotate the tm_status columns.
			$sql .=
				"compare_status = 'CHANGED', " .
				"tm_status = '{$this->status}', " .
				"tm_status_p = '" . $orderDetail['tm_status'] . "', " .
				"tm_status_o = '" . $orderDetail['tm_status_p'] . "'";
		}
		else
		{
			$sql .= "compare_status = 'UNCHANGED'";
		}

		$sql .=
			" WHERE " .
			"order_num LIKE '{$this->orderNumber}' " .
			"AND seat_group = $seatGroup " .
			"AND compare_status != 'PAST' ";

		tm_log("SQL: $sql\n");
		$rows = mysql_query($sql);

		if (mysql_errno() > 0)
			tm_log("Error: " . mysql_error());
	}

	// ---------------------------------------------------------------------
	// Save one set of order info to db.
	// ---------------------------------------------------------------------
	function saveToDB($account, $pagenum)
	{
		$existingOrder = Orders::checkExisting($this->orderNumber);
		tm_log('Existing? ' . ($existingOrder ? 'Y' : 'N'));

		// Each order can have more than one group of seats,
		// so iterate the arrays of ticket details.
		for ($seatGroup = 0; $seatGroup < count($this->ticketDetails); $seatGroup++)
		{
			// This is a hack!
			// Using $seatGroup is poor style.
			if ($existingOrder)
				$this->updateOrder($account, $pagenum, $seatGroup);
			else
				$this->insertOrder($account, $pagenum, $seatGroup);
		}
	}

	// ---------------------------------------------------------------------
	// Mark all orders with tix from this account as not getting tix today.
	// ---------------------------------------------------------------------
	function clearTixOrders($accountName)
	{
		$sql = 
			"UPDATE tickets SET " .
			"got_tix_today = 'N' " .
			"WHERE " .
			"got_tix_today = 'Y' ";
		"AND account_name LIKE '$accountName'";

		tm_log("SQL: $sql");
		$rows = mysql_query($sql);

		if (mysql_errno() > 0)
			tm_log("Error: " . mysql_error());
	}

	// ---------------------------------------------------------------------
	// Mark all orders from this account with a '-' compare status.
	// ---------------------------------------------------------------------
	function blankOrders($accountName)
	{
		$sql = 
			"UPDATE tickets SET " .
			"compare_status = '-' " .
			"WHERE " .
			"account_name LIKE '$accountName'";

		tm_log("SQL: $sql");
		$rows = mysql_query($sql);

		if (mysql_errno() > 0)
			tm_log("Error: " . mysql_error());
	}

	// ---------------------------------------------------------------------
	// Mark as 'PAST' all events before today.
	// ---------------------------------------------------------------------
	function pastOrders()
	{
		$sql =
			"UPDATE tickets SET " .
			"compare_status = 'PAST' " .
			"WHERE event_date < now()";

		tm_log("SQL: $sql");
		$rows = mysql_query($sql);

		if (mysql_errno() > 0)
			tm_log("Error: " . mysql_error());
	}

	// ---------------------------------------------------------------------
	// Rip a currency amount into a currency symbol and a number.
	// ---------------------------------------------------------------------
	function ripCurrency($data)
	{
		$data = str_replace('&nbsp;', ' ', $data);

		// Parse amount to split currency type from amount.
		$re = '/(\S+)\s*(?:.+?)([0-9,.]*)/';
		$currency = '';
		$amount = '';

		if (preg_match($re, $data, $matches))
		{
			$currency = $matches[1];
			$amount = $matches[2];

			tm_log("XXX: $data = |" . $currency . '| - |' . $amount . '|', true);

			// Ticketmaster puts commas in total.
			$amount = TMRip::scrubCSVText($amount);
			tm_log("XXX: $data = |" . $currency . '| - |' . $amount . '|', true);

			$amount = ($amount == '') ? 0 : $amount;
			tm_log("XXX: $data = |" . $currency . '| - |' . $amount . '|', true);
		}
		else
		{
			tm_log('Currency did not fit template: ' . $data);
			$amount = 0;
			$currency = 'US';
		}

		return array($amount, $currency);
	}

	function ripPerTicketValue($data)
	{
		if ($this->ticketQuantity == 0)
			$this->ticketQuantity = $this->ripTicketQuantity($data);

		$re = '/\S+\s*(?:.+?)([0-9,.]*)/';

		if (preg_match($re, $data, $matches))
			return $matches[0];
		else
			return false;

	}

	function ripTicketQuantity($data)
	{
		$re = '/x\s+([1-9]+)/';

		if (preg_match($re, $data, $matches))
			return $matches[1];
		else
			return false;

	}

	// ---------------------------------------------------------------------
	// Determine table type.
	// ---------------------------------------------------------------------
	function ripTableRole($table)
	{
		$row = TMRip::ripTableItem($table, 'tr', 0);
		if (preg_match('/order date.*order #.*amount/i', $row))
			return TMRIP_TABLE_PURCHASE;
		else if (preg_match('/event/i', $row))
			return TMRIP_TABLE_EVENT;
		else if (preg_match('/section.*row.*ticket\s+price/i', $row))
			return TMRIP_TABLE_TICKETS;
		else if (preg_match('/item.*charge/i', $row))
			return TMRIP_TABLE_CHARGE;
		else if (preg_match('/card\s+type.*charge.*last\s+4\s+digits/i', $row))
			return TMRIP_TABLE_BILLING;

		return TMRIP_TABLE_NONE;
	}

	// ---------------------------------------------------------------------
	// Extract data from each table type.
	// ---------------------------------------------------------------------
	function ripTableData($table, $table_role)
	{
		switch($table_role)
		{
		case TMRIP_TABLE_NONE:
			die('Rip - Bad table role: ' . $table_role . '<br>');
			break;

		case TMRIP_TABLE_PURCHASE:
			$row = TMRip::ripTableItem($table, 'tr', 1);
			$this->orderDate = TMRip::ripTableItem($row, 'td', 0);
			$this->orderDate = TMRip::extendYear($this->orderDate);
			$this->orderNumber = TMRip::ripTableItem($row, 'td', 1);
			$this->orderAmount = TMRip::ripTableItem($row, 'td', 2);

			list($this->orderAmount,
				 $this->orderCurrency) = $this->ripCurrency($this->orderAmount);
			break;

			// XXX This is new version, where 3 tables are combined into 1.
		case TMRIP_TABLE_EVENT:
			$row = TMRip::ripTableItem($table, 'tr', 0);
			$this->eventName = TMRip::ripTableItem($row, 'h3', 0);
			$this->eventName = TMRip::scrubCSVText($this->eventName);

			// Save the 'td' raw contents.
			$this->eventDetails = TMRip::ripTableItem($row, 'td', 0);
			tm_log('Details: ' . $this->eventDetails, true);

			// Extract bold text from eventDetails.
			$re = '/<h3>(?:[^<]*)<\/h3>(.*)/i';

			if (preg_match($re, $this->eventDetails, $matches))
			{
				$this->eventDetails = $matches[1];

				// Extract event date, venue, city, state,
				// day, date, time.
				$re = '/(?P<venue>.*?),&nbsp;
							(?P<city>[^,]*),&nbsp;
							(?P<state>[^,]*)<br>
							(?P<day>[^,]*),\s*
							(?P<date>[^,]*),\s*
							(?P<time>.*)/x';

				if (preg_match($re, $this->eventDetails, $matches))
				{
					$this->venue = $matches["venue"];
					$this->city = $matches["city"];
					$this->state = $matches["state"];
					$this->eventDate = $matches["date"];

					$this->venue = TMRip::scrubCSVText($this->venue);
					$this->eventDate = TMRip::extendYear($this->eventDate);
				}
				else
				{
					// Probably canceled.
					// Extract event date, venue, city, state.
					$re = '/(?P<venue>.*?),&nbsp;
							(?P<city>[^,]*),&nbsp;
							(?P<state>[^,]*)<br>
							(?P<other>.*)/x';

					if (preg_match($re, $this->eventDetails, $matches))
					{
						$this->venue = $matches["venue"];
						$this->city = $matches["city"];
						$this->state = $matches["state"];

						$this->venue = TMRip::scrubCSVText($this->venue);
					}
				}
			}
			else
			{
				echo $this->eventName;
				echo $this->eventDetails;

				// Table structure has changed!!!
				die('No outer (h3) details match!');
			}

			// Extract event time.
			$re = '/(\d+:\d+\s+(A|P)M)/';

			if (preg_match($re, $this->eventDetails, $matches))
			{
				$this->eventTime = $matches[1];
				tm_log('Event time: ' . $this->eventTime, true);
			}
			else
			{
				tm_log('No event time found: ' . $this->eventDetails);
			}

			break;

		case TMRIP_TABLE_BILLING:

			// XXX Iterate all rows until we find the card that worked.
			for ($i = 1, $row = true; $row; $i++)
			{
				// XXX New structure here.
				$row = TMRip::ripTableItem($table, 'tr', $i);
					
				if ($row)
				{
					$this->customerName = TMRip::ripTableItem($row, 'td', 0);
					$this->cardType = TMRip::ripTableItem($row, 'td', 1);
					$this->totalCharges = TMRip::ripTableItem($row, 'td', 2);

					list($this->totalCharges, $temp) =
						$this->ripCurrency($this->totalCharges);

					$this->last4Digits = TMRip::ripTableItem($row, 'td', 3);
					$this->billingAddress = TMRip::ripTableItem($row, 'td', 4);
				}
			}
			break;

		default:
			die('Unhandled table role.<br>');
			break;
		}
	}
}
