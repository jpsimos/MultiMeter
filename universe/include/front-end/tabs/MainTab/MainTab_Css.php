<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<style type="text/css">
    #MainTab_tabContainer {
        /*background-color:#333;*/
        color: #FFFFFF;
    }
    
    #clockPanel {
    	position: absolute;
    	width: 242px;
    	height: 242px;
    	margin: 0;
    	padding: 0;
    }
    
    #clockPanel .panel-heading {
    	height: 20px;
    }
    
    #clockPanelBody {
    	width: 240px;
    	height: 240px;
    	margin:0;
    	padding:0;
    }
    
    #analogClock {
    	margin: 0;
    	padding: 0;
    	margin-left: 10px;
    }
</style>