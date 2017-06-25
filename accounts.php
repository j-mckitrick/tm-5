<?php

require_once 'accounts.inc';

function showAccounts()
{
    if (Accounts::getAccounts($accounts, $err))
    {
		echo '<table border=1>';

		echo '<tr>';

		echo '<th>' . 'id number' . '</th>';
		echo '<th>' . 'username' . '</th>';
		echo '<th>' . 'password' . '</th>';

		echo '</tr>';

		foreach ($accounts as $account)
		{
			echo '<tr>';

			echo '<td>' . $account['username'] . '</td>';
			echo '<td>' . $account['password'] . '</td>';
			echo '<td>';
			echo '<a href="delete-account.php?id=' . $account['id']  . '">Delete</a>';

			echo '</td>';
			echo '</tr>';
		}

		echo '</table>';

    }
    else
    {
		echo 'Error: ' . $errmsg;
    }
}

?>

<html><body>

<?php showAccounts(); ?>

<br/>
<form method="post" action="add-account.php">
  <div>
    <input type="text" name="username" />Username<br/>
    <input type="text" name="password" />Password<br/>
    <input type="submit" value="Add" />
  </div>
</form>
<a href="index.html">Main Menu</a><br/>

</body></html>

