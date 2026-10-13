/*
	Author Jacob A. Psimos
*/

class LineChart
{
	constructor(canvasId)
	{
		this.padding = 10;
		this.drawAreaX = 0;
		this.drawAreaY = 0;
		this.drawAreaWidth = 0;
		this.drawAreaHeight = 0;
		this.canvas = document.getElementById(canvasId);
		this.ctxLock = false;
		
		if(this.canvas == undefined)
		{
			throw new Error(toString(canvasId) + ' is not valid.');
		}
		
		this.ctx = this.canvas.getContext('2d');
		this.lastResize = Date.now();
		this.resizeTimeout = null;
		this.dataSet = new Object();
		this.maxDataValues = new Object();
		this.minDataValues = new Object();
		this.dataArrayColors = new Object();
		
		// Handle when the window resizes with a 75 millisecond grace to avoid excessive CPU usage.
		$(window).on(
			'resize', 
			(event) => {
				if(this.lastResize + 75 < Date.now())
				{
					this.Refresh();
					this.lastResize = Date.now();
				}
				else
				{
					if(this.resizeTimeout != null)
					{
						clearTimeout(this.resizeTimeout);
						this.resizeTimeout = null;
					}
					
					this.resizeTimeout = setTimeout(
						() => {
							this.resizeTimeout = null;
							this.Refresh();
							this.lastResize = Date.now();
						},
						75
					);
				}
			}
		);
	}
	
	IsLegit()
	{
		if(this.canvas == undefined || this.ctxLock === true)
		{
			return false;
		}
		return true;
	}
	
	IsDataAvailable()
	{
		var dataAvailable = false;
		for(var dataSetName in this.dataSet)
		{
			if(this.dataSet[dataSetName].length > 0)
			{
				dataAvailable = true;
				break;
			}
		}
		return dataAvailable;
	}
	
	RefreshBoundaries()
	{
		if(this.IsLegit())
		{
			this.ctxLock = true;
			var width = this.canvas.parentElement.clientWidth - this.padding;
			var height = this.canvas.parentElement.clientHeight - this.padding;
			this.canvas.width = width;
			this.canvas.height = height;
			this.drawAreaX = this.padding / 2;
			this.drawAreaY = this.padding / 2;
			this.drawAreaWidth = this.canvas.width - (this.padding / 2);
			this.drawAreaHeight = this.canvas.height - (this.padding / 2);
			this.ctxLock = false;
		}
	}
	
	RefreshData()
	{
		if(this.IsDataAvailable())
		{
			for(var dataArrayName in this.dataSet)
			{
				this.DrawData(dataArrayName);
			}
		}
	}
	
	Refresh()
	{
		if(this.IsLegit())
		{
			this.RefreshBoundaries();
			this.Clear();
			this.DrawAxis();
			this.RefreshData();
		}
	}
	
	Clear()
	{
		if(this.IsLegit())
		{
			this.ctxLock = true;
			this.ctx.beginPath();
			this.ctx.clearRect(
				this.drawAreaX,
				this.drawAreaY,
				this.drawAreaWidth,
				this.drawAreaHeight
			);
			this.ctx.stroke();
			this.ctxLock = false;
		}
	}
	
	DrawAxis()
	{
		if(this.IsLegit())
		{
			this.ctxLock = true;
			this.ctx.beginPath();
			this.ctx.lineWidth = 1;
			this.ctx.strokeStyle = "black";
			// X
			this.ctx.moveTo(
				this.drawAreaX,
				this.drawAreaHeight
			);
			this.ctx.lineTo(
				this.drawAreaWidth,
				this.drawAreaHeight
			);
			// Y
			this.ctx.moveTo(
				this.drawAreaX,
				this.drawAreaHeight
			);
			this.ctx.lineTo(
				this.drawAreaX,
				this.drawAreaY
			);
			this.ctx.stroke();
			this.ctxLock = false;
		}
	}
	
	SetData(dataArrayName, dataArray, dataColor)
	{
		this.dataSet[dataArrayName] = dataArray;
		this.dataArrayColors[dataArrayName] = dataColor;
		
		if(this.minDataValues[dataArrayName] === undefined)
		{
			this.minDataValues[dataArrayName] = Number.MAX_VALUE;
		}
		
		if(this.maxDataValues[dataArrayName] === undefined)
		{
			this.maxDataValues[dataArrayName] = Number.MIN_VALUE;
		}
		
		for(var i = 0; i < dataArray.length; i++)
		{
			if(dataArray[i] < this.minDataValues[dataArrayName])
			{
				this.minDataValues[dataArrayName] = dataArray[i];
			}
			if(dataArray[i] > this.maxDataValues[dataArrayName])
			{
				this.maxDataValues[dataArrayName] = dataArray[i];
			}
		}
		
		this.Refresh();
	}
	
	DrawData(dataArrayName)
	{		
		if(this.IsLegit())
		{
			this.ctxLock = true;
		
			var dataArray = this.dataSet[dataArrayName];	
			var dataPointWidth = (this.drawAreaWidth - 1) / dataArray.length;
			var dataCursorX = this.drawAreaX + 1;
			var dataCursorY = this.drawAreaHeight - 1;
			var lastDataCursorX = dataCursorX;
			var lastDataCursorY = dataCursorY;
			
			for(var i = 0; i < dataArray.length; i++)
			{
				var dataPoint = (dataArray[i] / this.maxDataValues[dataArrayName])
					* (this.drawAreaHeight * 0.8);
					
				dataCursorY = this.drawAreaHeight - dataPoint;
				
				this.ctx.beginPath();
				this.ctx.lineWidth = 1;
				this.ctx.strokeStyle = this.dataArrayColors[dataArrayName];
				this.ctx.moveTo(
					dataCursorX,
					dataCursorY
				);
				this.ctx.lineTo(
					lastDataCursorX,
					lastDataCursorY
				);
				this.ctx.stroke();
				lastDataCursorX = dataCursorX;
				lastDataCursorY = dataCursorY;
				dataCursorX += dataPointWidth;
			}
			
			this.ctxLock = false;	
		}
	}
	
}