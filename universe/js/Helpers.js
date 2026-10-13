/*
	Author Jacob A. Psimos
*/

function FloatDelta(a, b)
{
	var A = Math.abs(a);
	var B = Math.abs(b);
	var d = Math.abs(A < B ? (b - a) : (a - b));
	return d;
}

function FloatRoundPrecise(val, numDecPoints)
{
	var mul = 1.0;
	val = parseFloat(val);
	for(var dec = 0; dec < numDecPoints; dec++){
		mul *= 10.0;
	}
	val = Math.round(parseFloat(val) * mul) / mul;
	return val;
}