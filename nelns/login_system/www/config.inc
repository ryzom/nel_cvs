<?php

// This file contains all variables needed by other php scripts

// ---------------------------------------------------------------------------------------- 
// Variables for index.php
// ---------------------------------------------------------------------------------------- 

	// where we can find the mysql database
	$DBHost		= "localhost";
	$DBUserName	= "";
	$DBPassword	= "";
	$DBName		= "nel";

	// where we can find the dir.gz and other files
	// you can use a secure access using .htaccess and .htpasswd, in this case, set the url like this:
	// "http://login:password@localhost/patch"
	$PatchURL = "http://localhost/patch";

// ---------------------------------------------------------------------------------------- 
// Variables for service_connection.inc
// ---------------------------------------------------------------------------------------- 

	$LSHost = "localhost";
	$LSPort = 49998;

?>
