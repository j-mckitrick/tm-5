<?php

// Serve a file located in a temporary directory.

require_once 'tmcurl.inc';

$file = $_REQUEST["file"];
TMCurl::sendCSVFile($file);
