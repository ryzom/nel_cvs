<?php

	class CMemStream
	{
		var $Buffer;
		var $InputStream;
		var $Pos;

		function CMemStream ()
		{
			$this->InputStream = false;
			$this->Pos = 0;
		}

		function setBuffer ($buffer)
		{
			$this->InputStream = true;
			$this->Buffer = $buffer;
			$this->Pos = 0;
		}

		function isReading () { return $this->InputStream; }

		function serialuint8 (&$val)
		{
			if ($this->isReading())
			{
				$val = ord($this->Buffer{$this->Pos++});
				//printf ("read uint8 '%d'<br>", $val);
			}
			else
			{
				$this->Buffer .= chr($val & 0xFF);
				$this->Pos++;
				//printf ("write uint8 '%d' %d<br>", $val, $this->Pos);
			}
		}

		function serialuint32 (&$val)
		{
			if ($this->isReading())
			{
				$val = ord($this->Buffer{$this->Pos++});
				$val += ord($this->Buffer{$this->Pos++})<<8;
				$val += ord($this->Buffer{$this->Pos++})<<16;
				$val += ord($this->Buffer{$this->Pos++})<<32;
				//printf ("read uint32 '%d'<br>", $val);
			}
			else
			{
				$this->Buffer .= chr($val & 0xFF);
				$this->Buffer .= chr(($val>>8) & 0xFF);
				$this->Buffer .= chr(($val>>16) & 0xFF);
				$this->Buffer .= chr(($val>>24) & 0xFF);
				$this->Pos += 4;
				//printf ("write uint32 '%d' %d<br>", $val, $this->Pos);
			}
		}

		function serialstring (&$val)
		{
			if ($this->isReading())
			{
				$this->serialuint32($size);
				$val = substr ($this->Buffer, $this->Pos, $size);
				//printf ("read string '%s'<br>", $val);
				$this->Pos += strlen($val);
			}
			else
			{
				$this->serialuint32(strlen($val));
				$this->Buffer .= $val;
				$this->Pos += strlen($val);
				//printf ("write string '%s' %d<br>", $val, $this->Pos);
			}
		}
	}

	// This function connect to the LS, ask it if the user uid can connect
	// to shard shardid and return a bool that say it s ok or not.
	// If true, $res contains the url to connect.
	// If false, $res contains the reason why it s not okay.

	function connectToLS(&$fp, &$res)
	{
		// connect to the login service that must be localhost:49990
		$fp = fsockopen ("localhost", 49990, $errno, $errstr, 30);
		if (!$fp)
		{
			$res = "Can't connect to the login service ($errstr ($errno))";
		}
		else
		{
			$res = "";
		}
	}

	function sendMessage ($fp, $msgout)
	{
		$size = $msgout->Pos;
		$buffer = chr(($size>>24)&0xFF);
		$buffer .= chr(($size>>16)&0xFF);
		$buffer .= chr(($size>>8)&0xFF);
		$buffer .= chr($size&0xFF);
		$buffer .= $msgout->Buffer;

		fwrite ($fp, $buffer);

		//printf ("sent packet size '%d'<br>", strlen($buffer));
		
		fflush ($fp);
	}

	function waitMessage ($fp, &$msgin)
	{
		//echo "waiting a message";
		$size = 0;
		$val = fread ($fp, 1);
		$size = ord($val) << 24;
		$val = fread ($fp, 1);
		$size = ord($val) << 16;
		$val = fread ($fp, 1);
		$size += ord($val) << 8;
		$val = fread ($fp, 1);
		$size += ord($val);
		//printf ("receive packet size '%d'<br>", $size);
		$fake = fread ($fp, 4);
		$size -= 4; // remove the fake

		$buffer = fread ($fp, $size);
		$msgin = new CMemStream;
		$msgin->setBuffer ($buffer);
	}

	function askClientConnection ($shardid, $uid, &$res)
	{
		$ok = false;

		connectToLS($fp, $res);
		if($res!="")
			return $ok;

		// send the message that say that we want to add a user
		$msgout = new CMemStream;
		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet
		$messageType = 0;
		$msgout->serialuint8 ($messageType);
		$msgout->serialuint32 ($shardid);
		$msgout->serialuint32 ($uid);

		sendMessage ($fp, $msgout);

		waitMessage ($fp, $msgin);

		$msgin->serialstring($reason);
		//printf("reason size %d", strlen($reason));
			
		if(strlen($reason) == 0)
		{
			// it s ok, let's connect
			$msgin->serialstring($cookie);
			$msgin->serialstring($addr);
			$res = '<!--nel="launch" nelArgs="'.$cookie.' '.$addr.'"-->';
			$ok = true;
		}
		else
		{
			// can't accept it, display the error
			// echo "Can't connect to the shard: $reason";
			$res = $reason;
		}

		//printf("receive response '$reason' '$cookie' '$addr'<br>");

		fclose ($fp);
		//echo "sent OK.<br><br>";

		return $ok;
	}

	// This function connect to the LS, ask the LS to disconnect the user

	function disconnectClient($shardid, $uid, &$res)
	{
		$ok = false;

		connectToLS($fp, $res);
		if(strlen($res) != 0)
			return $ok;

		// send the message that say that we want to add a user
		$msgout = new CMemStream;
		$fake = 0;
		$msgout->serialuint32 ($fake);			// fake used to number the packet
		$messageType = 1;
		$msgout->serialuint8 ($messageType);
		$msgout->serialuint32 ($shardid);
		$msgout->serialuint32 ($uid);

		sendMessage ($fp, $msgout);

		waitMessage ($fp, $msgin);

		$msgin->serialstring($res);
		//printf("reason size %d", strlen($res));
			
		if(strlen($res) == 0)
		{
			$ok = true;
		}

		//printf("receive response '$reason' '$cookie' '$addr'<br>");

		fclose ($fp);
		//echo "sent OK.<br><br>";

		return $ok;
	}

?>
