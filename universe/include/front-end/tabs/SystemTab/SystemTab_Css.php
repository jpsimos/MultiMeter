<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<style type="text/css">
    #SystemTab_tabContainer {
    
    }
    
    .system-tab-inner {
    	margin-top: 10px;
    }

</style>