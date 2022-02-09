(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-5a31d56c"],{"0e90":function(t,e,s){"use strict";s.r(e);var a=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-content"},[s("div",{staticClass:"setting-model",staticStyle:{"margin-bottom":"0"}},[s("h3",{staticClass:"setting-model-title"},[t._v("Input HDCP Appearance")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Input 1")]),s("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,1)}},model:{value:t.hdcp[0],callback:function(e){t.$set(t.hdcp,0,e)},expression:"hdcp[0]"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Input 2")]),s("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,2)}},model:{value:t.hdcp[1],callback:function(e){t.$set(t.hdcp,1,e)},expression:"hdcp[1]"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Input 3")]),s("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:function(e){return t.setHDCP(e,3)}},model:{value:t.hdcp[2],callback:function(e){t.$set(t.hdcp,2,e)},expression:"hdcp[2]"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("ALL")]),s("button",{staticClass:"btn btn-plain-primary",on:{click:function(e){return t.setAllHDCP("1")}}},[t._v("ON")]),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},on:{click:function(e){return t.setAllHDCP("0")}}},[t._v("OFF")])])]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("Display")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Sleep (5V-off) Delay On Video Signal Loss (sec)")]),s("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay["sleep delay on signal loss sec"],callback:function(e){t.$set(t.delay,"sleep delay on signal loss sec",e)},expression:"delay['sleep delay on signal loss sec']"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Shutdown (CEC) Delay On Video Signal Loss (sec)")]),s("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay["shutdown delay on signal loss sec"],callback:function(e){t.$set(t.delay,"shutdown delay on signal loss sec",e)},expression:"delay['shutdown delay on signal loss sec']"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Wake-up (CEC) Delay On Video Signal Detection (sec)")]),s("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.delay["wake-up delay on signal detection sec"],callback:function(e){t.$set(t.delay,"wake-up delay on signal detection sec",e)},expression:"delay['wake-up delay on signal detection sec']"}})],1)]),s("div",{staticClass:"setting-model"},[s("div",{staticClass:"radio-setting"},[s("span",{staticClass:"setting-title"},[t._v("Sleep Image")]),s("div",{staticClass:"overlay-setting"},[s("div",{staticClass:"overlay-setting-item overlay-img"},[s("span",{staticClass:"file-name",staticStyle:{display:"inline-block"}},[t._v("sleep.png")]),s("span",{staticClass:"upload-icon",on:{click:t.clickUpload}},[s("icon-svg",{attrs:{"icon-class":"upload_img"}})],1),s("input",{ref:"upload",staticStyle:{display:"none"},attrs:{type:"file",accept:"image/jpeg"}}),s("br"),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-top":"15px","margin-bottom":"15px"}},[t._v("SLEEP")])])])]),t._m(0),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Maximum Bit Rate")]),s("multiselect",{attrs:{options:t.bitRateParam},model:{value:t.avSignal["maximum bit rate"],callback:function(e){t.$set(t.avSignal,"maximum bit rate",e)},expression:"avSignal['maximum bit rate']"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Maximum Video Frame Rate (%)")]),s("el-slider",{staticStyle:{width:"200px"},attrs:{min:0,max:100,"show-tooltip":!1,marks:t.marks},model:{value:t.avSignal["frame rate percentage"],callback:function(e){t.$set(t.avSignal,"frame rate percentage",e)},expression:"avSignal['frame rate percentage']"}}),s("span",{staticStyle:{"margin-left":"15px"}},[t._v(t._s(t.avSignal["frame rate percentage"]))])],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Force 8-bit Color Depth")]),s("v-checkbox",{attrs:{"active-value":"8-bit","inactive-value":"Follow Output"},model:{value:t.avSignal["color depth"],callback:function(e){t.$set(t.avSignal,"color depth",e)},expression:"avSignal['color depth']"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Force RGB")]),s("v-checkbox",{attrs:{"active-value":"1","inactive-value":"0"},model:{value:t.forceRGB,callback:function(e){t.forceRGB=e},expression:"forceRGB"}})],1)])]),s("footer",[s("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},i=[function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"24px"}},[s("span",{staticClass:"setting-title"},[t._v("Image Preview")]),s("img",{staticStyle:{"max-width":"320px"},attrs:{src:"/preview",alt:""}})])}],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),s("99af"),s("17c1")),l={name:"audioPage",components:{vCheckbox:n["a"]},data:function(){return{forceRGB:"0",maxBitRate:50,marks:{0:"0",100:"100"},avSignal:{"input maximum resolution":"Pass Through","maximum bit rate":"Best Effort","frame rate percentage":100,"color depth":"Follow Output","audio connection guard time sec":0,"dante vlan tag":""},delay:{"sleep delay on signal loss sec":0,"shutdown delay on signal loss sec":0,"wake-up delay on signal detection sec":0},hdcp:["0","0","0"],bitRateParam:[{value:"Best Effort",label:"Best Effort"},{value:"200 Mbps",label:"200 Mbps"},{value:"150 Mbps",label:"150 Mbps"},{value:"100 Mbps",label:"100 Mbps"},{value:"50 Mbps",label:"50 Mbps"},{value:"10 Mbps",label:"10 Mbps"}]}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#HDCP-MOD? 1"),this.$socket.sendMsg("#HDCP-MOD? 2"),this.$socket.sendMsg("#HDCP-MOD? 3"),this.$socket.sendMsg("#CS-CONVERT? 1"),this.getAVSignal(),this.getDisplayDelay()},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@HDCP-MOD /i)?-1!==t.search(/@CS-CONVERT /i)&&this.handleForceRGB(t):this.handleHDCP(t)},handleHDCP:function(t){var e=t.split(" ")[1].split(",");this.hdcp[parseInt(e[0])-1]=e[1]},setAllHDCP:function(t){this.$socket.sendMsg("#HDCP-MOD 1,".concat(t)),this.$socket.sendMsg("#HDCP-MOD 2,".concat(t)),this.$socket.sendMsg("#HDCP-MOD 3,".concat(t))},setHDCP:function(t,e){this.$socket.sendMsg("#HDCP-MOD ".concat(e,",").concat(t))},clickUpload:function(){this.$refs.upload.click()},getAVSignal:function(){var t=this;this.$http.post("/av_signal").then((function(e){e.data["AV Signal"]&&(t.avSignal=e.data["AV Signal"])}))},setAVSingle:function(){this.$http.post("/set_av_signal",{"AV Signal":this.avSignal})},getDisplayDelay:function(){var t=this;this.$http.post("/display/display_sleep").then((function(e){e.data["Display Delays"]&&(t.delay=e.data["Display Delays"])}))},setDisplayDelay:function(){this.$http.post("/display/set_display_sleep",{"Display Delays":this.delay})},handleForceRGB:function(t){this.forceRGB=t.split(",")[1]},save:function(){this.setDisplayDelay(),this.setAVSingle(),this.$socket.sendMsg("#CS-CONVERT 1,".concat(this.forceRGB))}}},c=l,o=(s("ad79"),s("2877")),r=Object(o["a"])(c,a,i,!1,null,"64c4f9cf",null);e["default"]=r.exports},"17c1":function(t,e,s){"use strict";var a=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("label",{staticClass:"checkbox",class:{"is-checked":t.checked,isDisabled:t.disabled},on:{click:t.changeVal}},[s("span",{staticClass:"checkbox-input",class:{"is-checked":t.checked}},[s("span",{staticClass:"checkbox-inner"}),s("span",{staticClass:"checkbox-inner-label"},[t._v(t._s(t.label))])])])},i=[],n=(s("a9e3"),{name:"checkBox",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},activeValue:{type:[Boolean,String,Number],default:!0},inactiveValue:{type:[Boolean,String,Number],default:!1},value:{}},data:function(){return{}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}},checked:function(){return this.value===this.activeValue}},methods:{changeVal:function(){if(!this.disabled){var t=this.checked?this.inactiveValue:this.activeValue;this.$emit("input",t),this.$emit("click",t)}}},watch:{checked:function(t){this.isChecked=t}}}),l=n,c=(s("3791"),s("2877")),o=Object(c["a"])(l,a,i,!1,null,null,null);e["a"]=o.exports},3791:function(t,e,s){"use strict";s("69b6")},6041:function(t,e,s){},"69b6":function(t,e,s){},ad79:function(t,e,s){"use strict";s("6041")}}]);