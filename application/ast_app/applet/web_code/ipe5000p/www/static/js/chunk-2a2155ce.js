(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-2a2155ce"],{"1a7f":function(t,e,s){"use strict";s.r(e);var a=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-model"},[1===this.$global.deviceType?s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Input Selection")]),s("multiselect",{attrs:{options:t.inputSelect.param},on:{input:t.setInputSelect},model:{value:t.inputSelect.val,callback:function(e){t.$set(t.inputSelect,"val",e)},expression:"inputSelect.val"}})],1):t._e(),2===this.$global.deviceType?s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Input Selection")]),s("multiselect",{attrs:{options:t.inputSelect.param2},on:{input:t.setInputSelect},model:{value:t.inputSelect.val,callback:function(e){t.$set(t.inputSelect,"val",e)},expression:"inputSelect.val"}})],1):t._e(),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Stream ID")]),s("el-input-number",{attrs:{"controls-position":"right",max:999,min:1},model:{value:t.channel,callback:function(e){t.channel=e},expression:"channel"}}),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{type:"button"},on:{click:t.setChannel}},[t._v("APPLY")])],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Stream Name")]),s("div",{staticStyle:{position:"relative"}},[s("input",{directives:[{name:"model",rawName:"v-model",value:t.channelName,expression:"channelName"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"24"},domProps:{value:t.channelName},on:{input:function(e){e.target.composing||(t.channelName=e.target.value)}}}),t.isChannelName(t.channelName)?t._e():s("span",{staticClass:"range-alert",staticStyle:{top:"34px","white-space":"nowrap"}},[t._v("Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed")])]),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{type:"button"},on:{click:t.setChannelName}},[t._v("APPLY")])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Volume (dB)")]),s("el-slider",{staticStyle:{width:"200px"},attrs:{min:0,max:100,"show-tooltip":!1,marks:t.marks},on:{change:t.setVolume},model:{value:t.volume,callback:function(e){t.volume=e},expression:"volume"}}),s("span",{staticStyle:{"margin-left":"15px"}},[t._v(t._s(t.volume))])],1),s("div",{staticClass:"setting",staticStyle:{"margin-top":"36px"}},[s("span",{staticClass:"setting-title"},[t._v("Mute")]),s("v-switch",{attrs:{"active-value":"on","inactive-value":"off"},on:{change:t.setAudioMute},model:{value:t.muteVal,callback:function(e){t.muteVal=e},expression:"muteVal"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Play/Stop")]),s("v-switch",{attrs:{"open-text":"Play","close-text":"Stop","active-value":"1","inactive-value":"0"},on:{change:t.setAction},model:{value:t.action,callback:function(e){t.action=e},expression:"action"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("HDCP Encryption")]),s("span",[t._v(t._s("1"==t.hdcp?"On":"Off"))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Resolution")]),s("span",[t._v(t._s(t.resolution))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Aspect Ratio")]),s("span",[t._v(t._s(t.aspectRatio))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v(" Audio Channels")]),s("span",[t._v(t._s(t.audioChannel))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v(" Audio Rate")]),s("span",[t._v(t._s(t.audioRate))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Audio Format")]),s("span",[t._v(t._s(t.audioFormat))])])])])},i=[],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),{0:"No Signal ",1:"640x480p@60Hz",2:"720x480p@60Hz",3:"720x480p@60Hz",4:"1280x720p@60Hz",5:"1920x1080i@60Hz",6:"720x480i@60Hz",7:"720x480i@60Hz",8:"720x240p@60Hz",9:"720x240p@60Hz",10:"2880x480i@60Hz",11:"2880x480i@60Hz",12:"2880x240p@60Hz",13:"2880x240p@60Hz",14:"1440x480p@60Hz",15:"1440x480p@60Hz",16:"1920x1080p@60Hz",17:"720x576p@50Hz",18:"720x576p@50Hz",19:"1280x720p@50Hz",20:"1920x1080i@50Hz",21:"720x576i@50Hz",22:"720x576i@50Hz",23:"720x288p@50Hz",24:"720x288p@50Hz",25:"2880x576i@50Hz",26:"2880x576i@50Hz",27:"2880x288p@50Hz",28:"2880x288p@50Hz",29:"1440x576p@50Hz",30:"1440x576p@50Hz",31:"1920x1080p@50Hz",32:"1920x1080p@24Hz",33:"1920x1080p@25Hz",34:"1920x1080p@30Hz",35:"2880x480p@60Hz",36:"2880x480p@60Hz",37:"2880x576p@50Hz",38:"2880x576p@50Hz",39:"1920x1080i@50Hz",40:"1920x1080i@100Hz",41:"1280x720p@100Hz",42:"720x576p@100Hz",43:"720x576p@100Hz",44:"720x576i@100Hz",45:"720x576i@100Hz",46:"1920x1080i@120Hz",47:"1280x720p@120Hz",48:"720x480p@120Hz",49:"720x480p@120Hz",50:"720x480i@120Hz",51:"720x480i@120Hz",52:"720x576p@200Hz",53:"720x576p@200Hz",54:"720x576i@200Hz",55:"720x576i@200Hz",56:"720x480p@240Hz",57:"720x480p@240Hz",58:"720x480i@240Hz",59:"720x480i@240Hz",60:"1280x720p@24Hz",61:"1280x720p@25Hz",62:"1280x720p@30Hz",63:"1920x1080p@120Hz",64:"1920x1080p@100Hz",65:"800x600p@60Hz",66:"1024x768@60Hz",67:"1280x768p@60Hz",68:"1280x1024p@60Hz",69:"1600x1200p@60Hz",70:"1680x1050p@60Hz",71:"1920x1200@60Hz",72:"3840x2160p@24Hz",73:"3840x2160p@25Hz",74:"3840x2160p@30Hz",75:"3840x2160p@50Hz",76:"3840x2160p@60Hz",100:"Custom resolution 1",101:"Custom resolution 2",102:"Custom resolution 3",103:"Custom resolution 4",104:"Custom resolution 5"}),l=n,o={name:"av_routing",data:function(){return{marks:{0:"0",100:"100"},volume:0,inputSelect:{val:"1",param:[{value:"1",label:"HDMI IN1"},{value:"2",label:"HDMI IN2"},{value:"3",label:"USB IN3"}],param2:[{value:"1",label:"HDMI IN1"},{value:"2",label:"USB IN2"}]},channel:"",channelName:"",action:"0",muteVal:"0",aspectRatio:"",resolution:"",hdcp:"0",audioChannel:"",audioFormat:"",audioRate:"",channelList:[]}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$global.deviceType&&this.$socket.sendMsg("#X-ROUTE? out.hdmi.1.video.1"),this.$socket.sendMsg("#KDS-DEFINE-CHANNEL? "),this.$socket.sendMsg("#KDS-DEFINE-CHANNEL-NAME? "),this.$socket.sendMsg("#X-AUD-LVL? out.analog_audio.1.audio.1"),this.$socket.sendMsg("#KDS-ACTION? "),this.$socket.sendMsg("#X-MUTE? out.stream.1.audio.1"),this.$socket.sendMsg("#HDCP-STAT? 1,1"),this.$socket.sendMsg("#KDS-RESOL? 1,1,1"),this.$socket.sendMsg("#KDS-RATIO? 1,1,1"),this.$socket.sendMsg("#X-AUD-DESC? out.hdmi.1")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@X-ROUTE /i)?-1===t.search(/@KDS-DEFINE-CHANNEL /i)?-1===t.search(/@KDS-DEFINE-CHANNEL-NAME /i)?-1===t.search(/@X-MUTE /i)?-1===t.search(/@X-AUD-LVL /i)?-1===t.search(/@KDS-ACTION /i)?-1===t.search(/@HDCP-STAT /i)?-1===t.search(/@KDS-RESOL /i)?-1===t.search(/@KDS-RATIO /i)?-1!==t.search(/@X-AUD-DESC /i)&&this.handleAudioChannelDesc(t):this.handleRatio(t):this.handleResolution(t):this.handleHDCPStatus(t):this.handleAction(t):this.handleVolume(t):this.handleAudioMute(t):this.handleChannelName(t):this.handleChannel(t):this.handleInputSelect(t)},handleInputSelect:function(t){this.inputSelect.val=t.split(",")[1].split(".")[2]},setInputSelect:function(t){this.$socket.sendMsg("#X-ROUTE out.hdmi.1.video.1,in.hdmi.".concat(t,".video.1"))},handleChannel:function(t){this.channel=t.split(" ")[1]},handleChannelName:function(t){this.channelName=t.split(" ")[1]},setChannelName:function(){this.isChannelName(this.channelName)&&this.$socket.sendMsg("#KDS-DEFINE-CHANNEL-NAME ".concat(this.channelName))},handleAudioMute:function(t){this.muteVal=t.split(",").pop()},setAudioMute:function(t){this.$socket.sendMsg("#X-MUTE out.stream.1.audio.1,".concat(t))},handleVolume:function(t){var e=t.split(",");this.volume=parseInt(e[e.length-1])},setVolume:function(){this.$socket.sendMsg("#X-AUD-LVL out.analog_audio.1.audio.1,".concat(this.volume))},handleAction:function(t){this.action=t.split(" ")[1]},setAction:function(t){this.$socket.sendMsg("#KDS-ACTION ".concat(t))},handleHDCPStatus:function(t){var e=t.split(",");this.hdcp=e[e.length-1]},handleResolution:function(t){var e=t.split(",");this.resolution=l[e[e.length-1]]||"unknown"},handleRatio:function(t){this.aspectRatio=t.split(" ")[1]},handleAudioChannelDesc:function(t){var e=t.split(",");this.audioFormat=e[e.length-1],this.audioRate=e[e.length-2],this.audioChannel=e[e.length-3]},setChannel:function(){this.$socket.sendMsg("#KDS-DEFINE-CHANNEL "+(this.channel||1))},isChannelName:function(t){return/^[a-zA-Z0-9][_\-a-zA-Z0-9]{0,23}$/.test(t)}}},c=o,p=(s("e4d2"),s("2877")),u=Object(p["a"])(c,a,i,!1,null,"c17a9c14",null);e["default"]=u.exports},"7eca":function(t,e,s){},e4d2:function(t,e,s){"use strict";s("7eca")}}]);