/*
	Author Jacob A. Psimos
*/

class Clock
{
	constructor(panelId)
	{
		this.clockPanel = $('#' + panelId);
		this.dragging = false;
		this.offsetX = 0;
		this.offsetY = 0;
		this.refreshAnalogClockBusy = false;
	}
	
	initializeAnalogClock()
	{
		var titleBar = this.clockPanel.find('.panel-heading').first();
		
		titleBar.on('mousedown', (e) => {
			this.dragging = true;
			this.offsetX = e.clientX - parseInt(this.clockPanel.css('left'));
			this.offsetY = e.clientY - parseInt(this.clockPanel.css('top'));
		});
		
		$(document).on('mousemove', (e) => {
			if(this.dragging)
			{
				this.clockPanel.css({
					left : e.clientX - this.offsetX,
					top : e.clientY - this.offsetY
				});
			}
		});
		
		$(document).on('mouseup', () => {
			this.dragging = false;
		});
		
		this.refreshAnalogClock();
	
		var calMillisec = 1000 - (Date.now() % 1001);

		setTimeout(() => {
			this.refreshAnalogClock();
			
			setInterval(() => {
				this.refreshAnalogClock();
			}, 1000);
		}, calMillisec);
	}

	refreshAnalogClock(){
		if(this.refreshAnalogClockBusy){
			return false;
		}
		this.refreshAnalogClockBusy = true;
		var clock = document.getElementById('analogClock');
		var containerWidth = this.roundAFloat(clock.parentElement.clientWidth, 0, false);
		var containerHeight = this.roundAFloat(clock.parentElement.clientHeight, 0, false);
		var padding = 4.0;
		var width = this.roundAFloat(clock.width - padding, 0, false);
		var height = this.roundAFloat(clock.height - padding, 0, false);
		var centerX = this.roundAFloat((width / 2.0) + (padding / 2.0), 0, false);
		var centerY = this.roundAFloat((height / 2.0) + (padding / 2.0), 0, false);
		var radius = this.roundAFloat(width / 2.0, 0, false);
		var ctx = clock.getContext('2d');
		ctx.clearRect(0, 0, clock.width, clock.height);
		this.drawOutline(ctx, radius, centerX, centerY);
		this.drawTickMarks(ctx, radius, centerX, centerY);
		this.drawHands(ctx, radius, centerX, centerY, true);
		this.refreshAnalogClockBusy = false;
	}

	drawOutline(ctx, radius, centerX, centerY){
		ctx.beginPath();
		ctx.arc(centerX, centerY, radius, 0, (Math.PI * 2.0));
	}

	drawTickMarks(ctx, radius, centerX, centerY){
		var tickMarkLength = (radius / 16.0) * 1.5;
		var longTickMarkLength = (radius / 16.0) * 3.0;
	
		for(var i = 0.0; i < 360.0; i += 6.0){
			var radians = i * (Math.PI / 180.0);
			var length = ((parseInt(i) % 30) == 0) ? longTickMarkLength : tickMarkLength;
			var endX = Math.cos(radians);
			var endY = Math.sin(radians);
			var startX = (radius - length) * endX + centerX;
			var startY = (radius - length) * endY + centerY;
			endX = endX * radius + centerX;
			endY = endY * radius + centerY;
	
			ctx.moveTo(startX, startY);
			ctx.lineTo(endX, endY);
		}
	}

	drawHands(ctx, radius, centerX, centerY, seconds){
		var twoPies = this.roundAFloat((Math.PI * 2.0), 0, false);
		var x = this.roundAFloat((radius / 32.0), 0, false);
		var y = 0;
		
		ctx.stroke();
		ctx.beginPath();
		ctx.arc(centerX, centerY, x, y, twoPies);
		ctx.fill();
		ctx.stroke();
		ctx.beginPath();
		
		var now = new Date();
		var hour = now.getHours();
		var minute = now.getMinutes();
		var second = now.getSeconds();
		var secondAngle = (((second * 360.0) / 60.0) - 90.0) * (Math.PI / 180.0);
		var minuteAngle = (((minute * 360.0) / 60.0) - 90.0) * (Math.PI / 180.0);
		var hourAngle = ((((hour * 360.0) / 12.0) + 270.0) * (Math.PI / 180.0)) + ((minuteAngle * twoPies) / 60.0);
		var mendX = ((radius / 16.0 * 10.0) * Math.cos(minuteAngle)) + centerX;
		var mendY = ((radius / 16.0 * 10.0) * Math.sin(minuteAngle)) + centerY;
		var sendX =	((radius / 16.0 * 12.0) * Math.cos(secondAngle)) + centerX;
		var sendY =	((radius / 16.0 * 12.0) * Math.sin(secondAngle)) + centerY;
		var hendX = ((radius / 16.0 * 7.0) * Math.cos(hourAngle)) + centerX;
		var hendY = ((radius / 16.0 * 7.0) * Math.sin(hourAngle)) + centerY;
	
		var points = [ [hendX,hendY], [mendX, mendY] ];
		if(seconds === true){
			points.push([sendX,sendY]);
		}
	
		ctx.stroke();
		ctx.beginPath();
	
		var prevLineWidth = ctx.lineWidth;
		var prevFillStyle = ctx.fillStyle;
		ctx.lineWidth = 1.6;
	
		for(var i = 0; i < points.length; i++){
			ctx.moveTo(centerX, centerY);
			ctx.lineTo(points[i][0], points[i][1]);
		}
	
		ctx.stroke();
		ctx.lineWidth = prevLineWidth;
	}

	roundAFloat(val, ndec, tostring){
		var mul = 1.0;
		var result = null;
		for(var dec = 0; dec < ndec; dec++){
			mul *= 10.0;
		}
		val = Math.round(parseFloat(val) * mul) / mul;
		if(tostring !== false){
			result = val.toString();
			var period = result.indexOf('.');
			if(period == -1){
				if(ndec >= 1){
					result = result + ('.'.padEnd(ndec + 1, '0'));
				}
			}else{
				result = result.padEnd(period + ndec + 1, tostring);
			}
		}else{
			result = val;
		}
		return result;
	}
}
