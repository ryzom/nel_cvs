<?php 

	include_once('config.inc');

	function compressFile($filename)
	{
		$fp = fopen ($filename, "rb");
		if (!$fp) return false;

		$gz = gzopen ("$filename.gz", "wb9");
		if (!$gz) { fclose ($fp); return false; }
		
		while (!feof($fp))
		{
			$contents = fread($fp, filesize ($filename));
			gzwrite ($gz, $contents);
		}

		gzclose($gz);
		fclose($fp);
		
		return true;
	}

	echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">';
	echo "<html><head><title>make patch</title></head><body>";

	$ok = true;

	if (!isset($shard_version))
	{
		die("Bad shard version, you must provide a shard version to know where data are. For example: http://myurl/make_distrib?shard_version=12");
	}

	if (strlen($shard_version) == 0)
	{
		$path = $RootPatchInternalAccess."/default";
	}
	else
	{
		$path = $RootPatchInternalAccess."/".$shard_version;
	}
	
	$dir = "dir.gz";

	$d = dir($path);
	echo "<h1>Making patch in '$path' directory</h1>\n";

	$zp = gzopen("$path/$dir", "wb9");

	$nbfiles = 0;

	echo "<table border=1><tr><th>Res</th><th>Filename</th><th>Size</th><th>Date</th></tr>";

	$tline=0;

	if ($zp == false)
	{
		echo "<font color=red>ERROR</font> Can't open file $path/$dir for writing data<br>\n";
		$ok = false;
	}
	else
	{
		gzwrite ($zp, "FILESLIST\n");
		while (false !== ($entry = $d->read()))
		{
			// only add file that are not .gz
			if (is_file("$path/$entry") && substr("$entry", -3) != ".gz")
			{
				// if the gz associated doesn't exist, create it
				if (!file_exists("$path/$entry.gz"))
				{
					if (compressFile ("$path/$entry"))
					{
						echo "<tr><td><font color=green>OK</font></td><td>Compressed '$entry' into .gz</td></tr>\n";
						$tline++;
					}
					else
					{
						$ok = false;
						echo "<tr><td><font color=red>ERROR</font></td><td>Can't compress '$entry'</td></tr>\n";
						$tline++;
					}
				}

				$fsize = filesize("$path/$entry");
				$fdate = filemtime ("$path/$entry");
				if ($fsize >= 0 && $fdate > 0)
				{
					$str = "$entry/$fsize/$fdate\n";
					echo "<tr><td><font color=green>OK</font></td><td>$entry</td><td>$fsize</td><td>$fdate</td></tr>";
					$tline++;
					gzwrite ($zp, $str);
					$nbfiles++;
				}
				else
				{
					echo "<tr><td><font color=red>ERROR</font></td><td>$entry</td><td>$fsize</td><td>$fdate</td><br>\n";
					$tline++;
					$ok = false;
				}

				if ($tline >= 20)
				{
					echo "</table><table border=1><tr><th>Res</th><th>Filename</th><th>Size</th><th>Date</th></tr>";
					$tline = 0;
				}
			}
		}
		$d->close();
		gzclose($zp);
	}

	echo "</table>";

	if ($ok)
		echo "<br><font color=green>OK file $path/$dir successfully created and contains $nbfiles files</font>\n";
	else
		echo "<br><font color=red>ERROR something goes wrong during the process</font>\n";

	echo "</body>";
?>
