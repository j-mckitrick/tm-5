<?php

  // Curl functions to grab various Ticketmaster pages.

require_once 'globals.inc';
require_once 'tmrip.inc';

$GLOBALS['cookie_jar'] = $GLOBALS['tmpdir'] . 'tm-cookiejar';
$GLOBALS['user_agent'] = 'mozilla/5.0 (x11; u; linux i686; en-us; rv:1.8.1.6) gecko/20061201 firefox/2.0.0.6 (ubuntu-feisty)';
$GLOBALS['user_agent'] = 'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_1; en-us) AppleWebKit/531.9 (KHTML, like Gecko) Version/4.0.3 Safari/531.9';
$GLOBALS['user_agent'] = 'Mozilla/4.0 (MSIE 6.0; Windows NT 5.0)';

class TMCurl
{
	// ---------------------------------------------------------------------
	// Handle curl errors for a curl handle.
	// ---------------------------------------------------------------------
	function handleError($c)
	{
	    $errmsg = curl_error($c);
	    tm_log('* Curl error: ' . $errmsg);

	    //die("Curl error getting page: $errmsg <br>");
	}

	// ---------------------------------------------------------------------
	// Get an http page.
	// ---------------------------------------------------------------------
	function getPageSimple($url)
	{
	    $c = curl_init($url);

	    curl_setopt($c, CURLOPT_RETURNTRANSFER, 1); // data in return value
	    curl_setopt($c, CURLOPT_SSL_VERIFYPEER, 0); // needed for windows
	    curl_setopt($c, CURLOPT_TIMEOUT, 120); // plenty of time to wait
	    curl_setopt($c, CURLOPT_CONNECTTIMEOUT, 0); // don't timeout on connect
	    curl_setopt($c, CURLOPT_VERBOSE, 1);	// show error info

	    $page = curl_exec($c);
	    if (!$page)
	    {
			$errmsg = curl_error($c);
			echo "Curl error getting page: $errmsg";
	    }

	    curl_close($c);

	    return $page;
	}

	// ---------------------------------------------------------------------
	// Get a ticketmaster https page with cookies.
	// ---------------------------------------------------------------------
	function getSecurePage($page_url, $clear_cookies=false)
	{
	    $url = 'https://www.ticketmaster.com' . $page_url;

	    tm_log('Getting secure page: ' . $url, true);
	    tm_log('Cookie jar: ' . $GLOBALS['cookie_jar'], true);

	    $c = curl_init($url);

	    curl_setopt($c, CURLOPT_RETURNTRANSFER, 1); // data in return value
	    curl_setopt($c, CURLOPT_SSL_VERIFYPEER, 0); // needed for windows
	    curl_setopt($c, CURLOPT_USERAGENT, $GLOBALS['user_agent']); // fake browser
	    curl_setopt($c, CURLOPT_TIMEOUT, 120); // plenty of time to wait
	    curl_setopt($c, CURLOPT_CONNECTTIMEOUT, 0); // don't timeout on connect
	    curl_setopt($c, CURLOPT_COOKIEJAR, $GLOBALS['cookie_jar']); // save
	    curl_setopt($c, CURLOPT_HEADER, 1);
//	    curl_setopt($c, CURLOPT_VERBOSE, 1);	// show error info
	    if (!$clear_cookies)
			curl_setopt($c, CURLOPT_COOKIEFILE, $GLOBALS['cookie_jar']); // send

	    $page = curl_exec($c);
	    if (!$page)
			TMCurl::handleError($c);
	    curl_close($c);

	    return $page;
	}

	// --------------------------------------------------------------------
	// POST login data, including seed.
	// --------------------------------------------------------------------
	function postLogin($seed, $user, $pass, $url=false)
	{
	    tm_log('Seed: ' . $seed, true);
	    tm_log('User: ' . $user);
	    tm_log('Pass: ' . $pass);

	    // -------------------------------------------------------
	    // Build POST form body.
	    // If Ticketmaster changes their login form,
	    // this form body will need to be updated.
	    // -------------------------------------------------------
	    $body = "v=$seed&email_address=$user&password=$pass";

	    // POST login form data and save cookies.
	    $c = curl_init($url ?
					   "https://www.ticketmaster.com" . $url :
					   "https://www.ticketmaster.com/member");

	    curl_setopt($c, CURLOPT_HEADER, 1);
	    curl_setopt($c, CURLOPT_POST, 1);
	    curl_setopt($c, CURLOPT_USERAGENT, $GLOBALS['user_agent']);
	    curl_setopt($c, CURLOPT_COOKIEJAR, $GLOBALS['cookie_jar']);
	    curl_setopt($c, CURLOPT_POSTFIELDS, $body);
	    curl_setopt($c, CURLOPT_RETURNTRANSFER, 1);
	    curl_setopt($c, CURLOPT_SSL_VERIFYPEER, 0);

	    // Do we care about the resulting page?
	    // We *could* parse it for the link to 'account history'
	    // but for now we just hard-code it.
	    $page = curl_exec($c);
	    if (!$page)
			TMCurl::handleError($c);
	    curl_close($c);

	    TMRip::savePage($page, 'login_result');

	    // -------------------------------------------------------
	    // Some accounts return an error even before login!
	    // -------------------------------------------------------
//	    $re_bad_gateway = '/(502.bad.gateway)/i';
	    $re_ok = '/(200.ok)/i';

//	    if (preg_match($re_bad_gateway, $page))
	    if (!preg_match($re_ok, $page))
	    {
			return "Login failed - http error.";
	    }

	    // -------------------------------------------------------
	    // Confirm this is a working login page.
	    // We might need to revise this from time to time.
	    // -------------------------------------------------------
	    $re_login = '/(<h1>login<\/h1>|sorry|problem)/i';

	    if (preg_match($re_login, $page))
	    {
			//echo $page;
			tm_log('Possible problem logging in.  Checking messages.');

			// Do this here, because we don't keep this page content
			// around after the login.
			TMRip::ripAnnoyingMessages($page);

			return "Login failed.";
	    }

	    // -------------------------------------------------------
	    // Confirm we have successfully logged in.
	    // We might need to revise this from time to time.
	    // -------------------------------------------------------
	    $re_login = '/(logout|order_history)/i';

	    if (preg_match($re_login, $page, $matches))
	    {
			//echo $page;

			// Do this here, because we don't keep this page content
			// around after the login.
			//TMRip::ripAnnoyingMessages($page);

			tm_log('Matched: "' . $matches[0] . '"');
			tm_log('Login appears successful.');
	    }
	    else
	    {
			return "Login failed.";
	    }
	}

	// ---------------------------------------------------------------------
	// Send a csv file from the temp directory.
	// ---------------------------------------------------------------------
	function sendCSVFile($file)
	{
	    $fullpath = $GLOBALS['tmpdir'] . "$file";

	    if (($size = @filesize($fullpath)) === false)
			exit("Error checking filesize for file [$file]");

	    header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
	    header("Content-type: application/csv");
	    header("Content-disposition: attachment; filename=\"$file\"");
	    header("Content-length: $size");
	    
	    readfile($fullpath);
	    
	    exit();
	}

	// -------------------------------------------------------
	// Some accounts return an error even before login!
	// -------------------------------------------------------
	function checkForError($page)
	{
	    $re_ok = '/(200.ok)/i';

	    if (!preg_match($re_ok, $page))
			return "Page failed - http error.";
	    else
			return false;
	}
}
