<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<script type="text/javascript">
	class MainTab extends Tab
	{
		constructor()
		{
            <?php
            	$thisTab->GetJsTabSuperCtorCallWithArgs();
            ?>
            
            this.TvStandCommandBusy = false;
            
            $('#btnTvUp').click(
            	(event) => {
            		this.TvStandCommand('tv up');
            	}
            );
            
            $('#btnTvDown').click(
            	(event) => {
            		this.TvStandCommand('tv down');
            	}
            );
		}
		
		OnOpen()
		{
			super.OnOpen();
		}
		
		OnClose()
		{
			super.OnClose();
		}
		
		TvStandCommand(request)
		{
			var hadError = false;
			
			if(!this.TvStandCommandBusy)
			{
				this.TvStandCommandBusy = true;
				$.ajax({
					url: 'invoke/tvStandInvoker.php',
					type: 'POST',
					dataType: 'json',
					data: { 'request' : request },
					success: (data) =>
					{
						this.TvStandCommandBusy = false;
						if(data.success)
						{
 							ShowMessageBox({
 								title: data.messageBoxTitle,
								body: data.messageBoxBody,
 								ok: true
 							});
						}
					},
					error: (jqXHR, exception) =>
					{
						this.TvStandCommandBusy = false;
						console.log('TvStandCommand exception: ' + exception);
					},
					always: () =>
					{
						this.TvStandCommandBusy = false;
					}
				});
			}
			else
			{
				console.log('TvStandCommand busy');
				hadError = true;
			}
			
			return !hadError;
		}
	}
</script>
