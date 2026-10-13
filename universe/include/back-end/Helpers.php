<?php
	/*
		Author Jacob A. Psimos
	*/

    define('__DIR__', dirname(__FILE__));

	function failAndExit($exception, $exitCode = 1){
		echo json_encode(array('success' => FALSE, 'exception' => $exception));
		exit($exitCode);
	}

	function successAndExit(array $results = array()){
		$results['success'] = TRUE;
		echo json_encode($results);
		exit(0);
	}

?>
