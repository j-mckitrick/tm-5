<?php

// Rip data from ticketmaster web pages and export as csv.

require_once 'globals.inc';
require_once 'logging.inc';
require_once 'orders.inc';
require_once 'tmcurl.inc';
require_once 'tmrip.inc';

define('TMDATA_FAIL',	0x0);
define('TMDATA_PASS',	0x1);
define('TMDATA_SKIP',	0x2);

class TMData
{
	var $user = false;
	var $pass = false;

	var $status = false;
	var $csvFile = false;
	var $loggedIn = false;

	var $queryPages = false;
	var $queryDates = false;
	var $orderDates = false;
	var $eventDates = false;
	var $doneDates = false;
	var $saveTickets = false;
	var $saveOldTickets = false;
	var $skipFailed = false;
	var $cliMode = false;

	var $startDay = false;
	var $startMonth = false;
	var $startYear = false;
	var $startDate = false;

	var $endDay = false;
	var $endMonth = false;
	var $endYear = false;
	var $endDate = false;

	var $startPage = 0;
	var $maxOrders = 10;
	var $maxPages = 99;

	var $pageCount = 0;	// number of pages for current account
	var $ticketNumber = 0;
	
	// Stats to report
	var $totalPages = 0;
	var $totalOrders = 0;
	var $ticketsSaved = 0;
	var $skippedOrders = 0;
	var $expectedOrders = 0;

	var $orderRetries = array();

	function getDelay()
	{
		return ($this->cliMode ? 4 : 4);
	}

	function getNewline()
	{
		return ($this->cliMode ? "\n" : "<br>");
	}

	// ---------------------------------------------------------------------
	// Get secure tm page after a delay, save it, return (stripped) html.
	// ---------------------------------------------------------------------
	function getPage($url,		       // page to get
					 $sec,		       // seconds delay
					 $name,		       // name of page
					 $clear_cookies=false, // start new cookie session
					 $strip_content=true)  // remove CR/LF
	{
		$filename = $GLOBALS['tmpdir'] . 'tm_saved_' . $name . '.html';

		if ($sec > 0)
		{
			tm_log('Sleep ' . $sec . ' second(s) for ' . $name . ' content.');
			sleep($sec * 2);
		}

		$status_string = 'Getting ' . $name . ' page.';
		if ($clear_cookies) $status_string .= '  Clear cookies.';
		if ($strip_content) $status_string .= '  Strip page content.';
		tm_log($status_string);

		$page = TMCurl::getSecurePage($url, $clear_cookies);
		$filename = TMRip::savePage($page, $name);

		tm_log('Got ' . $name . ' page: ' . $filename, true);

		if ($strip_content)
		{
			tm_log('Returning stripped content.', true);

			return TMRip::stripHTMLFile($filename);
		}
		else
		{
			tm_log('Returning unstripped content.', true);

			return $page;
		}
	}

	// --------------------------------------------------------------------
	// Login to a ticketmaster account.
	// --------------------------------------------------------------------
	function login($user, $pass, $page=false, $url=false)
	{
		$numtries = 0;
		$maxtries = 12;	// 12
		$done = false;

		do
		{
			if ($numtries > 0)
			{
				tm_log("\n" . '*** Retrying login.');
				tm_log('Tries so far: ' . $numtries);
				tm_log('Sleeping a bit.');

				sleep(5);	// 2, 5
			}

			unlink($GLOBALS['cookie_jar']);

			tm_log('Getting login seed.');

			// Get login page.
			if (!$page)
				$page = $this->getPage("/member?", $this->getDelay(), "login",
									   true, true);

			$seed = TMRip::ripSeed($page);

			if ($seed)
			{
				tm_log('Got seed, logging in.');

				$msg = TMCurl::postLogin($seed, $user, $pass, $url);

				if ($msg)
				{
					tm_log('Login result: ' . $msg);
				}
				else
				{
					tm_log('Login successful.');

					$done = true;
					$this->loggedIn = true;
				}
			}
			else
			{
				tm_log('Failed to get seed.');

				die('Failed to get login seed.<br>Aborting.');
			}

			$numtries++;

			if ($numtries > $maxtries &&!$this->skipFailed)
			{
				$done = true;

				//die('Login failed.<br>Aborting.');
				echo 'Login failed.  Skipping account.';
				echo $this->getNewline();
			}

		} while (!$done);
	}

	// --------------------------------------------------------------------
	// Logout of ticketmaster.
	// --------------------------------------------------------------------
	function logout()
	{
		$numtries = 0;
		$maxtries = 3;
		$done = false;

		do
		{
			if ($numtries > 0)
			{
				tm_log("\n" . '*** Retrying logout.');
				tm_log('Tries so far: ' . $numtries);
			}

			// Get logout page.
			$page = $this->getPage("/member/logout", $this->getDelay(),
								   "logout", true);

			if ($page)
			{
				$done = true;
				$this->loggedIn = false;

				unlink($GLOBALS['cookie_jar']);

				tm_log('Logout succeeded.');

//					tm_log('Sleeping a bit.');
//					sleep(10);
			}
			else
			{
				$numtries++;

				tm_log('Logout failed.');
			}

			if ($numtries > $maxtries)
			{
				die('Logout timed out.  Aborting.');
			}

		} while (!$done);
	}

	// ---------------------------------------------------------------------
	// Check that a page of content matches a regex.
	// If not, save to a file and log the error.
	// Returns:
	// TMDATA_PASS - page validated successfully
	// TMDATA_FAIL - page failed to validate
	// TMDATA_SKIP - page failed to validate, but for unknown reason
	// ---------------------------------------------------------------------
	function validateContentPage($page, $re, $name='')
	{
		if (!$page)
			return TMDATA_FAIL;

		if (preg_match($re, $page, $matches))
		{
			return TMDATA_PASS;
		}
		else
		{
			tm_log('* Validate failed.  No ' . $re . ' found in page.');

			$bad_name = 'delayed_' . $name . '_page_BAD';
			TMRip::savePage($page, $bad_name);

			if (!TMRip::ripAnnoyingMessages($page, $this))
			{
				// If we fail, check post-mortem file to find out why.
				// Then add this condition to 'annoying message' handler.
				tm_log('Validate failed.  Check ' . $bad_name);

				$httpError = TMCurl::checkForError($page);
				if ($httpError)
					tm_log($httpError);

//					if ($this->cliMode)
//						die('Validate failed.  Check ' . $bad_name);

				return TMDATA_SKIP;
			}

			return TMDATA_FAIL;
		}

		return TMDATA_FAIL;
	}

	// ---------------------------------------------------------------------
	// Validate this page as a refresh page.
	// Return TRUE on success, FALSE on failure.
	// ---------------------------------------------------------------------
	function validateRefreshPage($or_page, $page_name)
	{
		$re = '/<meta http-equiv="refresh" (?:[^>]*)>/i';
		$metatag = '';

		if (preg_match($re, $or_page, $matches))
		{
			tm_log('Found refresh meta tag match: ' . $matches[0], true);
			tm_log('Found refresh meta tag.');

			$metatag = $matches[0];
		}
		else
		{
			tm_log('Not a refresh page, probably real content.', true);

//				TMRip::savePage($or_page, $page_name . '_BAD');
	    
//				TMRip::ripAnnoyingMessages($or_page);

			return false;
		}

		tm_log("\n" . '"Refresh" meta tag: ' . $metatag . "\n", true);

		if ($metatag == '')
		{
			tm_log('* No refresh meta tag found.');
			//tm_log('* No refresh meta tag found in: ' . $refreshurl);

			//TMRip::ripAnnoyingMessages($or_page);

			return false;
		}

		return $metatag;
	}

	// ---------------------------------------------------------------------
	// Extract and fetch content url from refresh page content.
	// ---------------------------------------------------------------------
	function getRealContentFromRefresh($or_page, $page_name, $stripped=true)
	{
		tm_log('> getRealContentFromRefresh', true);
		tm_log('Extracting url from refresh page.', true);

		// Get the actual details page url from 'refresh' META tag.
		if ($metatag = $this->validateRefreshPage($or_page, $page_name))
		{
			// Check for 'content' tag.
			$re_tag_attr = '/content="([^"]*)"/i';

			if (preg_match($re_tag_attr, $metatag, $matches))
			{
				$metacontent = $matches[1];
		    
				tm_log('"Refresh" content tag: ' . $metacontent . "\n", true);

				// Extract the url and refresh interval.
				preg_match('/^(\d);url=(.*)/', $metacontent, $matches);
				$refreshsec = ($matches[1] == '0') ? 1 : $matches[1];
				$orderurl = $matches[2];
	
				tm_log('Actual content url: ' . $orderurl . "\n", true);
				tm_log('Seconds for content : ' . $refreshsec . "\n", true);

				// Get the actual order details page with the data we want.
				if ($stripped)
				{
					$od_page = $this->getPage($orderurl, $refreshsec,
											  $page_name, false, true);

					tm_log('Got real stripped data, set done.', true);
				}
				else
				{
					$od_page = $this->getPage($orderurl, $refreshsec,
											  $page_name, false, false);

					tm_log('Got real data, set done.', true);
				}

				// Recurse until we do not find 'refresh' tag.
				return $this->getRealContentFromRefresh($od_page, $page_name,
														$stripped);
			}
			else
			{
				tm_log('* No content url found in refresh meta tag link.'); 
				tm_log('refresh url: ' . $refreshurl);
				tm_log('event name: ' . $eventname);
				tm_log('Meta: ' . $metatag);

				// Is there a message in the page that will explain
				// why we failed?
				//TMRip::ripAnnoyingMessages($or_page);
				TMRip::savePage($or_page, 'refresh_BAD');

				if (isset($GLOBALS['debugr']))
					echo 'Page: ' . $or_page . '<br>';
			}
		}

		tm_log('< getRealContentFromRefresh', true);

		// Not a valid refresh page, so return content unchanged.
		return $or_page;
	}

	// ---------------------------------------------------------------------
	// Extract and fetch content url from refresh page content.
	// ---------------------------------------------------------------------
	function pollRealContentFromRefresh($or_page, $page_name, $stripped=true)
	{
		$numtries = 0;
		$maxtries = 10;
		$done = false;
		$od_page = false;

		tm_log('> pollRealContentFromRefresh', true);

		do
		{
			if ($numtries > 0)
			{
				tm_log("\n" . '*** Retrying refresh ' . $eventname . '...');
				tm_log('Tries so far: ' . $numtries);
			}

			$od_page = $this->getRealContentFromRefresh($or_page, $page_name,
														$stripped);

			tm_log('Got real content?  Testing.', true);

			if ($od_page)
			{
				tm_log('Got real content, should be done.', true);
				$done = true;
			}
			else
			{
				tm_log('Did not get real content, NOT done.', true);
				$numtries++;

				if ($numtries > $maxtries)
				{
					tm_log('Did not get real content, done by timeout.', true);
					$done = true;
				}
			}

		} while (!$done);

		tm_log('Done fetching real contents from refresh page.');
		tm_log('< pollRealContentFromRefresh', true);

		return $od_page;
	}

	// ---------------------------------------------------------------------
	// Get the ticket download link from a page and save the resulting pdf.
	// ---------------------------------------------------------------------
	function saveTicketPDF($od_page, $riptm)
	{
		$url = TMRip::ripPrintButtonDivLink($od_page);

		tm_log('Ready to print: ' . $url, true);

		$numtries = 0;
		$maxtries = 3;
		$done = false;
		$success = false;

		sleep($this->getDelay());	// try this to make TM happy

		do
		{
			if ($numtries > 0)
			{
				tm_log("\n" . '*** Retrying ticket download....');
				tm_log('Tries so far: ' . $numtries);
			}

			$refresh_page = $this->getPage($url, $this->getDelay(),
										   'pdf_refresh', false, true);
			$pdf = $this->pollRealContentFromRefresh($refresh_page, 'pdf', false);
			$pdf_stripped = preg_replace('/(\r|\n)/', '', $pdf);

			$result = $this->validateContentPage($pdf_stripped, '/PDF-1/', 'pdf');
		
			switch ($result)
			{
			case TMDATA_PASS:
				$done = true;
				$success = true;

				// Build a name for the file with all the info we want displayed.
				// Danielle changes this often.
				$pdf_name  = substr($riptm->eventName, 0, 20) . '-';
				$pdf_name .= preg_replace('/\//', '-', $riptm->eventDate) . '-';
				$pdf_name .= substr($riptm->venue, 0, 20) . '-';
				$pdf_name .= preg_replace('/\//', '-', $riptm->orderNumber) . '-';
				$pdf_name .= $riptm->ticketDetails[0]['section'] . '-';
				$pdf_name .= $riptm->ticketDetails[0]['row'] . '-';
				$pdf_name .= $riptm->seats[0]['from'] . '-';
				$pdf_name .= $riptm->seats[0]['thru'];

				tm_log('Section: ' . $riptm->ticketDetails[0]['section']);
				tm_log('Row: ' . $riptm->ticketDetails[0]['row']);
				tm_log('Seats from: ' . $riptm->seats[0]['from']);
				tm_log('Seats thru: ' . $riptm->seats[0]['thru']);

				TMRip::savePDF($pdf, $pdf_name);
				Orders::gotTicketPDF($riptm->orderNumber);
				tm_log('Got PDF file: ' . $pdf_name);
				break;

			case TMDATA_SKIP:
				tm_log('No valid PDF file!  SKIPPING.');
				$done = true;
				break;

			case TMDATA_FAIL:
				$numtries++;

				tm_log('No valid PDF file!');

				if ($numtries > $maxtries)
				{
					$done = true;

					tm_log('No tickets downloaded after ' . $maxtries . ' tries.');
				}
				break;
			}

		} while (!$done); 

		return $success;
	}

	// ---------------------------------------------------------------------
	// Get refresh page, extract details page url, then fetch and return it.
	// ---------------------------------------------------------------------
	function getOrderDetailsPage($refreshurl, $eventname)
	{
		$refreshsec = 0;

		tm_log('> getOrderDetailsPage', true);
		tm_log('Getting order refresh page.', true);
		tm_log('Seconds to wait: ' . $refreshsec, true);

		// Start by getting the 'please wait' refresh page ticketmaster
		// displays while they are getting the order details from their db.
		$or_page = $this->getPage($refreshurl, $refreshsec, 'order_refresh',
								  false, true);

		if (isset($GLOBALS['debugr']))
			tm_log('refresh page' . $or_page . 'end refresh page');

		$od_page = $this->pollRealContentFromRefresh($or_page, 'order_details');

		$re = '/(Order Detail|Your Ticket)/i';

		$result = $this->validateContentPage($od_page, $re, 'order_details');

		if ($result == TMDATA_PASS)
		{
			tm_log('Found valid Order Detail page.', true);
		}
		else
		{
			$od_page = false;

			tm_log('Order Detail page did NOT validate.');
		}

		tm_log('< getOrderDetailsPage', true);

		return $od_page;
	}

	// ---------------------------------------------------------------------
	// Get the refresh page, extract details page url, fetch and return it.
	// ---------------------------------------------------------------------
	function pollOrderDetailsPage($order, $retry=false)
	{
		$numtries = 0;
		$maxtries = 4;	// 4-10
		$done = false;

		list($refreshurl, $eventname) = $order;

		tm_log('> pollOrderDetailsPage', true);
		tm_log('Polling event: ' . $eventname . "\n");
		tm_log('Link: ' . $refreshurl . "\n", true);

		do
		{
			if ($numtries > 0)
			{
				tm_log("\n" . '*** Retrying ' . $eventname . '...');
				tm_log('Tries so far: ' . $numtries);
				sleep(6);	// try this to make TM happy
			}

			if ($od_page = $this->getOrderDetailsPage($refreshurl, $eventname))
			{
				$done = true;

				tm_log('Got order details page.');
			}
			else
			{
				$numtries++;

				tm_log('Did NOT get order details page!');
			}

			if ($numtries > $maxtries && !$this->skipFailed)
			{
				$od_page = false;
				$done = true;

				tm_log('* Giving up on ' . $eventname . " after $maxtries tries.");

				if (!$retry)
				{
					$this->orderRetries[] = $order;
					$this->skippedOrders++;
				}
				else
				{
					echo "* Giving up on order: $eventname";
					echo " after $numtries tries.";

					if ($this->cliMode)
						echo "\n";
					else
						echo "<br>";
				}

				//die('Refresh page took too many tries.');
			}

		} while (!$done);

		tm_log('< pollOrderDetailsPage', true);

		return $od_page;
	}

	// ---------------------------------------------------------------------
	// Get one order and export as csv data.
	// ---------------------------------------------------------------------
	function exportOrder($order,
						 $pagenum,
						 &$ordernum,
						 $account,
						 $retry=false)
	{
		tm_log('> exportOrder', true);
		tm_log("--- Begin $account page $pagenum, order number $ordernum.");
		update_dropfile("Reading $account page $pagenum, order $ordernum.");

		// When we GET an order link, we actually end up
		// at a 'waiting' page that refreshes/redirects
		// to the actual details page when it is ready.
		// Poll this page until we get order details content.
		$od_page = $this->pollOrderDetailsPage($order, $retry);

		// This is one spot where we check that page is not null,
		// because rather than just matching a regex, we call a series
		// of ripper functions, so it's easier to catch a null page here.
		if ($od_page)
		{
			tm_log('Extracting order details.');

			// Be consistent with static/dynamic member function calls.
			// Underscored function names are static,
			// and camel-cased functions are dynamic.
			$riptm = new TMRip($this->csvFile);
			$riptm->ripTfBits($od_page);
			$riptm->ripMessageText($od_page);
			$ready = $riptm->ripReady($od_page);

			// ------------------------------------------------------------
			// Iterate tables on order details page and extract info.
			// This section will need revision as ticketmaster
			// changes their order details page layout.
			// ------------------------------------------------------------
			$table_roles = array(TMRIP_TABLE_NONE,
								 TMRIP_TABLE_PURCHASE,
								 TMRIP_TABLE_BILLING);

			for ($i = 1, $table = true; $table; $i++)
			{
				$tablespec = array('class', 'detailsTable', $i);
				$table = TMRip::ripTable($od_page, $tablespec);

				if ($table)
				{
					tm_log('Got detailsTable: ' . $i, true);

					// Figure out which details table this is.
					$act_table_role = $riptm->ripTableRole($table);
					$exp_table_role = $table_roles[$i];

					// There may be more than 1 purchase, event, or tickets tables,
					// but the last 2 should always be charge and billing.
					if ($act_table_role == $exp_table_role ||
						$act_table_role == TMRIP_TABLE_CHARGE ||
						$act_table_role == TMRIP_TABLE_BILLING)
					{
						$riptm->ripTableData($table, $act_table_role);
					}
					else
					{
						echo 'Got bad details for order: ' . $order[1] . '<br>';
						tm_log('Expected table role: ' . $exp_table_role);
						tm_log('Actual table role: ' . $act_table_role);
					}
				}
			}

			$table_roles = array(TMRIP_TABLE_NONE, TMRIP_TABLE_EVENT);

			// Instead of iterating $table_roles, interate all shopList tables.
			for ($i = 1, $table = true; $table; $i++)
			{
				$tablespec = array('class', 'shopList', $i);
				$table = TMRip::ripTable($od_page, $tablespec);

				if ($table)
				{
					tm_log('Got shopList: ' . $i, true);

					// Figure out which details table this is.
					$act_table_role = $riptm->ripTableRole($table);
					$exp_table_role = $table_roles[1];

					// There may be more than 1 purchase, event, or tickets tables,
					// but the last 2 should always be charge and billing.
					if ($act_table_role == $exp_table_role ||
						$act_table_role == TMRIP_TABLE_CHARGE ||
						$act_table_role == TMRIP_TABLE_BILLING
//							|| true // force!
						)
					{
						$riptm->ripTableData($table, $act_table_role);
					}
					else
					{
						echo 'Got bad details for order: ' . $order[1] . '<br>';
						tm_log('Expected table role: ' . $exp_table_role);
						tm_log('Actual table role: ' . $act_table_role);
					}
				}
			}

			// XXX This is the new format stuff.
			$riptm->ripShipDivs($od_page);
/*
// If more than 1 table (?), extra info will be AFTER the last table.
// Instead of table, use div where style=padding-right.
for ($i = 1, $div = true; $div; $i++)
{
$divspec = array('style', 'padding-right(?:.*?)px', $i);
$div = TMRip::ripDiv($od_page, $divspec);

if ($div)
{
tm_log('Got div: ' . $i, true);

// XXX This regex should be modified.
// Ends at first closing div, not second.
$divspec2 = array('class', 'shipValueRight', 1);
$div2 = TMRip::ripDiv($row, $divspec);
$d = TMRip::ripDivContents($div2);
}
}
*/
			tm_log("Done parsing order number $ordernum details tables.");

			$riptm->saveToDB($account, $pagenum);

			tm_log('Ready: ' . $ready, true);
			tm_log('Tickets ready: ' . ($ready ? 'YES' : 'No') . '.');

			// Tickets are ready, download pdf if we don't already have it.
			if ($ready && $this->saveTickets &&
				($this->saveOldTickets || !Orders::haveTicketPDF($order[1])))
			{
				if ($this->saveTicketPDF($od_page, $riptm))
					$this->ticketsSaved++;
				else
					echo 'Failed to get tickets for order ' . $order[1] .
						$this->getNewline();
			}
			else
			{
				if (!$ready)
					tm_log('No tickets to print.');

				if (!$this->saveTickets)
					tm_log('Not saving tickets on this run.');

				if (Orders::haveTicketPDF($order[1]))
					tm_log('Already have this ticket.');
			}

			$riptm->ticketsFetchDate = Orders::haveTicketPDF($order[1]);
			$order = Orders::getOrderDetail($riptm->orderNumber);
			Orders::export($order, $this->csvFile);
			tm_log("Order data export complete.\n");

			if (!$retry)
			{
				$this->totalOrders++;
				$ordernum++;
			}
		}
		else
		{
			tm_log("\nNo details page returned from polling.");
		}

		tm_log('< exportOrder', true);
	}

	// ---------------------------------------------------------------------
	// Compare this order to any search conditions we have,
	// such as date range, artist, etc.
	// ---------------------------------------------------------------------
	function filterOrder($order)
	{
		if (!$this->queryDates)
			return true;

		$allow = true;
/*
  tm_log('Start month: ' . $this->startMonth, true);
  tm_log('Start day: ' . $this->startDay, true);
  tm_log('Start year: ' . $this->startYear, true);

  tm_log('Raw event date: ' . $order[3], true);
  tm_log('Raw order date: ' . $order[2], true);
*/
		$ed = strtotime($order[3]);
		tm_log('Event date: ' . strftime('%c', $ed), true);

		$od = strtotime($order[2]);
		tm_log('Order date: ' . strftime('%c', $od), true);

		$targetDate = false;

		if ($this->eventDates)
		{
			$targetDate = $ed;
		
			tm_log('Checking event date.');
		}
		else if ($this->orderDates)
		{
			$targetDate = $od;

			tm_log('Checking order date.');
		}
		else
		{
			die('Bad dates.'); // ;-)
		}

		tm_log('Target date: ' . strftime('%c', $targetDate));

		if ($targetDate >= $this->startDate &&
			$targetDate <= $this->endDate)
		{
			$allow = true;

			tm_log('Date in target range.');
		}
		else
		{
			$allow = false;

			tm_log('Date NOT in date range.');

			if ($this->orderDates &&
				$this->startDate > $targetDate)
			{
				tm_log('No more orders should be checked after this.');
				$this->doneDates = true;
			}
		}

		tm_log("\n");

		return $allow;
	}

	// ---------------------------------------------------------------------
	// Get one set or page of orders and export them as csv data.
	// ---------------------------------------------------------------------
	function exportOrders($orders, $pagenum, $account, $retry=false)
	{
		tm_log('> exportOrders', true);

		foreach ($orders as $order)
		{
			tm_log('Event: ' . $order[1]);
			tm_log('Link:  ' . $order[0], true);
			tm_log("\n", true);
		}
		tm_log("\n");

		// Max ten orders on a page, AFAICT.
		$ordernum = 1;

		foreach ($orders as $order)
		{
			if ($this->doneDates)
				break;

			if ($this->filterOrder($order))
			{
				$this->exportOrder($order, $pagenum, $ordernum, $account, $retry);

				if (($this->maxOrders > 0 && $ordernum > $this->maxOrders) ||
					$this->maxOrders == 1)
					break;
			}

			// XXX Not sure this makes sense, since Safari times out anyway.
			if (!$this->cliMode)
			{
				// Keep newer apache from timing out.
				echo ' ';
				flush();
			}

			if ($this->cliMode)
				sleep($this->getDelay());
		}

		tm_log('< exportOrders', true);
	}

	// ---------------------------------------------------------------------
	// Get a ticketmaster order history page containing up to 10 orders.
	// ---------------------------------------------------------------------
	function getOrderHistoryPage($pagenum)
	{
		tm_log('> getOrderHistoryPage', true);

		// Not sure which version of the link we want.
		// The first one works fine, but the second one
		// is the one found on the 'my ticketmaster' page.

		//$start = $pagenum ? "?start=$pagenum" : "";
		$start = $pagenum ? "?start=$pagenum" : "?tm_link=mytm_myacct5";
		$url = '/member/order_history' . $start;

		tm_log('Getting order_history page: ' . $pagenum);
		update_dropfile("Reading page $pagenum, order history.");

		$oh_page = $this->getPage($url, $this->getDelay() * 2,
								  'order_history', false, true);

		// Validate this page as being the one we want.
		$re = '/<h4>(Ticketmaster Order History)[^<]*<\/h4>/i';

		$result = $this->validateContentPage($oh_page, $re, 'order_history');

		if ($result == TMDATA_PASS)
		{
			tm_log('Search for "N (of M)" order count in validated page.', true);

			// Make sure this is the right kind of page to extract
			// order details links from.  There should be an order count.
			// We could even check for 'prev' and/or 'next' in this regex.
			$re_oc = '/<strong>[-0-9]+<\/strong>\s*\(of\s*([0-9]*)\)/';
		    
			if (preg_match($re_oc, $oh_page, $matches))
			{
				tm_log('Order count found: ' . $matches[1]);

				$this->expectedOrders += $matches[1];
			}
		}
		else
		{
			$oh_page = false;
		}
    
		tm_log('< getOrderHistoryPage', true);

		return $oh_page;
	}

	// --------------------------------------------------------------------
	// Get a ticketmaster order history page containing up to 10 orders.
	// --------------------------------------------------------------------
	function pollOrderHistoryPage($pagenum)
	{
		$numtries = 0;
		$maxtries = 4;
		$done = false;

		tm_log('> pollOrderHistoryPage', true);

		do
		{
			if ($numtries > 0)
			{
				tm_log('*** Retrying order_history page.');
				tm_log('Tries so far: ' . $numtries);
			}

			$oh_page = $this->getOrderHistoryPage($pagenum);
		
			if ($oh_page)
			{
				$done = true;
			}
			else
			{
				$numtries++;
			}

			if ($numtries > $maxtries)
			{
				tm_log('OK: No order count found after ' . $numtries . ' tries.');

				return 0;
			}

		} while (!$done);
    
		tm_log('< pollOrderHistoryPage', true);

		return $oh_page;
	}

	// --------------------------------------------------------------------
	// Parse and export one page of ticketmaster orders.
	// --------------------------------------------------------------------
	function exportPage($pagenum, $account)
	{
		tm_log('> exportPage', true);

		if ($this->cliMode)
			sleep($this->getDelay());

		// Get a page of orders.
		$oh_page = $this->pollOrderHistoryPage($pagenum);

		if ($oh_page)
		{
			// Once we have a valid page of orders from the order history page,
			// we extract the links that represent those orders from the table.
			tm_log("Getting order detail links table from page $pagenum");

			$tablespec = array('class', 'detailsTable', 1);
			$table = TMRip::ripTable($oh_page, $tablespec);
			$orders = TMRip::ripOrders($table);

			tm_log('Got ' . count($orders) . ' order detail links.');

			if (count($orders) > 10)
			{
				tm_log('Possible bad link count: ' . count($orders));
				echo 'Possible bad link count: ' . count($orders);
			}
			else if (count($orders) > 0)
			{
				// Export these orders to csv.
				$this->exportOrders($orders, $pagenum, $account);
			}
			else
			{
				// This can happen if there is a page of events that have
				// all already passed.  In that case, the entire page will list
				// orders but none of them will have links.   Not an error.
				tm_log("OK: No current orders in page to export.\n");
			}
		}

		if ($this->maxPages > 1 && !$this->doneDates)
		{
			// Search for a link to the next page of orders.
			$links = TMRip::ripLinks($oh_page);

			foreach ($links as $link)
			{
				list($nexturl, $nextanchor) = $link;

				if ($nextanchor == 'next')
				{
					// Expecting: '/member/order_history?start=N'
					$params = explode('?', $nexturl);
					$param = explode('=', $params[1]);
		
					list($propname, $propval) = $param;
					if (!strstr($propname, 'start'))
						tm_log("No 'start' found in nextpage url: " . $nexturl);

					tm_log('next page -> ' . $propval);

					return $propval;
				}
			}
	
			tm_log('Done: No "next page" link in order_history page ' . $pagenum);
			//echo 'No "next page" link in order_history page ' . $pagenum . '<br>';
		}

		tm_log('< exportPage', true);

		return 0;
	}

	// --------------------------------------------------------------------
	// Export all Ticketmaster orders to csv.
	// --------------------------------------------------------------------
	function exportAll()
	{
		tm_log('> exportAll', true);

		// Start at page 0 by default.
		$pagenum = $this->startPage;
		$this->pageCount = 0;
		tm_log('Beginning page loop.');

		do
		{
			// Convince apache we're not hung.  Do we need this?
			echo 'Getting page ' . $pagenum . $this->getNewline();

			// Output the orders from page number $pagenum.
			$pagenum = $this->exportPage($pagenum, $this->user);

			$this->pageCount++;
			$this->totalPages++;

			if (($this->maxPages > 0 && $this->pageCount >= $this->maxPages) ||
				$this->doneDates)
				$pagenum = 0;

			tm_log('Next order page: ' . $pagenum);
			tm_log('Get another page? ' . ($pagenum > 0 ? 'YES' : 'NO'));

		} while ($pagenum);

		tm_log('Getting retries: ' . count($this->orderRetries));

		if (count($this->orderRetries) && !$this->skipFailed)
		{
			$this->exportOrders($this->orderRetries, "retries", $this->user, true);
		}

		tm_log('Done retries.');
		tm_log('< exportAll', true);
	}

	// ---------------------------------------------------------------------
	// Start log, open output file, setup initial settings.
	// ---------------------------------------------------------------------
	function startRun()
	{
		// Wamp needs this. Apparently the default setting is rather short.
		set_time_limit(0);

		if ($this->cliMode)
		{
			if (isset($_ENV['windir']))
				$GLOBALS['tmpdir'] .= 'tm-batch\\';
			else
				$GLOBALS['tmpdir'] .= 'tm-batch/';

			@mkdir($GLOBALS['tmpdir']);

			echo 'Output sent to: ' . $GLOBALS['tmpdir'] . 'tm-log.txt' . "\n";

			// XXX Very important!  Otherwise we end up using
			// the cookie jar from web mode for batch mode!
			$GLOBALS['cookie_jar'] = $GLOBALS['tmpdir'] . 'tm-cookiejar';
		}

		$this->status = init_log();
		if ($this->status && sha1($this->status[0]) != $this->status[1])
			$this->status = false;

		tm_log('Tmpdir is set to: ' . $GLOBALS['tmpdir']);
		tm_log('Beginning run.');

		// Setup csv file for exported data.
		$this->csvFile = fopen($GLOBALS['tmpdir'] . 'tm-data.csv', 'wt');
		Orders::writeCSVHeader($this->csvFile);

		update_dropfile('Starting...');
	}

	// ---------------------------------------------------------------------
	// Show summary report, close files.
	// ---------------------------------------------------------------------
	function endRun()
	{
		if ($this->expectedOrders == 0 ||
			$this->totalOrders == 0 ||
			$this->totalPages == 0)
		{
			echo "Possible problem getting data?" . $this->getNewline();
			update_dropfile('0');
		}
		else
		{
			echo "Pages scanned: " . $this->totalPages . $this->getNewline();
			echo "Orders expected: " . $this->expectedOrders . $this->getNewline();
			echo "Orders scanned: " . $this->totalOrders . $this->getNewline();
			echo "Orders skipped: " . $this->skippedOrders . $this->getNewline();
			echo "Tickets saved: " . $this->ticketsSaved . $this->getNewline();

			if (count($this->orderRetries))
			{
				echo "Skipped orders retried." . $this->getNewline();
				echo "Data may be incorrect or missing." . $this->getNewline();

				foreach ($this->orderRetries as $retry)
				{
					echo 'Retried: ' . $retry[1] . $this->getNewline();
				}
			}
		}

		fclose($this->csvFile);
		tm_log('End of run.');
		close_log();
	}

	// ---------------------------------------------------------------------
	// Initialize state, call main export routine.
	// ---------------------------------------------------------------------
	function Main()
	{
//			if (!$this->status)
//				return false;

		if ($this->queryPages)
		{
			echo 'Getting at most ' . $this->maxPages .
				' pages per account starting at page ' . $this->startPage .
				$this->getNewline();;

			tm_log('Getting at most ' . $this->maxPages .
				   ' pages per account starting at page ' . $this->startPage);
		}
		else if ($this->queryDates)
		{
			if ($this->eventDates)
				echo 'Fetching by event date.' . $this->getNewline();
			if ($this->orderDates)
				echo 'Fetching by order date.' . $this->getNewline();

			echo 'Start date: ' . strftime('%c', $this->startDate) .
				' End date: ' . strftime('%c', $this->endDate) .
				$this->getNewline();

			tm_log('Start date: ' . strftime('%c', $this->startDate) .
				   ' End date: ' . strftime('%c', $this->endDate));
		}
		else
		{
			tm_log('Aaah!  No valid query mode!');
			return false;
		}

		$this->totalPages = 0;
		$this->doneDates = false;
		$this->orderRetries = array();

		tm_log("Logging in: $this->user\n");
		update_dropfile("Logging in: $this->user");
		echo "Logging in: $this->user" . $this->getNewline();

		$this->logout();
		$this->login($this->user, $this->pass);
		if (!$this->loggedIn)
			return false;

		// Prep orders from this account for updates.
		TMRip::clearTixOrders($this->user);
		TMRip::blankOrders($this->user);
		TMRip::pastOrders();

		$this->exportAll();
	}
}
