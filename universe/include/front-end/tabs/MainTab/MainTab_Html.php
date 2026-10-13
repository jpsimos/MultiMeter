<?php
    // Author Jacob A. Psimos
 
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<div class="container-fluid main-tab-inner">
	<div class="row">
		<div class="col-xs-2">
			<input id="btnTvUp" type="button" class="btn btn-primary" value="Up" />
		</div>
		<div class="col-xs-2">
			<input id="btnTvDown" type="button" class="btn btn-primary" value="Down" />
		</div>
	</div>
</div>


<!--
<div class="container-fluid">
	<div id="clockPanel" class="panel panel-primary">
		<div class="panel-heading">
		</div>
		<div id="clockPanelBody" class="panel-body">
			<canvas id="analogClock" width="220" height="220"></canvas>
		</div>
	</div>
</div>
-->
