(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-5a36021b"],{"466d":function(t,e,a){"use strict";var s=a("d784"),i=a("825a"),n=a("50c4"),l=a("1d80"),c=a("8aa5"),o=a("14c3");s("match",1,(function(t,e,a){return[function(e){var a=l(this),s=void 0==e?void 0:e[t];return void 0!==s?s.call(e,a):new RegExp(e)[t](String(a))},function(t){var s=a(e,t,this);if(s.done)return s.value;var l=i(t),r=String(this);if(!l.global)return o(l,r);var d=l.unicode;l.lastIndex=0;var u,v=[],h=0;while(null!==(u=o(l,r))){var p=String(u[0]);v[h]=p,""===p&&(l.lastIndex=c(r,n(l.lastIndex),d)),h++}return 0===h?null:v}]}))},6934:function(t,e,a){"use strict";a("e75e")},9273:function(t,e,a){"use strict";a.r(e);var s=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-model-title"},[t._v("CEC Settings")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Gateway")]),a("v-switch",{attrs:{"open-text":"Enable","close-text":"Disable","active-value":"1","inactive-value":"0"},on:{input:t.setCECGateway},model:{value:t.cecGateWay,callback:function(e){t.cecGateWay=e},expression:"cecGateWay"}})],1),a("div",[2!==t.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Gateway HDMI Port")]),a("el-select",{attrs:{disabled:"0"===t.cecGateWay},on:{change:t.setCECPort},model:{value:t.cecGateWayPort,callback:function(e){t.cecGateWayPort=e},expression:"cecGateWayPort"}},t._l([{value:"1",label:"HDMI Input"},{value:"3",label:"HDMI Loop Through"}],(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1):t._e(),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Command ")]),a("div",{staticStyle:{position:"relative"}},[a("input",{directives:[{name:"model",rawName:"v-model",value:t.cecCmd,expression:"cecCmd"}],staticClass:"setting-text",attrs:{type:"text",maxLength:"32",disabled:"0"===t.cecGateWay},domProps:{value:t.cecCmd},on:{input:function(e){e.target.composing||(t.cecCmd=e.target.value)}}}),t.hexError?a("span",{staticClass:"range-alert",staticStyle:{"white-space":"nowrap",top:"35px"}},[t._v("Please enter hexadecimal (example: ff36)")]):t._e()]),a("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},attrs:{disabled:"0"===t.cecGateWay},on:{click:t.sendCECCmd}},[t._v("SEND")])]),a("div",{staticClass:"radio-setting"},[a("span",{staticClass:"setting-title"},[t._v("Responses ")]),a("div",[t._m(0),a("div",{staticClass:"res-info"},t._l(t.cecResList,(function(e,s){return a("div",{key:s,staticClass:"res-info-item"},[a("span",[t._v(t._s(e.cmd))])])})),0)])])])]),a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-model-title"},[t._v("RS-232 Settings")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Gateway")]),a("v-switch",{attrs:{"open-text":"Enable","close-text":"Disable"},on:{input:t.setRs232GW},model:{value:t.rs232GW,callback:function(e){t.rs232GW=e},expression:"rs232GW"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Gateway Port")]),a("el-input-number",{attrs:{disabled:!t.rs232GW,"controls-position":"right",max:65535,min:5e3},model:{value:t.rs232Port,callback:function(e){t.rs232Port=e},expression:"rs232Port"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Baud Rate ")]),a("el-select",{model:{value:t.baudRate,callback:function(e){t.baudRate=e},expression:"baudRate"}},t._l(t.baudRateParam,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Data Bits ")]),a("el-select",{model:{value:t.dataBits,callback:function(e){t.dataBits=e},expression:"dataBits"}},t._l(t.dataBitsParam,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Parity ")]),a("el-select",{model:{value:t.parity,callback:function(e){t.parity=e},expression:"parity"}},t._l(t.parityParam,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Stop Bits ")]),a("el-select",{model:{value:t.stopBits,callback:function(e){t.stopBits=e},expression:"stopBits"}},t._l(t.stopBitsParam,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1),a("button",{staticClass:"btn btn-primary",on:{click:t.saveBaudRate}},[t._v("SAVE")])]),2!==this.$global.deviceType?a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-model-title"},[t._v("IR Settings")]),a("div",{staticClass:"radio-setting"},[a("span",{staticClass:"setting-title"},[t._v("IR Direction IN/OUT")]),a("div",[a("radio-component",{attrs:{label:"in"},on:{change:t.setIrDirection},model:{value:t.irDirection,callback:function(e){t.irDirection=e},expression:"irDirection"}},[t._v("IN")]),a("radio-component",{attrs:{label:"out"},on:{change:t.setIrDirection},model:{value:t.irDirection,callback:function(e){t.irDirection=e},expression:"irDirection"}},[t._v("OUT")])],1)])]):t._e()])},i=[function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"res-title"},[a("span",[t._v("Command")])])}],n=(a("ac1f"),a("841c"),a("1276"),a("99af"),a("466d"),a("b2c7")),l={name:"users",components:{radioComponent:n["a"]},data:function(){return{cecCmd:"",castMode:"1",rs232InOut:"1",cecGateWay:"0",cecGateWayPort:"1",baudRate:"115200",baudRateParam:[{value:"115200",label:"115200"},{value:"57600",label:"57600"},{value:"38400",label:"38400"},{value:"19200",label:"19200"},{value:"9600",label:"9600"}],dataBits:"8",dataBitsParam:[{value:"8",label:"8"},{value:"7",label:"7"},{value:"6",label:"6"},{value:"5",label:"5"}],parity:"none",parityParam:[{value:"none",label:"None"},{value:"odd",label:"Odd"},{value:"even",label:"Even"}],stopBits:"2",stopBitsParam:[{value:"2",label:"2"},{value:"1",label:"1"}],cecResList:[],responseType:{0:"Sent",1:"Gateway disabled",2:"Inactive CEC-Master",3:"Busy",4:"Illegal Message Parameter",5:"Illegal CEC Address Parameter",6:"Illegal CEC Command",7:"Timeout",8:"Error"},rs232Port:5e3,irDirection:"in",rs232GW:!1,irGW:"0",hexError:!1,saveFlag:!1}},created:function(){this.$socket.setCallback(this.handleMsg),this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE? "),this.$socket.sendMsg("#UART? 1"),this.$socket.sendMsg("#PORT-DIRECTION? both.ir.1.ir"),this.$socket.sendMsg("#COM-ROUTE? *")},methods:{handleMsg:function(t){if(-1===t.search(/@CEC-GW-PORT-ACTIVE /i))if(-1===t.search(/@UART /i))if(-1===t.search(/@PORT-DIRECTION /i))if(-1===t.search(/@KDS-IR-GW /i))if(-1===t.search(/@CEC-NTFY /i))-1!==t.search(/@COM-ROUTE /i)&&this.handleRs232Gateway(t);else{var e=t.split(",").pop();this.cecResList.unshift({cmd:e,type:e})}else this.handleIRGateway(t);else this.handleIRDirection(t);else this.handleRS232Param(t);else this.handleCECPort(t)},handleCECPort:function(t){var e=t.split(" ")[1];"0"===e?this.cecGateWay=e:(this.cecGateWay="1",this.cecGateWayPort=e)},setCECPort:function(){this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE "+this.cecGateWayPort)},sendCECCmd:function(){this.checkHex(this.cecCmd)?(this.hexError=!1,this.$socket.sendMsg("#CEC-SND 1,1,1,".concat(this.cecCmd.length/2,",").concat(this.cecCmd))):this.hexError=!0},handleCECResponse:function(t){var e=t.split(",");this.cecResList.unshift({cmd:e[3],type:e[4]})},checkHex:function(t){return t.match(/^([0-9a-fA-F]{2}([0-9a-fA-F]{2}){0,14})$/)},handleRS232Param:function(t){var e=t.split(",");this.baudRate=e[1],this.dataBits=e[2],this.parity=e[3],this.stopBits=e[4],this.saveFlag&&(this.saveFlag=!1,this.$msg.successAlert())},handleIRDirection:function(t){-1!==t.search(/ir/i)&&(this.irDirection=t.split(",")[1].toLowerCase())},setCECGateway:function(t){"0"===t?this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE 0"):2===this.$global.deviceType?this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE 1"):this.$socket.sendMsg("#CEC-GW-PORT-ACTIVE "+this.cecGateWayPort)},saveBaudRate:function(){this.rs232GW&&this.$socket.sendMsg("#COM-ROUTE-ADD 1,1,".concat(this.rs232Port,",1,1")),this.saveFlag=!0,this.$socket.sendMsg("#UART 1,".concat(this.baudRate,",").concat(this.dataBits,",").concat(this.parity,",").concat(this.stopBits))},handleRs232Gateway:function(t){var e=t.split(" ");if(e[1].length>0){var a=e[1].split(",");this.rs232Port=parseInt(a[2]),this.rs232GW=!0}else this.rs232GW=!1},setRs232GW:function(t){t?this.$socket.sendMsg("#COM-ROUTE-ADD 1,1,".concat(this.rs232Port,",1,1")):this.$socket.sendMsg("#COM-ROUTE-REMOVE 1")},handleIRGateway:function(t){this.irGW=t.split(" ").pop()},setIrDirection:function(){this.$socket.sendMsg("#PORT-DIRECTION both.ir.1.ir,".concat(this.irDirection))},setIRGateway:function(t){this.$socket.sendMsg("#KDS-IR-GW ".concat(t))}}},c=l,o=(a("6934"),a("2877")),r=Object(o["a"])(c,s,i,!1,null,"c653fb5a",null);e["default"]=r.exports},"9f7c":function(t,e,a){"use strict";a("e075")},b2c7:function(t,e,a){"use strict";var s=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[a("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[a("span",{staticClass:"radio__inner"}),a("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(e){t.model=t.label},t.handleChange]}})]),a("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},i=[],n={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;this.$nextTick((function(){t.$emit("change",t.model)}))}}},l=n,c=(a("9f7c"),a("2877")),o=Object(c["a"])(l,s,i,!1,null,null,null);e["a"]=o.exports},e075:function(t,e,a){},e75e:function(t,e,a){}}]);