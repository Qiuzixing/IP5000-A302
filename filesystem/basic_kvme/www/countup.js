
var CntInterval = 4500; // How many ms will it take to program 64KB data
var FWSizeTotal = 0; //from cgi Bytes into 64KB
var FWSizeRemain = 0; //from cgi Bytes into 64KB
var lastFWSizeTotal = 0; //from cgi Bytes into 64KB
var lastFWSizeRemain = 0; //from cgi Bytes into 64KB
var CountingSize = 0; //Counter's current counting number
var CounterIsActive = false;

function dbgMsg(str)
{
	//document.getElementById("debug").innerHTML += str
}
function startCounting()
{
	dbgMsg("Try to start counter");
	if (CounterIsActive)
		return;

	dbgMsg("Start counter");
	CounterIsActive = true;
	setTimeout('counting_by_size()', CntInterval);
}

function gotSizeTotal(responseText, responseStatus, responseXML)
{
	if (responseStatus != 200)
		return;
	//Update the FWSizeTotal
	eval(responseText);
	
	FWSizeTotal = Math.round(FWSizeTotal/(1024*64));
	dbgMsg("Got SizeTotal as" + FWSizeTotal);
	if (FWSizeTotal != lastFWSizeTotal) {
		lastFWSizeTotal = FWSizeTotal;
		if (lastFWSizeRemain <= lastFWSizeTotal) {
			CountingSize = (lastFWSizeTotal - lastFWSizeRemain);
			startCounting();
		}
	}
	setTimeout("ReqSizeTotal.update('', 'GET')", 1*1000);
}

function gotSizeRemain(responseText, responseStatus, responseXML)
{
	if (responseStatus != 200)
		return;
	//Update the FWSizeRemain
	eval(responseText);
	
	FWSizeRemain = Math.round(FWSizeRemain/(1024*64));
	dbgMsg("Got FWSizeRemain as" + FWSizeRemain);
	if (FWSizeRemain != lastFWSizeRemain) {
		lastFWSizeRemain = FWSizeRemain;
		if (lastFWSizeRemain <= lastFWSizeTotal) {
			CountingSize = (lastFWSizeTotal - lastFWSizeRemain);
			startCounting();
		}
		//Start requesting SizeTotal when SizeRemain got
		ReqSizeTotal.update('', 'GET');
	}
	setTimeout("ReqSizeRemain.update('', 'GET')", 1*1000);
}

function counting_by_size()
{
	if (lastFWSizeTotal <= 0) {
		document.getElementById("cntup").innerHTML = "";
		CounterIsActive = false;
		return;
	}

	if (CountingSize < lastFWSizeTotal) {
		dbgMsg("CountingSize = " + CountingSize);

		CountingSize++;
		//var Progress = Math.round((CountingSize/lastFWSizeTotal)*100);
		//document.getElementById("cntup").innerHTML = CountingSize + "/" + lastFWSizeTotal + " , ";
		//document.getElementById("cntup").innerHTML += Progress + " %";
		var Progress = (CountingSize/lastFWSizeTotal)*100;
		if (false && lastFWSizeTotal > 100 && Progress.toFixed) //javascript v1.5 above
			Progress = Progress.toFixed(2);
		else
			Progress = Math.round(Progress);
		
		document.getElementById("cntup").innerHTML = Progress + " %";
		setTimeout('counting_by_size()', CntInterval);
	}
	else {
		document.getElementById("cntup").innerHTML = "";
		CounterIsActive = false;		
	}
}

function putspan() {
	document.write("<span id='cntup'></span>");
	document.write("<span id='debug'></span>");
}

CounterIsActive = false;
putspan();

var ReqSizeRemain = new ajaxObject('fw_size_remain.js', gotSizeRemain);
var ReqSizeTotal = new ajaxObject('fw_size_total.js', gotSizeTotal);

ReqSizeRemain.update('', 'GET');

