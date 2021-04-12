

function on_doc_loaded()
{
	mylog("on_doc_loaded");
	g.$dfd_query_lm_param.done(function(){
		if (!LM_PARAM.EN_VIDEO_WALL || (LM_PARAM.EN_VIDEO_WALL == 'n')) {
			//window.location="../"
			LM_PARAM.EN_VIDEO_WALL='n';
		}
		if (is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "video_wall") && (LM_PARAM.EN_VIDEO_WALL == 'y')) {
			$("#tab-video_wall").show();
		}
		if (is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "network")) {
			$("#tab-network").show();
		}
		if (is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "functions")) {
			$("#tab-functions").show();
		}
		if (is_cfg_page_on(LM_PARAM.WEB_UI_CFG, "god")) {
			$("#tab-god").show();
		}
	});
}

function on_doc_ready()
{
	mylog("on_doc_ready");
	if ($.browser.msie)
		notice("#dummy_hdr_main", 'highlight', 'Note!', 'Best viewed with Chrome 9+ @ 1024x768.', function() {
				setTimeout(function (){
					notice("#dummy_hdr_main", 'clear');
				}, 5000);
			});

	$('#tabs').tabs({
		show:on_tab_show,
		load:on_tab_loaded
	});
	$('#tab_list').show();

	$("#tabs").triggerHandler("tabsshow");
	//$.uicornerfix('5px');

}

function on_doc_init()
{
	g.$view = $(window);
	notice_block_init();
	//Create "$dialog_loading"
	g.$dialog_loading = $("<div></div>")
					.html("<table style='margin-bottom:auto;margin-top:auto;height:50px'><tr><td style='vertical-align:middle'><h4 style='margin-bottom:0'>Loading...<img src=\"images/ajax-loader.gif\"/></h4></td></tr></table>")
					.dialog({
						autoOpen: false,
						modal: true,
						closeOnEscape: false,
						draggable: false,
						resizable: false,
						hide: 'fade', //TBD effect
						//title: 'Welcome',
						width: 200,
						height: 50,
						//minHeight: 50,
						//dialogClass: 'no-close',
						open: function(event, ui) {
							$(".ui-dialog-titlebar-close", $(this).parent()).hide(); //Hide close button
							$(".ui-dialog-titlebar", $(this).parent()).hide();  //Hide the title bar
							$(".ui-dialog-content", $(this).parent()).height('50px').css('min-height','10px'); //A heck to patch the height value in chrome
						}
					});

	g.$dfd_query_lm_param = $.Deferred(function(dfd){
		g.$dialog_loading.dialog('open');
		query_lm_param(function(lm_param) {
			g.$dialog_loading.dialog('close');
			dfd.resolve();
		});
	});
}

function on_tab_loaded(event, ui) {
	switch (ui.tab.id)
	{
	case "tab-system":
	/*
		load_scripts('js/tab-system.js', function() {
			on_tab_system_loaded();
		});
	*/
		g.$dfd_query_lm_param.done(on_tab_system_loaded);
		break;
	case "tab-network":
	/*
		load_scripts('js/tab-network.js', function() {
			on_tab_network_loaded();
		});
	*/
		g.$dfd_query_lm_param.done(on_tab_network_loaded);
		break;
	case "tab-video_wall":
	/*
		load_scripts(
			[
				'js/vw_target_selection_lan.js',
				'js/tab-video_wall.js'				
			],
			function() {
				on_tab_video_wall_loaded();
			}
		);
	*/
		g.$dfd_query_lm_param.done(on_tab_video_wall_loaded);
		break;
	case "tab-functions":
		g.$dfd_query_lm_param.done(on_tab_functions_loaded);
		break;
	default:
		break;
	}
}

//
// Handle Events of tab changes
//
function on_tab_show(event, ui) {
	var selected = ui.tab.id;

	switch (ui.tab.id)
	{
	case "tab-system":
		break;
	case "tab-video_wall":
		g.$dialog_loading.dialog('open');
		query_nodes(function(){
			g.$dialog_loading.dialog('close');
		});
		break;
	default:
		break;
	}
}