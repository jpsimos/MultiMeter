<?php
// Author Jacob A. Psimos
require_once (dirname(__FILE__) . '/../../../front-end/TabController.php');

$thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<script type="text/javascript">

	class MultimeterTab extends Tab
	{
		constructor()
		{
			<?php
				$thisTab->GetJsTabSuperCtorCallWithArgs();
			?>
			
			this.getMeasurementsTimeout = null;
			this.getMeasurementsBusy = false;
			this.measurementsChart = new LineChart('measurementsChart');
			this.Open();
		}
		
		OnOpen()
		{
			super.OnOpen();
			this.measurementsChart.Refresh();
			this.getMeasurements();
		}
		
		OnClose()
		{
			super.OnClose();
			if(this.getMeasurementsTimeout == null)
			{
				clearTimeout(this.getMeasurementsTimeout);
			}
		}
		
		getMeasurements()
		{
			var hadError = false;
			
			if(!this.getMeasurementsBusy)
			{
				this.getMeasurementsBusy = true;
				$.ajax({
					url: 'invoke/pzemInvoker.php',
					type: 'POST',
					dataType: 'json',
					data: { 'request' : 'get_measurements_and_alarms', 'id_start' : this.measurementIdStart },
					success: (data) =>
					{
						this.getMeasurementsBusy = false;
						if(data.success)
						{
							this.drawMeasurementsChart(data);
							this.populateMultimeterTable(data);
							
							if(this.getMeasurementsTimeout == null)
							{
								clearTimeout(this.getMeasurementsTimeout);
							}
							
							this.getMeasurementsTimeout = setTimeout(
								() => {
									this.getMeasurements();
								},
								1000
							);
						}
					},
					error: (jqXHR, exception) =>
					{
						this.getMeasurementsBusy = false;
						console.log('getMeasurements exception: ' + exception);
					},
					always: () =>
					{
						this.getMeasurementsBusy = false;
					}
				});
			}
			else
			{
				console.log('getMeasurements busy');
				hadError = true;
			}
			
			return !hadError;
		}
		
		drawMeasurementsChart(rows)
		{
			var watts = new Array();
			var volts = new Array();
			var current = new Array();
			
			for(var i = rows.measurements.length - 1; i >= 0; i--)
			{
				watts.push(rows.measurements[i].power);
				volts.push(rows.measurements[i].voltage);
				current.push(rows.measurements[i].current);
			}
			
			this.measurementsChart.SetData('Wattage', watts, 'blue');
			this.measurementsChart.SetData('Voltage', volts, 'red');
			this.measurementsChart.SetData('Current', current, 'green');
		}
		
		populateMultimeterTable(data)
		{
			$('#lblWatts').text(data.measurements[0].power.toFixed(1) + 'W');
			$('#lblVolts').text(data.measurements[0].voltage.toFixed(1) + 'V');
			$('#lblCurrent').text(data.measurements[0].current.toFixed(3) + 'A');
		}
	}

</script>
