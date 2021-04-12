
/* Create root module. */
g.module.statics || (g.module.statics = {});
g.module.edid || (g.module.edid = {});

g.module.statics.lm = (function (selector) {
	var $root = $(selector),
		$lm_stat = $root.find(".ID_lm_stat");

	function init() {
		//Initialize UI, also bind events
		//ui_refresh();
		$lm_stat.before("<span><img src=\"images/ajax-loader.gif\"/></span>");
		$root.show();
		return this;
	};
	function ui_refresh() {
		var cmd_str;
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "single_line"
			});
			$lm_stat.html("").prev("span").show();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					//Update internal variables
					$lm_stat.html(json && json.stdout)
						.prev("span").hide();
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Can't get LM state?!");})
				.complete(function(){});
		};
		cmd_str = "cat /var/ast_device_status;";
		submit(host_ip(), cmd_str);
	};
	//Public members.
	// The module interface
	return {
		init: init,
		ui_refresh: ui_refresh
	};	
});

g.module.statics.net = (function (selector) {
	var $root = $(selector),
		$hostname_id = $root.find(".ID_net_hostname_id"),
		$ip = $root.find(".ID_net_stat_ip"),
		$netmask = $root.find(".ID_net_stat_netmask"),
		$gatewayip = $root.find(".ID_net_stat_gatewayip"),
		$mac = $root.find(".ID_net_stat_mac"),
		$cast = $root.find(".ID_net_stat_casting"),
		$link_state = $root.find(".ID_net_stat_link_state"),
		$link_mode = $root.find(".ID_net_stat_link_mode");

	function init() {
		//Initialize UI, also bind events
		//ui_refresh();
		$link_state.before("<span><img src=\"images/ajax-loader.gif\"/></span>");
		$link_mode.before("<span><img src=\"images/ajax-loader.gif\"/></span>");
		$root.show();
		return this;
	};
	function ui_refresh() {
		ui_refresh_link_state();
		ui_refresh_link_mode();
		$hostname_id.html((LM_PARAM.HOSTNAME_ID)?(LM_PARAM.HOSTNAME_ID):("Unknown"));
		$ip.html((LM_PARAM.MY_IP)?(LM_PARAM.MY_IP):("Unknown"));
		$netmask.html((LM_PARAM.MY_NETMASK)?(LM_PARAM.MY_NETMASK):("Unknown"));
		$gatewayip.html((LM_PARAM.MY_GATEWAYIP)?(LM_PARAM.MY_GATEWAYIP):("Unknown"));
		$mac.html((LM_PARAM.MY_MAC)?(LM_PARAM.MY_MAC):("Unknown"));
		$cast.html((LM_PARAM.MULTICAST_ON == 'y')?("Multicast Mode"):("Unicast Mode"));
	};
	function ui_refresh_link_mode() {
		var cmd_str;
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "single_line"
			});
			$link_mode.html("").prev("span").show();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					//Update internal variables
					$link_mode.html(json && json.stdout)
						.prev("span").hide();
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Can't get Ethernet link mode?!");})
				.complete(function(){});
		};
		cmd_str = "cat /sys/devices/platform/ftgmac/link_mode;";
		submit(host_ip(), cmd_str);
	};
	function ui_refresh_link_state() {
		var cmd_str;
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "single_line"
			});
			$link_state.html("").prev("span").show();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					//Update internal variables
					$link_state.html(json && json.stdout)
						.prev("span").hide();
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Can't get Ethernet link state?!");})
				.complete(function(){});
		};
		cmd_str = "cat /sys/devices/platform/ftgmac/link_state;";
		submit(host_ip(), cmd_str);
	};
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "network")) {
		return {
			init: (function(){return this;}),
			ui_refresh: (function(){})
		};	
	}
	return {
		init: init,
		ui_refresh: ui_refresh
	};	
});

g.module.statics.video = (function (selector) {
	var $root = $(selector),
		$mon_info = $root.find(".ID_video_stat_monitor_info"),
		$edid = $root.find(".ID_video_stat_edid"),
		$timing_info = $root.find(".ID_video_stat_timing_info");

	function init() {
		//Initialize UI, also bind events
		//ui_refresh();
		LM_PARAM.IS_HOST || (LM_PARAM.IS_HOST = 'n');
		//Add loader gif
		$mon_info.before("<span><img src=\"images/ajax-loader.gif\"/></span>");
		$edid.before("<span><img src=\"images/ajax-loader.gif\"/></span>");
		$timing_info.before("<span><img src=\"images/ajax-loader.gif\"/></span>");

		if (LM_PARAM.IS_HOST == 'y') {
			$edid.parent("div").show();
		}
		$root.show();
		return this;
	};
	function ui_refresh() {
		ui_refresh_monitor_info();
		ui_refresh_timing_info();
		if (LM_PARAM.IS_HOST == 'y') {
			ui_refresh_edid();
		}
	};
	function ui_refresh_edid() {
		var cmd_str;
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "multi_line"
			});
			$edid.html("").prev("span").show();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					//Update internal variables
					$edid.html(json && "<pre>" + json.stdout + "</pre>")
						.prev("span").hide();
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Can't get EDID?!");})
				.complete(function(){});
		};
		cmd_str = "cat /sys/devices/platform/videoip/edid_cache;";
		submit(host_ip(), cmd_str);
	};
	function ui_refresh_monitor_info() {
		var cmd_str;
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "multi_line"
			});
			$mon_info.html("").prev("span").show();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					//Update internal variables
					$mon_info.html(json && "<pre>" + json.stdout + "</pre>")
						.prev("span").hide();
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Can't get monitor info?!");})
				.complete(function(){});
		};
		cmd_str = "cat /sys/devices/platform/display/monitor_info;";
		submit(host_ip(), cmd_str);
	};
	function ui_refresh_timing_info() {
		var cmd_str;
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "multi_line"
			});
			$timing_info.html("").prev("span").show();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					//Update internal variables
					$timing_info.html(json && (json.stdout.length)?("<pre>" + json.stdout + "</pre>"):("<pre>Not Available</pre>"))
						.prev("span").hide();
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Can't get video timing info?!");})
				.complete(function(){});
		};
		cmd_str = "cat /sys/devices/platform/videoip/timing_info;";
		submit(host_ip(), cmd_str);
	};
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "video") || (LM_PARAM.NO_VIDEO == 'y')) {
		return {
			init: (function(){return this;}),
			ui_refresh: (function(){})
		};	
	}
	return {
		init: init,
		ui_refresh: ui_refresh
	};	
});


g.module.edid.functions = (function (selector) {
	var $root = $(selector),
		$edid_reset = $root.find(".ID_video_default_edid"),
		$edid_reset_type = $root.find(".ID_video_default_edid_type"),
	    $btn_apply = $root.find("button");

	function init() {
		//Initialize LM_PARAM
		LM_PARAM || (LM_PARAM = {});
		LM_PARAM.IS_HOST || (LM_PARAM.IS_HOST = 'y');
		LM_PARAM.NO_VIDEO || (LM_PARAM.NO_VIDEO = 'n');

		//Initialize UI, also bind events
		ui_refresh();
		$btn_apply.button().click(on_apply);
		if (LM_PARAM.IS_HOST == 'y') {
			$edid_reset.show();
		}
		$root.show();
		return this;
	};
	function ui_refresh() {
		//$edid_reset.find("input:checkbox").attr("checked", false); //Always clear this checkbox
	};
	function on_apply() {
		//var edid_reset = $edid_reset.find("input:checkbox").attr("checked");
		var edid_reset_type = $edid_reset_type.find("input:radio[name=default_edid]:checked").val();

		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str
			});
			g.$dialog_loading.dialog('open');
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){
					highlight("#dummy_hdr_main", "Success:", "EDID reset to default.");
					//msg_stick("#dummy_hdr_main", "Warning:", g.notice_block.STR_RESET);
					//Update internal variables
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){g.$dialog_loading.dialog('close');});
		};

		cmd_str = "";
		if (true /*edid_reset*/) {
			cmd_str += "cat /sys/devices/platform/display/default_edid_" + edid_reset_type + " > /sys/devices/platform/videoip/eeprom_content;"
		}

		submit(host_ip(), cmd_str);
	};
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "video") || (LM_PARAM.NO_VIDEO == 'y')) {
		return {
			init: (function(){return this;})
		};	
	}
	return {
		init: init
	};	
});


function on_tab_system_loaded()
{
	$("#button_sys_reset2default_apply").button().click(function(){
		var shell = "reset_to_default.sh;sleep 1;reboot;";
		var submit_stdout = function(host, shell) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: shell,
				wrap_type: "multi_line"
			});
			//Trigger fn_reload_web_after() first before firing reset command. Otherwise will failed to load .gif.
			(fn_reload_web_after(40))();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					highlight("#dummy_hdr_main", "NOTE:", "Command applied. System reboot, please reload after 40 seconds.");
					//$("#console_api_stdout").html(json && "<pre>" + json.stdout + "</pre>");
				})
				.error(function(){/*error("#dummy_hdr_main", "Error:", "Command not supported? Try again.");*/})
				.complete(/*fn_reload_web_after(40)*/);
		};
		submit_stdout(host_ip(), shell);
	});

	$("#button_sys_reboot_apply").button().click(function(){
		var shell = "reboot;"
		var submit_stdout = function(host, shell) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: shell,
				wrap_type: "multi_line"
			});
			//Trigger fn_reload_web_after() first before firing reset command. Otherwise will failed to load .gif.
			(fn_reload_web_after(40))();
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					highlight("#dummy_hdr_main", "NOTE:", "Command applied. System reboot, please reload after 40 seconds.");
				})
				.error(function(){/*error("#dummy_hdr_main", "Error:", "Command not supported? Try again.");*/})
				.complete(/*fn_reload_web_after(40)*/);
		};
		submit_stdout(host_ip(), shell);
	});

	$("#button_sys_console_api_apply").button().click(function(){
		var shell = $("#text_sys_console_api").val();
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
				.complete(function(){highlight("#dummy_hdr_main", "NOTE:", "Command applied.");});
		};
		var submit_stdout = function(host, shell) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: shell,
				wrap_type: "multi_line"
			});
			g.$dialog_loading.dialog('open');
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					highlight("#dummy_hdr_main", "NOTE:", "Command applied.");
					$("#console_api_stdout").html(json && "<pre>" + json.stdout + "</pre>");
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Command not supported? Try again.");})
				.complete(function(){g.$dialog_loading.dialog('close');});
		};
		submit_stdout(host_ip(), shell);
	});

	if (!g.is_mfg_mode) {
		g.module.edid.functions("#field_func_edid").init();
		var s_lm = g.module.statics.lm("#sys_statistics .ID_field_lm_stat").init();
		var s_net = g.module.statics.net("#sys_statistics .ID_field_net_stat").init();
		var s_video = g.module.statics.video("#sys_statistics .ID_field_video_stat").init();

		//
		// Handle events of video wall accordion change
		//
		$("#accordion_sys").bind("accordionchange", function(event, ui) {
			if (ui.newHeader.attr('id') == 'accordion_sys_statistics') {
				s_lm.ui_refresh();
				s_net.ui_refresh();
				s_video.ui_refresh();
			}
		});
	}
}