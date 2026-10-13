<?php
	/*
		Author Jacob A. Psimos
	*/

    session_set_cookie_params(60 * 60 * 24 * 7);
    session_start();

    class Session
    {
        private static $sm_session = NULL;
        
        public static function CreateInstance()
        {
            if(Session::$sm_session == NULL)
            {
                Session::$sm_session = new Session();
            }
            
            return Session::$sm_session;
        }
        
        public static function GetInstance()
        {
            if(Session::$sm_session == NULL)
            {
                throw new Exception("Must call CreateInstance first.");
            }
            
            return Session::$sm_session;
        }

        private function __construct()
        {
        	if(!isset($_SESSION['REMOTE_ADDR']))
        	{
	        	$_SESSION['REMOTE_ADDR'] = $_SERVER['REMOTE_ADDR'];
        	}
        	else
        	{
        		if($_SESSION['REMOTE_ADDR'] != $_SERVER['REMOTE_ADDR'])
        		{
        			throw new Exception('Possible spoofed ip address.');
        			exit(1);
        		}
        	}
        }

    }
    
    Session::CreateInstance();
?>
