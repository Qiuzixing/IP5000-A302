
function vw_adv_update_target_list()
{
	var $targets = g.$vw_adv_setup_all_chkbox.filter(":checked"),
		row_size = (Number(LM_PARAM.VW_VAR_TAIL_IDX) + 1),
		col_size = (Number(LM_PARAM.VW_VAR_POS_MAX_COL) + 1),
		optimize = true,
		$t = null;
	
	if (g.$chkbox_vw_adv_use_r99.attr('checked'))
		optimize = false;

	g.vw_adv_setup_target_h = [];
	g.vw_adv_setup_target_c = [];
	if (typeof $targets == "undefined") return;

	if ($targets.filter("[id^='host_r']").length >= row_size) {
		g.vw_adv_setup_target_h[g.vw_adv_setup_target_h.length] = 'r99_';
	} else {
		for (var i = 0; i < row_size; i++) {
			if ($("#host_r"+i).attr('checked'))
				g.vw_adv_setup_target_h[g.vw_adv_setup_target_h.length] = "r" + i + "_";
		}
	}
	$t = $targets.filter("[id^='client_r']");
	if ($t.length >= row_size * col_size) {
		g.vw_adv_setup_target_c[g.vw_adv_setup_target_c.length] = 'r99_msg_toFFFFFFFF_';
	} else {
		for (var i = 0; i < row_size; i++) {
			$t1 = $t.filter("[id^='client_r"+i+"']");
			if (optimize && $t1.length >= col_size) {
				g.vw_adv_setup_target_c[g.vw_adv_setup_target_c.length] = "r" + i + "_msg_toFFFFFFFF_";
			} else {
				for (var j = 0; j < $t1.length; j++) {
					var pos_id = $($t1[j]).val();
					if (optimize) {
						g.vw_adv_setup_target_c[g.vw_adv_setup_target_c.length] = "r"+i+"_"+pos_id+"_";
					} else {
						g.vw_adv_setup_target_c[g.vw_adv_setup_target_c.length] = "r99_"+pos_id+"_";
					}
				}
			}
		}
	}

}

function _vw_adv_target_selection_init()
{
	if (typeof LM_PARAM == "undefined") return;
	if (typeof LM_PARAM.VW_VAR_POS_MAX_COL == "undefined") return;
	if (typeof LM_PARAM.VW_VAR_TAIL_IDX == "undefined") return;

	if ((g.vw_adv_current_max_row == LM_PARAM.VW_VAR_TAIL_IDX) && (g.vw_adv_current_max_col == LM_PARAM.VW_VAR_POS_MAX_COL)) {
		g.$dialog_loading.dialog('close');
		return;
	}
	g.vw_adv_current_max_row = LM_PARAM.VW_VAR_TAIL_IDX;
	g.vw_adv_current_max_col = LM_PARAM.VW_VAR_POS_MAX_COL;

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
	$all_chkbox.button(); // !!!!This line takes most of time in this function.

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
				.change();
				//.button('rfresh');
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
				.change();
				//.button('refresh');
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
				.change();
				//.button('refresh');
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
				.change();
				//.button('refresh');
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
				.change();
				//.button('refresh');

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
				.change();
				//.button('refresh');
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
	g.$vw_adv_setup_all_chkbox.change(function(){
		vw_adv_update_target_list();
		if (g.$button_vw_adv_show_osd.attr('checked')) {
			// Turn off all OSD. And toggle the "show OSD" button.
			g.$button_vw_adv_show_osd.attr('checked', false).change();
		}
	});

	g.$dialog_loading.dialog('close');

}

function vw_adv_target_selection_init()
{
	g.$dialog_loading.dialog('open');
	query_vw_var_tail_idx(_vw_adv_target_selection_init);
	//setTimeout(_vw_adv_target_selection_init, 0);

}

