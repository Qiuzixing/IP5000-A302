(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-7245a7f0"],{"0550":function(t,s,i){},"466d":function(t,s,i){"use strict";var e=i("d784"),a=i("825a"),n=i("50c4"),o=i("1d80"),l=i("8aa5"),c=i("14c3");e("match",1,(function(t,s,i){return[function(s){var i=o(this),e=void 0==s?void 0:s[t];return void 0!==e?e.call(s,i):new RegExp(s)[t](String(i))},function(t){var e=i(s,t,this);if(e.done)return e.value;var o=a(t),d=String(this);if(!o.global)return c(o,d);var r=o.unicode;o.lastIndex=0;var p,u=[],g=0;while(null!==(p=c(o,d))){var v=String(p[0]);u[g]=v,""===v&&(o.lastIndex=l(d,n(o.lastIndex),r)),g++}return 0===g?null:u}]}))},"9f7c":function(t,s,i){"use strict";i("e075")},a15b:function(t,s,i){"use strict";var e=i("23e7"),a=i("44ad"),n=i("fc6a"),o=i("a640"),l=[].join,c=a!=Object,d=o("join",",");e({target:"Array",proto:!0,forced:c||!d},{join:function(t){return l.call(n(this),void 0===t?",":t)}})},b2c7:function(t,s,i){"use strict";var e=function(){var t=this,s=t.$createElement,i=t._self._c||s;return i("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[i("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[i("span",{staticClass:"radio__inner"}),i("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(s){t.model=t.label},t.handleChange]}})]),i("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},a=[],n={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;this.$nextTick((function(){t.$emit("change",t.model)}))}}},o=n,l=(i("9f7c"),i("2877")),c=Object(l["a"])(o,e,a,!1,null,null,null);s["a"]=c.exports},c8d8:function(t,s,i){"use strict";i.r(s);var e=function(){var t=this,s=t.$createElement,i=t._self._c||s;return i("div",{staticClass:"main-setting"},[i("div",{staticClass:"setting-model"},[i("div",{staticClass:"setting",staticStyle:{"margin-bottom":"0"}},[i("h3",{staticClass:"setting-model-title"},[t._v("IP Settings")]),i("h3",{staticClass:"setting-model-title"},[t._v("Eth 0")]),"0"===t.daisyChain?i("h3",{staticClass:"setting-model-title"},[t._v("Eth 1")]):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("DHCP")]),i("div",{staticClass:"setting-title"},[i("v-switch",{staticStyle:{width:"120px"},attrs:{"active-value":"1","inactive-value":"0"},model:{value:t.ipMode0,callback:function(s){t.ipMode0=s},expression:"ipMode0"}})],1),"0"===t.daisyChain?i("div",{staticClass:"setting-title"},[i("v-switch",{staticStyle:{width:"120px"},attrs:{"active-value":"1","inactive-value":"0"},model:{value:t.ipMode1,callback:function(s){t.ipMode1=s},expression:"ipMode1"}})],1):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("IP Address")]),i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[0],expression:"ipInfo0[0]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[0]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,0,s.target.value)}}})]),"0"===t.daisyChain?i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[0],expression:"ipInfo1[0]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[0]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,0,s.target.value)}}})]):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Mask Address")]),i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[1],expression:"ipInfo0[1]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[1]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,1,s.target.value)}}})]),"0"===t.daisyChain?i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[1],expression:"ipInfo1[1]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[1]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,1,s.target.value)}}})]):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Gateway Address")]),i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[2],expression:"ipInfo0[2]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[2]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,2,s.target.value)}}})]),"0"===t.daisyChain?i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[2],expression:"ipInfo1[2]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[2]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,2,s.target.value)}}})]):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Primary DNS")]),i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[3],expression:"ipInfo0[3]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[3]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,3,s.target.value)}}})]),"0"===t.daisyChain?i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[3],expression:"ipInfo1[3]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[3]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,3,s.target.value)}}})]):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Secondary DNS")]),i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[4],expression:"ipInfo0[4]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[4]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,4,s.target.value)}}})]),"0"===t.daisyChain?i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[4],expression:"ipInfo1[4]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[4]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,4,s.target.value)}}})]):t._e()])]),i("div",{staticClass:"setting"},[i("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"0"}},[i("span",{staticClass:"setting-model-title"},[t._v("Daisy Chain")]),i("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.daisyChainChange},model:{value:t.daisyChain,callback:function(s){t.daisyChain=s},expression:"daisyChain"}})],1)]),i("div",{staticClass:"setting-model"},[i("div",{staticClass:"setting",staticStyle:{"margin-bottom":"0"}},[i("h3",{staticClass:"setting-model-title"},[t._v("Port Configuration")]),i("h3",{staticClass:"setting-model-title",staticStyle:{width:"80px"}},[t._v("Eth 0")]),"0"===t.daisyChain?i("h3",{staticClass:"setting-model-title",staticStyle:{width:"80px"}},[t._v("Eth 1")]):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("P3K Port")]),i("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[i("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"0",isEmpty:!0},model:{value:t.configPort0,callback:function(s){t.configPort0=s},expression:"configPort0"}})],1),"0"===t.daisyChain?i("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[i("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"1",isEmpty:!0},model:{value:t.configPort0,callback:function(s){t.configPort0=s},expression:"configPort0"}})],1):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("RS232 Gateway Port")]),i("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[i("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"0",isEmpty:!0},model:{value:t.configPort1,callback:function(s){t.configPort1=s},expression:"configPort1"}})],1),i("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},["0"===t.daisyChain?i("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"1",isEmpty:!0},model:{value:t.configPort1,callback:function(s){t.configPort1=s},expression:"configPort1"}}):t._e()],1)])]),i("div",{staticClass:"setting"},[i("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"0"}},[i("span",{staticClass:"setting-model-title"},[t._v("IP Casting Mode")]),i("div",[i("radio-component",{attrs:{label:"1"},model:{value:t.castMode,callback:function(s){t.castMode=s},expression:"castMode"}},[t._v("Unicast")]),i("radio-component",{attrs:{label:"2"},model:{value:t.castMode,callback:function(s){t.castMode=s},expression:"castMode"}},[t._v("Multicast")])],1)])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("IP Multicast Address")]),i("div",{staticClass:"setting-title"},[i("input",{directives:[{name:"model",rawName:"v-model",value:t.multicastAddress,expression:"multicastAddress"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"==t.castMode},domProps:{value:t.multicastAddress},on:{input:function(s){s.target.composing||(t.multicastAddress=s.target.value)}}})])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("TTL")]),i("div",{staticClass:"setting-title"},[i("el-input-number",{attrs:{"controls-position":"right",max:255,min:1,disabled:"1"==t.castMode},model:{value:t.ttl,callback:function(s){t.ttl=s},expression:"ttl"}})],1)]),i("div",{staticClass:"setting-model"},[i("h3",{staticClass:"setting-model-title"},[t._v("TCP/UDP Management")]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("TCP Port")]),i("el-input-number",{attrs:{"controls-position":"right",max:5099,min:5e3},model:{value:t.tcp,callback:function(s){t.tcp=s},expression:"tcp"}})],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("UDP Port")]),i("el-input-number",{attrs:{"controls-position":"right",min:5e4,max:50999},model:{value:t.udp,callback:function(s){t.udp=s},expression:"udp"}})],1)]),i("footer",[i("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},a=[],n=(i("498a"),i("ac1f"),i("841c"),i("1276"),i("466d"),i("99af"),i("a15b"),i("b2c7")),o={name:"autoSwitch",components:{radioComponent:n["a"]},data:function(){return{port1:"1",port2:"1",ipMode0:"0",ipMode1:"0",ipInfo0:[],ipInfo1:[],configPort0:"0",configPort1:"0",configPort2:"0",danteTag:"",tcp:"5000",serverTcpPort:5e3,udp:5e4,castMode:"1",multicastAddress:"",ttl:64,danteTag1:1,danteTag2:1,danteTag3:1,daisyChain:"0"}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(s){t.handleMsg(s.data.trim())}},created:function(){this.$socket.sendMsg("#KDS-DAISY-CHAIN? "),this.$socket.sendMsg("#NET-DHCP? 0"),this.$socket.sendMsg("#NET-DHCP? 1"),this.$socket.sendMsg("#NET-CONFIG? 0"),this.$socket.sendMsg("#NET-CONFIG? 1"),this.$socket.sendMsg("#KDS-GW-ETH? 0"),this.$socket.sendMsg("#KDS-GW-ETH? 1"),this.$socket.sendMsg("#KDS-METHOD? "),this.$socket.sendMsg("#KDS-MULTICAST? "),this.$socket.sendMsg("#ETH-PORT? TCP"),this.$socket.sendMsg("#ETH-PORT? UDP")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@NET-DHCP /i)?-1===t.search(/@NET-CONFIG /i)?-1===t.search(/@KDS-GW-ETH /i)?-1===t.search(/@KDS-METHOD /i)?-1===t.search(/@KDS-MULTICAST /i)?-1===t.search(/@ETH-PORT /i)?-1!==t.search(/@KDS-DAISY-CHAIN /i)&&(this.daisyChain=t.split(" ").pop()):this.handleETHPort(t):this.handleMulticast(t):this.handleIpCastMode(t):this.handlePortConfig(t):this.handleIP(t):this.handleIPMode(t)},handleIPMode:function(t){var s=t.split(" ")[1].split(","),i="1"===s[1]?s[1]:"0";this["ipMode"+s[0]]=i},handleIP:function(t){var s=t.split(" ")[1],i="ipInfo"+s[0],e=[],a=/(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/g,n=s.match(a);e.push(n[0]||""),e.push(n[1]||""),e.push(n[2]||""),e.push(n[3]||""),e.push(n[4]||""),this[i]=e},handlePortConfig:function(t){var s=t.split(" ")[1].split(",");this["configPort"+s[0]]=s[1]},handleETHPort:function(t){var s=t.toLowerCase().split(" ")[1].split(",");this[s[0]]=parseInt(s[1])},handleIpCastMode:function(t){this.castMode=t.split(" ")[1]},handleMulticast:function(t){var s=t.split(" ")[1].split(",");this.multicastAddress=parseInt(s[0]),this.ttl=parseInt(s[1])},save:function(){this.$socket.sendMsg("#KDS-DAISY-CHAIN "+this.daisyChain),this.setPortConfig(),this.setIpCastingMode(),this.setTcpUDP(),this.setIp()},setPortConfig:function(){this.$socket.sendMsg("#KDS-GW-ETH 0,"+this.configPort0),this.$socket.sendMsg("#KDS-GW-ETH 1,"+this.configPort1)},setIpCastingMode:function(){this.$socket.sendMsg("#KDS-METHOD "+this.castMode),"2"===this.castMode&&this.$socket.sendMsg("#KDS-MULTICAST ".concat(this.multicastAddress,",").concat(this.ttl))},setTcpUDP:function(){this.$socket.sendMsg("#ETH-PORT? TCP,"+this.tcp),this.$socket.sendMsg("#ETH-PORT? UDP,"+this.udp)},setIp:function(){"1"!==this.ipMode0?(this.$socket.sendMsg("#NET-DHCP 0,0"),this.$socket.sendMsg("#NET-CONFIG 0,"+this.ipInfo0.join(","))):this.$socket.sendMsg("#NET-DHCP 0,1"),"0"===this.daisyChain&&("1"!==this.ipMode1?(this.$socket.sendMsg("#NET-DHCP 1,0"),this.$socket.sendMsg("#NET-CONFIG 1,"+this.ipInfo1.join(","))):this.$socket.sendMsg("#NET-DHCP 1,1"))},daisyChainChange:function(t){"1"===t&&(this.configPort0="0",this.configPort1="0")}}},l=o,c=(i("fda2"),i("2877")),d=Object(c["a"])(l,e,a,!1,null,"744640dc",null);s["default"]=d.exports},e075:function(t,s,i){},fda2:function(t,s,i){"use strict";i("0550")}}]);