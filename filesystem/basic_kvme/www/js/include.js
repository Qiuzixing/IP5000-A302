/*
** Global variables
*/
var TEST_ONLY=false;
var THIS_IP='ast2-gateway0000.local';  //For Test Only
//var THIS_IP='';  //For Test Only

var VW_PATH="";
// Following path are defined related to VW_PATH
var CGI_PATH="/cgi-bin/";

window.g =
{
	// All global objs should be here.
	module:{}, //The module container
	this_ip:'',
	NODE_LIST_H:{},
	NODE_LIST_C:{},
	THIS_NODE: {}, //Used to save the NODE info of "This"
	THIS_NODE_IDX: 0, //Used to save the index of NODE_LIST of "This"
	THE_NODE: {}, //The current NODE info which is filled in adv setup's apply fields.
	THE_NODE_IDX: 0,
	$vw_adv_setup_all_chkbox: null,
	$chkbox_vw_basic_show_osd: null,
	$chkbox_vw_adv_show_osd: null,
	$view: null,
	vw_adv_current_max_row: '-1',
	vw_adv_current_max_col: '-1',
	vw_adv_setup_target_h: [],
	vw_adv_setup_target_c: [],
	is_mfg_mode: true
};

var LM_PARAM={
		VW_VAR_POS_MAX_ROW: "4",
		VW_VAR_TAIL_IDX: "4",
		VW_VAR_POS_MAX_COL: "4",
		VW_VAR_POS_R: "0",
		VW_VAR_POS_C: "2",
		MY_MAC: "821F6414FDED",
		EN_VIDEO_WALL: "n"
	};

if (TEST_ONLY) {
	//LM_PARAM.EN_VIDEO_WALL='y';
	g.this_ip=THIS_IP;
}

//For test Only
var dummy_nodes_legacy = {
		"ast-client821F6414FDEE":
		{
				"ip":"169.254.9.186",
				"host_name":"ast-client821F6414FDEE",
				"status":"s_srv_on",
				"is_host":"n"
		},
		"ast-client821F6414FDE7":
		{
				"ip":"169.254.9.187",
				"host_name":"ast-client821F6414FDE7",
				"status":"s_srv_on",
				"is_host":"n"
		},
		"ast-gateway0100":
		{
				"ip":"169.254.9.188",
				"host_name":"ast-gateway0100",
				"status":"s_srv_on",
				"is_host":"y"
		},
		"ast-client821F6414FDED":
		{
				"ip":"169.254.9.185",
				"host_name":"ast-client821F6414FDED",
				"status":"s_srv_on",
				"is_host":"n"
		},
		"ast-gateway0001":
		{
				"ip":"169.254.9.249",
				"host_name":"ast-gateway0001",
				"status":"s_srv_on",
				"is_host":"y"
		}
	};

/* node_query --get_key IS_HOST --get_key MY_MAC --get_key HOSTNAME --get_key MY_IP --get_key STATE --json */
var dummy_nodes = {
	"nodes":[
	{
			"IS_HOST":"y",
			"MY_MAC":"02C7C324D7E3",
			"HOSTNAME":"ast3-gateway0000",
			"MY_IP":"169.254.10.133",
			"STATE":"s_srv_on"
	},
	{
			"IS_HOST":"n",
			"MY_MAC":"82EAA841BDF1",
			"HOSTNAME":"ast3-client82EAA841BDF1",
			"MY_IP":"169.254.10.167",
			"STATE":"s_srv_on"
	}
	],
	"count":2
	};

var dummy_lm_param_h= [
	{
		VW_VAR_POS_MAX_ROW: "4",
		VW_VAR_TAIL_IDX: "4",
		VW_VAR_POS_MAX_COL: "4",
		VW_VAR_POS_IDX: "2",
		VW_VAR_POS_R: "2",
		VW_VAR_POS_C: "0",
		VW_VAR_MAX_ROW: "3",
		VW_VAR_MAX_COLUMN: "4",
		VW_VAR_ROW: "5",
		VW_VAR_COLUMN: "6",
		VW_VAR_H_SHIFT: "-1",
		VW_VAR_V_SHIFT: "2",
		VW_VAR_H_SCALE: "300",
		VW_VAR_V_SCALE: "400",
		VW_VAR_DELAY_KICK: "500",
		MY_MAC: "801F6414FDED",
		MY_IP: "169.254.11.22"
	},
	{
		VW_VAR_POS_MAX_ROW: "4",
		VW_VAR_TAIL_IDX: "4",
		VW_VAR_POS_MAX_COL: "4",
		VW_VAR_POS_IDX: "0",
		VW_VAR_POS_R: "0",
		VW_VAR_POS_C: "0",
		VW_VAR_MAX_ROW: "3",
		VW_VAR_MAX_COLUMN: "4",
		VW_VAR_ROW: "5",
		VW_VAR_COLUMN: "6",
		VW_VAR_H_SHIFT: "-100",
		VW_VAR_V_SHIFT: "200",
		VW_VAR_H_SCALE: "300",
		VW_VAR_V_SCALE: "4",
		VW_VAR_DELAY_KICK: "5",
		MY_MAC: "801F6414FDED",
		MY_IP: "169.254.11.33"
	}
	];

var dummy_lm_param_c= [
	{
		VW_VAR_POS_MAX_ROW: "4",
		VW_VAR_TAIL_IDX: "4",
		VW_VAR_POS_MAX_COL: "4",
		VW_VAR_POS_IDX: "r0c0",
		VW_VAR_POS_R: "0",
		VW_VAR_POS_C: "0",
		VW_VAR_MAX_ROW: "3",
		VW_VAR_MAX_COLUMN: "4",
		VW_VAR_ROW: "5",
		VW_VAR_COLUMN: "6",
		VW_VAR_H_SHIFT: "-1",
		VW_VAR_V_SHIFT: "2",
		VW_VAR_H_SCALE: "3",
		VW_VAR_V_SCALE: "4",
		VW_VAR_DELAY_KICK: "5",
		MY_MAC: "821F6414FDE7",
		MY_IP: "169.254.11.44"
	},
	{
		VW_VAR_POS_MAX_ROW: "4",
		VW_VAR_TAIL_IDX: "4",
		VW_VAR_POS_MAX_COL: "4",
		VW_VAR_POS_IDX: "r0c2",
		VW_VAR_POS_R: "0",
		VW_VAR_POS_C: "2",

		VW_VAR_MAX_ROW: "3",
		VW_VAR_MAX_COLUMN: "4",
		VW_VAR_ROW: "5",
		VW_VAR_COLUMN: "6",
		VW_VAR_H_SHIFT: "-100",
		VW_VAR_V_SHIFT: "200",
		VW_VAR_H_SCALE: "300",
		VW_VAR_V_SCALE: "400",
		VW_VAR_DELAY_KICK: "500",
		MY_MAC: "821F6414FDED",
		MY_IP: "169.254.11.55"
	},
	{
		VW_VAR_POS_MAX_ROW: "4",
		VW_VAR_TAIL_IDX: "4",
		VW_VAR_POS_MAX_COL: "4",
		VW_VAR_POS_IDX: "r1c1",
		VW_VAR_POS_R: "1",
		VW_VAR_POS_C: "1",
		VW_VAR_MAX_ROW: "3",
		VW_VAR_MAX_COLUMN: "4",
		VW_VAR_ROW: "5",
		VW_VAR_COLUMN: "6",
		VW_VAR_H_SHIFT: "10",
		VW_VAR_V_SHIFT: "20",
		VW_VAR_H_SCALE: "30",
		VW_VAR_V_SCALE: "40",
		VW_VAR_DELAY_KICK: "50",
		MY_MAC: "821F6414FDEE",
		MY_IP: "169.254.11.66"
	},
	];


function mylog(s) {
	if (false) console.log(s);
	//alert(s);
}

function notice_block_init() {
	/* Bruce121115. We need a global content to save inter-context information. */
	window.g.notice_block || (window.g.notice_block = {
		stick_fn: null,
		stick_head_str: '',
		stick_str: '&nbsp',
		is_occupied: false,
		original_pos: 0,
		$nb: {},
		STR_RESET: 'Reboot for new settings to take effect.'
	});

	var p = g.notice_block;
	p.$nb = $("#notice_block");
	p.original_pos = p.$nb.offset().top;
	//notice_block_fix_position();
	(function notice_block_fix_position(p) {
		// Following code is used to keep the notice block in the view
		// bind to the window scroll and resize events
		g.$view.bind("scroll resize", function(){
			var viewTop = g.$view.scrollTop();
			if ((viewTop > p.original_pos)) {
				if (!p.$nb.is(".notice-block-fixed")) {
					// First time
					p.$nb
						.removeClass("notice-block-top")
						.addClass("notice-block-fixed");
				}
				// IE6 fix: We always use "position:absolute + top:xxx px" instead of "position:fixed".
				p.$nb.css("top", g.$view.scrollTop() - 12 + "px");

			} else if ((viewTop <= p.original_pos) && p.$nb.is(".notice-block-fixed")) {
				p.$nb
					.removeClass("notice-block-fixed")
					.addClass("notice-block-top")
					.css("top", "0px");
			}
		});
	})(p);

}

/*
** Show notice message in selected .notice-block element. See .notice-block div block for format.
*/
function notice(select, type, head_str, str, callback) {
	//return;
	if (typeof head_str == "undefined") head_str = '';
	if (typeof str == "undefined" || !str.length) str = '&nbsp';
	if (typeof callback == "undefined") callback = null;
	if (typeof type == "undefined") type = 'clear';
	if (typeof select == "undefined") return;

	var p = g.notice_block;
	var repeat = function() {
		p.stick_fn &&
		setTimeout(
			function() {
				if (p.is_occupied) {
					repeat();
					return;
				}
				p.stick_fn && notice(select, 'stick_msg', p.stick_head_str, p.stick_str, repeat);
			}
			, 5000);
	};
	var fn = function () {
		$(select)
		.find("div")
			.removeClass("dummy ui-state-highlight ui-state-error ui-corner-all")
			.addClass(
				(type == 'highlight') ? ('ui-state-highlight ui-corner-all') :
				(type == 'error') ? ('ui-state-error ui-corner-all') :
				(type == 'stick_msg') ? ('ui-state-error ui-corner-all') :
				('dummy')
			)
			.end()
		.find("span.notice-icon")
			.removeClass("dummy ui-icon ui-icon-info ui-icon-alert")
			.addClass(
				(type == 'highlight') ? ('ui-icon ui-icon-info') :
				(type == 'error') ? ('ui-icon ui-icon-alert') :
				(type == 'stick_msg') ? ('ui-icon ui-icon-alert') :
				('dummy')
			)
			.end()
		.find("strong")
			.html(head_str)
			.end()
		.find("span.notice-str")
			.html(str)
			.end();

		if (type == 'clear') {
			$(select).show();
			p.is_occupied = false;
			(callback && callback());
		} else {
			if (type != 'stick_msg')
				p.is_occupied = true;
			$(select).effect('bounce', {}, 500, callback);
		}
	}
	switch(type){
		case 'clear':
			//$(select).effect('clip', {}, 500, fn);
			$(select).fadeOut(500, fn);
			break;
		case 'stick':
			p.stick_head_str = head_str;
			p.stick_str = str;
			p.stick_fn = repeat;
			p.stick_fn();
			(callback && callback());
			break;
		case 'unstick':
			p.stick_head_str = '';
			p.stick_str = '&nbsp';
			p.stick_fn = null;
			notice(select, 'clear', '', '', callback);
			break;
		default:
			fn();
			break;
	};
};

/*
** Simplified notice()
*/
function highlight(select, head_str, str) {
	notice(select, 'highlight', head_str, str, function(){
		setTimeout(function (){
			notice(select, 'clear');
		}, 10000);
	});
}

/*
** Simplified notice()
*/
function error(select, head_str, str) {
	notice(select, 'error', head_str, str, function(){
		setTimeout(function (){
			notice(select, 'clear');
		}, 15000);
	});
}

/*
** Simplified notice()
*/
function msg_stick(select, head_str, str) {
	notice(select, 'stick', head_str, str, null);
}


/* This is a uicornerfix plugin for IE. Include <script type="text/javascript" src="js/jquery.corner.js"></script>
$.uicornerfix = function(r){
	$('.ui-corner-all').corner(r);
	$('.ui-corner-top').corner("top "+r);
	$('.ui-corner-bottom').corner("bottom "+r);
	$('.ui-corner-right').corner("right "+r);
	$('.ui-corner-left').corner("left "+r);
	$('.ui-corner-tl').corner("tl "+r);
	$('.ui-corner-tr').corner("tr "+r);
	$('.ui-corner-br').corner("br "+r);
	$('.ui-corner-bl').corner("bl "+r);
};
*/
/*
function sleep(n)
{
	var start = new Date().getTime();
	while (true) if (new Date().getTime() - start > (n*1000)) break;
}
*/

function host_ip() {
	if (g.this_ip) {
		return "http://" + g.this_ip + VW_PATH;
	} else {
		return '';
	}

}
/*
Bruce151029. Under MFG mode, lmparam is not working (no LM) and will block forever.
So, We can't use "lmparam jdump" on doc init.
function query_lm_param(callback) {
	var host = host_ip();
	var cmd = $.param({
		cache: false,
		nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
		cmd: "lmparam jdump"
	});
	$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){
		})
		.error(function(){
			error("#dummy_hdr_main", "Error!", "Can't get LM_PARAM");
			if (typeof callback != "undefined")
				callback(LM_PARAM);
		})
		.complete(function(){
			if (typeof callback != "undefined")
				callback(LM_PARAM);
		})
		.success(function(json){
			//$.each(json, function(key, val) {
			//	alert("key:"+key+"\nval:"+val);
			//});
			if (typeof json != "undefined")
				LM_PARAM = json;
		});
	return LM_PARAM;
}
*/

function query_lm_param(callback) {
	var host = host_ip();
	var cmd = $.param({
		cache: false,
		nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
		cmd: "e e_debug_json"
	});
	$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){
		})
		.error(function(){
			error("#dummy_hdr_main", "Error!", "Can't get LM_PARAM");
			if (typeof callback != "undefined")
				callback(LM_PARAM);
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
			.error(function(){error("#dummy_hdr_main", "Error!", "Can't get LM_PARAM");})
			.complete(function(){
				if (typeof callback != "undefined")
					callback(LM_PARAM);
			})
			.success(function(json){
				//$.each(json, function(key, val) {
				//	alert("key:"+key+"\nval:"+val);
				//});
				if (typeof json != "undefined") {
					g.is_mfg_mode = false;
					LM_PARAM = json;
				}
			});
		});
	return LM_PARAM;
}

// ToDo. Asynchronously load the arrary of scripts and callback on all scripts loaded.
function load_scripts(scripts, on_loaded)
{
	if (typeof scripts == 'string') {
		scripts = [ scripts ];
	}
	var myGetScript = function (i, on_loaded) {
		$.getScript(scripts[i], function(){
			if (++i == scripts.length) {
				on_loaded();
			} else {
				myGetScript(i, on_loaded);
			}
		});
	};
	myGetScript(0, on_loaded);
}

function fn_reload_web_after(sec)
{
	return function() {
		g.$dialog_loading.html("<table style='margin-bottom:auto;margin-top:auto;height:50px'><tr><td style='vertical-align:middle'><h4 style='margin-bottom:0'>Rebooting...<img src=\"images/ajax-loader.gif\"/></h4></td></tr></table>")
		g.$dialog_loading.dialog('open');
		setTimeout(function(){
			g.$dialog_loading.dialog('close');
			location.reload(true);
		}, sec*1000);
	};
}

function is_cfg_page_on(cfg, page_str)
{
	if (!cfg || !page_str) {
		return ((TEST_ONLY)?true:false);
	}

	switch(page_str) {
	case "essential":
		return ((cfg.toLowerCase().indexOf('e') >= 0)?(true):(false));
		break;
	case "network":
		return ((cfg.toLowerCase().indexOf('n') >= 0)?(true):(false));
		break;
	case "video":
		return ((cfg.toLowerCase().indexOf('v') >= 0)?(true):(false));
		break;
	case "video_wall":
		return ((cfg.toLowerCase().indexOf('w') >= 0)?(true):(false));
		break;
	case "audio":
		return ((cfg.toLowerCase().indexOf('a') >= 0)?(true):(false));
		break;
	case "usb":
		return ((cfg.toLowerCase().indexOf('u') >= 0)?(true):(false));
		break;
	case "serial":
		return ((cfg.toLowerCase().indexOf('s') >= 0)?(true):(false));
		break;
	case "ir":
		return ((cfg.toLowerCase().indexOf('r') >= 0)?(true):(false));
		break;
	case "god":
		return ((cfg.toLowerCase().indexOf('g') >= 0)?(true):(false));
		break;
	case "functions":
		return (
				is_cfg_page_on(cfg, "video") |
				is_cfg_page_on(cfg, "audio") |
				is_cfg_page_on(cfg, "usb") |
				is_cfg_page_on(cfg, "serial") |
				is_cfg_page_on(cfg, "ir")
			);
		break;
	default:
		return false;
		break;
	}
}
