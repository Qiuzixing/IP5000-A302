(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-a641ed2e"],{"466d":function(t,s,e){"use strict";var i=e("d784"),a=e("825a"),n=e("50c4"),o=e("1d80"),l=e("8aa5"),c=e("14c3");i("match",1,(function(t,s,e){return[function(s){var e=o(this),i=void 0==s?void 0:s[t];return void 0!==i?i.call(s,e):new RegExp(s)[t](String(e))},function(t){var i=e(s,t,this);if(i.done)return i.value;var o=a(t),d=String(this);if(!o.global)return c(o,d);var r=o.unicode;o.lastIndex=0;var p,g=[],u=0;while(null!==(p=c(o,d))){var m=String(p[0]);g[u]=m,""===m&&(o.lastIndex=l(d,n(o.lastIndex),r)),u++}return 0===u?null:g}]}))},"9f7c":function(t,s,e){"use strict";e("e075")},a15b:function(t,s,e){"use strict";var i=e("23e7"),a=e("44ad"),n=e("fc6a"),o=e("a640"),l=[].join,c=a!=Object,d=o("join",",");i({target:"Array",proto:!0,forced:c||!d},{join:function(t){return l.call(n(this),void 0===t?",":t)}})},b2c7:function(t,s,e){"use strict";var i=function(){var t=this,s=t.$createElement,e=t._self._c||s;return e("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[e("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[e("span",{staticClass:"radio__inner"}),e("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(s){t.model=t.label},t.handleChange]}})]),e("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},a=[],n={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;console.log(this.disabled),this.$nextTick((function(){t.$emit("change",t.model)}))}}},o=n,l=(e("9f7c"),e("2877")),c=Object(l["a"])(o,i,a,!1,null,null,null);s["a"]=c.exports},c5e5:function(t,s,e){},c8d8:function(t,s,e){"use strict";e.r(s);var i=function(){var t=this,s=t.$createElement,e=t._self._c||s;return e("div",{staticClass:"main-setting"},[e("div",{staticClass:"setting-model"},[t._m(0),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("DHCP")]),e("div",{staticClass:"setting-title"},[e("v-switch",{staticStyle:{width:"120px"},attrs:{"active-value":"1","inactive-value":"0"},model:{value:t.ipMode0,callback:function(s){t.ipMode0=s},expression:"ipMode0"}})],1),e("div",{staticClass:"setting-title"},[e("v-switch",{staticStyle:{width:"120px"},attrs:{"active-value":"1","inactive-value":"0"},model:{value:t.ipMode1,callback:function(s){t.ipMode1=s},expression:"ipMode1"}})],1)]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("IP Address")]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[0],expression:"ipInfo0[0]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[0]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,0,s.target.value)}}})]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[0],expression:"ipInfo1[0]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[0]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,0,s.target.value)}}})])]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("Mask Address")]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[1],expression:"ipInfo0[1]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[1]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,1,s.target.value)}}})]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[1],expression:"ipInfo1[1]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[1]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,1,s.target.value)}}})])]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("Gateway Address")]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[2],expression:"ipInfo0[2]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[2]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,2,s.target.value)}}})]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[2],expression:"ipInfo1[2]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[2]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,2,s.target.value)}}})])]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("Primary DNS")]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[3],expression:"ipInfo0[3]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[3]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,3,s.target.value)}}})]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[3],expression:"ipInfo1[3]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[3]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,3,s.target.value)}}})])]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("Secondary DNS")]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[4],expression:"ipInfo0[4]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[4]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo0,4,s.target.value)}}})]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[4],expression:"ipInfo1[4]"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"===t.ipMode1},domProps:{value:t.ipInfo1[4]},on:{input:function(s){s.target.composing||t.$set(t.ipInfo1,4,s.target.value)}}})])])]),e("div",{staticClass:"setting-model"},[t._m(1),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("P3K Port")]),e("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[e("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"0",isEmpty:!0},model:{value:t.configPort0,callback:function(s){t.configPort0=s},expression:"configPort0"}})],1),e("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[e("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"1",isEmpty:!0},model:{value:t.configPort0,callback:function(s){t.configPort0=s},expression:"configPort0"}})],1),e("div",[e("el-input-number",{attrs:{"controls-position":"right",max:4095,min:1},model:{value:t.danteTag1,callback:function(s){t.danteTag1=s},expression:"danteTag1"}})],1)]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("RS232 Gateway Port")]),e("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[e("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"0",isEmpty:!0},model:{value:t.configPort1,callback:function(s){t.configPort1=s},expression:"configPort1"}})],1),e("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[e("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"1",isEmpty:!0},model:{value:t.configPort1,callback:function(s){t.configPort1=s},expression:"configPort1"}})],1),e("div",[e("el-input-number",{attrs:{"controls-position":"right",max:4095,min:1},model:{value:t.danteTag2,callback:function(s){t.danteTag2=s},expression:"danteTag2"}})],1)]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("Dante Port")]),e("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[e("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"0",isEmpty:!0},model:{value:t.configPort2,callback:function(s){t.configPort2=s},expression:"configPort2"}})],1),e("div",{staticClass:"setting-title",staticStyle:{width:"80px"}},[e("radio-component",{staticStyle:{"margin-bottom":"0"},attrs:{label:"1",isEmpty:!0},model:{value:t.configPort2,callback:function(s){t.configPort2=s},expression:"configPort2"}})],1),e("div",[e("el-input-number",{attrs:{"controls-position":"right",max:4095,min:1},model:{value:t.danteTag3,callback:function(s){t.danteTag3=s},expression:"danteTag3"}})],1)])]),e("div",{staticClass:"setting"},[e("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"0"}},[e("span",{staticClass:"setting-model-title"},[t._v("IP Casting Mode")]),e("div",[e("radio-component",{attrs:{label:"1"},model:{value:t.castMode,callback:function(s){t.castMode=s},expression:"castMode"}},[t._v("Unicast")]),e("radio-component",{attrs:{label:"2"},model:{value:t.castMode,callback:function(s){t.castMode=s},expression:"castMode"}},[t._v("Multicast")])],1)])]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("IP Multicast Address")]),e("div",{staticClass:"setting-title"},[e("input",{directives:[{name:"model",rawName:"v-model",value:t.multicastAddress,expression:"multicastAddress"}],staticClass:"setting-text",attrs:{type:"text",disabled:"1"==t.castMode},domProps:{value:t.multicastAddress},on:{input:function(s){s.target.composing||(t.multicastAddress=s.target.value)}}})])]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("TTL")]),e("div",{staticClass:"setting-title"},[e("el-input-number",{attrs:{"controls-position":"right",max:255,min:1,disabled:"1"==t.castMode},model:{value:t.ttl,callback:function(s){t.ttl=s},expression:"ttl"}})],1)]),e("div",{staticClass:"setting-model"},[e("h3",{staticClass:"setting-model-title"},[t._v("TCP/UDP Management")]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("TCP Port")]),e("el-input-number",{attrs:{"controls-position":"right",max:5099,min:5e3},model:{value:t.tcp,callback:function(s){t.tcp=s},expression:"tcp"}})],1),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("UDP Port")]),e("el-input-number",{attrs:{"controls-position":"right",max:5e4,min:50999},model:{value:t.udp,callback:function(s){t.udp=s},expression:"udp"}})],1)]),e("footer",[e("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},a=[function(){var t=this,s=t.$createElement,e=t._self._c||s;return e("div",{staticClass:"setting",staticStyle:{"margin-bottom":"0"}},[e("h3",{staticClass:"setting-model-title"},[t._v("IP Settings")]),e("h3",{staticClass:"setting-model-title"},[t._v("Eth 0")]),e("h3",{staticClass:"setting-model-title"},[t._v("Eth 1")])])},function(){var t=this,s=t.$createElement,e=t._self._c||s;return e("div",{staticClass:"setting",staticStyle:{"margin-bottom":"0"}},[e("h3",{staticClass:"setting-model-title"},[t._v("Port Configuration")]),e("h3",{staticClass:"setting-model-title",staticStyle:{width:"80px"}},[t._v("Eth 0")]),e("h3",{staticClass:"setting-model-title",staticStyle:{width:"80px"}},[t._v("Eth 1")]),e("h3",{staticClass:"setting-model-title"},[t._v("Dante VLAN tag")])])}],n=(e("498a"),e("ac1f"),e("841c"),e("1276"),e("466d"),e("99af"),e("a15b"),e("b2c7")),o={name:"autoSwitch",components:{radioComponent:n["a"]},data:function(){return{port1:"1",port2:"1",ipMode0:"0",ipMode1:"0",ipInfo0:[],ipInfo1:[],configPort0:"0",configPort1:"0",configPort2:"0",danteTag:"",tcp:"5000",serverTcpPort:"5001",udp:"50000",castMode:"1",multicastAddress:"",ttl:64,danteTag1:1,danteTag2:1,danteTag3:1}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(s){t.handleMsg(s.data.trim())}},created:function(){this.$socket.sendMsg("#NET-DHCP? 0"),this.$socket.sendMsg("#NET-DHCP? 1"),this.$socket.sendMsg("#NET-CONFIG? 0"),this.$socket.sendMsg("#NET-CONFIG? 1"),this.$socket.sendMsg("#KDS-GW-ETH? 0"),this.$socket.sendMsg("#KDS-GW-ETH? 1"),this.$socket.sendMsg("#KDS-GW-ETH? 2"),this.$socket.sendMsg("#KDS-DANTE-VLANTAG? 0"),this.$socket.sendMsg("#KDS-DANTE-VLANTAG? 1"),this.$socket.sendMsg("#KDS-DANTE-VLANTAG? 2"),this.$socket.sendMsg("#KDS-METHOD? "),this.$socket.sendMsg("#KDS-MULTICAST? "),this.$socket.sendMsg("#ETH-PORT? TCP"),this.$socket.sendMsg("#ETH-PORT? UDP")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@NET-DHCP /i)?-1===t.search(/@NET-CONFIG /i)?-1===t.search(/@KDS-DANTE-VLANTAG /i)?-1===t.search(/@KDS-GW-ETH /i)?-1===t.search(/@KDS-METHOD /i)?-1===t.search(/@KDS-MULTICAST /i)?-1!==t.search(/@ETH-PORT /i)&&this.handleETHPort(t):this.handleMulticast(t):this.handleIpCastMode(t):this.handlePortConfig(t):this.handleDanteTag(t):this.handleIP(t):this.handleIPMode(t)},handleIPMode:function(t){var s=t.split(" ")[1].split(","),e="1"===s[1]?s[1]:"0";this["ipMode"+s[0]]=e},handleIP:function(t){var s=t.split(" ")[1],e="ipInfo"+s[0],i=[],a=/(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/g,n=s.match(a);i.push(n[0]||""),i.push(n[1]||""),i.push(n[2]||""),i.push(n[3]||""),i.push(n[4]||""),this[e]=i},handleDanteTag:function(t){var s=t.split(" ")[1].split(",");"0"===s[0]?this.danteTag1=parseInt(s[1]):"1"===s[0]?this.danteTag2=parseInt(s[1]):"2"===s[0]&&(this.danteTag3=parseInt(s[1]))},handlePortConfig:function(t){var s=t.split(" ")[1].split(",");this["configPort"+s[0]]=s[1]},handleETHPort:function(t){var s=t.toLowerCase().split(" ")[1].split(",");this[s[0]]=parseInt(s[1])},handleIpCastMode:function(t){this.castMode=t.split(" ")[1]},handleMulticast:function(t){var s=t.split(" ")[1].split(",");this.multicastAddress=parseInt(s[0]),this.ttl=parseInt(s[1])},save:function(){this.setPortConfig(),this.setIpCastingMode(),this.setTcpUDP(),this.setIp()},setPortConfig:function(){this.$socket.sendMsg("#KDS-GW-ETH 0,"+this.configPort0),this.$socket.sendMsg("#KDS-GW-ETH 1,"+this.configPort1),this.$socket.sendMsg("#KDS-GW-ETH 2,"+this.configPort2),this.$socket.sendMsg("#KDS-DANTE-VLANTAG 0,"+this.danteTag1),this.$socket.sendMsg("#KDS-DANTE-VLANTAG 1,"+this.danteTag2),this.$socket.sendMsg("#KDS-DANTE-VLANTAG 2,"+this.danteTag3)},setIpCastingMode:function(){this.$socket.sendMsg("#KDS-METHOD "+this.castMode),"2"===this.castMode&&this.$socket.sendMsg("#KDS-MULTICAST ".concat(this.multicastAddress,",").concat(this.ttl))},setTcpUDP:function(){this.$socket.sendMsg("#ETH-PORT? TCP,"+this.tcp),this.$socket.sendMsg("#ETH-PORT? UDP,"+this.udp)},setIp:function(){"1"!==this.ipMode0?this.$socket.sendMsg("#NET-CONFIG 0,"+this.ipInfo0.join(",")):this.$socket.sendMsg("#NET-DHCP 0,1"),"1"!==this.ipMode1?this.$socket.sendMsg("#NET-CONFIG 1,"+this.ipInfo1.join(",")):this.$socket.sendMsg("#NET-DHCP 1,1")}}},l=o,c=(e("f8cc"),e("2877")),d=Object(c["a"])(l,i,a,!1,null,"5c7d2398",null);s["default"]=d.exports},e075:function(t,s,e){},f8cc:function(t,s,e){"use strict";e("c5e5")}}]);