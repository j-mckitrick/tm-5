<?php

class Util
{
	function getServer()
	{
		if(isset($GLOBALS["_SERVER"]))
			return($GLOBALS["_SERVER"]);

		return(array());
	}

	function getServerValue($valueName)
	{
		$value  = "";
		$server = Util::getServer();

		if(isset($server[$valueName]))
			$value = $server[$valueName];

		return($value);
	}

	function getPhpSelf()
	{
		return(Util::getServerValue("PHP_SELF"));
	}

	function getRequest()
	{
		if(isset($GLOBALS["_REQUEST"]))
			return($GLOBALS["_REQUEST"]);

		return(array());
	}

	function getRequestValue($name)
	{
		$request = Util::getRequest();

		return(isset($request[$name]) ? trim($request[$name]) : "");
	}

	function getRequestArray($name)
	{
		$request = Util::getRequest();
		$value   = array();

		if(isset($request[$name]))
		{
			$theValue = $request[$name];

			if(is_array($theValue))
				$value = $theValue;
		}

		return($value);
	}

	function getRequiredVariables($variableNames, &$errMsg)
	{
		$missing = array();
		$found   = array();
                                                                                
		for($n = count($variableNames), $i = 0; $i < $n; $i ++)
		{
			$name  = $variableNames[$i];
			$value = Util::getRequestValue($name);
                                                                                
			if($value == "")
			{
				$missing[] = $name;
			}
			else
			{
				$found[$name] = $value;
			}
		}
                                                                                
		if(($n = count($missing)) > 0)
		{
			$errMsg =
				"Missing Required Variable(s)\n" .
				"<p>\n" .
				"<ul>\n";
                                                                                
			for($i = 0; $i < $n; $i ++)
				$errMsg .= sprintf(" <li>%s\n", $missing[$i]);
                                                                                
			$errMsg .= "</ul>\n";
                                                                                
			return(false);
		}
                                                                                
		while(list($variableName, $variableValue) = each($found))
			$GLOBALS[$variableName] = $variableValue;
                                                                                
		return(true);
	}

	function getUploadedFileInfo($variableName)
	{
		$info = array();

		if(isset($GLOBALS["_FILES"]) &&
		   isset($GLOBALS["_FILES"][$variableName]))
		{
			$info = $GLOBALS["_FILES"][$variableName];
		}

		return($info);
	}

	function getUploadedFile(
		$variableName, &$name, &$tmpName, &$errMsg
	    )
	{
		$info = Util::getUploadedFileInfo($variableName);

		if(!isset($info["name"])     ||
		   !isset($info["tmp_name"]) ||
		   !isset($info["error"])    ||
		   !isset($info["size"]))
		{
			$errMsg = sprintf(
				"No uploaded file data found, variable: [%s]",
				$variableName
				);

			return(false);
		}

		switch($info["error"])
		{
	    case UPLOAD_ERR_OK:
	    {
			$name    = $info["name"];
			$tmpName = $info["tmp_name"];

			return(true);
	    }
	    break;

	    case UPLOAD_ERR_INI_SIZE:
	    case UPLOAD_ERR_FORM_SIZE:
	    {
			$errMsg = sprintf(
				"Uploaded file: [%s] is too large (%d bytes)",
				$info["name"], $info["size"]
				);
	    }
	    break;

	    case UPLOAD_ERR_PARTIAL:
	    {
			$errMsg = sprintf(
				"Uploaded file: [%s] not fully received",
				$info["name"]
				);
	    }
	    break;

	    case UPLOAD_ERR_NO_FILE:
	    {
			$errMsg = "No File Supplied For Upload";
	    }
	    break;

	    default:
	    {
			$errMsg = sprintf(
				"Unknown error: [%s], file: [%s]",
				$info["error"], $info["name"]
				);
	    }
	    break;
		}

		return(false);
	}

	function getDocumentRoot()
	{
		return(Util::getServerValue("DOCUMENT_ROOT"));
	}

	function getDist()
	{
		if(eregi("(.*/dist)/.*", Util::getDocumentRoot(), $values))
			return($values[1]);

		return("");
	}

	function getMySQLBin()
	{
		return(Util::getDist() . "/tools/mysql/bin");
	}

	function getResources()
	{
		$value = "";

		if(($documentRoot = Util::getDocumentRoot()) != "")
			$value = $documentRoot . "/../resources";

		return($value);
	}

	function getSurveys()
	{
		$value = "";

		if(($documentRoot = Util::getDocumentRoot()) != "")
			$value = $documentRoot . "/../../surveys";

		return($value);
	}

	function getDocs()
	{
		$value = "";

		if(($documentRoot = Util::getDocumentRoot()) != "")
			$value = $documentRoot . "/../../docs";

		return($value);
	}

	function spewFile($fileName)
	{
		$map = array(
			"xls" => "application/vnd.ms-excel",
			"pdf" => "application/pdf"
			);

		if(eregi("^/", $fileName))
			$fullPath = $fileName;
		else
			$fullPath = sprintf("%s/%s", Util::getDocs(), $fileName);

		if(file_exists($fullPath))
		{
			$type = "application/octet-stream";

			while(list($theExtension, $theType) = each($map))
			{
				if(eregi($theExtension. "$", $fullPath))
				{
					$type = $theType;
					break;
				}
			}

			header("Content-Type: $type");
			readfile($fullPath);
		}
	}

	function getMicroTime()
	{
		$theValues = split(" ", microtime());
		$microTime = (double)$theValues[0] + (double)$theValues[1];

		return($microTime);
	}

	function getPID()
	{
		return(getmypid());
	}

	function getRemoteAddress()
	{
		return(Util::getServerValue("REMOTE_ADDR"));
	}

	function createID()
	{
		return(
			md5(uniqid(Util::getRemoteAddress() . Util::getPID()))
			);
	}

	function getFileType($fileName, &$isAscii, &$lineNumber, &$errMsg)
	{
		if(($handle = @fopen($fileName, "rb")) == false)
		{
			$errMsg = "Could not open: [$fileName]";
			return(false);
		}

		$isAscii    = true;
		$lineNumber = 0;

		while($isAscii && ($line = fgets($handle, 4096)) !== false)
		{
			$lineNumber++;

			for($n = strlen($line), $i = 0; $isAscii && $i < $n; $i ++)
			{
				$c = ord($line[$i]);
      
				// Allowed chars: '\b' -> '\r', ' ' -> '~'
      
				if($c < 8 || ($c > 13 && $c < 32) || $c > 126)
				{
					$isAscii = false;
				}
			}
		}
      
		fclose($handle);
      
		return(true);
	}

	function trimNewLine($input)
	{
		$input = str_replace("\r", "", $input);
		$input = str_replace("\n", "", $input);

		return($input);
	}

	function split($separator, $values)
	{
		$exploded = explode($separator, $values);
		$list     = array();

		for($n = count($exploded), $i = 0; $i < $n; $i ++)
		{
			if(($theValue = trim($exploded[$i])) != "")
			{
				$list[] = $theValue;
			}
		}

		return($list);
	}

	function getDaysInMonth($month, $year)
	{
		$answer = 0;

		switch($month)
		{
	    case 1: case 3: case 5: case 7: case 8: case 10: case 12:
	    {
			$answer = 31;
	    }
	    break;

	    case 4: case 6: case 9: case 11:
	    {
			$answer = 30;
	    }
	    break;

	    case 2:
	    {
			if($year % 400 == 0 || ($year % 4 == 0 && $year % 100 != 0))
			{
				$answer = 29;
			}
			else
			{
				$answer = 28;
			}
	    }
	    break;

	    default:
	    {
			die("assert: [" . __FILE__ . ":" . __LINE__ . "]");
	    }
		}

		return($answer);
	}
}
