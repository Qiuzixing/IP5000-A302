/*
** Global variables
*/

function update_node_list(json)
{
	g.NODE_LIST_H = {};
	g.NODE_LIST_C = {};
	g.node_h_cnt = 0;
	g.node_c_cnt = 0;
	var i, j, t;
	var re4tx = new RegExp(LM_PARAM.HOSTNAME_PREFIX + LM_PARAM.HOSTNAME_TX_MIDDLE, 'i');
	var re4rx = new RegExp(LM_PARAM.HOSTNAME_PREFIX + LM_PARAM.HOSTNAME_RX_MIDDLE, 'i');

/* For legacy 'node_list -j'
	for ( i in json ) {
		if (!json.hasOwnProperty(i))
			continue;
		if (json[i].is_host == 'y') {
			g.NODE_LIST_H[g.node_h_cnt] = json[i];
			//g.NODE_LIST_H[g.node_h_cnt].id = String(g.NODE_LIST_H[g.node_h_cnt].host_name.replace(/.+Tx-/i, ""));
			g.NODE_LIST_H[g.node_h_cnt].id = String(g.NODE_LIST_H[g.node_h_cnt].host_name.replace(re4tx, ""));
			g.node_h_cnt++;
		} else {
			g.NODE_LIST_C[g.node_c_cnt] = json[i];
			//g.NODE_LIST_C[g.node_c_cnt].id = String(g.NODE_LIST_C[g.node_c_cnt].host_name.replace(/.+Rx-/i, ""));
			g.NODE_LIST_C[g.node_c_cnt].id = String(g.NODE_LIST_C[g.node_c_cnt].host_name.replace(re4rx, ""));
			g.node_c_cnt++;
		}
	}
*/
	/* For new node_query. */
	for ( i in json.nodes ) {
		if (!json.nodes.hasOwnProperty(i))
			continue;
		if (json.nodes[i].IS_HOST == 'y') {
			g.NODE_LIST_H[g.node_h_cnt] = {};
			g.NODE_LIST_H[g.node_h_cnt].host_name = json.nodes[i].HOSTNAME;
			g.NODE_LIST_H[g.node_h_cnt].ip =  json.nodes[i].MY_IP;
			g.NODE_LIST_H[g.node_h_cnt].status =  json.nodes[i].STATE;
			g.NODE_LIST_H[g.node_h_cnt].is_host =  json.nodes[i].IS_HOST;
			//g.NODE_LIST_H[g.node_h_cnt].id = String(g.NODE_LIST_H[g.node_h_cnt].host_name.replace(/.+Tx-/i, ""));
			g.NODE_LIST_H[g.node_h_cnt].id = String(g.NODE_LIST_H[g.node_h_cnt].host_name.replace(re4tx, ""));
			g.node_h_cnt++;
		} else {
			g.NODE_LIST_C[g.node_c_cnt] = {};
			g.NODE_LIST_C[g.node_c_cnt].host_name = json.nodes[i].HOSTNAME;
			g.NODE_LIST_C[g.node_c_cnt].ip =  json.nodes[i].MY_IP;
			g.NODE_LIST_C[g.node_c_cnt].status =  json.nodes[i].STATE;
			g.NODE_LIST_C[g.node_c_cnt].is_host =  json.nodes[i].IS_HOST;
			//g.NODE_LIST_C[g.node_c_cnt].id = String(g.NODE_LIST_C[g.node_c_cnt].host_name.replace(/.+Rx-/i, ""));
			g.NODE_LIST_C[g.node_c_cnt].id = String(g.NODE_LIST_C[g.node_c_cnt].host_name.replace(re4rx, ""));
			g.node_c_cnt++;
		}
	}

	// bubble Sort host and client
	for ( i = 0; i < g.node_h_cnt; i++ ) {
		if (!g.NODE_LIST_H.hasOwnProperty(i))
			continue;
		for ( j = i + 1; j < g.node_h_cnt; j++) {
			if (!g.NODE_LIST_H.hasOwnProperty(j))
				continue;
			if ( g.NODE_LIST_H[i].id > g.NODE_LIST_H[j].id ) {
				t = g.NODE_LIST_H[j];
				g.NODE_LIST_H[j] = g.NODE_LIST_H[i];
				g.NODE_LIST_H[i] = t;
			}
		}
	}
	for ( i = 0; i < g.node_c_cnt; i++ ) {
		if (!g.NODE_LIST_C.hasOwnProperty(i))
			continue;
		for ( j = i + 1; j < g.node_c_cnt; j++) {
			if (!g.NODE_LIST_C.hasOwnProperty(j))
				continue;
			if ( g.NODE_LIST_C[i].id > g.NODE_LIST_C[j].id ) {
				t = g.NODE_LIST_C[j];
				g.NODE_LIST_C[j] = g.NODE_LIST_C[i];
				g.NODE_LIST_C[i] = t;
			}
		}
	}
	//
	// g.NODE_LIST_C[i].lm_param.MY_MAC requires lm_param query of every nodes. And it is now done in vw_adv_setup page.
	// So that the information is not available at this stage.
	// BruceToDo. Maybe I should do this at this stage?!
	/*
	// Find "This" and its idx
	for ( i in g.NODE_LIST_C ) {
		if (!g.NODE_LIST_C.hasOwnProperty(i))
			continue;
		if (LM_PARAM.MY_MAC == g.NODE_LIST_C[i].lm_param.MY_MAC) {
			g.THIS_NODE = g.NODE_LIST_C[i];
			g.THIS_NODE_IDX = i;
		}
	}
	for ( i in g.NODE_LIST_H ) {
		if (!g.NODE_LIST_H.hasOwnProperty(i))
			continue;
		if (LM_PARAM.MY_MAC == g.NODE_LIST_H[i].lm_param.MY_MAC) {
			g.THIS_NODE = g.NODE_LIST_H[i];
			g.THIS_NODE_IDX = i;
		}
	}
	*/

	// Calling ui_refresh_select_Node() is necessary because ajax will not always be completed on "document ready".
	ui_refresh_select_Node();
}

function query_nodes(callback) {
	var host = host_ip();
	var _c = $.param({
		cache: false,
		nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
		//cmd: "node_list -j"
		cmd: "node_query --get_key IS_HOST --get_key MY_MAC --get_key HOSTNAME --get_key MY_IP --get_key STATE --json"
	});

	$.getJSON(host+CGI_PATH+"query.cgi?callback=?", _c, function(){
	})
	.error(function(){
		error("#dummy_hdr_main", "Error!", "Can't get NODE_LIST");
		if (TEST_ONLY) {
			setTimeout(function(){
				update_node_list(dummy_nodes);
			}, 1000);
		}
	})
	.complete(function(){
		if (typeof callback != "undefined")
			callback();
	})
	.success(function(json){
		//$.each(json, function(key, val) {
		//	alert("key:"+key+"\nval:"+val);
		//});
		update_node_list(json);
	});
}


function osd_show_number(show)
{
	var host, cmd, _c, i;

	for ( i in g.NODE_LIST_C ) {
		host = "http://" + g.NODE_LIST_C[i].ip + VW_PATH;
		cmd = (show)?("big_osd_on.sh " + i):("osd_off.sh 0");

		_c = $.param({
			cache: false,
			nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
			cmd: cmd
		});
		$.getJSON(host+CGI_PATH+"query.cgi?callback=?", _c, function(){
		})
		.error(function(){error("#dummy_hdr_main", "Error!", "");})
		.complete(function(){
		})
		.success(function(json){
		});
	}
}

