(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-663e540a"],{"091b":function(t,e,s){"use strict";s("11a1")},"11a1":function(t,e,s){},a97c:function(t,e,s){"use strict";s.r(e);var i=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-model"},[s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Device Status")]),"0"===t.deviceStatus?s("div",{staticClass:"device-icon device-active"}):t._e(),"0"===t.deviceStatus?s("span",{staticStyle:{"margin-left":"12px"}},[t._v("Active")]):t._e(),"1"===t.deviceStatus?s("div",{staticClass:"device-icon device-default"}):t._e(),"1"===t.deviceStatus?s("span",{staticStyle:{"margin-left":"12px"}},[t._v("Standby")]):t._e()]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Heat level")]),t.temperature<60?s("div",{staticClass:"device-icon device-active"}):t._e(),t.temperature>=60&&t.temperature<=70?s("div",{staticClass:"device-icon device-active"}):t._e(),t.temperature>70?s("div",{staticClass:"device-icon device-active"}):t._e(),s("span",{staticStyle:{"margin-left":"12px"}},[t._v(t._s(t.temperature)+"℃ ")]),s("span",{staticStyle:{padding:"0 5px"}},[t._v("|")]),t.temperature<60?s("span",[t._v("Normal")]):t._e(),t.temperature>=60&&t.temperature<=70?s("span",[t._v("High")]):t._e(),t.temperature>70?s("span",[t._v("Overheat")]):t._e()]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("Input status")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("HDMI IN")]),s("div",{staticClass:"device-icon",class:[t.hdmiin1?"device-active":"device-default"]}),s("span",{staticStyle:{"margin-left":"12px"}},[t._v(t._s(t.hdmiin1?"On":"Off"))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("LAN")]),s("div",{staticClass:"device-icon",class:[t.hdmiin2?"device-active":"device-default"]}),s("span",{staticStyle:{"margin-left":"12px"}},[t._v(t._s(t.hdmiin2?"On":"Off"))])])]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("Output status")]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("HDMI OUT")]),s("div",{staticClass:"device-icon",class:[t.hdmiout?"device-active":"device-default"]}),s("span",{staticStyle:{"margin-left":"12px"}},[t._v(t._s(t.hdmiout?"On":"Off"))])])]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("Reset events log")]),t._m(0),s("div",{staticClass:"res-info"},t._l(t.log,(function(e,i){return s("div",{key:i,staticClass:"res-info-item"},[s("span",[t._v(t._s(e.date)+" "+t._s(e.time))]),s("span",[t._v(t._s("1"===e.type?"Auto":"Manual"))])])})),0)])])])},a=[function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"res-title"},[s("span",[t._v("Time")]),s("span",[t._v("Trigger")])])}],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),{name:"status",data:function(){return{deviceStatus:"1",temperature:50,log:[],hdmiin1:!1,hdmiin2:!1,usbin3:!1,hdmiout:!1,lan:!1}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#DEV-STATUS? "),this.$socket.sendMsg("#HW-TEMP? 0"),this.$socket.sendMsg("#LOG-RESET? "),this.$socket.sendMsg("#SIGNALS-LIST? ")},methods:{handleMsg:function(t){-1===t.search(/@DEV-STATUS /i)?-1===t.search(/@HW-TEMP /i)?-1===t.search(/@SIGNALS-LIST /i)?-1!==t.search(/@LOG-RESET /i)&&this.handleLog(t):this.handleSignal(t):this.handleTemp(t):this.handleDevStatus(t)},handleDevStatus:function(t){this.deviceStatus=t.split(" ")[1]},handleTemp:function(t){this.temperature=parseInt(t.split(" ")[1].split(",")[1])},handleLog:function(t){var e=t.split(" ")[1].split(",");this.log.push({type:e[0],date:e[1],time:e[2]})},handleSignal:function(t){this.hdmiin1=-1!==t.search(/in.hdmi.1/i),this.hdmiin2=-1!==t.search(/in.stream.1/i),this.hdmiout=-1!==t.search(/out.hdmi/i)}}}),c=n,l=(s("091b"),s("2877")),d=Object(l["a"])(c,i,a,!1,null,"06a26ae8",null);e["default"]=d.exports}}]);