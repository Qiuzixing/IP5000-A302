

function get_node_by_optvalue(value)
{
	if ( value < g.node_h_cnt ) {
		// This is host node
		return g.NODE_LIST_H[value];
	} else {
		// This is client node
		value -= g.node_h_cnt;
		return g.NODE_LIST_C[value];
	}
}


function ui_refresh_select_Node()
{
	/* This is the format
	<select id="select_Node" class="select-item" name="select_Node"> 
		<option value="255.255.255.255">All</option>
		<option value="0.0.0.0">This</option>
		<optgroup label="Hosts:" id="select_Node_host_list">
			<!--<option value="0">0</option>-->
			<!--<option value="1">1</option>-->
			<!--<option value="2">2</option>-->
		</optgroup>
		<optgroup label="Clients:" id="select_Node_client_list">
			<!--<option value="3">3</option>-->
			<!--<option value="4">4</option>-->
			<!--<option value="5">5</option>-->
			<!--<option value="6">6</option>-->
			<!--<option value="7">7</option>-->
			<!--<option value="8">8</option>-->
			<!--<option value="9">9</option>-->
			<!--<option value="10">10</option>-->
			<!--<option value="11">11</option>-->
			<!--<option value="12">12</option>-->
			<!--<option value="13">13</option>-->
			<!--<option value="14">14</option>-->
			<!--<option value="15">15</option>-->
		</optgroup>
	</select>
	*/
	
	var $select = $("#select_Node"),
		cnt = 0,
		tmp = [];

	tmp[tmp.length] = "<select id=\"select_Node\" class=\"select-item\" name=\"select_Node\"> "
	if ((g.node_h_cnt + g.node_c_cnt) > 1) {
		tmp[tmp.length] = "<option value=\"255.255.255.255\">All</option>"
	}
	tmp[tmp.length] = "<option value=\"0.0.0.0\">This</option>"
	cnt = 0; //The value of <option> must be unique.
	if (g.node_h_cnt) {
		tmp[tmp.length] = "<optgroup label=\"Hosts:\" id=\"select_Node_host_list\">"
		for ( i in g.NODE_LIST_H ) {
			if (!g.NODE_LIST_H.hasOwnProperty(i))
				continue;
			tmp[tmp.length] = "<option value=\""+ cnt +"\">"+ g.NODE_LIST_H[i].id + ":" + g.NODE_LIST_H[i].ip +"</option>";
			cnt++;
		}
		tmp[tmp.length] = "</optgroup>"
	}
	if (g.node_c_cnt) {
		tmp[tmp.length] = "<optgroup label=\"Clients:\" id=\"select_Node_client_list\">"
		for ( i in g.NODE_LIST_C ) {
			if (!g.NODE_LIST_C.hasOwnProperty(i))
				continue;
			tmp[tmp.length] = "<option value=\""+ cnt +"\">"+ i + ":" + g.NODE_LIST_C[i].ip +"</option>";
			cnt++;
		}
		tmp[tmp.length] = "</optgroup>"
	}
	tmp[tmp.length] = "</select>"

	$select.replaceWith(tmp.join(''));
	//
	// $select is gone after this stage. So, we have to JQuery it again and re-install the change handler.
	//
	$select = $("#select_Node");
	$select.change(function(){
		//alert($("#select_Node").attr("value"));
		var v = $select.attr("value"),
			$label = $("#label_select_Node");
		if (v == "255.255.255.255") {
			$label.html("Apply To: \"All\" device(s) in the list");
		} else if (v == "0.0.0.0") {
				$label.html("Apply To: \"This\" device connected by your browser");
		} else {
				$label.html("Apply To:");
		}
	}).change();
	//alert(tmp.join(''));
}



function vw_layout_apply_host(apply_info)
{
	var maxRow = apply_info.maxRow,
		maxCol = apply_info.maxCol,
		rowPos = apply_info.rowPos,
		colPos = apply_info.colPos,
		singleHostMode = apply_info.singleHostMode,
		ht = apply_info.ht,
		vt = apply_info.vt,
		ha = apply_info.ha,
		va = apply_info.va,
		auto_id = apply_info.auto_id,
		posidx = apply_info.force_posidx,
		tailidx = apply_info.force_tailidx,
		host = apply_info.host;

	if (!auto_id) {
		posidx = rowPos;
		tailidx = rowPos;
	} else {
		rowPos = posidx;
	}
	var cmd = $.param({
		cache: false,
		nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
		cmd: "e e_vw_pos_layout_" + maxRow + "_" + maxCol 
		     + ";e e_vw_enable_" + ((singleHostMode)?"0":maxRow) + "_" + ((singleHostMode)?"0":maxCol) + "_" + rowPos + "_" + colPos 
		     + ";e e_vw_moninfo_" + ha + "_" + ht + "_" + va + "_" + vt
		     + ";e e_vw_refresh_pos_idx_force_" + ((singleHostMode)?"0":posidx) + "_" + ((singleHostMode)?"0":tailidx)
/*
		     + ";astparam s vw_pos_idx " + rowPos
		     + ";astparam s vw_tail_idx " + rowPos
		     + ";astparam save"
*/
	});
	$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
		.error(function(){error("#dummy_hdr_main", "Error:", "Failed to apply change?! Check network.");})
		.complete(function(){
		})
		.success(function(json){
		});
}

function vw_layout_apply_client(apply_info)
{
	var maxRow = apply_info.maxRow,
		maxCol = apply_info.maxCol,
		rowPos = apply_info.rowPos,
		colPos = apply_info.colPos,
		singleHostMode = apply_info.singleHostMode,
		ht = apply_info.ht,
		vt = apply_info.vt,
		ha = apply_info.ha,
		va = apply_info.va,
		stretch_type = apply_info.stretch_type,
		rotate = apply_info.rotate,
		auto_id = apply_info.auto_id,
		host = apply_info.host;

	var cmd = $.param({
		cache: false,
		nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
		cmd: "e e_vw_refresh_pos_idx_" + ((auto_id)?"x":rowPos) + "_" + ((auto_id)?"x":colPos) 
		     + ";e e_vw_moninfo_" + ha + "_" + ht + "_" + va + "_" + vt
		     + ";e e_vw_stretch_type_" + stretch_type
		     + ";e e_vw_rotate_" + rotate
		     + ";e e_vw_enable_" + maxRow + "_" + maxCol + "_" + ((singleHostMode)?rowPos:"0") + "_" + ((auto_id)?"x":colPos)
		     + ";e e_vw_pos_layout_" + maxRow + "_" + maxCol
	});
	$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
		.error(function(){error("#dummy_hdr_main", "Error:", "Failed to apply change?! Check network.");})
		.complete(function(){
		})
		.success(function(json){
		});
}

function vw_layout_apply()
{
	var apply_info = {},
		i,
		regExp=/^\d+$/i, //Match for all digit
		target = $("#select_Node").attr("value");

	apply_info.maxRow = $("#select_MaxRow").attr("value");
	apply_info.maxCol = $("#select_MaxCol").attr("value");
	apply_info.rowPos = $("#select_RowPos").attr("value");
	apply_info.colPos = $("#select_ColPos").attr("value");
	apply_info.singleHostMode = g.$chkbox_vw_basic_single_host_mode.attr('checked');
	apply_info.ht = $("#vw_HT").attr("value");
	apply_info.vt = $("#vw_VT").attr("value");
	apply_info.ha = $("#vw_HA").attr("value");
	apply_info.va = $("#vw_VA").attr("value");
	apply_info.stretch_type = $("#select_StretchType").attr("value");
	apply_info.rotate = $("#select_Rotate").attr("value");

	/* Start of monitor info validation */
	if (!regExp.test(apply_info.ht)) {
		error("#dummy_hdr_main", "Error:", "Outside width(OW) is not a valid integer number!");
		return;
	}
	if (!regExp.test(apply_info.vt)) {
		error("#dummy_hdr_main", "Error:", "Outside height(OH) is not a valid integer number!");
		return;
	}
	if (!regExp.test(apply_info.ha)) {
		error("#dummy_hdr_main", "Error:", "Viewable width(VW) is not a valid integer number!");
		return;
	}
	if (!regExp.test(apply_info.va)) {
		error("#dummy_hdr_main", "Error:", "Viewable height(VH) is not a valid integer number!");
		return;
	}
	apply_info.ht = parseInt(apply_info.ht);
	if (isNaN(apply_info.ht)) {
		error("#dummy_hdr_main", "Error:", "Outside width(OW) is not a valid integer number!");
		return;
	}
	apply_info.vt = parseInt(apply_info.vt);
	if (isNaN(apply_info.vt)) {
		error("#dummy_hdr_main", "Error:", "Outside height(OH) is not a valid integer number!");
		return;
	}
	apply_info.ha = parseInt(apply_info.ha);
	if (isNaN(apply_info.ha)) {
		error("#dummy_hdr_main", "Error:", "Viewable width(VW) is not a valid integer number!");
		return;
	}
	apply_info.va = parseInt(apply_info.va);
	if (isNaN(apply_info.va)) {
		error("#dummy_hdr_main", "Error:", "Viewable height(VH) is not a valid integer number!");
		return;
	}
	if (apply_info.ha > apply_info.ht) {
		error("#dummy_hdr_main", "Error:", "Viewable width(VW) MUST less than Outside width(OW)!");
		return;
	}
	if (apply_info.va > apply_info.vt) {
		error("#dummy_hdr_main", "Error:", "Viewable height(VH) MUST less than Outside height(OH)!");
		return;
	}
	/* End of monitor info validation */

	/* apply */
	apply_info.auto_id = false;
	apply_info.posidx = apply_info.rowPos;
	apply_info.tailidx = apply_info.rowPos;

	if (target == "255.255.255.255") {
		// Apply to every box
		apply_info.auto_id = true;
		for ( i in g.NODE_LIST_H ) {
			if (!g.NODE_LIST_H.hasOwnProperty(i))
				continue;

			apply_info.host = "http://" + g.NODE_LIST_H[i].ip + VW_PATH;
			apply_info.force_posidx = String(i);
			apply_info.force_tailidx = String(g.node_h_cnt - 1);
			vw_layout_apply_host(apply_info);
		}
		for ( i in g.NODE_LIST_C ) {
			if (!g.NODE_LIST_C.hasOwnProperty(i))
				continue;

			apply_info.host = "http://" + g.NODE_LIST_C[i].ip + VW_PATH;
			vw_layout_apply_client(apply_info);
		}
		apply_info.auto_id = false;
	} else if (target == "0.0.0.0") {
		// Apply to the attached box
		apply_info.host = "";
		if (LM_PARAM.IS_HOST == 'y') {
			vw_layout_apply_host(apply_info);
		} else {
			vw_layout_apply_client(apply_info);
		}
	} else {
		// Apply to specified box
		var node = get_node_by_optvalue(target);
		apply_info.host = "http://" + node.ip + VW_PATH;
		if ( node.is_host == "y" ) {
			vw_layout_apply_host(apply_info);
		} else {
			vw_layout_apply_client(apply_info);
		}
	}

	highlight("#dummy_hdr_main", "Change Applied.", "");
/* Useless?!
	LM_PARAM.VW_VAR_MAX_ROW = maxRow;
	LM_PARAM.VW_VAR_MAX_COL = maxCol;
	LM_PARAM.VW_VAR_MONINFO_HT = ht;
	LM_PARAM.VW_VAR_MONINFO_VT = vt;
	LM_PARAM.VW_VAR_MONINFO_HA = ha;
	LM_PARAM.VW_VAR_MONINFO_VA = va;
*/
}


function on_doc_load_vw()
{
	// Should refresh the list "on load" instead of "document ready" event. For IE6.
	ui_refresh_select_Node();

	g.$chkbox_vw_basic_show_osd = $("#chkbox_vw_basic_show_osd"); g.$chkbox_vw_basic_show_osd.change(function(){
		osd_show_number(g.$chkbox_vw_basic_show_osd.attr("checked"));
	});
	//
	// Handle of vw_adv target selection options
	//
	g.$chkbox_vw_adv_show_osd = $("#chkbox_vw_adv_show_osd");
	g.$chkbox_vw_adv_show_osd.change(function(){
		//vw_adv_update_target_list();
		vw_adv_on_change_show_osd();
	});

	//
	// Handle events of video wall accordion change
	//
	$("#accordion").bind("accordionchange", function(event, ui) {
		if (ui.oldHeader.attr('id') == 'accordion_vw_basic_setup') {
			if (g.$chkbox_vw_basic_show_osd.attr("checked")) {
				g.$chkbox_vw_basic_show_osd.attr("checked", false);
				osd_show_number(false);
			}
		}
		if (ui.oldHeader.attr('id') == 'accordion_vw_adv_setup') {
			if (g.$chkbox_vw_adv_show_osd.attr("checked")) {
				g.$chkbox_vw_adv_show_osd.attr("checked", false);
				osd_show_number(false);
			}
		}
		if (ui.newHeader.attr('id') == 'accordion_vw_adv_setup') {
			vw_adv_target_selection_init();
		}
		
	});

	$("#select_Node").change(function(){
		//alert($("#select_Node").attr("value"));
		var v = $("#select_Node").attr("value");
		if (v == "255.255.255.255") {
			$("#label_select_Node").html("Apply To: \"All\" device(s) in the list");
		} else if (v == "0.0.0.0") {
				$("#label_select_Node").html("Apply To: \"This\" device connected by your browser");
		} else {
				$("#label_select_Node").html("Apply To:");
		}
	})
	$("#select_Node").change();

	$("#button_vw_layout_apply").click(function(){
		vw_layout_apply();
	});

	g.$chkbox_vw_basic_single_host_mode = $("#chkbox_vw_basic_single_host_mode"); g.$chkbox_vw_basic_single_host_mode.change(function(){
		highlight("#dummy_hdr_main", "", g.$chkbox_vw_basic_single_host_mode.attr('checked')?"Use Single Host Mode":"Use Multi-Host Mode");
	});

	//
	// Handle of vw_adv apply buttons
	//
	$("#button_vw_adv_reset_to_basic_apply").click(function(){
		var submit = function(host, maxRow, maxCol) {
			if (typeof maxRow == "undefined") return;
			if (typeof maxCol == "undefined") return;
			
			var single_host_mode = $("#chkbox_vw_adv_single_host_mode").attr('checked');
			var cmd_str = (single_host_mode)?("e e_vw_reset_to_pos_s_"):("e e_vw_reset_to_pos_");
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str + maxRow + "_"+ maxCol
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "Reset applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		
		for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
			var t = g.vw_adv_setup_target_h[i];
			submit(t.host_name, g.NODE_LIST_H[t.idx].lm_param.VW_VAR_POS_MAX_ROW, g.NODE_LIST_H[t.idx].lm_param.VW_VAR_POS_MAX_COL);
		}
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, g.NODE_LIST_C[t.idx].lm_param.VW_VAR_POS_MAX_ROW, g.NODE_LIST_C[t.idx].lm_param.VW_VAR_POS_MAX_COL);
		}
		
	});

	$("#button_vw_adv_stretch_apply").click(function(){
		var stretch_type = $("#select_vw_adv_StretchType").attr("value");
		var submit =  function(host, stretch_type) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_stretch_type_" + stretch_type + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New stretch type applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, stretch_type);
		}
	});

	$("#button_vw_adv_rotate_apply").click(function(){
		var rotate = $("#select_vw_adv_Rotate").attr("value");
		var submit =  function(host, rotate) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_rotate_" + rotate + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New rotate type applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, rotate);
		}
	});

	$("#button_vw_adv_layout_apply").click(function(){
		var maxRow = $("#select_vw_adv_MaxRow").attr("value"),
			maxCol = $("#select_vw_adv_MaxCol").attr("value");

		var submit = function(host, maxRow, maxCol) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_enable_" + maxRow + "_"+ maxCol + "_x_x;"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New layout applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};

		if (LM_PARAM.SOC_OP_MODE == 1) {
			for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
				var t = g.vw_adv_setup_target_h[i];
				submit(t.host_name, maxRow, maxCol);
			}
		}
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, maxRow, maxCol);
		}
	});

	$("#button_vw_adv_rownum_apply").click(function(){
		var rowNum = $("#select_vw_adv_RowNum").attr("value");
		var submit = function(host, rowNum) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_enable_x_x_" + rowNum + "_x;"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New row position applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		if (LM_PARAM.SOC_OP_MODE == 1) {
			for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
				var t = g.vw_adv_setup_target_h[i];
				submit(t.host_name, rowNum);
			}
		}
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, rowNum);
		}
	});

	$("#button_vw_adv_colnum_apply").click(function(){
		var colNum = $("#select_vw_adv_ColNum").attr("value");
		var submit = function(host, colNum) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_enable_x_x_x_" + colNum + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New column position applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		/* Host doesn't support colNum
		for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
			var t = g.vw_adv_setup_target_h[i];
			submit(t.host_name, rowNum);
		}
		*/
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, colNum);
		}
	});

	$("#button_vw_adv_h_shift_apply").click(function(){
		var dir = $("input:radio[name=vw_adv_h_shift_direction]:checked").val(),
			shift = $("#text_vw_adv_h_shift").val();
		var submit = function(host, dir, shift) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_h_shift_" + dir + "_" + shift + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New horizontal shift value applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		/* Host doesn't support colNum
		for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
			var t = g.vw_adv_setup_target_h[i];
			submit(t.host_name, dir, shift);
		}
		*/
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, dir, shift);
		}
	});

	$("#button_vw_adv_v_shift_apply").click(function(){
		var dir = $("input:radio[name=vw_adv_v_shift_direction]:checked").val(),
			shift = $("#text_vw_adv_v_shift").val();
		var submit =  function(host, dir, shift) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_v_shift_" + dir + "_" + shift + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New vertical shift value applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		if (LM_PARAM.SOC_OP_MODE == 1) {
			for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
				var t = g.vw_adv_setup_target_h[i];
				submit(t.host_name, dir, shift);
			}
		}
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, dir, shift);
		}
	});

	$("#button_vw_adv_h_scalue_up_apply").click(function(){
		var scale = $("#text_vw_adv_h_scale_up").val();
		var submit =  function(host, scale){
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_h_scale_" + scale + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New horizontal scale value applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		/* Host not support
		for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
			var t = g.vw_adv_setup_target_h[i];
			submit(t.host_name, scale);
		}
		*/
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, scale);
		}
	});

	$("#button_vw_adv_v_scalue_up_apply").click(function(){
		var scale = $("#text_vw_adv_v_scale_up").val();
		var submit = function(host, scale) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_v_scale_" + scale + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New vertical scale value applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		/* Host not support
		for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
			var t = g.vw_adv_setup_target_h[i];
			submit(t.host_name, scale);
		}
		*/
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, scale);
		}
	});

	$("#button_vw_adv_delay_kick_apply").click(function(){
		var us = $("#text_vw_adv_delay_kick").val();
		var submit = function(host, us) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: "e e_vw_delay_kick_" + us + ";"
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){highlight("#dummy_hdr_main", "Success:", "New delay value applied.");})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		/* Host not support
		for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
			var t = g.vw_adv_setup_target_h[i];
			submit(t.host_name, us);
		}
		*/
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, us);
		}
	});

	$("#button_vw_adv_console_api_apply").click(function(){
		var shell = $("#text_vw_adv_console_api").val();
		var submit = function(host, shell) {
			// e_sh_cmd=xxxxxxxxxxxx&
			var cmd = $.param([
				{
					name: "e_sh_cmd",
					value: shell
				},
				{ name: 'cache', value: false },
				{ name: 'nocache', value: (new Date()).getTime() }
			]);
			$.getJSON(host+CGI_PATH+"e_jsonp.cgi?callback=?", cmd, function(){})
				.success(function(){})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};
		for (var i = 0; i < g.vw_adv_setup_target_h.length; i++) {
			var t = g.vw_adv_setup_target_h[i];
			submit(t.host_name, shell);
		}
		for (var i = 0; i < g.vw_adv_setup_target_c.length; i++) {
			var t = g.vw_adv_setup_target_c[i];
			submit(t.host_name, shell);
		}
	});
}

function on_doc_ready_vw()
{
	$("#button_vw_layout_apply").button();

	g.$vw_setup_adv_step2_all_apply_btn = $("#container_vw_setup_adv_step2 button.button-vw-adv-apply");
	g.$vw_setup_adv_step2_all_apply_btn.button();

	$("#vw_adv_h_shift_direction").buttonset();
	$("#vw_adv_v_shift_direction").buttonset();
	
	//g.$chkbox_vw_adv_show_osd = $("#chkbox_vw_adv_show_osd").button();
}

function on_tab_video_wall_loaded()
{
	if (!LM_PARAM.VW_VAR_POS_MAX_ROW) LM_PARAM.VW_VAR_POS_MAX_ROW='0';
	if (!LM_PARAM.VW_VAR_POS_MAX_COL) LM_PARAM.VW_VAR_POS_MAX_COL='0';
	if (!LM_PARAM.VW_VAR_MAX_ROW) LM_PARAM.VW_VAR_MAX_ROW='0';
	if (!LM_PARAM.VW_VAR_MAX_COLUMN) LM_PARAM.VW_VAR_MAX_COLUMN='0';
	if (!LM_PARAM.VW_VAR_ROW) LM_PARAM.VW_VAR_ROW='0';
	if (!LM_PARAM.VW_VAR_COLUMN) LM_PARAM.VW_VAR_COLUMN='0';
	if (!LM_PARAM.VW_VAR_MONINFO_HT) LM_PARAM.VW_VAR_MONINFO_HT='0';
	if (!LM_PARAM.VW_VAR_MONINFO_VT) LM_PARAM.VW_VAR_MONINFO_VT='0';
	if (!LM_PARAM.VW_VAR_MONINFO_HA) LM_PARAM.VW_VAR_MONINFO_HA='0';
	if (!LM_PARAM.VW_VAR_MONINFO_VA) LM_PARAM.VW_VAR_MONINFO_VA='0';
	if (!LM_PARAM.VW_VAR_TAIL_IDX) LM_PARAM.VW_VAR_TAIL_IDX='0';
	if (!LM_PARAM.VW_VAR_STRETCH_TYPE) LM_PARAM.VW_VAR_STRETCH_TYPE='2';
	if (!LM_PARAM.VW_VAR_ROTATE) LM_PARAM.VW_VAR_ROTATE='0';
	if (!LM_PARAM.IS_HOST) LM_PARAM.IS_HOST='n';
	
	$("#select_MaxRow").attr("value", LM_PARAM.VW_VAR_POS_MAX_ROW);
	$("#select_MaxCol").attr("value", LM_PARAM.VW_VAR_POS_MAX_COL);

	$("#select_RowPos").attr("value", LM_PARAM.VW_VAR_ROW);
	$("#select_ColPos").attr("value", LM_PARAM.VW_VAR_COLUMN);

	$("#vw_HT").attr("value", LM_PARAM.VW_VAR_MONINFO_HT);
	$("#vw_VT").attr("value", LM_PARAM.VW_VAR_MONINFO_VT);
	$("#vw_HA").attr("value", LM_PARAM.VW_VAR_MONINFO_HA);
	$("#vw_VA").attr("value", LM_PARAM.VW_VAR_MONINFO_VA);

	if (LM_PARAM.SOC_OP_MODE == 1) {
		$("#chkbox_vw_basic_single_host_mode").attr('checked', false);
		$("#chkbox_vw_adv_single_host_mode").attr('checked', false);
	} else {
		$("#chkbox_vw_basic_single_host_mode").attr('checked', true);
		$("#chkbox_vw_adv_single_host_mode").attr('checked', true);
	}

	if (LM_PARAM.SOC_OP_MODE >= 3) {
		$("#select_StretchType").attr("value", LM_PARAM.VW_VAR_STRETCH_TYPE);
		$("#select_Rotate").attr("value", LM_PARAM.VW_VAR_ROTATE);
		$('#fieldset_vw_basic_Preferences').show();
		/* Force single host mode. */
		$("#chkbox_vw_basic_single_host_mode").attr('disabled', true);
		$("#div_vw_basic_single_host_mode").hide();
		$("#chkbox_vw_adv_single_host_mode").attr('disabled', true);
		$("#span_vw_adv_single_host_mode").hide();

		$('#span_vw_adv_ctrl_options_v3').show();
	} else if (LM_PARAM.SOC_OP_MODE == 1) {
		$('.soc_v1_only').show();
	}
	on_doc_ready_vw();
	on_doc_load_vw();
}
