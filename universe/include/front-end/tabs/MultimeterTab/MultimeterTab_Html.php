<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<div class="container-fluid">
	<div class="row">
		<div class="col-xs-12" id="measurementsChartContainer">
			<canvas id="measurementsChart"></canvas>
		</div>
	</div>
	<div class="row">
		<div class="col-xs-1">
			<span class="label label-danger" id="lblVolts">--</span>
		</div>
		<div class="col-xs-1">
			<span class="label label-primary" id="lblWatts">--</span>
		</div>
		<div class="col-xs-1">
			<span class="label label-success" id="lblCurrent">--</span>
		</div>
	</div>
	<!--
    <table class="table table-hover table-striped" id="tblMultimeter">
        <thead>
            <tr>
            	<td>When</td>
                <td>Voltage (V)</td>
                <td>Frequency (Hz)</td>
                <td>Current (A)</td>
                <td>Power (W)</td>
            </tr>
        </thead>
        <tbody></tbody>
    </table>
    -->

</div>