function gotFWInfo(responseText, responseStatus, responseXML)
{
	if (responseStatus != 200)
		return;

	document.getElementById("update_info").innerHTML = "<pre>" + responseText + "</pre>";
	setTimeout("ReqFWInfo.update('', 'GET')", 5000);
}

function putspan() {
	document.write("<span id='update_info'></span>");
}

putspan();
var ReqFWInfo = new ajaxObject('update_fw_info.txt', gotFWInfo);
setTimeout("ReqFWInfo.update('', 'GET')", 500);


