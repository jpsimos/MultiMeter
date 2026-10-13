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
	'get_measurements_and_alarms'
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

switch($_POST['request'])
{
	case 'get_measurements_and_alarms':
        $minute_ago = time() - 60;
        $query = "SELECT * FROM Measurements_And_Alarms WHERE epoch_taken >= $minute_ago ORDER BY id DESC LIMIT 120";
        $res = $sql->getData($query, TRUE);

		
		if($res)
		{
			successAndExit(array(
				'measurements' => $res
			));
		}
		else
		{
			failAndExit('Internal error.');
		}
		break;
	case 'get_summary':
		$query = "SELECT * FROM Measurements_And_Alarms LIMIT 1";
		
		if(($res = $sql->getData($query, TRUE)))
		{
			successAndExit(array(
				'measurements' => $res
			));
		}
		else
		{
			failAndExit('Internal error.');
		}
		break;
	default:
		failAndExit('Bogus request ' . $_POST['request']);
		break;
}

?>
