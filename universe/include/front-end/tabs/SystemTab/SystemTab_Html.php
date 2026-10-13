<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<div class="container-fluid system-tab-inner">
	<div class="row">
		<div class="col-xs-2">
			<input id="btnReboot" type="button" class="btn btn-primary" value="Reboot"/>
		</div>
	</div>
</div>