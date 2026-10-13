<?php
/*
 * Author Jacob A. Psimos
 */
require_once (dirname(__FILE__) . '/../include/back-end/Helpers.php');
require_once (dirname(__FILE__) . '/../include/back-end/Session.php');
require_once (dirname(__FILE__) . '/../include/back-end/SqliteObject.php');

header('Content-type: application/json');

// Globals
$validRequests = [
	'tv up',
	'tv down'
];
$validRequest = FALSE;
$sql = NULL;
$result = -1;
$output = array();

// Request Processing
if(isset($_POST['request']))
{
	if(in_array($_POST['request'], $validRequests))
	{
		$validRequest = TRUE;
		$sql = new SqliteObject();
	}
}

if(!$validRequest || !$sql)
{
	failAndExit('Unexpected error.');
}

openlog('tvStandInvoker', LOG_PID, LOG_USER);

switch($_POST['request'])
{
	case 'tv up':
			if(syslog(LOG_NOTICE, 'tv stand motor up requested'))
			{
				exec("/usr/bin/curl --silent --get --data-urlencode 'command=lift up 2000' http://tvstand.lan", $output, $result);
				successAndExit(array(
					'messageBoxTitle' => 'TV stand raise up requested',
					'messageBoxBody' => 'Please be patient.'
				));
			}
		break;

	case 'tv down':
			if(syslog(LOG_NOTICE, 'tv stand motor down requested'))
			{
				exec("/usr/bin/curl --silent --get --data-urlencode 'command=lift down 2000' http://tvstand.lan", $output, $result);
				successAndExit(array(
					'messageBoxTitle' => 'TV stand raise down requested',
					'messageBoxBody' => 'Please be patient.'
				));
			}
		break;
	default:
		failAndExit('Bogus request: ' . $_POST['request']);
		break;
}

?>
