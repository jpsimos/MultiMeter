<?php
    // Author Jacob A. Psimos
    
    require_once(dirname(__FILE__) . '/../../../front-end/TabController.php');
    
    $thisTab = TabController::GetInstance()->GetTabFromJsFile(__FILE__);
?>

<script type="text/javascript">
	class SystemTab extends Tab
	{
		constructor()
		{
            <?php
            	$thisTab->GetJsTabSuperCtorCallWithArgs();
            ?>
            
            this.systemCommandBusy = false;
            
            $('#btnReboot').click(
            	(event) => {
            		this.SystemCommand('reboot');
            	}
            );
            
            $('#btnGoDark').click(
            	(event) => {
            		this.SystemCommand('godark');
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
		
		SystemCommand(request)
		{
			var hadError = false;
			
			if(!this.systemCommandBusy)
			{
				this.systemCommandBusy = true;
				$.ajax({
					url: 'invoke/systemInvoker.php',
					type: 'POST',
					dataType: 'json',
					data: { 'request' : request },
					success: (data) =>
					{
						this.systemCommandBusy = false;
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
						this.systemCommandBusy = false;
						console.log('systemCommand exception: ' + exception);
					},
					always: () =>
					{
						this.systemCommandBusy = false;
					}
				});
			}
			else
			{
				console.log('systemCommand busy');
				hadError = true;
			}
			
			return !hadError;
		}
	}
</script>