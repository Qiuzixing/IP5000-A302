(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-044db9b9"],{"029c":function(t,e,a){},"0e90":function(t,e,a){"use strict";a.r(e);var s=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-content"},[a("div",{staticClass:"setting-model",staticStyle:{"margin-bottom":"0"}},[a("h3",{staticClass:"setting-model-title"},[t._v("Input HDCP Appearance")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Input 1")]),a("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,1)}},model:{value:t.hdcp[0],callback:function(e){t.$set(t.hdcp,0,e)},expression:"hdcp[0]"}})],1),this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Input 2")]),a("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,2)}},model:{value:t.hdcp[1],callback:function(e){t.$set(t.hdcp,1,e)},expression:"hdcp[1]"}})],1):t._e(),1===this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Input 3")]),a("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,3)}},model:{value:t.hdcp[2],callback:function(e){t.$set(t.hdcp,2,e)},expression:"hdcp[2]"}})],1):t._e(),this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("ALL")]),a("button",{staticClass:"btn btn-plain-primary",on:{click:function(e){return t.setAllHDCP("1")}}},[t._v("ON")]),a("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},on:{click:function(e){return t.setAllHDCP("0")}}},[t._v("OFF")])]):t._e()]),a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-model-title"},[t._v("Display")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Sleep (5V-off) Delay On Video Signal Loss (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay.sleep_delay_on_signal_loss_sec,callback:function(e){t.$set(t.delay,"sleep_delay_on_signal_loss_sec",e)},expression:"delay.sleep_delay_on_signal_loss_sec"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Shutdown (CEC) Delay On Video Signal Loss (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay.shutdown_delay_on_signal_loss_sec,callback:function(e){t.$set(t.delay,"shutdown_delay_on_signal_loss_sec",e)},expression:"delay.shutdown_delay_on_signal_loss_sec"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Wake-up (CEC) Delay On Video Signal Detection (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay.wake_up_delay_on_signal_detection_sec,callback:function(e){t.$set(t.delay,"wake_up_delay_on_signal_detection_sec",e)},expression:"delay.wake_up_delay_on_signal_detection_sec"}})],1)]),a("div",{staticClass:"setting-model"},[a("div",{staticClass:"radio-setting"},[a("span",{staticClass:"setting-title"},[t._v("Sleep Image")]),a("div",{staticClass:"overlay-setting"},[a("div",{staticClass:"overlay-setting-item overlay-img"},[a("span",{staticClass:"file-name",staticStyle:{display:"inline-block"}},[t._v("sleep.png")]),a("span",{staticClass:"upload-icon",on:{click:t.clickUpload}},[a("icon-svg",{attrs:{"icon-class":"upload_img"}})],1),a("input",{ref:"upload",staticStyle:{display:"none"},attrs:{type:"file",accept:"image/jpeg"}}),a("br"),a("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-top":"15px","margin-bottom":"15px"}},[t._v("SLEEP")])])])]),t._m(0),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Maximum Bit Rate")]),a("multiselect",{attrs:{options:t.bitRateParam},model:{value:t.avSignal.maximum_bit_rate,callback:function(e){t.$set(t.avSignal,"maximum_bit_rate",e)},expression:"avSignal.maximum_bit_rate"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Maximum Video Frame Rate (%)")]),a("el-slider",{staticStyle:{width:"200px"},attrs:{min:0,max:100,"show-tooltip":!1,marks:t.marks},model:{value:t.avSignal.frame_rate_percentage,callback:function(e){t.$set(t.avSignal,"frame_rate_percentage",e)},expression:"avSignal.frame_rate_percentage"}}),a("span",{staticStyle:{"margin-left":"15px"}},[t._v(t._s(t.avSignal.frame_rate_percentage))])],1)])]),a("footer",[a("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},i=[function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"24px"}},[a("span",{staticClass:"setting-title"},[t._v("Image Preview")]),a("img",{staticStyle:{"max-width":"320px"},attrs:{src:"/stream",alt:""}})])}],n=(a("498a"),a("ac1f"),a("841c"),a("1276"),a("a434"),a("99af"),a("2ef0")),l={name:"audioPage",data:function(){return{forceRGB:"0",maxBitRate:50,marks:{0:"0",100:"100"},avSignal:{audio_connection_guard_time_sec:90,color_depth:"bypass",force_rgb:"off",frame_rate_percentage:100,input_maximum_resolution:"pass_through",maximum_bit_rate:"best_effort"},delay:{shutdown_delay_on_signal_loss_sec:10,sleep_delay_on_signal_loss_sec:5,wake_up_delay_on_signal_detection_sec:10},hdcp:["0","0","0"],bitRateParam:[{value:"best_effort",label:"Best Effort"},{value:"200",label:"200 Mbps"},{value:"150",label:"150 Mbps"},{value:"100",label:"100 Mbps"},{value:"50",label:"50 Mbps"},{value:"10",label:"10 Mbps"}]}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#HDCP-MOD? 1"),this.$global.deviceType&&(this.$socket.sendMsg("#HDCP-MOD? 2"),1===this.$global.deviceType&&this.$socket.sendMsg("#HDCP-MOD? 3")),this.getAVSignal(),this.getDisplayDelay()},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@HDCP-MOD /i)?-1!==t.search(/@CS-CONVERT /i)&&this.handleForceRGB(t):this.handleHDCP(t)},handleHDCP:function(t){var e=t.split(" ")[1].split(",");this.hdcp.splice(parseInt(e[0])-1,1,e[1])},setAllHDCP:function(t){this.$socket.sendMsg("#HDCP-MOD 1,".concat(t)),this.$socket.sendMsg("#HDCP-MOD 2,".concat(t)),1===this.$global.deviceType&&this.$socket.sendMsg("#HDCP-MOD 3,".concat(t))},setHDCP:function(t,e){this.$socket.sendMsg("#HDCP-MOD ".concat(e,",").concat(t))},clickUpload:function(){this.$refs.upload.click()},getAVSignal:function(){var t=this;this.$http.get("/device/json?path=/av_signal/av_signal.json&t="+Math.random()).then((function(e){e.data.av_signal&&(t.avSignal=e.data.av_signal)}))},setAVSingle:function(){this.$http.post("/device/json",{path:"/av_signal/av_signal.json",info:{av_signal:this.avSignal}})},getDisplayDelay:function(){var t=this;this.$http.get("/device/json?path=/display/display_sleep.json&t="+Math.random()).then((function(e){e.data.display_delays&&(t.delay=e.data.display_delays)}))},setDisplayDelay:function(){this.$http.post("/device/json",{path:"/display/display_sleep.json",info:{display_delays:this.delay}})},handleForceRGB:function(t){this.forceRGB=t.split(",")[1]},save:Object(n["debounce"])((function(){this.setDisplayDelay(),this.setAVSingle()}),2e3,{leading:!0,trailing:!0})}},c=l,o=(a("da59"),a("2877")),d=Object(o["a"])(c,s,i,!1,null,"7f1901ae",null);e["default"]=d.exports},a434:function(t,e,a){"use strict";var s=a("23e7"),i=a("23cb"),n=a("a691"),l=a("50c4"),c=a("7b0b"),o=a("65f0"),d=a("8418"),r=a("1dde"),p=r("splice"),_=Math.max,g=Math.min,u=9007199254740991,v="Maximum allowed length exceeded";s({target:"Array",proto:!0,forced:!p},{splice:function(t,e){var a,s,r,p,h,m,f=c(this),y=l(f.length),b=i(t,y),C=arguments.length;if(0===C?a=s=0:1===C?(a=0,s=y-b):(a=C-2,s=g(_(n(e),0),y-b)),y+a-s>u)throw TypeError(v);for(r=o(f,s),p=0;p<s;p++)h=b+p,h in f&&d(r,p,f[h]);if(r.length=s,a<s){for(p=b;p<y-s;p++)h=p+s,m=p+a,h in f?f[m]=f[h]:delete f[m];for(p=y;p>y-s+a;p--)delete f[p-1]}else if(a>s)for(p=y-s;p>b;p--)h=p+s-1,m=p+a-1,h in f?f[m]=f[h]:delete f[m];for(p=0;p<a;p++)f[p+b]=arguments[p+2];return f.length=y-s+a,r}})},da59:function(t,e,a){"use strict";a("029c")}}]);