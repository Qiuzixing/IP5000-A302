(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-7eaf720d"],{"466d":function(t,e,s){"use strict";var a=s("d784"),i=s("825a"),n=s("50c4"),l=s("1d80"),c=s("8aa5"),o=s("14c3");a("match",1,(function(t,e,s){return[function(e){var s=l(this),a=void 0==e?void 0:e[t];return void 0!==a?a.call(e,s):new RegExp(e)[t](String(s))},function(t){var a=s(e,t,this);if(a.done)return a.value;var l=i(t),r=String(this);if(!l.global)return o(l,r);var d=l.unicode;l.lastIndex=0;var u,p=[],v=0;while(null!==(u=o(l,r))){var C=String(u[0]);p[v]=C,""===C&&(l.lastIndex=c(r,n(l.lastIndex),d)),v++}return 0===v?null:p}]}))},9273:function(t,e,s){"use strict";s.r(e);var a=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("CEC Settings")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Gateway")]),s("v-switch",{attrs:{"open-text":"Enable","close-text":"Disable","active-value":"1","inactive-value":"0"},on:{input:t.setCECGateway},model:{value:t.cecGateWay,callback:function(e){t.cecGateWay=e},expression:"cecGateWay"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Gateway HDMI Port")]),s("multiselect",{attrs:{disabled:"0"===t.cecGateWay,options:[{value:"1",label:"HDMI Input"},{value:"3",label:"HDMI Loop Through"}]},on:{input:t.setCECPort},model:{value:t.cecGateWayPort,callback:function(e){t.cecGateWayPort=e},expression:"cecGateWayPort"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Command ")]),s("input",{directives:[{name:"model",rawName:"v-model",value:t.cecCmd,expression:"cecCmd"}],staticClass:"setting-text",attrs:{type:"text",disabled:"0"===t.cecGateWay},domProps:{value:t.cecCmd},on:{input:function(e){e.target.composing||(t.cecCmd=e.target.value)}}}),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},attrs:{disabled:"0"===t.cecGateWay},on:{click:t.sendCECCmd}},[t._v("SEND")])]),s("div",{staticClass:"radio-setting"},[s("span",{staticClass:"setting-title"},[t._v("responses ")]),s("div",[t._m(0),s("div",{staticClass:"res-info"},t._l(t.cecResList,(function(e,a){return s("div",{key:a,staticClass:"res-info-item"},[s("span",[t._v(t._s(e.cmd))]),s("span",[t._v(t._s(t.responseType[e.type]))])])})),0)])])]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("RS-232 Settings")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Gateway")]),s("v-switch",{attrs:{"open-text":"Enable","close-text":"Disable"},on:{input:t.setRs232GW},model:{value:t.rs232GW,callback:function(e){t.rs232GW=e},expression:"rs232GW"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Gateway Port")]),s("el-input-number",{attrs:{disabled:!t.rs232GW,"controls-position":"right",max:65535,min:5e3},on:{change:t.setRs232GW},model:{value:t.rs232Port,callback:function(e){t.rs232Port=e},expression:"rs232Port"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Baud Rate ")]),s("multiselect",{attrs:{disabled:!t.rs232GW,options:t.baudRateParam},model:{value:t.baudRate,callback:function(e){t.baudRate=e},expression:"baudRate"}}),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},attrs:{disabled:!t.rs232GW},on:{click:t.saveBaudRate}},[t._v("SAVE")])],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Data Bits ")]),s("multiselect",{attrs:{disabled:!t.rs232GW,options:t.dataBitsParam},model:{value:t.dataBits,callback:function(e){t.dataBits=e},expression:"dataBits"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Parity ")]),s("multiselect",{attrs:{disabled:!t.rs232GW,options:t.parityParam},model:{value:t.parity,callback:function(e){t.parity=e},expression:"parity"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Stop Bits ")]),s("multiselect",{attrs:{disabled:!t.rs232GW,options:t.stopBitsParam},model:{value:t.stopBits,callback:function(e){t.stopBits=e},expression:"stopBits"}})],1),s("div",{staticClass:"radio-setting"},[s("span",{staticClass:"setting-title"},[t._v("Connection")]),s("button",{staticClass:"btn btn-plain-primary",attrs:{disabled:!t.rs232GW}},[t._v("CHECK")])])]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("IR Settings")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Gateway")]),s("v-switch",{attrs:{"open-text":"Enable","close-text":"Disable","active-value":"1","inactive-value":"0"},on:{input:t.setIRGateway},model:{value:t.irGW,callback:function(e){t.irGW=e},expression:"irGW"}})],1),s("div",{staticClass:"radio-setting"},[s("span",{staticClass:"setting-title"},[t._v("IR Direction IN/OUT")]),s("div",[s("radio-component",{attrs:{disabled:"0"==t.irGW,label:"in"},on:{change:t.setIrDirection},model:{value:t.irDirection,callback:function(e){t.irDirection=e},expression:"irDirection"}},[t._v("IN")]),s("radio-component",{attrs:{disabled:"0"==t.irGW,label:"out"},on:{change:t.setIrDirection},model:{value:t.irDirection,callback:function(e){t.irDirection=e},expression:"irDirection"}},[t._v("OUT")])],1)])])])},i=[function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"res-title"},[s("span",[t._v("Command")]),s("span",[t._v("Result")])])}],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),s("99af"),s("466d"),s("b2c7")),l={name:"users",components:{radioComponent:n["a"]},data:function(){return{cecCmd:"",castMode:"1",rs232InOut:"1",cecGateWay:"0",cecGateWayPort:"1",baudRate:"115200",baudRateParam:[{value:"115200",label:"115200"},{value:"57600",label:"57600"},{value:"38400",label:"38400"},{value:"19200",label:"19200"},{value:"9600",label:"9600"}],dataBits:"8",dataBitsParam:[{value:"8",label:"8"},{value:"7",label:"7"},{value:"6",label:"6"},{value:"5",label:"5"}],parity:"none",parityParam:[{value:"none",label:"None"},{value:"odd",label:"Odd"},{value:"even",label:"Even"},{value:"mark",label:"Mark"},{value:"space",label:"Space"}],stopBits:"2",stopBitsParam:[{value:"2",label:"2"},{value:"1.5",label:"1.5"},{value:"1",label:"1"}],cecResList:[],responseType:{0:"Sent",1:"Gateway disabled",2:"Inactive CEC-Master",3:"Busy",4:"Illegal Message Parameter",5:"Illegal CEC Address Parameter",6:"Illegal CEC Command",7:"Timeout",8:"Error"},rs232Port:5e3,irDirection:"in",rs232GW:!1,irGW:"0"}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE? "),this.$socket.sendMsg("#UART? 1"),this.$socket.sendMsg("#PORT-DIRECTION? both.ir.1.ir"),this.$socket.sendMsg("#COM-ROUTE? *"),this.$socket.sendMsg("#KDS-IR-GW? ")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@CEC-GW-PORT-ACTIVE /i)?-1===t.search(/@CEC-SND /i)?-1===t.search(/@UART /i)?-1===t.search(/@PORT-DIRECTION /i)?-1===t.search(/@KDS-IR-GW /i)?-1!==t.search(/@COM-ROUTE /i)&&this.handleRs232Gateway(t):this.handleIRGateway(t):this.handleIRDirection(t):this.handleRS232Param(t):this.handleCECResponse(t):this.handleCECPort(t)},handleCECPort:function(t){var e=t.split(" ")[1];"0"===e?this.cecGateWay=e:(this.cecGateWay="1",this.cecGateWayPort=e)},setCECPort:function(){this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE "+this.cecGateWayPort)},sendCECCmd:function(){this.checkHex(this.cecCmd)&&this.$socket.sendMsg("#CEC-SND 1,1,1,".concat(this.cecCmd/2,",").concat(this.cecCmd))},handleCECResponse:function(t){var e=t.split(",");this.cecResList.unshift({cmd:e[3],type:e[4]})},checkHex:function(t){return t.match(/^([0-9a-fA-F]{2}([0-9a-fA-F]{2}){0,14})$/)},handleRS232Param:function(t){var e=t.split(",");this.baudRate=e[1],this.dataBits=e[2],this.parity=e[3],this.stopBits=e[4]},handleIRDirection:function(t){-1!==t.search(/ir/i)&&(this.irDirection=t.split(",")[1].toLowerCase())},setCECGateway:function(t){"0"===t?this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE 0"):this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE "+this.cecGateWayPort)},saveBaudRate:function(){this.rs232Port&&this.$socket.sendMsg("#COM-ROUTE-ADD 1,1,".concat(this.rs232Port,",1,1")),this.$socket.sendMsg("#UART 1,".concat(this.baudRate,",").concat(this.dataBits,",").concat(this.parity,",").concat(this.stopBits))},handleRs232Gateway:function(t){var e=t.split(" ");if(e[1].length>0){var s=e[1].split(",");this.rs232Port=parseInt(s[2]),this.rs232GW=!0}else this.rs232GW=!1},setRs232GW:function(t){t?this.$socket.sendMsg("#COM-ROUTE-ADD 1,1,".concat(this.rs232Port,",1,1")):this.$socket.sendMsg("#COM-ROUTE-REMOVE 1")},handleIRGateway:function(t){this.irGW=t.split(" ").pop()},setIrDirection:function(){this.$socket.sendMsg("#PORT-DIRECTION both.ir.1.ir,".concat(this.irDirection))},setIRGateway:function(t){this.$socket.sendMsg("#KDS-IR-GW ".concat(t))}}},c=l,o=(s("fabd"),s("2877")),r=Object(o["a"])(c,a,i,!1,null,"3fb84af3",null);e["default"]=r.exports},"9f7c":function(t,e,s){"use strict";s("e075")},b2c7:function(t,e,s){"use strict";var a=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[s("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[s("span",{staticClass:"radio__inner"}),s("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(e){t.model=t.label},t.handleChange]}})]),s("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},i=[],n={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;console.log(this.disabled),this.$nextTick((function(){t.$emit("change",t.model)}))}}},l=n,c=(s("9f7c"),s("2877")),o=Object(c["a"])(l,a,i,!1,null,null,null);e["a"]=o.exports},e075:function(t,e,s){},fabd:function(t,e,s){"use strict";s("fbab")},fbab:function(t,e,s){}}]);