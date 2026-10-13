<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<style type="text/css">
    #MultimeterTab_tabContainer {
        overflow-y: scroll;
        background-color: white;
        padding:0;
        margin:0;
    }
    
    #tblMultimeter {
        margin-top: 15px;
        margin-bottom: 15px;
    }
    
    #measurementsChartContainer {
    	height: 400px;
    	width: 100%;
    	margin: 0;
    	padding: 0;
    }
    
    #measurementsChart {
		margin: 0;
		padding: 0;
    }
</style>