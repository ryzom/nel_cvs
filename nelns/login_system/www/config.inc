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

	// where we can find directories where the dir.ngz and other files are.
	// you can use a secure access using .htaccess and .htpasswd, in this case, set the url like this:
	// "http://login:password@localhost/patch" (no / at the end of the path)
	$RootPatchURL = "http://localhost/patch";

	// where the patch directory is on the server side. It used by the make_patch.php script.
	// no / at the end of the path
	$RootPatchInternalAccess = "patch";

	// If true, the server will add automatically unknown user in the database
	$AcceptUnknownUser = false;

// ---------------------------------------------------------------------------------------- 
// Variables for service_connection.inc
// ---------------------------------------------------------------------------------------- 

	$LSHost = "linuxdev01";
	$LSPort = 49998;

?>
