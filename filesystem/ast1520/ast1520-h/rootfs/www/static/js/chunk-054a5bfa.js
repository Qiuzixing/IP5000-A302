(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-054a5bfa"],{"466d":function(t,e,s){"use strict";var i=s("d784"),a=s("825a"),n=s("50c4"),o=s("1d80"),l=s("8aa5"),c=s("14c3");i("match",1,(function(t,e,s){return[function(e){var s=o(this),i=void 0==e?void 0:e[t];return void 0!==i?i.call(e,s):new RegExp(e)[t](String(s))},function(t){var i=s(e,t,this);if(i.done)return i.value;var o=a(t),r=String(this);if(!o.global)return c(o,r);var d=o.unicode;o.lastIndex=0;var p,h=[],u=0;while(null!==(p=c(o,r))){var f=String(p[0]);h[u]=f,""===f&&(o.lastIndex=l(r,n(o.lastIndex),d)),u++}return 0===u?null:h}]}))},"49c4":function(t,e,s){"use strict";s("7def")},"7def":function(t,e,s){},"9f7c":function(t,e,s){"use strict";s("e075")},a15b:function(t,e,s){"use strict";var i=s("23e7"),a=s("44ad"),n=s("fc6a"),o=s("a640"),l=[].join,c=a!=Object,r=o("join",",");i({target:"Array",proto:!0,forced:c||!r},{join:function(t){return l.call(n(this),void 0===t?",":t)}})},b2c7:function(t,e,s){"use strict";var i=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[s("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[s("span",{staticClass:"radio__inner"}),s("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(e){t.model=t.label},t.handleChange]}})]),s("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},a=[],n={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;this.$nextTick((function(){t.$emit("change",t.model)}))}}},o=n,l=(s("9f7c"),s("2877")),c=Object(l["a"])(o,i,a,!1,null,null,null);e["a"]=c.exports},c8d8:function(t,e,s){"use strict";s.r(e);var i=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("Interface Settings")]),s("table",{staticClass:"table"},[t._m(0),s("tbody",[s("tr",[s("th",[t._v("Stream")]),s("th",[t._v("Media")]),s("th",[t._v("N/A")]),s("th",[t._v("N/A")]),s("th",[s("v-switch",{staticStyle:{width:"120px"},attrs:{"active-value":"1","inactive-value":"0"},model:{value:t.ipMode0,callback:function(e){t.ipMode0=e},expression:"ipMode0"}})],1),s("th",[s("div",{staticStyle:{position:"relative"}},[s("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[0],expression:"ipInfo0[0]"}],staticClass:"setting-text",staticStyle:{width:"140px"},attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[0]},on:{input:function(e){e.target.composing||t.$set(t.ipInfo0,0,e.target.value)}}}),t.ipInfo0Error?s("span",{staticClass:"range-alert",staticStyle:{top:"36px","white-space":"nowrap"}},[t._v("The IP address is invalid")]):t._e()])]),s("th",[s("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[1],expression:"ipInfo0[1]"}],staticClass:"setting-text",staticStyle:{width:"140px"},attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[1]},on:{input:function(e){e.target.composing||t.$set(t.ipInfo0,1,e.target.value)}}})]),s("th",[s("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo0[2],expression:"ipInfo0[2]"}],staticClass:"setting-text",staticStyle:{width:"140px"},attrs:{type:"text",disabled:"1"===t.ipMode0},domProps:{value:t.ipInfo0[2]},on:{input:function(e){e.target.composing||t.$set(t.ipInfo0,2,e.target.value)}}})])]),s("tr",[s("th",[t._v("P3K & Gateway")]),s("th",[s("el-select",{staticStyle:{width:"100px"},model:{value:t.configPort0,callback:function(e){t.configPort0=e},expression:"configPort0"}},[s("el-option",{attrs:{value:"0",label:"Media"}}),s("el-option",{attrs:{value:"1",label:"Service"}})],1)],1),s("th",[s("v-switch",{staticStyle:{width:"120px"},model:{value:t.p3k802Q,callback:function(e){t.p3k802Q=e},expression:"p3k802Q"}})],1),s("th",[s("el-input-number",{staticStyle:{width:"80px"},attrs:{disabled:!t.p3k802Q,"controls-position":"right",max:4093,min:2},model:{value:t.danteTag1,callback:function(e){t.danteTag1=e},expression:"danteTag1"}})],1),s("th",[s("v-switch",{staticStyle:{width:"120px"},attrs:{"active-value":"1",disabled:"0"===t.configPort0&&!t.p3k802Q,"inactive-value":"0"},model:{value:t.ipMode1,callback:function(e){t.ipMode1=e},expression:"ipMode1"}})],1),s("th",[s("div",{staticStyle:{position:"relative"}},[s("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[0],expression:"ipInfo1[0]"}],staticClass:"setting-text",staticStyle:{width:"140px"},attrs:{type:"text",disabled:"1"===t.ipMode1||"0"===t.configPort0&&!t.p3k802Q},domProps:{value:t.ipInfo1[0]},on:{input:function(e){e.target.composing||t.$set(t.ipInfo1,0,e.target.value)}}}),t.ipInfo1Error&&("0"!==this.configPort0||this.p3k802Q)?s("span",{staticClass:"range-alert",staticStyle:{top:"36px","white-space":"nowrap"}},[t._v("The IP address is invalid")]):t._e()])]),s("th",[s("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[1],expression:"ipInfo1[1]"}],staticClass:"setting-text",staticStyle:{width:"140px"},attrs:{type:"text",disabled:"1"===t.ipMode1||"0"===t.configPort0&&!t.p3k802Q},domProps:{value:t.ipInfo1[1]},on:{input:function(e){e.target.composing||t.$set(t.ipInfo1,1,e.target.value)}}})]),s("th",[s("input",{directives:[{name:"model",rawName:"v-model",value:t.ipInfo1[2],expression:"ipInfo1[2]"}],staticClass:"setting-text",staticStyle:{width:"140px"},attrs:{type:"text",disabled:"1"===t.ipMode1||"0"===t.configPort0&&!t.p3k802Q},domProps:{value:t.ipInfo1[2]},on:{input:function(e){e.target.composing||t.$set(t.ipInfo1,2,e.target.value)}}})])]),1===this.$global.deviceType?s("tr",[s("th",[t._v("Dante")]),s("th",[s("el-select",{staticStyle:{width:"100px"},model:{value:t.configPort1,callback:function(e){t.configPort1=e},expression:"configPort1"}},[s("el-option",{attrs:{value:"0",label:"Media"}}),s("el-option",{attrs:{value:"1",label:"Service"}})],1)],1),s("th",[s("v-switch",{staticStyle:{width:"120px"},model:{value:t.dante802Q,callback:function(e){t.dante802Q=e},expression:"dante802Q"}})],1),s("th",[s("el-input-number",{staticStyle:{width:"80px"},attrs:{disabled:!t.dante802Q,"controls-position":"right",max:4093,min:2},model:{value:t.danteTag2,callback:function(e){t.danteTag2=e},expression:"danteTag2"}})],1)]):t._e()])])]),s("div",{staticClass:"setting"},[s("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"0"}},[s("span",{staticClass:"setting-model-title"},[t._v("IP Casting Mode")]),s("div",[s("radio-component",{attrs:{label:"1"},model:{value:t.castMode,callback:function(e){t.castMode=e},expression:"castMode"}},[t._v("Unicast ")]),s("radio-component",{attrs:{label:"2"},model:{value:t.castMode,callback:function(e){t.castMode=e},expression:"castMode"}},[t._v("Multicast ")])],1)])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("TTL")]),s("div",{staticClass:"setting-title"},[s("el-input-number",{attrs:{"controls-position":"right",max:255,min:1,disabled:"1"==t.castMode},model:{value:t.ttl,callback:function(e){t.ttl=e},expression:"ttl"}})],1)]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("TCP/UDP Management")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("TCP Port")]),s("el-input-number",{attrs:{"controls-position":"right",max:5099,min:5e3},model:{value:t.tcp,callback:function(e){t.tcp=e},expression:"tcp"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("UDP Port")]),s("el-input-number",{attrs:{"controls-position":"right",min:5e4,max:50999},model:{value:t.udp,callback:function(e){t.udp=e},expression:"udp"}})],1)]),s("footer",[s("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE ")])])])},a=[function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("thead",[s("tr",[s("th",[t._v("Service Name")]),s("th",[t._v("Port")]),s("th",[t._v("802.1Q")]),s("th",[t._v("VLAN Tag")]),s("th",[t._v("DHCP")]),s("th",[t._v("IP Address")]),s("th",[t._v("Mask Address")]),s("th",[t._v("Gateway Address")])])])}],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),s("466d"),s("99af"),s("a15b"),s("2ca0"),s("b2c7")),o=s("2ef0"),l={name:"autoSwitch",components:{radioComponent:n["a"]},data:function(){return{port1:"1",port2:"1",ipMode0:"0",ipMode1:"0",ipInfo0:[],ipInfo1:[],configPort0:"0",configPort1:"0",danteTag:"",tcp:"5000",serverTcpPort:"5001",udp:"50000",castMode:"1",multicastAddress:"0,0,0,0",ttl:64,danteTag1:2,danteTag2:2,p3k802Q:!1,dante802Q:!1,ipInfo0Error:!1,ipInfo1Error:!1}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#NET-DHCP? 0"),this.$socket.sendMsg("#NET-DHCP? 1"),this.$socket.sendMsg("#NET-CONFIG? 0"),this.$socket.sendMsg("#NET-CONFIG? 1"),this.$socket.sendMsg("#KDS-GW-ETH? 0"),this.$socket.sendMsg("#KDS-VLAN-TAG? 0"),this.$socket.sendMsg("#KDS-METHOD? "),this.$socket.sendMsg("#KDS-MULTICAST? "),this.$socket.sendMsg("#ETH-PORT? TCP"),this.$socket.sendMsg("#ETH-PORT? UDP"),1===this.$global.deviceType&&(this.$socket.sendMsg("#KDS-GW-ETH? 1"),this.$socket.sendMsg("#KDS-VLAN-TAG? 1"))},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@NET-DHCP /i)?-1===t.search(/@NET-CONFIG /i)?-1===t.search(/@KDS-VLAN-TAG /i)?-1===t.search(/@KDS-GW-ETH /i)?-1===t.search(/@KDS-METHOD /i)?-1===t.search(/@KDS-MULTICAST /i)?-1!==t.search(/@ETH-PORT /i)&&this.handleETHPort(t):this.handleMulticast(t):this.handleIpCastMode(t):this.handlePortConfig(t):this.handleDanteTag(t):this.handleIP(t):this.handleIPMode(t)},handleIPMode:function(t){var e=t.split(" ")[1].split(","),s="1"===e[1]?e[1]:"0";this["ipMode"+e[0]]=s},handleIP:function(t){var e=t.split(" ")[1],s="ipInfo"+e[0],i=[],a=/(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/g,n=e.match(a);i.push(n[0]||""),i.push(n[1]||""),i.push(n[2]||""),this[s]=i},handleDanteTag:function(t){var e=t.split(" ")[1].split(",");if("0"===e[0]){var s=parseInt(e[1]);this.p3k802Q=1!==s,1!==s&&(this.danteTag1=s)}else if("1"===e[0]){var i=parseInt(e[1]);this.dante802Q=1!==i,1!==i&&(this.danteTag2=i)}},handlePortConfig:function(t){var e=t.split(" ")[1].split(",");this["configPort"+e[0]]=e[1]},handleETHPort:function(t){var e=t.toLowerCase().split(" ")[1].split(",");this[e[0]]=parseInt(e[1])},handleIpCastMode:function(t){this.castMode=t.split(" ")[1]},handleMulticast:function(t){var e=t.split(" ")[1].split(",");this.multicastAddress=e[0],this.ttl=parseInt(e[1])},save:Object(o["debounce"])((function(){this.validIP()&&(this.setPortConfig(),this.setIpCastingMode(),this.setTcpUDP(),this.setIp())}),2e3,{leading:!0,trailing:!0}),setPortConfig:function(){this.$socket.sendMsg("#KDS-GW-ETH 0,"+this.configPort0),this.$socket.sendMsg("#KDS-VLAN-TAG 0,"+(this.p3k802Q?this.danteTag1:1)),1===this.$global.deviceType&&(this.$socket.sendMsg("#KDS-GW-ETH 1,"+this.configPort1),this.$socket.sendMsg("#KDS-VLAN-TAG 1,"+(this.dante802Q?this.danteTag2:1)))},setIpCastingMode:function(){this.$socket.sendMsg("#KDS-METHOD "+this.castMode),"2"===this.castMode&&this.$socket.sendMsg("#KDS-MULTICAST ".concat(this.multicastAddress,",").concat(this.ttl))},setTcpUDP:function(){this.$socket.sendMsg("#ETH-PORT TCP,"+this.tcp),this.$socket.sendMsg("#ETH-PORT UDP,"+this.udp)},setIp:function(){"1"!==this.ipMode0?(this.$socket.sendMsg("#NET-DHCP 0,0"),this.$socket.sendMsg("#NET-CONFIG 0,"+this.ipInfo0.join(","))):this.$socket.sendMsg("#NET-DHCP 0,1"),("0"!==this.configPort0||this.p3k802Q)&&("1"!==this.ipMode1?(this.$socket.sendMsg("#NET-DHCP 1,0"),this.$socket.sendMsg("#NET-CONFIG 1,"+this.ipInfo1.join(","))):this.$socket.sendMsg("#NET-DHCP 1,1"))},validIP:function(){return("1"===this.ipMode0||(this.ipInfo0Error=!this.isValidIP(this.ipInfo0[0],this.ipInfo0[1],this.ipInfo0[2]),!this.ipInfo0Error))&&(("0"===this.configPort0&&!this.p3k802Q||"1"===this.ipMode1||(this.ipInfo1Error=!this.isValidIP(this.ipInfo1[0],this.ipInfo1[1],this.ipInfo1[2]),!this.ipInfo1Error))&&(this.ipInfo0Error=!1,this.ipInfo1Error=!1,!0))},isValidIP:function(t,e,s){if(t.startsWith("255")||t.startsWith("127"))return!1;if(r(t)&&r(e)&&r(s)){var i=t.split("."),a=e.split("."),n=s.split("."),o=[],l=[],c=[];return o[0]=255&parseInt(i[0]),o[1]=255&parseInt(i[1]),o[2]=255&parseInt(i[2]),o[3]=255&parseInt(i[3]),c[0]=255&parseInt(n[0]),c[1]=255&parseInt(n[1]),c[2]=255&parseInt(n[2]),c[3]=255&parseInt(n[3]),l[0]=255&parseInt(a[0]),l[1]=255&parseInt(a[1]),l[2]=255&parseInt(a[2]),l[3]=255&parseInt(a[3]),!(o[3]&0===~l[3]||o[3]&255===~l[3])&&((o[0]&l[0])===(c[0]&l[0])&&(o[1]&l[1])===(c[1]&l[1])&&(o[2]&l[2])===(c[2]&l[2])&&(o[3]&l[3])===(c[3]&l[3])||"0.0.0.0"===s)}return!1;function r(t){var e=/^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;return e.test(t)}}}},c=l,r=(s("49c4"),s("2877")),d=Object(r["a"])(c,i,a,!1,null,"16d250aa",null);e["default"]=d.exports},e075:function(t,e,s){}}]);