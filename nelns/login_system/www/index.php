<?php 

// Warning: This script is not safety, It doesn't check if a user is already online and other possibility

	include_once('service_connection.inc');

	include_once('config.inc');

// ---------------------------------------------------------------------------------------- 
// Functions
// ---------------------------------------------------------------------------------------- 

	// $reason contains the reason why the check failed or success
	// return true if the check is ok
	function checkUserValidity ($login, $password, $clientApplication, &$id, &$reason)
	{
		global $DBHost, $DBUserName, $DBPassword, $DBName, $AcceptUnknownUser;

		$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die ("Can't connect to database host:$DBHost user:$DBUserName");
		mysql_select_db ($DBName) or die ("Can't access to the table dbname:$DBName");
		$query = "SELECT * FROM user where Login='$login'";
		$result = mysql_query ($query) or die ("Can't execute the query: ".$query);

		if (mysql_num_rows ($result) == 0)
		{
			if ($AcceptUnknownUser)
			{
				// login doesn't exist, create it
				$query = "INSERT INTO user (Login, Password) VALUES ('$login', '$password')";
				$result = mysql_query ($query) or die ("Can't execute the query: ".$query);

				// get the user to have his UId
				$query = "SELECT * FROM user WHERE Login='$login'";
				$result = mysql_query ($query) or die ("Can't execute the query: ".$query);

				if (mysql_num_rows ($result) == 1)
				{
					$reason = "Login '".$login."' was created because it was not found in database";
					$row = mysql_fetch_row ($result);
					$id = $row[0];

					// add the default permission
					$query = "INSERT INTO permission (UId) VALUES ('$id')";
					$result = mysql_query ($query) or die ("Can't execute the query: ".$query);

					$res = true;
				}
				else
				{
					$reason = "Can't fetch the login '".$login."'";
					$res = false;
				}
			}
			else
			{
				$reason = "Unknown login '".$login."'";
				$res = false;
			}
		}
		else
		{
			$row = mysql_fetch_row ($result);
			if ($row[2] == $password)
			{
				// check if the user can use this application

				$query = "SELECT * FROM permission WHERE UId='$row[0]' and ClientApplication='$clientApplication'";
				$result = mysql_query ($query) or die ("Can't execute the query: ".$query);
				if (mysql_num_rows ($result) == 0)
				{
					// no permission
					$reason = "You can't use the client application '$clientApplication'";
					$res = false;
				}
				else
				{
					// check if the user not already online

					if ($row[4] != "Offline")
					{
						$reason = "$login is already online and ";
						// ask the LS to remove the client
						if (disconnectClient ($row[3], $row[0], $tempres))
						{
							$reason =  $reason."was just disconnected. Now you can retry the identification.";

							$query = "update user set ShardId=-1, State='Offline' where UId=$row[0]";
							$result = mysql_query ($query) or die ("Can't execute the query: '$query' errno:".mysql_errno().": ".mysql_error());
						}
						else
						{
							$reason = $reason."can't be disconnected: $tempres.";
						}
						$res = false;
					}
					else
					{
						$id = $row[0];
						$res = true;
					}
				}
			}
			else
			{
				$reason = "Bad password";
				$res = false;
			}
		}
		mysql_close($link);
		return $res;
	}

	function logged($login, $password, $clientApplication, &$id, $add)
	{
		if (isset($login) && isset($password) && isset($clientApplication))
		{
			// mean that the user just enterer the login and password
			if (checkUserValidity($login, $password, $clientApplication, $id, $reason))
			{
				if ($add)
				{
					// ok, valid login/pass, set the cookie
					setcookie ("login", $login, time()+3600*24*15);
					setcookie ("password", $password, time()+3600*24*15);
					if(strlen($reason) != 0)
					{
						echo $reason."<br>\n";
					}
				}
				return 1;
			}
			else
			{
				echo "Authentification failed: ".$reason."<br>\n";
			}
		}
		else
		{
			if (!isset($login)) $str = "login ";
			if (!isset($password)) $str .= "password ";
			if (!isset($clientApplication)) $str .= "clientApplication";
			echo "Authentification failed: $str not set<br>\n";
			return 0;
		}
	}

	function patchForm($shardid, $clientApplication, $clientVersion, $serverVersion)
	{
		global $PHP_SELF, $RootPatchURL;

		echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n";
		echo "<html><head><title>patch form</title></head><body>\n";
		echo "<h1>Please wait while patching $clientApplication version $clientVersion to version $serverVersion </h1>\n";
		echo '<!--nel="patch" serverVersion="'.$serverVersion.'" nelUrl="'.$PHP_SELF.'?cmd=login&shardid='.$shardid.'" nelServerPath="'.$RootPatchURL.'/'.$serverVersion.'" nelUrlFailed="'.$PHP_SELF.'?cmd=patchFailed"'."\n";
		echo "<h1>Current state</h1>\n";
		echo "<!--nel_start_state--><!--nel_end_state--><br>\n";
		echo "<h1>Log</h1>\n";
		echo "<!--nel_start_log--><!--nel_end_log-->\n";
		echo "</body></html>\n";
	}

	function patchFailed($reason)
	{
		global $PHP_SELF; 
		echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n";
		echo "<html><head><title>patch failed</title></head><body>\n";
		echo "<h1>Patching failed</h1>\n";
		echo "reason is: $reason<br>\n";
		echo "</body></html>\n";
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
			<h1>Welcome to the NeL login system</h1>
			<p>Please enter your login and your password. If the login is not already existing, it'll be automatically added to the database.</p>
			<p>You have to enable cookies in your browser to use this application.</p>
			<form method="get" action="<?php echo basename($PHP_SELF); ?>" name=loginform> 
				<table frame=void rules=none WIDTH="300"> 
					<tr>
						<td align="left"> login: </td>
						<td align="left"> <input type="text" name="newlogin" maxlength=50 size=15> </td>
					<tr> 
						<td align="left"> password: </td>
						<td align="left"> <input type="password" name="newpassword" maxlength=50 size=15> </td>
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
					document.loginform.newlogin.focus(); 
				} 
				// --> 
				</script> 
			</body>
		</html>
		<?php 
	}

	function displayAvailableShards($clientApplication, $clientVersion)
	{
		global $PHP_SELF;
		global $DBHost, $DBUserName, $DBPassword, $DBName;

		$link = mysql_connect($DBHost, $DBUserName, $DBPassword) or die ("Can't connect to database host:$DBHost user:$DBUserName");
		mysql_select_db ($DBName) or die ("Can't access to the table dbname:$DBName");
		$query = "SELECT * FROM shard WHERE ClientApplication='".$clientApplication."'";
		$result = mysql_query ($query) or die ("Can't execute the query: ".$query);
		
		if (mysql_num_rows ($result) == 0)
		{
			echo "<h1>Sorry but no shards are available</h1>\n";
		}
		else
		{
			echo "<h1>Please, select a shard:</h1>\n";
			while($row = mysql_fetch_array($result))
			{
				if ($row["ProgramName"] == $programName)
				{
					$versionok = true;
					
					if ($row["Version"] == "")
						$ver = "no version";
					else if ($row["Version"] != $clientVersion)
					{
						$ver = "version ".$row["Version"]." <font style=\"color:red\">You'll need to patch the client to access to this shard</font>";
						$versionok = false;
					}
					else
						$ver = "version ".$row["Version"];

					echo "<li>\n";
					if ($row["Online"])
					{
						if ($versionok)
							echo '<a href="'.basename($PHP_SELF).'?cmd=login&shardid='.$row["ShardId"].'"> '.$row["Name"].'</a> '.$ver.' ('.$row["NbPlayers"].' players), shardid:'.$row["ShardId"].' wsip:'.$row["WsAddr"]."\n";
						else
							echo '<a href="'.basename($PHP_SELF).'?cmd=patch&serverVersion='.$row["Version"].'&shardid='.$row["ShardId"].'"> '.$row["Name"].'</a> '.$ver.' ('.$row["NbPlayers"].' players), shardid:'.$row["ShardId"].' wsip:'.$row["WsAddr"]."\n";
					}
					else
					{
						echo $row["Name"].' '.$ver.' (offline), shardid:'.$row["ShardId"].' wsip:'.$row["WsAddr"]."\n";
					}
					echo "</li>\n";
				}
			}

			echo "</ul>\n";
		}

		mysql_close($link);
		return $res;
	}

// --------------------------------------------------------------------------------------
// main 
// --------------------------------------------------------------------------------------

	if ($HTTP_USER_AGENT != "nel_launcher")
	{
		die ("You must use the Nel Launcher to connect to a shard");
	}

	if (isset ($newClientVersion))
	{
		$clientVersion = $newClientVersion;
		setcookie ("clientVersion", $clientVersion, time()+3600*24*15);
	}

	if (isset ($newClientApplication))
	{
		$clientApplication = $newClientApplication;
		setcookie ("clientApplication", $clientApplication, time()+3600*24*15);
	}

	if (isset ($newlogin) && isset($newpassword))
	{
		$login = $newlogin;
		$password = $newpassword;
		$add = true;
	}

	if(!logged($login, $password, $clientApplication, $id, $add))
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

			if (askClientConnection($shardid, $id, $login, $res))
			{
				echo "<h1>Access validated</h1>Please wait while launching the application...<br>\n";
				echo $res."<br>\n";
			}
			else
			{
				echo "<h1>Access denied</h1>\n";
				echo "<p>$res</p>\n";
				echo "<p><a href=\"".basename($PHP_SELF)."\">Back to the shards page</a></p>\n";
			}
		}
		else if ($cmd == "patch")
		{
			// user selected a shard that must be patched first, display the patch page
			patchForm ($shardid, $clientApplication, $clientVersion, $serverVersion);
		}
		else if ($cmd == "patchFailed")
		{
			// patching failed! display the reason
			patchFailed ($reason);
		}
		else
		{
			// user logged, display the available shard

echo "<!-- DEBUG: id: '$id' login: '$login' password: '$password' prog: '$clientApplication' version: '$clientVersion'\n";
echo "basename: '".basename($PHP_SELF)."'<br><br>\n -->";

			echo "Hello $login, nice to meet you.<br>\n";

			displayAvailableShards ($clientApplication, $clientVersion);

			echo "<a href=\"".basename($PHP_SELF)."?cmd=logout\">logout</a><br>\n";
		}
	}
?>
