/*
** Global variables
*/
// Following path are defined related to root path
var CGI_PATH="../cgi-bin/";


function query_vw_var_tail_idx(callback) {
	var cmd = $.param({
		cache: false,
		nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
		cmd: "e e_debug"
	});
	$.get(CGI_PATH+"sh.cgi", cmd)
		.error(function(){
			error("#dummy_hdr_main", "Error!", "Can't get VW_VAR_TAIL_IDX");
			if (typeof callback != "undefined")
				callback(LM_PARAM.VW_VAR_TAIL_IDX);
		})
		.complete(function(){})
		.success(function(){
			var _c = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "cat /usr/local/bin/lm_params 2>/dev/null | grep VW_VAR_TAIL_IDX= | ini2json.sh"
			});
			$.getJSON(CGI_PATH+"query.cgi", _c, function(){
			})
			.error(function(){error("#dummy_hdr_main", "Error!", "Can't get VW_VAR_TAIL_IDX?");})
			.complete(function(){
				if (typeof callback != "undefined")
					callback(LM_PARAM.VW_VAR_TAIL_IDX);
			})
			.success(function(json){
				//$.each(json, function(key, val) {
				//	alert("key:"+key+"\nval:"+val);
				//});
				LM_PARAM.VW_VAR_TAIL_IDX = json.VW_VAR_TAIL_IDX;
			});
		});
	return LM_PARAM.VW_VAR_TAIL_IDX;
}
