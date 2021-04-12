
/* Create root module. */
g.module.usb || (g.module.usb = {});
g.module.serial || (g.module.serial = {});
g.module.video || (g.module.video = {});

/* -----------------------------------------------------------------
// Module code pattern
g.module.usb.functions = (function (selector) {
	var $root = $(selector);
	    $btn_apply = $root.find("button");

	function init() {
		//Initialize LM_PARAM
		LM_PARAM || (LM_PARAM = {});
		LM_PARAM.IP_MODE || (LM_PARAM.IP_MODE = 'autoip');
		//Initialize UI, also bind events
		load_setting_from_runtime();
		ui_refresh_xxxxx_input();
		$btn_apply.button().click(on_apply);
		$root.show();
		return this;
	};
	function on_apply() {
		var multicast_on = $option.find("input:radio[name=network_casting_mode]:checked").val();
		var share_usb_auto_mode = (is_share_usb_auto_checked())?'y':'n';
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){
					highlight("#dummy_hdr_main", "Success:", "New casting mode applied.");
					msg_stick("#dummy_hdr_main", "Warning:", g.notice_block.STR_RESET);
					//Update internal variables
					is_multicast = (multicast_on == 'y')?(true):(false);
					LM_PARAM.MULTICAST_ON = multicast_on;
					if (apply_usb)
						LM_PARAM.SHARE_USB_AUTO_MODE = share_usb_auto_mode;
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};

		cmd_str = "astparam s multicast_on " + multicast_on + ";";
		if (apply_usb) {
			cmd_str += "astparam s share_usb_auto_mode " + share_usb_auto_mode + ";"
			        + "lmparam s SHARE_USB_AUTO_MODE " + share_usb_auto_mode + ";";
		}
		cmd_str +=  "astparam save;"
		        + "lmparam s MULTICAST_ON " + multicast_on + ";";

		submit(host_ip(), cmd_str);
	};
	//Public members.
	// The module interface
	return {
		init: init
	};	
});

------------------------------------------------------------------*/

g.module.usb.functions = (function (selector) {
	var $root = $(selector),
		$usb_on = $root.find(".ID_usb_enable"),
		$share_usb_option = $root.find(".ID_share_usb_option"),
		$usb_compatibility = $root.find(".ID_usb_compatibility"),
		$set_addr_hack = $usb_compatibility.find(".ID_set_addr_hack"),
		$hid_urb_interval = $usb_compatibility.find(".ID_hid_urb_interval"),
		$no_kmoip = $usb_compatibility.find(".ID_no_kmoip"),
		$btn_apply = $root.find("button");

	function init() {
		//Initialize LM_PARAM
		LM_PARAM || (LM_PARAM = {});
		LM_PARAM.NO_USB || (LM_PARAM.NO_USB = 'n');
		LM_PARAM.SHARE_USB_AUTO_MODE || (LM_PARAM.SHARE_USB_AUTO_MODE = 'n');
		LM_PARAM.SHARE_USB || (LM_PARAM.SHARE_USB = 'n');
		LM_PARAM.USB_SET_ADDR_HACK || (LM_PARAM.USB_SET_ADDR_HACK = '0');
		LM_PARAM.USB_HID_URB_INTERVAL || (LM_PARAM.USB_HID_URB_INTERVAL = '0');
		LM_PARAM.NO_KMOIP || (LM_PARAM.NO_KMOIP = 'n');
		//Initialize UI, also bind events
		ui_refresh();
		$usb_compatibility.show();
		if (LM_PARAM.IS_HOST == 'y') {
			$hid_urb_interval.show();
			if (LM_PARAM.SOC_VER == '1')
				$set_addr_hack.show();
		}
		$btn_apply.button().click(on_apply);
		$root.show();
		return this;
	}
	function ui_refresh() {
		$usb_on.find("input:checkbox").attr("checked", (LM_PARAM.NO_USB == 'n')?(true):(false));
		if (LM_PARAM.SHARE_USB_AUTO_MODE == 'y') {
			$share_usb_option.find(".ID_auto").attr("checked", true);
		} else if (LM_PARAM.SHARE_USB == 'y') {
			$share_usb_option.find(".ID_y").attr("checked", true);
		} else {
			$share_usb_option.find(".ID_n").attr("checked", true);
		}
		if (LM_PARAM.IS_HOST == 'y') {
			$set_addr_hack.find("input:checkbox").attr("checked", (LM_PARAM.USB_SET_ADDR_HACK == '0')?(false):(true));
			$hid_urb_interval.find("input:checkbox").attr("checked", (LM_PARAM.USB_HID_URB_INTERVAL == '0')?(false):(true));
		}
		$no_kmoip.find("input:checkbox").attr("checked", (LM_PARAM.NO_KMOIP == 'y')?(false):(true));
	}
	function on_apply() {
		var cmd_str;
		var no_usb = ($usb_on.find("input:checkbox").attr("checked")?('n'):('y'));
		var share_usb_auto_mode = ($share_usb_option.find(".ID_auto").attr("checked")?('y'):('n'));
		var share_usb = $share_usb_option.find(".ID_y").attr("checked")?('y'):('n');
		var usb_set_addr_hack = $set_addr_hack.find("input:checkbox").attr("checked")?('20'):('0');
		var usb_hid_urb_interval = $hid_urb_interval.find("input:checkbox").attr("checked")?('35'):('0');
		var no_kmoip = $no_kmoip.find("input:checkbox").attr("checked")?('n'):('y');

		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str
			});
			g.$dialog_loading.dialog('open');
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){
					highlight("#dummy_hdr_main", "Success:", "New USB options applied.");
					msg_stick("#dummy_hdr_main", "Warning:", g.notice_block.STR_RESET);
					//Update internal variables
					LM_PARAM.NO_USB = no_usb;
					LM_PARAM.SHARE_USB_AUTO_MODE = share_usb_auto_mode;
					if (share_usb_auto_mode == 'n') {
						LM_PARAM.SHARE_USB = share_usb;
					}
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){g.$dialog_loading.dialog('close');});
		};

		cmd_str = "astparam s no_usb " + no_usb + ";"
		        + "astparam s share_usb_auto_mode " + share_usb_auto_mode + ";"
		        + "astparam s no_kmoip " + no_kmoip + ";";

		if (LM_PARAM.IS_HOST == 'y') {
			if (LM_PARAM.SOC_VER == '1')
				cmd_str += "astparam s usb_set_addr_hack " + usb_set_addr_hack + ";";
			cmd_str += "astparam s usb_hid_urb_interval " + usb_hid_urb_interval + ";";
		}
		if (share_usb_auto_mode == 'n') {
			cmd_str += "astparam s share_usb " + share_usb + ";"
			        + "lmparam s SHARE_USB " + share_usb + ";";
		}
		cmd_str += "astparam save;"
		        + "lmparam s NO_USB " + no_usb + ";"
		        + "lmparam s SHARE_USB_AUTO_MODE " + share_usb_auto_mode + ";";

		submit(host_ip(), cmd_str);
	}
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "usb")) {
		return {
			init: (function(){return this;})
		};	
	}
	return {
		init: init
	};	
});


g.module.serial.functions = (function (selector) {
	var $root = $(selector),
		$soip_on = $root.find(".ID_serial_enable"),
		$soip_type_option = $root.find(".ID_soip_type_option"),
		$select_s0_baudrate = $root.find(".ID_s0_baudrate"),
		$select_s0_data = $root.find(".ID_s0_data"),
		$select_s0_parity = $root.find(".ID_s0_parity"),
		$select_s0_stop = $root.find(".ID_s0_stop"),
	    $btn_apply = $root.find("button");

	function init() {
		//Initialize LM_PARAM
		LM_PARAM || (LM_PARAM = {});
		LM_PARAM.NO_SOIP || (LM_PARAM.NO_SOIP = 'y');
		LM_PARAM.SOIP_TYPE2 || (LM_PARAM.SOIP_TYPE2 = 'y');
		LM_PARAM.SOIP_GUEST_ON || (LM_PARAM.SOIP_GUEST_ON = 'n');
		LM_PARAM.S0_BAUDRATE || (LM_PARAM.S0_BAUDRATE = '115200-8n1');
		//Initialize UI, also bind events
		//load_setting_from_runtime();
		ui_refresh();
		$btn_apply.button().click(on_apply);
		$root.show();
		return this;
	};
	/* input 115200-8n1 and split the result into an {} */
	function split_s0_baudrate(str) {
		var s = str.split('-');
		var ret = {};

		ret.baudrate = s[0];
		ret.data = s[1].substr(0,1);
		ret.parity = s[1].substr(1,1);
		ret.stop = s[1].substr(2,1);

		return ret;
	}
	/* input string and output string */
	function merge_s0_baudrate(baudrate, data, parity, stop) {
		return (baudrate + '-' + data + parity + stop);
	}
	function ui_refresh() {
		var s0b = {};

		$soip_on.find("input:checkbox").attr("checked", (LM_PARAM.NO_SOIP == 'n')?(true):(false));

		if (LM_PARAM.SOIP_TYPE2 == 'n') {
			if (LM_PARAM.SOIP_GUEST_ON == 'n') {
				//Type 1
				$soip_type_option.find(".ID_type1").attr("checked", true);
			} else {
				//Type 1 guest
				$soip_type_option.find(".ID_type1G").attr("checked", true);
			}
		} else {
			if (LM_PARAM.SOIP_GUEST_ON == 'n') {
				//Type 2
				$soip_type_option.find(".ID_type2").attr("checked", true);
			} else {
				//Type 2 guest
				$soip_type_option.find(".ID_type2G").attr("checked", true);
			}
		}
		s0b = split_s0_baudrate(LM_PARAM.S0_BAUDRATE);
		$select_s0_baudrate.attr('value', s0b.baudrate);
		$select_s0_data.attr('value', s0b.data);
		$select_s0_parity.attr('value', s0b.parity);
		$select_s0_stop.attr('value', s0b.stop);
	};
	function on_apply() {
		var cmd_str;
		var no_soip = ($soip_on.find("input:checkbox").attr("checked")?('n'):('y'));
		var soip_type2, soip_guest_on, s0_baudrate;

		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str
			});
			g.$dialog_loading.dialog('open');
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(json){
					highlight("#dummy_hdr_main", "Success:", "New Serial over IP options applied.");
					msg_stick("#dummy_hdr_main", "Warning:", g.notice_block.STR_RESET);
					//Update internal variables
					LM_PARAM.NO_SOIP = no_soip;
					LM_PARAM.SOIP_TYPE2 = soip_type2;
					LM_PARAM.SOIP_GUEST_ON = soip_guest_on;
					LM_PARAM.S0_BAUDRATE = s0_baudrate;
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){g.$dialog_loading.dialog('close');});
		};

		switch($soip_type_option.find("input:radio:checked").val()) {
			case '1':
				soip_type2 = 'n';
				soip_guest_on = 'n';
				break;
			case '2':
				soip_type2 = 'y';
				soip_guest_on = 'n';
				break;
			case '1g':
				soip_type2 = 'n';
				soip_guest_on = 'y';
				break;
			case '2g':
				soip_type2 = 'y';
				soip_guest_on = 'y';
				break;
			default:
				soip_type2 = 'y';
				soip_guest_on = 'n';
				break;
		}

		s0_baudrate = merge_s0_baudrate(
				$select_s0_baudrate.attr('value'),
				$select_s0_data.attr('value'),
				$select_s0_parity.attr('value'),
				$select_s0_stop.attr('value')
			);

		cmd_str = "astparam s no_soip " + no_soip + ";"
				+ "astparam s soip_type2 " + soip_type2 + ";"
				+ "astparam s soip_guest_on " + soip_guest_on + ";"
				+ "astparam s s0_baudrate " + s0_baudrate + ";"
				+ "astparam save;"
				+ "lmparam s NO_SOIP " + no_soip + ";"
				+ "lmparam s SOIP_TYPE2 " + soip_type2 + ";"
				+ "lmparam s SOIP_GUEST_ON " + soip_guest_on + ";"
				+ "lmparam s S0_BAUDRATE " + s0_baudrate + ";";

		submit(host_ip(), cmd_str);
	};
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "serial")) {
		return {
			init: (function(){return this;})
		};	
	}
	return {
		init: init
	};	
});


g.module.video.functions = (function (selector) {
	var $root = $(selector),
		$video_on = $root.find(".ID_video_enable"),
		$en_vw = $root.find(".ID_video_en_vw"),
		$screen_off = $root.find(".ID_turn_off_screen"),
		$edid_use = $root.find(".ID_video_edid_pri"),
		//$edid_reset = $root.find(".ID_video_default_edid"),
		//$edid_reset_type = $root.find(".ID_video_default_edid_type"),
		$scaler_output = $root.find(".ID_video_scaler_output"),
		$fc = $root.find(".ID_frame_rate_ctrl"),
		$profile = $root.find(".ID_bit_rate_control"),
		$btn_apply = $root.find("button");

	function frame_rate_ctrl_handler($fc) {
		var fc;
		var $fc_percent = $fc.find(".ID_fc_percent");
		var $fc_slider = $fc.find(".ID_fc_slider");
		var submit_fc = function(host, frame_ctrl) {
			var cmd_str = "echo " + frame_ctrl + " > /sys/devices/platform/videoip/frame_rate_control";
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "drop"
			});
			/* g.$dialog_loading.dialog('open'); */
			$.getJSON(host + CGI_PATH + "query.cgi?callback=?", cmd, function() {})
				.success(function() {
					highlight("#dummy_hdr_main", "Success:", "New Maximum Frame Rate (" + frame_ctrl + ") updated. Press 'Apply' button to save.");
				})
				.error(function() { error("#dummy_hdr_main", "Error:", "Try again."); })
				.complete(function() { /* g.$dialog_loading.dialog('close'); */ });
		};
		function fc_to_percent_str(fc) {
			return (fc * 100 / 60).toFixed(0);
		}
		LM_PARAM.V_FRAME_RATE || (LM_PARAM.V_FRAME_RATE = '0');
		fc = parseInt(LM_PARAM.V_FRAME_RATE, 10);
		if (fc == 0)
			fc = 60;
		/* Construct the slider with default value. */
		$fc_slider.slider({
			min : 1,
			max : 60,
			step : 1,
			slide: function(event, ui) {
				$fc_percent.html(fc_to_percent_str(ui.value));
			},
			change: function(event, ui) {
				$fc_percent.html(fc_to_percent_str(ui.value));
				if (ui.value != fc) {
					submit_fc(host_ip(), ui.value);
					fc = ui.value;
				}
			}
		}).slider("value", fc);
	}
	function init() {
		//Initialize LM_PARAM
		LM_PARAM || (LM_PARAM = {});
		LM_PARAM.IS_HOST || (LM_PARAM.IS_HOST = 'y');
		LM_PARAM.NO_VIDEO || (LM_PARAM.NO_VIDEO = 'n');
		LM_PARAM.EN_VIDEO_WALL || (LM_PARAM.EN_VIDEO_WALL = 'n');
		LM_PARAM.EDID_USE || (LM_PARAM.EDID_USE = 'secondary');
		LM_PARAM.V_OUTPUT_TIMING_CONVERT || (LM_PARAM.V_OUTPUT_TIMING_CONVERT = '0');
		LM_PARAM.V_TURN_OFF_SCREEN_ON_PWR_SAVE || (LM_PARAM.V_TURN_OFF_SCREEN_ON_PWR_SAVE = 'n');
		LM_PARAM.V_SRC_UNAVAILABLE_TIMEOUT || (LM_PARAM.V_SRC_UNAVAILABLE_TIMEOUT = '10000');
		LM_PARAM.PROFILE || (LM_PARAM.PROFILE = "auto");

		//Initialize UI, also bind events
		ui_refresh();
		$btn_apply.button().click(on_apply);
		if ((is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "video_wall")) && (LM_PARAM.SOC_OP_MODE != '2')) {
			$en_vw.show();
		}
		if (LM_PARAM.IS_HOST == 'y') {
			if (LM_PARAM.SOC_OP_MODE >= 3) {
				frame_rate_ctrl_handler($fc);
				$fc.show();
				$profile.show();
			}
			//$edid_reset.show();
		} else {
			//Client
			$screen_off.show();
			if (LM_PARAM.MULTICAST_ON == 'y') {
				//We only show EDID_USE on multicast client.
				$edid_use.show();
			}
			if (LM_PARAM.SOC_OP_MODE >= 3) {
				$scaler_output.show();
				if (LM_PARAM.AST_PLATFORM != "ast1525-c-v1")
					$scaler_output.find(".select-item").find(".option-4k").show();
				else //IE hack to 'hide' these 4K options
					$scaler_output.find(".select-item").find(".option-4k").remove();
			}
		}
		$root.show();
		return this;
	}
	function ui_refresh() {
		var select, timeout;

		$video_on.find("input:checkbox").attr("checked", (LM_PARAM.NO_VIDEO == 'n')?(true):(false));
		$en_vw.find("input:checkbox").attr("checked", (LM_PARAM.EN_VIDEO_WALL == 'y')?(true):(false));
		$screen_off.find("input:checkbox").attr("checked", (LM_PARAM.V_TURN_OFF_SCREEN_ON_PWR_SAVE == 'y')?(true):(false));
		$edid_use.find("input:checkbox").attr("checked", (LM_PARAM.EDID_USE == 'primary')?(true):(false));
		$profile.find(".select-item").attr("value", LM_PARAM.PROFILE);
		//$edid_reset.find("input:checkbox").attr("checked", false); //Always clear this checkbox

		timeout = parseInt(LM_PARAM.V_SRC_UNAVAILABLE_TIMEOUT, 10).toString(10).toUpperCase();
		$screen_off.find(".select-item").attr("value", timeout);

		if ((LM_PARAM.IS_HOST == 'n') && (LM_PARAM.SOC_OP_MODE >= 3)) {
			select = parseInt(LM_PARAM.V_OUTPUT_TIMING_CONVERT, 16).toString(16).toUpperCase();
			switch (select) {
				case "0": /* 1:1 */
				case "10000000": /* strict mode */
				case "82000000": /* Per EDID */
				case "80000010": /* Full HD 1080p60 */
				case "8000001F": /* Full HD 1080p50 */
				case "8000005F": /* Ultra HD 2160p30 */
				case "8000005E": /* Ultra HD 2160p25 */
				case "8000005D": /* Ultra HD 2160p24 */
					break;
				default:
					$scaler_output.find(".ID_customize_text").attr("value", select);
					select = "FFFFFFFF"; //It is a customized value
					break;
			}
			$scaler_output.find(".select-item")
				.attr("value", select)
				.change(function(){
					if ($scaler_output.find(".select-item").attr("value") == "FFFFFFFF")
						$scaler_output.find(".ID_customize_text").show();
					else
						$scaler_output.find(".ID_customize_text").hide();
				}).change();
		}
	}
	function on_apply() {
		var no_video = ($video_on.find("input:checkbox").attr("checked")?('n'):('y'));
		var en_video_wall = ($en_vw.find("input:checkbox").attr("checked")?('y'):('n'));
		var v_turn_off_screen_on_pwr_save = ($screen_off.find("input:checkbox").attr("checked")?('y'):('n'));
		var edid_use = ($edid_use.find("input:checkbox").attr("checked")?('primary'):('secondary'));
		var v_output_timing_convert = $scaler_output.find(".select-item").attr("value");
		var v_src_unavailable_timeout = $screen_off.find(".select-item").attr("value");
		var v_frame_rate = $fc.find(".ID_fc_slider").slider("value");
		var profile = $profile.find(".select-item").attr("value");

		//var edid_reset = $edid_reset.find("input:checkbox").attr("checked");
		//var edid_reset_type = $edid_reset_type.find("input:radio[name=default_edid]:checked").val();

		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str,
				wrap_type: "drop"
			});
			g.$dialog_loading.dialog('open');
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){
					highlight("#dummy_hdr_main", "Success:", "New video mode applied.");
					msg_stick("#dummy_hdr_main", "Warning:", g.notice_block.STR_RESET);
					//Update internal variables
					LM_PARAM.NO_VIDEO = no_video;
					LM_PARAM.EN_VIDEO_WALL = en_video_wall;
					if ((LM_PARAM.IS_HOST == 'n') && (LM_PARAM.MULTICAST_ON == 'y')) {
						LM_PARAM.EDID_USE = edid_use;
					}
					LM_PARAM.V_OUTPUT_TIMING_CONVERT = v_output_timing_convert;
					//Clear this checkbox after action
					//$edid_reset.find("input:checkbox").attr("checked", false);
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){g.$dialog_loading.dialog('close');});
		};

		cmd_str = "astparam s no_video " + no_video + ";"
				+ "astparam s en_video_wall " + en_video_wall + ";";

		if ((LM_PARAM.IS_HOST == 'y')) {
			if (LM_PARAM.SOC_OP_MODE >= 3) {
				if (v_frame_rate == 60)
					v_frame_rate = 0;
				cmd_str += "astparam s v_frame_rate " + v_frame_rate + ";";
				cmd_str += "astparam s profile " + profile + ";";
			}
		}
		if ((LM_PARAM.IS_HOST == 'n')) {
			cmd_str += "astparam s v_turn_off_screen_on_pwr_save " + v_turn_off_screen_on_pwr_save + ";"
			         + "astparam s v_src_unavailable_timeout " + v_src_unavailable_timeout + ";";

			if (LM_PARAM.MULTICAST_ON == 'y') {
				//Clear astparam edid_use instead of set it to 'secondary'. So that unicast mode won't be affected.
				var edid_use_t = ((edid_use == 'primary')?('primary'):(''));
				cmd_str += "astparam s edid_use " + edid_use_t + ";"
				+ "lmparam s EDID_USE " + edid_use + ";";
			}
			if (LM_PARAM.SOC_OP_MODE >= 3) {
				if (v_output_timing_convert == "FFFFFFFF") {
					v_output_timing_convert = parseInt($scaler_output.find(".ID_customize_text").attr("value"), 16);
					v_output_timing_convert || (v_output_timing_convert = '0');
					/* TODO. more validate on v_output_timing_convert  */
					if ((v_output_timing_convert != 0) && !(v_output_timing_convert & 0x80000000)) {
						error("#dummy_hdr_main", "Error:", "Invalid Scaler Output setting!!");
						/* Quit without apply. */
						return;
					}
					v_output_timing_convert = v_output_timing_convert.toString(16).toUpperCase();
				}
				cmd_str += "astparam s v_output_timing_convert " + v_output_timing_convert + ";"
				+ "lmparam s V_OUTPUT_TIMING_CONVERT " + v_output_timing_convert + ";";
			}
		}
		cmd_str	+= "astparam save;"
				+ "lmparam s NO_VIDEO " + no_video + ";"
				+ "lmparam s EN_VIDEO_WALL " + en_video_wall + ";";
		/*
		if (edid_reset) {
			cmd_str += "cat /sys/devices/platform/display/default_edid_" + edid_reset_type + " > /sys/devices/platform/videoip/eeprom_content;"
		}
		*/

		submit(host_ip(), cmd_str);
	}
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "video")) {
		return {
			init: (function(){return this;})
		};
	}
	return {
		init: init
	};
});

/* Callback when tab html loaded. */
function on_tab_functions_loaded()
{
	g.module.video.functions("#field_func_video").init();
	g.module.usb.functions("#field_func_usb").init();
	g.module.serial.functions("#field_func_serial").init();

	return this;
}
