function gotFWResult(responseText, responseStatus, responseXML)
{
	if (responseStatus == 200) {
		document.getElementById("update_result").innerHTML = responseText;
	}
	setTimeout("ReqFWResult.update('', 'GET')", 5000);
}

function putspan() {
	document.write("<span id='update_result'></span>");
}

putspan();
var ReqFWResult = new ajaxObject('update_fw_result.txt', gotFWResult);
setTimeout("ReqFWResult.update('', 'GET')", 500);


