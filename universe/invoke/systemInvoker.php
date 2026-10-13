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
	'reboot'
];
$validRequest = FALSE;
$sql = NULL;

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

openlog('systemInvoker', LOG_PID, LOG_USER);

switch($_POST['request'])
{
	case 'reboot':
			if(syslog(LOG_NOTICE, 'reboot requested'))
			{
				successAndExit(array(
					'messageBoxTitle' => 'Reboot requested',
					'messageBoxBody' => 'A reboot was requested. Please be patient.'
				));
			}
		break;
	default:
		failAndExit('Bogus request: ' . $_POST['request']);
		break;
}

?>
