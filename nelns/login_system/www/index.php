<?php 

// Warning: This script is not sure, It doesn't check if a user is already online and other possibility

	include_once('service_connection.inc');

// ---------------------------------------------------------------------------------------- 
// functions 
// ---------------------------------------------------------------------------------------- 

	// $state is the state that should be good when checking the login password
	// $reason contains the reason why the check failed
	// return true if the check is ok
	function checkLoginPassword ($login, $password, &$id, &$reason)
	{
		$link = mysql_connect() or die ("Can't connect to database: ".mysql_error());
		mysql_select_db ("nel") or die ("Can't access to the table: ".mysql_error());
		$query = "SELECT * FROM user where Login='".$login."'";
		$result = mysql_query ($query) or die ("Can't execute the query: ".$query);
		
		if (mysql_num_rows ($result) == 0)
		{
			// login doesn't exist, create it

			$query = "INSERT INTO user (Login, Password) VALUES ('".$login."', '".$password."')";
			$result = mysql_query ($query) or die ("Can't execute the query: ".$query);

			$query = "SELECT * FROM user WHERE Login='".$login."'";
			$result = mysql_query ($query) or die ("Can't execute the query: ".$query);

			if (mysql_num_rows ($result) == 1)
			{
				$reason = "Login '".$login."' was created because it was not found in database";
				$row = mysql_fetch_row ($result);
				$id = $row[0];
				$res = 1;
			}
			else
			{
				$reason = "Can't fetch the login '".$login."'";
				$res = 0;
			}
		}
		else
		{
			$row = mysql_fetch_row ($result);
			if ($row[2] == $password)
			{
				if ($row[4] != "Offline")
				{
					$reason = $login." is already online";
					$res = 0;
				}
				else
				{
					$id = $row[0];
					$res = 1;
				}
			}
			else
			{
				$reason = "Bad password";
				$res = 0;
			}
		}
		mysql_close($link);
		return $res;
	}

	function logged($login, $password, &$id)
	{
		if (isset($login) && isset($password))
		{
			// mean that the user just enterer the login and password
			if (checkLoginPassword($login, $password, $id, $reason))
			{
				// ok, valid login/pass, set the cookie
				setcookie ("login", $login, time()+3600*24*15);
				setcookie ("password", $password, time()+3600*24*15);

				if(strlen($reason) != 0)
				{
					echo $reason."<br>";
				}
				return 1;
			}
			else
			{
				echo "Authentification failed: ".$reason."<br>";
			}
		}
		else
		{
			return 0;
		}
	}

	function loginForm()
	{
		global $PHP_SELF; 
		?> 
		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
		<html>
			<head>
				<title>login form</title>
			</head>
			<body>
basename:"<?php echo basename($PHP_SELF); ?>" 	
			<h1>Welcome to the NeL login system</h1>
			<p>Please enter your login and your password. If the login is not already existing, it'll be automatically added to the database.</p>
			<p>You have to enable cookies in your browser to use this application.</p>
			<form method="post" action="<?php echo basename($PHP_SELF); ?>" name=loginform> 
				<table frame=void rules=none WIDTH="300"> 
					<tr>
						<td align="left"> login: </td>
						<td align="left"> <input type="text" name="login" maxlength=50 size=15> </td>
					<tr> 
						<td align="left"> password: </td>
						<td align="left"> <input type="password" name="password" maxlength=50 size=15> </td>
					<tr>
						<td align="left">&nbsp; </td>
						<td align="left">&nbsp; </td>
					<tr> 
						<td align="left"> &nbsp; </td>
						<td align="left"> <input type=submit value="login" > </td>

				</table>
			</form> 
				<script type="text/javascript"> 
				<!-- 
					if (document.loginform) { 
					document.loginform.login.focus(); 
				} 
				// --> 
				</script> 
			</body>
		</html>
		<?php 
	}

	function displayAvailableShards()
	{
		$link = mysql_connect() or die ("Can't connect to database");
		mysql_select_db ("nel") or die ("Can't access to the table");
		$query = "SELECT * FROM shard";
		$result = mysql_query ($query) or die ("Can't execute the query: ".$query);
		
		if (mysql_num_rows ($result) == 0)
		{
			echo '<h1>Sorry but now shards are available</h1>';
		}
		else
		{
			echo '<h1>Please, select a shard:</h1>';
			while($row = mysql_fetch_array($result))
			{
				echo "<li>";
				if ($row["Online"])
				{
					echo '<a href="'.basename($PHP_SELF).'?cmd=login&shardid='.$row["ShardId"].'&programname='.$row["ProgramName"].'"> '.$row["Name"].' ('.$row["NbPlayers"].' players)</a>';
				}
				else
				{
					echo $row["Name"].' (offline)';
				}
				echo "</li>";
			}

			echo '</ul>';
		}

		mysql_close($link);
		return $res;
	}

// --------------------------------------------------------------------------------------
// main 
// --------------------------------------------------------------------------------------

	if(!logged($login, $password, $id))
	{
		loginForm();
	}
	else
	{
		if ($cmd == "logout")
		{
			// remove the cookie
			unset ($login);
			unset ($password);
			setcookie ("login");
			setcookie ("password");

			// go to the login form
			loginForm ();
		}
		else if ($cmd == "login")
		{
			// user selected a shard, try to add the user to the shard

			if (connectToLS($programname, $shardid, $id, $res))
			{
				echo '<h1>Access validated</h1>Please wait while launching the application...<br>';
				echo $res;
			}
			else
			{
				echo "<h1>Access denied</h1>";
				echo "<p>$res</p>";
				echo '<p><a href="'.basename($PHP_SELF).'">Back to the shards page</a></p>';
			}
		}
		else
		{
			// user logged, display the available shard

			// echo "id: '$id' login: '$login' password: '$password'<br>";
			echo "basename: ".basename($PHP_SELF).". hello $login, nice to meet you.";

			displayAvailableShards ();

			echo '<a href="'.basename($PHP_SELF).'?cmd=logout">logout</a><br>';
		}
	}
?>

