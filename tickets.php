<?php				// -*- mode: nxml -*-
  require_once 'ordersquery.inc';

  $oq = new OrdersQuery();
  $oq->doPerform(); // update, query, export, or default (show all orders)
  $rows = Orders::getAllOrders($oq->sql);
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml11/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
    <title>Ticketmaster Orders</title>
    <link href="style.css" rel="stylesheet" type="text/css"/>
    <script type="text/javascript" src="jscript-orders.js"></script>
  </head>

  <body>
    <span>Orders displayed: <?php echo mysql_num_rows($rows); ?></span>
    <form method='post' action='tickets.php'>
      <div>
	<input type='hidden' name='perform' value='query' />
	<table border="1" class="ordersTable">

	  <!-- column names -->
	  <tr>
	    <?php foreach (Orders::getColumnNames() as $col): ?>
	    <th><?php echo $col; ?></th>
	    <?php endforeach; ?>
	  </tr>

	  <!-- BEGIN order query controls -->
	  <tr>
	    <td>
	      <input type='button' value='Query Orders' onclick='doSubmit(0)' />
	      <input type='button' value='Clear Query' onclick='doSubmit(3)' />
	    </td>
	    <td>
	      <select id='ticketstatus' name='queryticketstatus'>
		<option value='-1'>All</option>
		<?php echo Orders::getTicketStatusOptions($oq->q_ticketstatus); ?>
	      </select>
	    </td>
	    <td>
	      <select id='comparestatus' name='querycomparestatus'>
		<option value='-1'>All</option>
		<?php echo Orders::getCompareStatusOptions($oq->q_comparestatus); ?>
	      </select>
	    </td>
	    <td>
	      <select name='queryreceivedprinted'>
		<option value='-1'>All</option>
		<?php echo Orders::getYesNoOptions($oq->q_receivedprinted); ?>
	      </select>
	    </td>
	    <td>
	      <select name='querylisted'>
		<option value='-1'>All</option>
		<?php echo Orders::getYesNoOptions($oq->q_listed); ?>
	      </select>
	    </td>
	    <td>
	    <input name='querycomment'
		   value='<?php echo $oq->q_comment ?>' />
	    </td>
	    <td>
	      From:
	      <input name='queryorderdate' class='datefield'
		     value='<?php echo $oq->q_orderdate ?>' />
	      To:
	      <input name='queryorderdate_to' class='datefield'
		     value='<?php echo $oq->q_orderdate_to ?>' />
	    </td>
	    <td>
	      <input name='queryordernum' class='mediumfield'
		     value='<?php echo $oq->q_ordernum ?>' />
	    </td>
	    <td></td>
	    <td>
	      <input name='queryevent' class='widefield'
		     value='<?php echo $oq->q_event ?>' />
	    </td>
	    <td>
	      From:
	      <input name='queryeventdate' class='datefield'
		     value='<?php echo $oq->q_eventdate ?>' />
	      <br/>
	      To:
	      <input name='queryeventdate_to' class='datefield'
		     value='<?php echo $oq->q_eventdate_to ?>' />
	    </td>
	    <td>
	      <input name='queryeventtime' class='mediumfield'
		     value='<?php echo $oq->q_eventtime ?>' />
	    </td>
	    <td>
	      <input name='queryvenue'
		     value='<?php echo $oq->q_venue ?>' />
	    </td>
	    <td>
	      <input name='querycity' class='widefield'
		     value='<?php echo $oq->q_city ?>' />
	    </td>
	    <td>
	      <input name='querystate' class='narrowfield'
		     value='<?php echo $oq->q_state ?>' />
	    </td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td>
	      <input name='queryticketfast' class='ro mediumfield' />
	    </td>
	    <td>
	      <input name='querytmstatus' class='hugefield'
		     value='<?php echo $oq->q_tmstatus ?>' />
	    </td>
	    <td>
	      <input name='querytmstatus_p' class='hugefield'
		     value='<?php echo $oq->q_tmstatus_p ?>' />
	    </td>
	    <td>
	      <input name='querytmstatus_o' class='hugefield'
		     value='<?php echo $oq->q_tmstatus_o ?>' />
	    </td>
	    <td>
	      <select name='querygottixtoday'>
		<option value='-1'>All</option>
		<?php echo Orders::getYesNoOptions($oq->q_gottixtoday); ?>
	      </select>
	    </td>
	    <td></td>
	    <td>
	      <input name='queryaccount' class='mediumfield' />
	    </td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	  </tr>
	  <!-- END order query controls -->

	  <!-- BEGIN order data -->
	  <?php
	    while ($row = mysql_fetch_assoc($rows)):
	    $orderNum = $row['order_num'];
	  ?>

	  <tr>
	    <td>
	      <input type='checkbox' name='orders[]' value='<?php echo $orderNum ?>'
	      onclick='doHighlite(this)' />
	    </td>
	    <?php foreach(Orders::getOrderRowStatus($row) as $col): ?>
	    <td><?php echo $col; ?></td>
	    <?php endforeach; ?>

	    <?php foreach(Orders::getOrderRowDetail($row) as $col): ?>
	    <td><?php echo htmlentities($col); ?></td>
	    <?php endforeach; ?>
	  </tr>

	  <?php endwhile; ?>
	  <!-- END order data -->

	  <!-- BEGIN order update controls -->
	  <tr>
	    <td>
	      <input type='button' value='Select All' onclick='doSelectAll()' />
	      <input type='button' value='Deselect All' onclick='doDeselectAll()' />
	    </td>
	    <td><!-- no user update of compare status --></td>
	    <td><!-- no user update of ticket status --></td>
	    <td>
	      <select name='updatereceivedprinted'>
		<option value='-1'>- Select -</option>
		<?php echo Orders::getYesNoOptions(-1); ?>
	      </select>
	    </td>
	    <td>
	      <select name='updatelisted'>
		<option value='-1'>- Select -</option>
		<?php echo Orders::getYesNoOptions(-1); ?>
	      </select>
	    </td>
	    <td><input name='updatecomment' /></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	    <td></td>
	  </tr>
	  <!-- END order update controls -->

	</table>
	<br/>
	
	<input type='button' value='Update Orders' onclick='doSubmit(1)' />
	<input type='button' value='Export Orders' onclick='doSubmit(2)' />

	<br />
	<?php if ($oq->exportReady): ?>
	<a href="dl-data.php?file=<?php echo $oq->exportFile ?>">Exported data</a><br/>
	<?php endif; ?>
	<!--
	    <br />
	    <span>The sql: <?php echo $oq->sql; ?></span>
	    <input type='button' value='Export Orders' onclick='doSubmit(2)' />
	    <span><?php echo $perform; ?></span>
	    <span><?php echo $oq->q_receivedprinted; ?></span>
	    <span><?php echo $oq->q_ticketstatus; ?></span>
	-->
      </div>
    </form>
    <div>
      <a href="index.html">Main Menu</a><br/>
    </div>
  </body>

</html>
