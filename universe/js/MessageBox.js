/*
	Author Jacob A. Psimos
*/

/*
	{
		title : string
		body : string
		ok : boolean
		cancel : boolean
		no : boolean
		yes : boolean,
		onDismiss : function(),
		onClickOk : function(event){ event.data.arg1 ... }
		onClickCancel : function(event){ event.data.arg1 ... }
		onClickNo : function(event){ event.data.arg1 ... }
		onClickYes : function(event){ event.data.arg1 ... }
		onClickArguments : object{arg1 : value, ...}
	}
*/
function ShowMessageBox(options){

	var msgbox = $('#prototypeMsgbox').clone();
	msgbox.removeAttr('id');

	if(options.onDismiss != undefined && jQuery.isFunction(options.onDismiss)){
		msgbox.on('hidden.bs.modal', function(){
			$(this).remove();
			options.onDismiss();
		});
	}else{
		msgbox.on('hidden.bs.modal', function(){
			$(this).remove();
		});
	}
	
	if(options.title != undefined){
		msgbox.find('[name="title"]').text(options.title);
	}else{
		msgbox.find('[name="title"]').text('');
	}

	if(options.body != undefined){
		msgbox.find('.modal-body').text(options.body);
	}else{
		msgbox.find('.modal-body').text('');
	}

	if(options.ok != undefined){
		if(options.ok === false){
			msgbox.find('[name="btnOk"]').hide();
		}else if(options.ok === true){
			msgbox.find('[name="btnOk"]').show();
		}
	}else{
		msgbox.find('[name="btnOk"]').hide();
	}

	if(options.cancel != undefined){
		if(options.cancel === false){
			msgbox.find('[name="btnCancel"]').hide();
		}else if(options.cancel === true){
			msgbox.find('[name="btnCancel"]').show();
		}
	}else{
		msgbox.find('[name="btnCancel"]').hide();
	}

	if(options.no != undefined){
		if(options.no === false){
			msgbox.find('[name="btnNo"]').hide();
		}else if(options.no === true){
			msgbox.find('[name="btnNo"]').show();
		}
	}else{
		msgbox.find('[name="btnNo"]').hide();
	}

	if(options.yes != undefined){
		if(options.yes === false){
			msgbox.find('[name="btnYes"]').hide();
		}else if(options.yes === true){
			msgbox.find('[name="btnYes"]').show();
		}
	}else{
		msgbox.find('[name="btnYes"]').hide();
	}

	if(options.onClickOk != undefined && jQuery.isFunction(options.onClickOk)){
		if(options.onClickArguments != undefined){
			msgbox.find('[name="btnOk"]').click(options.onClickArguments, options.onClickOk);
		}else{
			msgbox.find('[name="btnOk"]').click(options.onClickOk);
		}
	}

	if(options.onClickCancel != undefined && jQuery.isFunction(options.onClickCancel)){
		if(options.onClickArguments != undefined){
			msgbox.find('[name="btnCancel"]').click(options.onClickArguments, options.onClickCancel);
		}else{
			msgbox.find('[name="btnCancel"]').click(options.onClickCancel);
		}
	}

	if(options.onClickNo != undefined && jQuery.isFunction(options.onClickNo)){
		if(options.onClickArguments != undefined){
			msgbox.find('[name="btnNo"]').click(options.onClickArguments, options.onClickNo);
		}else{
			msgbox.find('[name="btnNo"]').click(options.onClickNo);
		}
	}

	if(options.onClickYes != undefined && jQuery.isFunction(options.onClickYes)){
		if(options.onClickArguments != undefined){
			msgbox.find('[name="btnYes"]').click(options.onClickArguments, options.onClickYes);
		}else{
			msgbox.find('[name="btnYes"]').click(options.onClickYes);
		}
	}

	$('body').append(msgbox);
	msgbox.modal('show');
}
