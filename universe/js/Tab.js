/*
	Author Jacob A. Psimos
*/

class Tab
{
	constructor(tabName, displayName, navButtonGlyphicon)
	{
		if(new.target == Tab)
		{
			throw new Error("Can not instantiate Tab directly.");
		}
		else
		{
			if(tabName in TabController.GetInstance().Tabs)
			{
				throw new Error('Can not create multiple instances of ' + tabName);
			}
			else
			{
				this.tabName = tabName;
				this.displayName = displayName;
				this.navButtonGlyphicon = navButtonGlyphicon;
				this.navbarButtonId = tabName + '_navButton';
				this.tabContainerId = tabName + '_tabContainer';
				
				// Create Navbar objects
				var navbarAnchor = $(document.createElement('a'));
				navbarAnchor.attr('href', '#');
				navbarAnchor.attr('tab-name', tabName);
				navbarAnchor.addClass('no-select');
				navbarAnchor.text(displayName);
				navbarAnchor.click(
					(event) => {
						this.Open();
					}
				);

				var navbarIcon = $(document.createElement('span'));
				navbarIcon.addClass(navButtonGlyphicon);
				navbarIcon.css('margin-right', '3px');
				navbarAnchor.prepend(navbarIcon);
				
				var navbarButton = $(document.createElement('li'));
				navbarButton.attr('id', this.navbarButtonId);
				navbarButton.append(navbarAnchor);
				$('#topNavBarList').append(navbarButton);
			}
		}
	}
	
	IsOpen()
	{
		return !$('#' + this.tabContainerId).hasClass('inactive');
	}
	
	Open()
	{
		if(!this.IsOpen())
		{
			for(var otherTabName in TabController.GetInstance().Tabs)
			{
				if(otherTabName != this.tabName && TabController.GetInstance().Tabs[otherTabName].IsOpen())
				{
					TabController.GetInstance().Tabs[otherTabName].Close();
				}
			}
			if(!$('#' + this.navbarButtonId).hasClass('active'))
			{
				$('#' + this.navbarButtonId).addClass('active')
			}
			if(!$('#' + this.navbarButtonId).hasClass('bolded'))
			{
				$('#' + this.navbarButtonId).addClass('bolded')
			}
			
			$('#' + this.tabContainerId).removeClass('inactive');
			
			this.OnOpen();
		}
	}
	
	Close()
	{
		if(this.IsOpen())
		{
			if($('#' + this.navbarButtonId).hasClass('active'))
			{
				$('#' + this.navbarButtonId).removeClass('active')
			}
			if($('#' + this.navbarButtonId).hasClass('bolded'))
			{
				$('#' + this.navbarButtonId).removeClass('bolded')
			}
			
			$('#' + this.tabContainerId).addClass('inactive');
			
			this.OnClose();
		}
	}
	
	OnOpen()
	{
		return;
	}
	
	OnClose()
	{
		return;
	}
	
}
