<?php 
    // Author Jacob A. Psimos

	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . '..'
		. DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'Tab.php');


    class MultimeterTab extends Tab
    {
        public function __construct()
        {
            parent::__construct(array(
                'm_tabName' => __CLASS__,
                'm_displayName' => 'Multimeter',
                'm_navButtonGlyphicon' => 'glyphicon glyphicon-phone'
            ));
        }

    
    }
?>

