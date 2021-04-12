
/* Create network root module. */
g.module.net || (g.module.net = {});

/* Create ip_mode module */
g.module.net.ip_mode = (function (selector) {
	//Private members.
	var that;
	var ip_mode;
	var ip;
	var netmask;
	var gateway;
	var static_ip;
	var static_netmask;
	var static_gateway;
	var dhcp_ip;
	var dhcp_netmask;
	var dhcp_gateway;
	var auto_ip;
	var auto_netmask;
	var auto_gateway;
	var $root = $(selector),
		$ip = $root.find("input:text[name~=text_network_ip_addr]"),
		$netmask = $root.find("input:text[name~=text_network_netmask]"),
		$gateway = $root.find("input:text[name~=text_network_gateway]"),
		$ip_mode_option = $root.find(".ID_ip_mode"),
		$btn_apply = $root.find("button");

	function init() {
		that = this;
		//Initialize LM_PARAM
		LM_PARAM || (LM_PARAM = {});
		LM_PARAM.IP_MODE || (LM_PARAM.IP_MODE = 'autoip');
		LM_PARAM.MY_IP || (LM_PARAM.MY_IP = '0.0.0.0');
		LM_PARAM.MY_NETMASK || (LM_PARAM.MY_NETMASK = '0.0.0.0');
		LM_PARAM.MY_GATEWAYIP || (LM_PARAM.MY_GATEWAYIP = '0.0.0.0');
		LM_PARAM.IPADDR || (LM_PARAM.IPADDR = '0.0.0.0');
		LM_PARAM.NETMASK ||(LM_PARAM.NETMASK = '0.0.0.0');
		LM_PARAM.GATEWAYIP || (LM_PARAM.GATEWAYIP = '0.0.0.0');
		//Initialize UI
		load_setting_from_runtime();
		$ip_mode_option.buttonset();
		$btn_apply.button().click(on_apply);
		//Refresh the ip_mode based on runtime setting
		//ui_refresh(ip_mode);
		ui_refresh_ip_mode_input(ip_mode);

		$ip_mode_option.find("input[type=radio]")
			.change(on_ip_mode_change) //Register change handler
			.change(); //fire change event. This is the last line of init()
		//$("#network_ip_mode").find("input[type=radio]").change(on_ip_mode_change);

		$root.show();
		return this;
	};
	function on_apply() {
		var the_ip_mode = $ip_mode_option.find("input:radio[name=network_ip_mode]:checked").val(),
		    the_ip,
		    the_netmask,
		    the_gateway;
		var cmd_str;
		var submit = function(host, cmd_str) {
			var cmd = $.param({
				cache: false,
				nocache: (new Date()).getTime(), //don't cache cgi get hack for IE
				cmd: cmd_str
			});
			$.getJSON(host+CGI_PATH+"query.cgi?callback=?", cmd, function(){})
				.success(function(){
					highlight("#dummy_hdr_main", "Success:", "New IP mode (" + the_ip_mode + ") applied.");
					msg_stick("#dummy_hdr_main", "Warning:", g.notice_block.STR_RESET);
					//Update internal variables
					ip_mode = LM_PARAM.IP_MODE = the_ip_mode;
					if (the_ip_mode == 'static') {
						static_ip = LM_PARAM.IPADDR = the_ip;
						static_netmask = LM_PARAM.NETMASK = the_netmask;
						static_gateway = LM_PARAM.GATEWAYIP = the_gateway;
					}
				})
				.error(function(){error("#dummy_hdr_main", "Error:", "Try again.");})
				.complete(function(){});
		};

		cmd_str = "astparam s ip_mode " + the_ip_mode + ";"
		        + "lmparam s IP_MODE " + the_ip_mode + ";";
		if (the_ip_mode == 'static') {
			the_ip = $ip.val();
			the_netmask = $netmask.val();
			the_gateway = $gateway.val();
			if (!is_ip_valid(the_ip)) {
				error("#dummy_hdr_main", "Error:", "Invalid IP Address: " + the_ip);
				$ip.val("");
				return;
			}
			if (!is_ip_valid(the_netmask)) {
				error("#dummy_hdr_main", "Error:", "Invalid Subnet Mask: " + the_netmask);
				$netmask.val("");
				return;
			}
			if (!is_ip_valid(the_gateway)) {
				error("#dummy_hdr_main", "Error:", "Invalid Default Gateway: " + the_gateway);
				$gateway.val("");
				return;
			}
			//Construct cmd string
			cmd_str += "astparam s ipaddr " + the_ip + ";"
			         + "astparam s netmask " + the_netmask + ";"
			         + "astparam s gatewayip " + the_gateway + ";"
			         + "lmparam s IPADDR " + the_ip + ";"
			         + "lmparam s NETMASK " + the_netmask + ";"
			         + "lmparam s GATEWAYIP " + the_gateway + ";";
		}
		cmd_str += "astparam save;";
		submit(host_ip(), cmd_str);
	};

	function is_ip_valid(ip_str) {
		//Reference from http://www.experts-exchange.com/Programming/Languages/Scripting/JavaScript/A_1074-Checking-for-valid-IP-addresses-using-JavaScript-Regular-Expressions.html
		var octet = '(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])';
		var ip    = '(?:' + octet + '\\.){3}' + octet;
		var ipRE  = new RegExp( '^' + ip + '$' );
		
		return ipRE.test(ip_str);
	};

	function load_setting_from_runtime() {
		ip_mode = LM_PARAM.IP_MODE;
		ip = LM_PARAM.MY_IP;
		netmask = LM_PARAM.MY_NETMASK;
		gateway = LM_PARAM.MY_GATEWAYIP;
		switch (LM_PARAM.IP_MODE) {
			case "autoip":
				auto_ip = LM_PARAM.MY_IP;
				auto_netmask = LM_PARAM.MY_NETMASK;
				auto_gateway = LM_PARAM.MY_GATEWAYIP;
				static_ip = LM_PARAM.IPADDR;
				static_netmask = LM_PARAM.NETMASK;
				static_gateway = LM_PARAM.GATEWAYIP;
				dhcp_ip = "(From DHCP Server)";
				dhcp_netmask = "(From DHCP Server)";
				dhcp_gateway = "(From DHCP Server)";
				break;
			case "static":
				auto_ip = "169.254.xxx.xxx (Auto)";
				auto_netmask = "255.255.0.0";
				auto_gateway = LM_PARAM.GATEWAYIP;
				static_ip = LM_PARAM.IPADDR;
				static_netmask = LM_PARAM.NETMASK;
				static_gateway = LM_PARAM.GATEWAYIP;
				dhcp_ip = "(From DHCP Server)";
				dhcp_netmask = "(From DHCP Server)";
				dhcp_gateway = "(From DHCP Server)";
				break;
			case "dhcp":
				auto_ip = "169.254.xxx.xxx (Auto)";
				auto_netmask = "255.255.0.0";
				auto_gateway = LM_PARAM.GATEWAYIP;
				static_ip = LM_PARAM.IPADDR;
				static_netmask = LM_PARAM.NETMASK;
				static_gateway = LM_PARAM.GATEWAYIP;
				dhcp_ip = LM_PARAM.MY_IP;
				dhcp_netmask = LM_PARAM.MY_NETMASK;
				dhcp_gateway = LM_PARAM.MY_GATEWAYIP;
				break;
			default:
				auto_ip = LM_PARAM.MY_IP;
				auto_netmask = LM_PARAM.MY_NETMASK;
				auto_gateway = LM_PARAM.MY_GATEWAYIP;
				static_ip = LM_PARAM.IPADDR;
				static_netmask = LM_PARAM.NETMASK;
				static_gateway = LM_PARAM.GATEWAYIP;
				dhcp_ip = "(From DHCP Server)";
				dhcp_netmask = "(From DHCP Server)";
				dhcp_gateway = "(From DHCP Server)";
				break;
		}
	};
	function on_ip_mode_change() {
		//load_setting_from_runtime();
		ui_refresh();
	};
	function ui_refresh(use_ip_mode) {
		var the_ip_mode = use_ip_mode || $ip_mode_option.find("input:radio[name=network_ip_mode]:checked").val();
		//Disable all text input
		$ip.attr("disabled", "disabled").attr("readonly", true);
		$netmask.attr("disabled", "disabled").attr("readonly", true);
		$gateway.attr("disabled", "disabled").attr("readonly", true);

		switch (the_ip_mode) {
			case "autoip":
				$ip.attr("value", auto_ip);
				$netmask.attr("value", auto_netmask);
				$gateway.attr("value", auto_gateway);
				break;
			case "static":
				$ip.attr("value", static_ip);
				$netmask.attr("value", static_netmask);
				$gateway.attr("value", static_gateway);

				$ip.removeAttr("disabled").removeAttr("readonly");
				$netmask.removeAttr("disabled").removeAttr("readonly");
				$gateway.removeAttr("disabled").removeAttr("readonly");
				break;
			case "dhcp":
				$ip.attr("value", dhcp_ip);
				$netmask.attr("value", dhcp_netmask);
				$gateway.attr("value", dhcp_gateway);
				break;
			default:
				$ip.attr("value", "??");
				$netmask.attr("value", "??");
				$gateway.attr("value", "??");
				break;
		}
	};
	function ui_refresh_ip_mode_input(ip_mode) {
		$ip_mode_option.find("[value~=" + ip_mode + "]").attr("checked", true);
		$ip_mode_option.buttonset("refresh");
	};

	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "network")) {
		return {
			init: (function(){return this;})
		};	
	}
	return {
		init: init
	};
});

/* Create casting_mode module */
g.module.net.casting_mode = (function (selector) {
	var is_multicast = false, apply_usb = false;
	var $root = $(selector), 
	    $option = $root.find(".ID_casting_option"),
	    $share_usb_auto = $root.find(".ID_share_usb_auto"),
	    $btn_apply = $root.find("button");

	function init() {
		//Initialize LM_PARAM
		LM_PARAM || (LM_PARAM = {});
		LM_PARAM.MULTICAST_ON || (LM_PARAM.MULTICAST_ON = 'n');
		LM_PARAM.SHARE_USB_AUTO_MODE || (LM_PARAM.SHARE_USB_AUTO_MODE = 'n');
		load_setting_from_runtime();
		$option.buttonset();
		$btn_apply.button().click(on_apply);
		ui_refresh_input(is_multicast);
		if (is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "usb")) {
			$share_usb_auto.show();
			apply_usb = true;
		}
		$root.show();
		return this;
	};
	function ui_refresh_input(is_multicast) {
		if (is_multicast) {
			$option.find(".multicast").attr("checked", true);
		} else {
			$option.find(".unicast").attr("checked", true);
		}
		$option.buttonset("refresh");

		$share_usb_auto.find("input:checkbox").attr("checked", 
			(LM_PARAM.SHARE_USB_AUTO_MODE == 'y')?(true):(false)
			);
	}
	function load_setting_from_runtime() {
		is_multicast = (LM_PARAM.MULTICAST_ON == 'y')?(true):(false);
	};
	function is_share_usb_auto_checked() {
		return ($share_usb_auto.find("input:checkbox").attr("checked"))?true:false;
	};
	function on_apply() {
		var cmd_str;
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
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "network")) {
		return {
			init: (function(){return this;})
		};	
	}
	return {
		init: init
	};	
});
/* Create host_name module */
g.module.net.host_name = (function () {
	function init() {
		return this;
	};
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "network")) {
		return {
			init: (function(){})
		};	
	}
	return {
		init: init
	};	
});

/* Create net_mac module */
g.module.net.mac = (function () {
	function init() {
		return this;
	};
	//Public members.
	// The module interface
	if (!is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "network")) {
		return {
			init: (function(){})
		};	
	}
	return {
		init: init
	};	
});

/* Callback when tab html loaded. */
function on_tab_network_loaded()
{
	g.module.net.ip_mode("#field_net_ip_setup").init();
	g.module.net.casting_mode("#field_net_casting").init();

	return this;
}