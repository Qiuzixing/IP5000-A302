(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-0b3fc146"],{"0e90":function(t,e,a){"use strict";a.r(e);var s=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-content"},[a("div",{staticClass:"setting-model",staticStyle:{"margin-bottom":"0"}},[a("h3",{staticClass:"setting-model-title"},[t._v("Input HDCP Appearance")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Input 1")]),a("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,1)}},model:{value:t.hdcp[0],callback:function(e){t.$set(t.hdcp,0,e)},expression:"hdcp[0]"}})],1),this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Input 2")]),a("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,2)}},model:{value:t.hdcp[1],callback:function(e){t.$set(t.hdcp,1,e)},expression:"hdcp[1]"}})],1):t._e(),1===this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Input 3")]),a("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,3)}},model:{value:t.hdcp[2],callback:function(e){t.$set(t.hdcp,2,e)},expression:"hdcp[2]"}})],1):t._e(),this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("ALL")]),a("button",{staticClass:"btn btn-plain-primary",on:{click:function(e){return t.setAllHDCP("1")}}},[t._v("ON")]),a("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},on:{click:function(e){return t.setAllHDCP("0")}}},[t._v("OFF")])]):t._e()]),a("div",{staticClass:"setting-model",staticStyle:{"margin-top":"50px"}},[t._m(0),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Maximum Bit Rate")]),a("multiselect",{attrs:{options:t.bitRateParam},model:{value:t.avSignal.maximum_bit_rate,callback:function(e){t.$set(t.avSignal,"maximum_bit_rate",e)},expression:"avSignal.maximum_bit_rate"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Maximum Video Frame Rate (%)")]),a("el-slider",{staticStyle:{width:"200px"},attrs:{min:0,max:100,"show-tooltip":!1,marks:t.marks},model:{value:t.avSignal.frame_rate_percentage,callback:function(e){t.$set(t.avSignal,"frame_rate_percentage",e)},expression:"avSignal.frame_rate_percentage"}}),a("span",{staticStyle:{"margin-left":"15px"}},[t._v(t._s(t.avSignal.frame_rate_percentage))])],1)])]),a("footer",[a("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},i=[function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"24px"}},[a("span",{staticClass:"setting-title"},[t._v("Image Preview")]),a("img",{staticStyle:{"max-width":"320px"},attrs:{src:"/stream",alt:""}})])}],n=(a("498a"),a("ac1f"),a("841c"),a("1276"),a("a434"),a("99af"),a("b0c0"),a("2ef0")),l={name:"audioPage",data:function(){return{forceRGB:"0",maxBitRate:50,marks:{0:"0",100:"100"},avSignal:{audio_connection_guard_time_sec:90,color_depth:"bypass",force_rgb:"off",frame_rate_percentage:100,input_maximum_resolution:"pass_through",maximum_bit_rate:"best_effort"},delay:{shutdown_delay_on_signal_loss_sec:10,sleep_delay_on_signal_loss_sec:5,wake_up_delay_on_signal_detection_sec:10},hdcp:["0","0","0"],bitRateParam:[{value:"best_effort",label:"Best Effort"},{value:"200",label:"200 Mbps"},{value:"150",label:"150 Mbps"},{value:"100",label:"100 Mbps"},{value:"50",label:"50 Mbps"},{value:"10",label:"10 Mbps"}],imgError:!1,imgName:"",uploadComplete:!1}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#HDCP-MOD? 1"),this.$global.deviceType&&(this.$socket.sendMsg("#HDCP-MOD? 2"),1===this.$global.deviceType&&this.$socket.sendMsg("#HDCP-MOD? 3")),this.getAVSignal()},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@HDCP-MOD /i)?-1!==t.search(/@CS-CONVERT /i)&&this.handleForceRGB(t):this.handleHDCP(t)},handleHDCP:function(t){var e=t.split(" ")[1].split(",");this.hdcp.splice(parseInt(e[0])-1,1,e[1])},setAllHDCP:function(t){this.$socket.sendMsg("#HDCP-MOD 1,".concat(t)),this.$socket.sendMsg("#HDCP-MOD 2,".concat(t)),1===this.$global.deviceType&&this.$socket.sendMsg("#HDCP-MOD 3,".concat(t))},setHDCP:function(t,e){this.$socket.sendMsg("#HDCP-MOD ".concat(e,",").concat(t))},clickUpload:function(){this.$refs.upload.click()},getAVSignal:function(){var t=this;this.$http.get("/device/json?path=/av_signal/av_signal.json&t="+Math.random()).then((function(e){e.data.av_signal&&(t.avSignal=e.data.av_signal)}))},setAVSingle:function(){this.$http.post("/device/json",{path:"/av_signal/av_signal.json",info:{av_signal:this.avSignal}})},getDisplayDelay:function(){var t=this;this.$http.get("/device/json?path=/display/display_sleep.json&t="+Math.random()).then((function(e){e.data.display_delays&&(t.delay=e.data.display_delays)}))},setDisplayDelay:function(){this.$http.post("/device/json",{path:"/display/display_sleep.json",info:{display_delays:this.delay}})},handleForceRGB:function(t){this.forceRGB=t.split(",")[1]},save:Object(n["debounce"])((function(){this.setAVSingle()}),2e3,{leading:!0,trailing:!0}),browseImg:function(t){var e,a=this;if(this.imgName=(null===(e=t.target.files[0])||void 0===e?void 0:e.name)||"",this.imgName){var s=t.target.files[0];if("image/jpeg"!==s.type)return void(this.imgError=!0);var i=new FileReader;i.onload=function(t){var e=t.target.result,i=new Image;i.onload=function(){var t=i.width,e=i.height;if(1280!==t||720!==e)a.imgError=!0;else{a.imgError=!1;var n=new XMLHttpRequest,l=new FormData;l.append("file",s),n.open("POST","/upload/sleepimage"),n.onload=function(){200===n.status&&(a.imgName="",a.uploadComplete=!0,setTimeout((function(){a.uploadComplete=!1}),2e3))},n.send(l)}},i.src=e},i.readAsDataURL(s)}else this.imgError=!1}}},c=l,o=(a("b156"),a("2877")),r=Object(o["a"])(c,s,i,!1,null,"42a3d95a",null);e["default"]=r.exports},6456:function(t,e,a){},a434:function(t,e,a){"use strict";var s=a("23e7"),i=a("23cb"),n=a("a691"),l=a("50c4"),c=a("7b0b"),o=a("65f0"),r=a("8418"),d=a("1dde"),p=d("splice"),g=Math.max,u=Math.min,h=9007199254740991,v="Maximum allowed length exceeded";s({target:"Array",proto:!0,forced:!p},{splice:function(t,e){var a,s,d,p,m,f,_=c(this),b=l(_.length),C=i(t,b),y=arguments.length;if(0===y?a=s=0:1===y?(a=0,s=b-C):(a=y-2,s=u(g(n(e),0),b-C)),b+a-s>h)throw TypeError(v);for(d=o(_,s),p=0;p<s;p++)m=C+p,m in _&&r(d,p,_[m]);if(d.length=s,a<s){for(p=C;p<b-s;p++)m=p+s,f=p+a,m in _?_[f]=_[m]:delete _[f];for(p=b;p>b-s+a;p--)delete _[p-1]}else if(a>s)for(p=b-s;p>C;p--)m=p+s-1,f=p+a-1,m in _?_[f]=_[m]:delete _[f];for(p=0;p<a;p++)_[p+C]=arguments[p+2];return _.length=b-s+a,d}})},b156:function(t,e,a){"use strict";a("6456")}}]);