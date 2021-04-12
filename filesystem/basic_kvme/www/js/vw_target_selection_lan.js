
function ui_vw_adv_update_target_selection_label(NODE, idx)
{
	if (typeof NODE == "undefined") return;
	if (typeof NODE.is_host == "undefined") return;

	if (NODE.is_host == 'y') {
		// Host code, Make sure we always show "This"
		var $b = $("#host_r"+ (Number(NODE.lm_param.VW_VAR_POS_IDX) % (g.vw_adv_current_max_row + 1)));
		if (LM_PARAM.MY_MAC == NODE.lm_param.MY_MAC) {
			$b.button("option", "label", "This").button("enable");
			g.THIS_NODE = NODE;
			g.THIS_NODE_IDX = idx;
			ui_vw_adv_update_apply_fields(NODE, idx);
		} else if ($b.button("option", "disabled") && $b.button("option", "label") != "This" ) {
			$b.button("option", "label", NODE.id).button("enable");
		} else {
			error("#dummy_hdr_main", "Error!", "Node:" + NODE.lm_param.MY_IP + " has duplicate position index: R" + NODE.lm_param.VW_VAR_POS_IDX);
		}
	} else {
		// Client code
		var r = Number(NODE.lm_param.VW_VAR_POS_R) % (g.vw_adv_current_max_row + 1),
			c = Number(NODE.lm_param.VW_VAR_POS_C) % (g.vw_adv_current_max_col + 1),
			$b = $("#client_r"+r+"c"+c);
		if (LM_PARAM.MY_MAC == NODE.lm_param.MY_MAC) {
			$b.button("option", "label", "This").button("enable");
			g.THIS_NODE = NODE;
			g.THIS_NODE_IDX = idx;
			ui_vw_adv_update_apply_fields(NODE, idx);
		} else if ($b.button("option", "disabled") && $b.button("option", "label") != "This" ) {
			$b.button("option", "label", idx).button("enable");
		} else {
			error("#dummy_hdr_main", "Error!", "Node:" + NODE.lm_param.MY_IP + " has duplicate position index: " + NODE.lm_param.VW_VAR_POS_IDX);
		}
	}

}

function vw_adv_query_lm_param(NODE, idx, callback)
{
	var host = "http://" + NODE.ip + VW_PATH;
	var cmd = $.param({
		cache: false,
		nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
		cmd: "e e_debug_json"
	});
	$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
		.error(function(){
			error("#dummy_hdr_main", "Error!", "Can't get LM_PARAM");
		})
		.complete(function(){})
		.success(function(){
			var _c = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "cat /usr/local/bin/lm_params_json 2>/dev/null"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", _c, function(){
			})
			.error(function(){
				error("#dummy_hdr_main", "Error!", "Can't get LM_PARAM");
			})
			.complete(function(){})
			.success(function(json){
				NODE.lm_param = json;
				if (typeof callback != "undefined")
					callback(NODE, idx);
			});
		});
	if (TEST_ONLY) {
		setTimeout(function(){
			if (NODE.is_host == 'y') {
				NODE.lm_param = dummy_lm_param_h[idx];
			} else {
				NODE.lm_param = dummy_lm_param_c[idx];
			}
			if (typeof callback != "undefined")
				callback(NODE, idx);
		}, 1500);
	}
	
}

function ui_vw_adv_update_apply_fields(NODE, idx)
{
	var p = NODE.lm_param;
	//Update the NODE's lm_param to the corresponding fields
	if (NODE.is_host == 'n') {
		$("#select_vw_adv_StretchType").attr("value", p.VW_VAR_STRETCH_TYPE);
		$("#select_vw_adv_Rotate").attr("value", p.VW_VAR_ROTATE);
	}
	$("#select_vw_adv_MaxRow").attr("value", p.VW_VAR_MAX_ROW);
	$("#select_vw_adv_MaxCol").attr("value", p.VW_VAR_MAX_COLUMN);
	$("#select_vw_adv_RowNum").attr("value", p.VW_VAR_ROW);
	$("#select_vw_adv_ColNum").attr("value", p.VW_VAR_COLUMN);
	var v = Number(p.VW_VAR_H_SHIFT);
	if (v >= 0) {
		// shift right
		$("#vw_adv_h_shift_right").attr("checked", true);
		$("#vw_adv_h_shift_direction").buttonset("refresh");
		$("#text_vw_adv_h_shift").attr("value", v);
	} else {
		// shift left
		$("#vw_adv_h_shift_left").attr("checked", true);
		$("#vw_adv_h_shift_direction").buttonset("refresh");
		$("#text_vw_adv_h_shift").attr("value", -v);
	}
	v = Number(p.VW_VAR_V_SHIFT);
	if (v >= 0) {
		// shift down
		$("#vw_adv_v_shift_down").attr("checked", true);
		$("#vw_adv_v_shift_direction").buttonset("refresh");
		$("#text_vw_adv_v_shift").attr("value", v);
	} else {
		// shift up
		$("#vw_adv_v_shift_up").attr("checked", true);
		$("#vw_adv_v_shift_direction").buttonset("refresh");
		$("#text_vw_adv_v_shift").attr("value", -v);
	}
	$("#text_vw_adv_h_scale_up").attr("value", p.VW_VAR_H_SCALE);
	$("#text_vw_adv_v_scale_up").attr("value", p.VW_VAR_V_SCALE);
	$("#text_vw_adv_delay_kick").attr("value", p.VW_VAR_DELAY_KICK);

	g.THE_NODE = NODE;
	g.THE_NODE_IDX = idx;
}

function ui_vw_adv_refresh_apply_fields()
{
	var refresh = function(NODE, idx){
		ui_vw_adv_update_apply_fields(NODE, idx);
		g.$dialog_loading.dialog('close');
	}

	var idx = 0;
	//Decide which NODE to use
	// Default is "This" -> first host -> first client
	if ((g.vw_adv_setup_target_h.length + g.vw_adv_setup_target_c.length) <= 1) {
		if (g.vw_adv_setup_target_h.length) {
			// select first host
			idx = g.vw_adv_setup_target_h[0].idx;
			if (g.NODE_LIST_H[idx] != g.THE_NODE) {
				g.$dialog_loading.dialog('open');
				setTimeout(function(){
					vw_adv_query_lm_param(g.NODE_LIST_H[idx], idx, refresh);
					}
					, 1);
				//In case of jsonp failed, close "loading..." after 10 seconds
				setTimeout(function(){
					g.$dialog_loading.dialog('close');
				}, 10000);
			}
		} else if (g.vw_adv_setup_target_c.length){
			// select first client
			idx = g.vw_adv_setup_target_c[0].idx;
			if (g.NODE_LIST_C[idx] != g.THE_NODE) {
				g.$dialog_loading.dialog('open');
				setTimeout(function(){
					vw_adv_query_lm_param(g.NODE_LIST_C[idx], idx, refresh);
				}
				, 1);
				//In case of jsonp failed, close "loading..." after 10 seconds
				setTimeout(function(){
					g.$dialog_loading.dialog('close');
				}, 10000);
			}
		} else {
			// Don't refresh when there is no target selected.
			/*
			if (g.THIS_NODE != g.THE_NODE) {
				g.$dialog_loading.dialog('open');
				setTimeout(function(){
					vw_adv_query_lm_param(g.THIS_NODE, g.THIS_NODE_IDX, refresh);
				}
				, 1);
				//In case of jsonp failed, close "loading..." after 10 seconds
				setTimeout(function(){
					g.$dialog_loading.dialog('close');
				}, 10000);
			}
			*/
		}
	}
	
}

function ui_vw_adv_refresh_apply_button()
{
	g.$vw_setup_adv_step2_all_apply_btn.button("disable");
	$("#vw_adv_h_shift_direction").buttonset("disable");
	$("#vw_adv_v_shift_direction").buttonset("disable");
	if (g.vw_adv_setup_target_h.length) {
		// Host selected
		//alert("Host:" + VW_ADV_SETUP_TARGET);
		$("#button_vw_adv_reset_to_basic_apply").button("enable");
		$("#button_vw_adv_layout_apply").button("enable");
		$("#button_vw_adv_rownum_apply").button("enable");
		//$("#button_vw_adv_colnum_apply").button("enable");
		//$("#button_vw_adv_h_shift_apply").button("enable");
		$("#button_vw_adv_v_shift_apply").button("enable");
		$("#vw_adv_v_shift_direction").buttonset("enable");
		//$("#button_vw_adv_h_scalue_up_apply").button("enable");
		//$("#button_vw_adv_v_scalue_up_apply").button("enable");
		//$("#button_vw_adv_delay_kick_apply").button("enable");
		$("#button_vw_adv_console_api_apply").button("enable");
	} 
	if (g.vw_adv_setup_target_c.length) {
		// Client selected
		//alert("Client:" + VW_ADV_SETUP_TARGET);
		$("#button_vw_adv_reset_to_basic_apply").button("enable");
		$("#button_vw_adv_stretch_apply").button("enable");
		$("#button_vw_adv_rotate_apply").button("enable");
		$("#button_vw_adv_layout_apply").button("enable");
		$("#button_vw_adv_rownum_apply").button("enable");
		$("#button_vw_adv_colnum_apply").button("enable");
		$("#button_vw_adv_h_shift_apply").button("enable");
		$("#vw_adv_h_shift_direction").buttonset("enable");
		$("#button_vw_adv_v_shift_apply").button("enable");
		$("#vw_adv_v_shift_direction").buttonset("enable");
		$("#button_vw_adv_h_scalue_up_apply").button("enable");
		$("#button_vw_adv_v_scalue_up_apply").button("enable");
		$("#button_vw_adv_delay_kick_apply").button("enable");
		$("#button_vw_adv_console_api_apply").button("enable");
	}
}

function vw_adv_query_targets()
{
	for ( var i in g.NODE_LIST_H ) {
		if (!g.NODE_LIST_H.hasOwnProperty(i))
			continue;

		vw_adv_query_lm_param(g.NODE_LIST_H[i], i, ui_vw_adv_update_target_selection_label);
	}

	for ( var i in g.NODE_LIST_C ) {
		if (!g.NODE_LIST_C.hasOwnProperty(i))
			continue;

		vw_adv_query_lm_param(g.NODE_LIST_C[i], i, ui_vw_adv_update_target_selection_label);
	}

}

function vw_adv_show_target_osd(show, str, host)
{
	var cmd, _c;
	
	cmd = (show)?("big_osd_on.sh " + str):("osd_off.sh 0");

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

function vw_adv_update_target_list()
{
	var $targets = g.$vw_adv_setup_all_chkbox.filter(":checked"),
		row_size = (g.vw_adv_current_max_row + 1),
		col_size = (g.vw_adv_current_max_col + 1),
		i = 0, j = 0, label = "",
		$t = null;

	g.vw_adv_setup_target_h = [];
	g.vw_adv_setup_target_c = [];
	if (typeof $targets == "undefined") return;

	// create host list like: http://169.254.xxx.yyy/vw_path/
	for (i = 0; i < row_size; i++) {
		$t = $("#host_r"+i);

		if (!$t.button("option", "disabled") && $t.attr('checked')) {
			label = $t.button("option", "label");
			var found = false;
			// We match the "id" like 0010 instead of NODE_LIST_H index
			for ( j in g.NODE_LIST_H ) {
				if (!g.NODE_LIST_H.hasOwnProperty(j))
					continue;
				if (label == "This" && g.NODE_LIST_H[j].lm_param.MY_MAC == LM_PARAM.MY_MAC) {
					found = true;
					break;
				}
				else if (g.NODE_LIST_H[j].id == label) {
					found = true;
					break;
				}
			}
			if (!found || !g.NODE_LIST_H.hasOwnProperty(j))
				continue;
			
			g.vw_adv_setup_target_h[g.vw_adv_setup_target_h.length] = {
				host_name : "http://" + g.NODE_LIST_H[j].ip + VW_PATH,
				idx: j
			};
		}
	}

	// create client list like: http://169.254.xxx.yyy/vw_path/
	for (i = 0; i < row_size; i++) {
		for (j = 0; j < col_size; j++) {
			$t = $("#client_r"+i+"c"+j);
			label = $t.button("option", "label");

			if (label == "This") {
				// Find and replace "This" with correct label.
				for (var v in g.NODE_LIST_C) {
					if (g.NODE_LIST_C[v].lm_param.MY_MAC == LM_PARAM.MY_MAC) {
						label = v;
						break;
					}
				}
			}
			if (!g.NODE_LIST_C.hasOwnProperty(label))
				continue;
			var host_name = "http://" + g.NODE_LIST_C[label].ip + VW_PATH;

			if (!$t.button("option", "disabled") && $t.attr('checked')) {
				g.vw_adv_setup_target_c[g.vw_adv_setup_target_c.length] = {
					host_name : host_name,
					idx : label
				};
				vw_adv_show_target_osd((g.$chkbox_vw_adv_show_osd.attr('checked')), label, host_name);
			} else {
				vw_adv_show_target_osd(false, label, host_name);
			}
		}
	}
	
	ui_vw_adv_refresh_apply_fields();
	ui_vw_adv_refresh_apply_button();
}

function vw_adv_on_change_target_list(id)
{
	var $t = null,
		j = 0,
		is_host = false,
		do_refresh = false;
	if (typeof id == "undefined") return;

	if ( id.substr(0, 5) == "host_" )
		is_host = true;

	$t = $("#"+id);

	if ($t.button("option", "disabled"))
		return;

	if (is_host) {
		label = $t.button("option", "label");
		var found = false;
		// We match the "id" like 0010 instead of NODE_LIST_H index
		for ( j in g.NODE_LIST_H ) {
			if (!g.NODE_LIST_H.hasOwnProperty(j))
				continue;
			if (label == "This" && g.NODE_LIST_H[j].lm_param.MY_MAC == LM_PARAM.MY_MAC) {
				found = true;
				break;
			}
			else if (g.NODE_LIST_H[j].id == label) {
				found = true;
				break;
			}
		}
		if (!found || !g.NODE_LIST_H.hasOwnProperty(j))
			return;
		
		if ($t.attr('checked')) {
			if (g.vw_adv_setup_target_h.length == 0)
				do_refresh = true;
			g.vw_adv_setup_target_h[g.vw_adv_setup_target_h.length] = {
				host_name : "http://" + g.NODE_LIST_H[j].ip + VW_PATH,
				idx: j
			};
		} else {
			//remove this element
			for (var idx in g.vw_adv_setup_target_h) {
				if (g.vw_adv_setup_target_h[idx].idx == j) {
					if (g.vw_adv_setup_target_h.length == 1)
						do_refresh = true;
					g.vw_adv_setup_target_h.splice(idx, 1);
					break;
				}
			}
		}
	} else { //is_client
		label = $t.button("option", "label");

		if (label == "This") {
			// Find and replace "This" with correct label.
			for (var v in g.NODE_LIST_C) {
				if (g.NODE_LIST_C[v].lm_param.MY_MAC == LM_PARAM.MY_MAC) {
					label = v;
					break;
				}
			}
		}
		if (!g.NODE_LIST_C.hasOwnProperty(label))
			return;
		var host_name = "http://" + g.NODE_LIST_C[label].ip + VW_PATH;

		if ($t.attr('checked')) {
			if (g.vw_adv_setup_target_c.length == 0)
				do_refresh = true;

			g.vw_adv_setup_target_c[g.vw_adv_setup_target_c.length] = {
				host_name : host_name,
				idx : label
			};
			vw_adv_show_target_osd((g.$chkbox_vw_adv_show_osd.attr('checked')), label, host_name);
		} else {
			if (g.$chkbox_vw_adv_show_osd.attr('checked'))
				vw_adv_show_target_osd(false, label, host_name);
			//remove this element
			if (g.vw_adv_setup_target_c.length == 1)
				do_refresh = true;

			for (var idx in g.vw_adv_setup_target_c) {
				if (g.vw_adv_setup_target_c[idx].idx == label) {
					g.vw_adv_setup_target_c.splice(idx, 1);
					break;
				}
			}
		}
	}
	ui_vw_adv_refresh_apply_fields();
	
	if (do_refresh)
		ui_vw_adv_refresh_apply_button();
}

function vw_adv_on_change_show_osd()
{
	for (var idx in g.vw_adv_setup_target_c) {
		var label = g.vw_adv_setup_target_c[idx].idx;
		var host_name = g.vw_adv_setup_target_c[idx].host_name;
		vw_adv_show_target_osd((g.$chkbox_vw_adv_show_osd.attr('checked')), label, host_name);
	}
}

function _vw_adv_target_selection_init()
{
	if (typeof LM_PARAM == "undefined") return;
	if (typeof LM_PARAM.VW_VAR_POS_MAX_COL == "undefined") return;
	if (typeof LM_PARAM.VW_VAR_POS_MAX_ROW == "undefined") return;

	/* Clear the list in case the list is not empty. */
	g.vw_adv_setup_target_h = [];
	g.vw_adv_setup_target_c = [];

	var max_row = ((Number(LM_PARAM.VW_VAR_POS_MAX_ROW) + 1) > Number(g.node_h_cnt))?(Number(LM_PARAM.VW_VAR_POS_MAX_ROW)):(Number(g.node_h_cnt) - 1);
	
	/* The pos_idx may change. So we need to re-draw and query again.
	if ((g.vw_adv_current_max_row == max_row) && (g.vw_adv_current_max_col == Number(LM_PARAM.VW_VAR_POS_MAX_COL))) {
		g.$dialog_loading.dialog('close');
		return;
	}
	*/
	g.vw_adv_current_max_row = max_row;
	g.vw_adv_current_max_col = Number(LM_PARAM.VW_VAR_POS_MAX_COL);

	var $tbody_host = $("#tbody_vw_adv_target_select_host"),
		$thead_client = $("#thead_vw_adv_target_select_client"),
		$tbody_client = $("#tbody_vw_adv_target_select_client"),
		$all_chkbox,
		tmp = [],
		tmp1 = [];

	//
	// Start re-drawing the target selection table
	//
	//$tbody_host.empty();
	//$thead_client.empty();
	//$tbody_client.empty();

	tmp[tmp.length] = "<thead id=\"thead_vw_adv_target_select_client\">"
	tmp[tmp.length] = "<tr>";
	tmp[tmp.length] = "<th class=\"tb-button\"><button type=\"button\" id=\"btn_all_client\">All clients</button></th>";
	for (var cidx = 0; cidx <= LM_PARAM.VW_VAR_POS_MAX_COL; cidx++) {
		tmp[tmp.length] = "\
			<th><button type=\"button\" id=\"btn_all_c"+cidx+"\" class=\"button-whole-col\" name=\"c"+cidx+"\">All column "+cidx+" clients</button></th>";
	}
	tmp[tmp.length] = "</tr>"
	tmp[tmp.length] = "</thead>"
	//$thead_client.append(tmp.join(''));
	$thead_client.replaceWith(tmp.join(''));
	tmp = [];
	tmp1 = [];
	
	tmp[tmp.length] = "<tbody id=\"tbody_vw_adv_target_select_host\">"
	tmp1[tmp1.length] = "<tbody id=\"tbody_vw_adv_target_select_client\">"
	for (var ridx = 0; ridx <= g.vw_adv_current_max_row; ridx++) {
		id = "r" + ridx;
		tmp[tmp.length] = "\
			<tr> \
				<td class=\"tb-button\"><button type=\"button\" class=\"button-whole-row-with-host\" name=\""+id+"\">All row "+ridx+" host and clients</button></td> \
				<td><input type=\"checkbox\" id=\"host_"+id+"\" /><label for=\"host_"+id+"\">R"+ridx+"</label></td> \
			</tr> \
			";
		tmp1[tmp1.length] = "\
			<tr> \
				<td class=\"tb-button\"><button type=\"button\" class=\"button-whole-row\" name=\""+id+"\">All row "+ridx+" clients</button></td> \
			";
		for (var cidx = 0; cidx <= LM_PARAM.VW_VAR_POS_MAX_COL; cidx++) {
			var cid = id + "c" + cidx;
			tmp1[tmp1.length] = "\
				<td><input type=\"checkbox\" id=\"client_"+cid+"\" value=\""+cid+"\"/><label for=\"client_"+cid+"\">"+cid+"</label></td>";
		}
		tmp1[tmp1.length] = "</tr>";
	}
	tmp[tmp.length] = "</tbody>"
	tmp1[tmp1.length] = "</tbody>"
	//$tbody_host.append(tmp.join(''));
	//$tbody_client.append(tmp1.join(''));
	$tbody_host.replaceWith(tmp.join(''));
	$tbody_client.replaceWith(tmp1.join(''));

	//
	// Buttons construction
	//
	g.$vw_adv_setup_all_chkbox = $all_chkbox = $("#target_selection :checkbox");
	$all_chkbox.button({disabled: true}); // !!!!This line takes most of time in this function.

	$("#btn_all_host_client")
		.button({
			icons: {
				primary: "ui-icon-triangle-1-se"
			},
			text: false
		})
		.click(function(){
			$all_chkbox//.click().button('refresh').change();
				.attr('checked', function(index, attr){
					//Toggle the value
					return !attr;
				})
				.button('refresh')
				.change();
			//vw_adv_update_target_list();
		});

	$("#btn_all_host")
		.button({
			icons: {
				primary: "ui-icon-triangle-1-s"
			},
			text: false
		})
		.click(function(){
			$all_chkbox.filter("[id^='host_']")//.click().button('refresh').change();
				.attr('checked', function(index, attr){
					//Toggle the value
					return !attr;
				})
				.button('refresh')
				.change();
			//vw_adv_update_target_list();
		});

	$("#btn_all_client")
		.button({
			icons: {
				primary: "ui-icon-triangle-1-se"
			},
			text: false
		})
		.click(function(){
			$all_chkbox.filter("[id^='client_']")//.click().button('refresh').change();
				.attr('checked', function(index, attr){
					//Toggle the value
					return !attr;
				})
				.button('refresh')
				.change();
			//vw_adv_update_target_list();
		});

	$("#tbody_vw_adv_target_select_host button"/*.button-whole-row-with-host"*/)
		.button({
			icons: {
				primary: "ui-icon-triangle-1-e"
			},
			text: false
		})
		.click(function(){
			var rowId = $(this).attr('name');
			$("#target_selection button.button-whole-row[name='" + rowId + "']").click();
			$("#host_" + rowId)//.click().button('refresh').change();
				.attr('checked', function(index, attr){
					return !attr;
				})
				.button('refresh')
				.change();
			//vw_adv_update_target_list();
		});

	$("#tbody_vw_adv_target_select_client button"/*.button-whole-row"*/)
		.button({
			icons: {
				primary: "ui-icon-triangle-1-e"
			},
			text: false
		})
		.click(function(){
			//IE8(JQuery?) Bug. The attr('value') is not correct under IE8. Use attr('name') instead.
			//var rowId = $(this).val();
			var rowId = $(this).attr('name');
			$all_chkbox.filter("[id^='client_" + rowId + "']")//.click().button('refresh').change();
				.attr('checked', function(index, attr){
					//Toggle the value
					return !attr;
				})
				.button('refresh')
				.change();
			//vw_adv_update_target_list();
		});
	$("#thead_vw_adv_target_select_client button.button-whole-col")
		.button({
			icons: {
				primary: "ui-icon-triangle-1-s"
			},
			text: false
		})
		.click(function(){
			var colId = $(this).attr('name');
			$all_chkbox.filter("[id*='" + colId + "']")//.click().button('refresh').change();
				.attr('checked', function(index, attr){
					//Toggle the value
					return !attr;
				})
				.button('refresh')
				.change();
			//vw_adv_update_target_list();
		});
/*
		if ($.browser.msie) {
			// This is a hack for IE. See. http://stackoverflow.com/questions/208471/getting-jquery-to-recognise-change-in-ie
			$(":input[type='checkbox']").click(function () {
				this.blur();
				this.focus();
			});
		}
*/
	//
	// Handle Target Click Event
	//
	g.$vw_adv_setup_all_chkbox.change(function(o){
		//vw_adv_update_target_list(o.target.id);
		vw_adv_on_change_target_list(o.target.id);
	});
	// Disable all apply buttons first.
	ui_vw_adv_refresh_apply_button();
	// Update list button labels.
	vw_adv_query_targets();

	g.$dialog_loading.dialog('close');

}

function vw_adv_target_selection_init()
{
	g.$dialog_loading.dialog('open');
	//
	// IE6 hack. Run following code in seperate thread by setTimeout(xx, 1), 
	// so that the "loading..." message can be seen.
	//
	setTimeout(function(){
		query_lm_param(_vw_adv_target_selection_init);
	}, 1);

}


