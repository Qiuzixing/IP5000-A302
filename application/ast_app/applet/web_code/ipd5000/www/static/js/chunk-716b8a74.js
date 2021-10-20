(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-716b8a74"],{"0e90":function(t,e,a){"use strict";a.r(e);var s=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-model"},[a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Maximum Resolution")]),a("multiselect",{attrs:{options:t.maxResolution.param},model:{value:t.maxResolution.val,callback:function(e){t.$set(t.maxResolution,"val",e)},expression:"maxResolution.val"}})],1),a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-model-title"},[t._v("Display")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Sleep (5V-off) Delay On Video Signal Loss (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay.sleep_delay_on_signal_loss_sec,callback:function(e){t.$set(t.delay,"sleep_delay_on_signal_loss_sec",e)},expression:"delay.sleep_delay_on_signal_loss_sec"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Shutdown (CEC) Delay On Video Signal Loss (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay.shutdown_delay_on_signal_loss_sec,callback:function(e){t.$set(t.delay,"shutdown_delay_on_signal_loss_sec",e)},expression:"delay.shutdown_delay_on_signal_loss_sec"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Wake-up (CEC) Delay On Video Signal Detection (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay.wake_up_delay_on_signal_detection_sec,callback:function(e){t.$set(t.delay,"wake_up_delay_on_signal_detection_sec",e)},expression:"delay.wake_up_delay_on_signal_detection_sec"}})],1)]),a("div",{staticClass:"radio-setting"},[a("span",{staticClass:"setting-title"},[t._v("Sleep Image")]),a("div",{staticClass:"overlay-setting"},[a("div",{staticClass:"overlay-setting-item overlay-img"},[a("span",{staticClass:"file-name",staticStyle:{display:"inline-block"}},[t._v("sleep.png")]),a("span",{staticClass:"upload-icon",on:{click:t.clickUpload}},[a("icon-svg",{attrs:{"icon-class":"upload_img"}})],1),a("input",{ref:"upload",staticStyle:{display:"none"},attrs:{type:"file",accept:"image/jpeg"}}),a("br"),a("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-top":"15px","margin-bottom":"15px"}},[t._v("SLEEP")])])])]),t._m(0),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Force RGB")]),a("v-checkbox",{attrs:{"active-value":"1","inactive-value":"0"},model:{value:t.forceRGB,callback:function(e){t.forceRGB=e},expression:"forceRGB"}})],1)]),a("footer",[a("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},i=[function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"24px"}},[a("span",{staticClass:"setting-title"},[t._v("Image Preview")]),a("img",{staticStyle:{"max-width":"320px"},attrs:{src:"/stream",alt:""}})])}],l=(a("498a"),a("ac1f"),a("841c"),a("1276"),a("99af"),function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("label",{staticClass:"checkbox",class:{"is-checked":t.checked,isDisabled:t.disabled},on:{click:t.changeVal}},[a("span",{staticClass:"checkbox-input",class:{"is-checked":t.checked}},[a("span",{staticClass:"checkbox-inner"}),a("span",{staticClass:"checkbox-inner-label"},[t._v(t._s(t.label))])])])}),n=[],o=(a("a9e3"),{name:"checkBox",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},activeValue:{type:[Boolean,String,Number],default:!0},inactiveValue:{type:[Boolean,String,Number],default:!1},value:{}},data:function(){return{}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}},checked:function(){return this.value===this.activeValue}},methods:{changeVal:function(){if(!this.disabled){var t=this.checked?this.inactiveValue:this.activeValue;this.$emit("input",t),this.$emit("click",t)}}},watch:{checked:function(t){this.isChecked=t}}}),c=o,d=(a("3791"),a("2877")),u=Object(d["a"])(c,l,n,!1,null,null,null),p=u.exports,r={name:"audioPage",components:{vCheckbox:p},data:function(){return{showColorDepth:!1,forceRGB:"0",value:50,switchMode:{val:"last_connected",param:[{value:"last_connected",label:"Last Connected"},{value:"priority",label:"Priority"},{value:"manual",label:"Manual"}]},hdcp:"off",hdcp2:"off",hdcp3:"off",test:"0",show:!1,maxResolution:{val:"16",param:[{value:"0",label:"Pass Through"},{value:"4",label:"Full HD (720p60)"},{value:"16",label:"Full HD (1080p60)"},{value:"31",label:"Full HD (1080p50)"},{value:"73",label:"Ultra HD 2160p25"},{value:"74",label:"Ultra HD 2160p30"}]},delay:{sleep_delay_on_signal_loss_sec:5,shutdown_delay_on_signal_loss_sec:10,wake_up_delay_on_signal_detection_sec:10}}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#KDS-SCALE? "),this.$socket.sendMsg("#CS-CONVERT? 1"),this.getDisplayDelay()},methods:{handleMsg:function(t){-1===t.search(/@X-ROUTE /i)?(-1!==t.search(/@CS-CONVERT /i)&&this.handleForceRGB(t),-1!==t.search(/@KDS-SCALE /i)&&this.handleResolution(t)):this.handleInputSelect(t)},clickUpload:function(){this.$refs.upload.click()},handleForceRGB:function(t){this.forceRGB=t.split(",")[1]},getAVSignal:function(){var t=this;this.$http.get("/device/json?path=/av_signal/av_signal.json&t="+Math.random()).then((function(e){e.data.av_signal&&(t.avSignal=e.data.av_signal)}))},setAVSingle:function(){this.$http.post("/device/json",{path:"/av_signal/av_signal.json",info:{av_signal:this.avSignal}})},getDisplayDelay:function(){var t=this;this.$http.get("/device/json?path=/display/display_sleep.json&t="+Math.random()).then((function(e){e.data.display_delays&&(t.delay=e.data.display_delays)}))},handleResolution:function(t){this.maxResolution.val=t.split(",").pop()},setDisplayDelay:function(){this.$http.post("/device/json",{path:"/display/display_sleep.json",info:{display_delays:this.delay}})},save:function(){this.$socket.sendMsg("#KDS-SCALE ".concat("0"===this.maxResolution.val?0:1,",").concat(this.maxResolution.val)),this.setDisplayDelay(),this.$socket.sendMsg("#CS-CONVERT 1,".concat(this.forceRGB))}}},h=r,_=(a("9f50"),Object(d["a"])(h,s,i,!1,null,"af8fde7a",null));e["default"]=_.exports},3791:function(t,e,a){"use strict";a("69b6")},"69b6":function(t,e,a){},"7bb4":function(t,e,a){},"9f50":function(t,e,a){"use strict";a("7bb4")}}]);