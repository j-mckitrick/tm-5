<?php

// Form to download Ticketmaster orders.

require_once 'globals.inc';
require_once 'accounts.inc';

$dropfilename = $GLOBALS['tmpdir'] . $GLOBALS['filedrop'];
file_put_contents($dropfilename, 'Waiting...');

?>
<html>
  <head>
    <title>Ticketmaster Downloader</title>
    <link href="style.css" rel="stylesheet" type="text/css"/>
  </head>

  <body>
    <form method="post" action="tm-web.php">
      <div class="login-window">
	<h3>Ticketmaster account data download utility</h3>
<!--
	<label for="user">Username:</label>
	<input type="text" name="tm_user" id="user" /><br />
	<label for="pass">Password:</label>
	<input type="password" name="tm_pass" id="pass" /><br /><br />
-->
    	<select name="sel_accounts[]" size="6" multiple="multiple">
<?php

    if (Accounts::getAccounts($accounts, $err))
    {
		foreach ($accounts as $account)
		{
			echo '<option value="' . $account['id'] . '" "selected=selected">';
			echo $account['username'];
			echo '</option>';
		}
    }

?>
    	</select><br/>
	<input type="radio" name="query_method" value="pages" checked="checked"/>
	Page count<br/>
	<input type="radio" name="query_method" value="orderdates"/>
	Order date range<br/>
	<input type="radio" name="query_method" value="eventdates"/>
	Event date range<br/>
	<hr/>
	<label for="tm_start_page">Starting page number</label><br/>
	<input type="text" name="tm_start_page" id="start_page" style="width:2em"/>
	<br/><br/>
	<label for="tm_max_page">Max page count</label><br/>
	<input type="text" name="tm_max_page" id="max_page" style="width:2em"/>
	<hr/>
	<b>From:</b><br/>
	<select name="month_start">
	  <option value="01">January</option>
	  <option value="02">February</option>
	  <option value="03">March</option>
	  <option value="04">April</option>
	  <option value="05">May</option>
	  <option value="06">June</option>
	  <option value="07">July</option>
	  <option value="08">August</option>
	  <option value="09">September</option>
	  <option value="10">October</option>
	  <option value="11">November</option>
	  <option value="12">December</option>
	</select>
	<br/>
	<select name="day_start">
	  <option value="01">01</option>
	  <option value="02">02</option>
	  <option value="03">03</option>
	  <option value="04">04</option>
	  <option value="05">05</option>
	  <option value="06">06</option>
	  <option value="07">07</option>
	  <option value="08">08</option>
	  <option value="09">09</option>
	  <option value="10">10</option>
	  <option value="11">11</option>
	  <option value="12">12</option>
	  <option value="13">13</option>
	  <option value="14">14</option>
	  <option value="15">15</option>
	  <option value="16">16</option>
	  <option value="17">17</option>
	  <option value="18">18</option>
	  <option value="19">19</option>
	  <option value="20">20</option>
	  <option value="21">21</option>
	  <option value="22">22</option>
	  <option value="23">23</option>
	  <option value="24">24</option>
	  <option value="25">25</option>
	  <option value="26">26</option>
	  <option value="27">27</option>
	  <option value="28">28</option>
	  <option value="29">29</option>
	  <option value="30">30</option>
	  <option value="31">31</option>
	</select>
	<br/>
	<input type="text" name="year_start" value="2008" style="width:4em" />
<!--
	<select name="year_start">
	  <option value="2008">2008</option>
	  <option value="2007">2007</option>
	</select>
-->
	<br/><br/>
	<b>To:</b><br/>
	<select name="month_end">
	  <option value="01">January</option>
	  <option value="02">February</option>
	  <option value="03">March</option>
	  <option value="04">April</option>
	  <option value="05">May</option>
	  <option value="06">June</option>
	  <option value="07">July</option>
	  <option value="08">August</option>
	  <option value="09">September</option>
	  <option value="10">October</option>
	  <option value="11">November</option>
	  <option value="12">December</option>
	</select>
	<br/>
	<select name="day_end">
	  <option value="01">01</option>
	  <option value="02">02</option>
	  <option value="03">03</option>
	  <option value="04">04</option>
	  <option value="05">05</option>
	  <option value="06">06</option>
	  <option value="07">07</option>
	  <option value="08">08</option>
	  <option value="09">09</option>
	  <option value="10">10</option>
	  <option value="11">11</option>
	  <option value="12">12</option>
	  <option value="13">13</option>
	  <option value="14">14</option>
	  <option value="15">15</option>
	  <option value="16">16</option>
	  <option value="17">17</option>
	  <option value="18">18</option>
	  <option value="19">19</option>
	  <option value="20">20</option>
	  <option value="21">21</option>
	  <option value="22">22</option>
	  <option value="23">23</option>
	  <option value="24">24</option>
	  <option value="25">25</option>
	  <option value="26">26</option>
	  <option value="27">27</option>
	  <option value="28">28</option>
	  <option value="29">29</option>
	  <option value="30">30</option>
	  <option value="31">31</option>
	</select>
	<br/>
	<input type="text" name="year_end" value="2008" style="width:4em" />
<!--
	<select name="year_end">
	  <option value="2008">2008</option>
	  <option value="2007">2007</option>
	</select>
-->
	<br/>
	<br/>
	<hr/>
	<input type="checkbox" name="save_tickets" value="1"/>Save pdf tickets<br/>
	<input type="checkbox" name="old_tickets" value="1"/>Re-fetch old tickets<br/>
	<input type="checkbox" name="skip_failed" value="1"/>Skip failed logins/tickets<br/>
	<input type="submit" value="Get data!"/>
	<br/><br/>
      </div>
<!--
      <div style="width:30%">
      </div>
      <div style="width:30%">
      </div>
-->
    </form>
    <a href="poll-file.php" target="_blank">Open Status Window</a><br/>
    <a href="index.html">Main Menu</a><br/>
  </body>
</html>
