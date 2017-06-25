<?php

// Global settings and declarations.

if (isset($_SERVER['WINDIR']))
{
	// For windows
	$GLOBALS['tmpdir'] = 'C:\\temp\\';
}
else
{
	// For *nux
	$GLOBALS['tmpdir'] = '/tmp/';
}

$settings = parse_ini_file('application.ini');
//    print_r($settings);
foreach ($settings as $key => $value)
	$GLOBALS[$key] = $value;

// Files
$GLOBALS['filedrop'] = 'tm-filedrop.txt';

// Debug settings
$GLOBALS['rotate_logs'] = false;
$GLOBALS['verbose_logs'] = false;

// Error code constants
define('ERROR_RETRY',	10);
define('ERROR_SKIP',	20);
define('ERROR_ABORT',	30);
