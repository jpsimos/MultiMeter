/*
	Author Jacob A. Psimos
*/

var TabControllerSingleton = null;

class TabController
{
	static GetInstance()
	{
		if(TabControllerSingleton == null)
		{
			TabControllerSingleton = new TabController();
		}
		return TabControllerSingleton;
	}
	
	constructor()
	{
		if(TabControllerSingleton != null)
		{
			throw new Error("Can not instantiate TabController directly.");
		}
		
		this.Tabs = new Object();
	}
	
	GetCurrentTab()
	{
		var tab = null;
		for(var tabName in this.Tabs)
		{
			if(this.Tabs[tabName].IsOpen())
			{
				tab = this.Tabs[tabName];
				break;
			}
		}
		return tab;
	}

}