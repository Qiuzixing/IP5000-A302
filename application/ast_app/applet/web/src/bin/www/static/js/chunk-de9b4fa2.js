(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-de9b4fa2"],{6437:function(t,e,s){"use strict";s("66e4")},"66e4":function(t,e,s){},"95f2":function(t,e,s){"use strict";s.r(e);var i=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("EDID Lock")]),s("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.setEDIDLock},model:{value:t.edidLock,callback:function(e){t.edidLock=e},expression:"edidLock"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("EDID Mode")]),s("multiselect",{attrs:{disabled:"1"==t.edidLock,options:t.edid.param},on:{input:t.setEDIDMode},model:{value:t.edid.val,callback:function(e){t.$set(t.edid,"val",e)},expression:"edid.val"}})],1),"custom"===t.edid.val?s("div",{staticClass:"radio-setting"},[s("span",{staticClass:"setting-title"},[t._v("User EDID")]),"1"!=t.edidLock?s("ul",{staticClass:"edid-list"},t._l(t.edidList,(function(e){return s("li",{key:e[0],class:{active:t.isSelectListIndex===e[0]},on:{click:function(s){t.isSelectListIndex=e[0]}}},[t._v(t._s(e[1]))])})),0):s("ul",{staticClass:"edid-list disabled"},t._l(t.edidList,(function(e){return s("li",{key:e[0],class:{active:t.isSelectListIndex===e[0]}},[t._v(t._s(e[1]))])})),0),s("div",[s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px","margin-bottom":"24px"},attrs:{disabled:"1"===t.edidLock}},[t._v("UPLOAD")]),s("br"),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px","margin-bottom":"24px"},attrs:{disabled:"1"===t.edidLock||"0"==t.isSelectListIndex},on:{click:t.deleteEDID}},[t._v("REMOVE")]),s("br"),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px",width:"97px"},attrs:{disabled:"1"===t.edidLock},on:{click:t.setEDID}},[t._v("APPLY")])])]):t._e(),"passthru"===t.edid.val?s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Read EDID from Specific Decoder")]),s("input",{directives:[{name:"model",rawName:"v-model",value:t.mac,expression:"mac"}],staticClass:"setting-text",attrs:{type:"text",placeholder:"MAC/IP Address",disabled:"1"==t.edidLock},domProps:{value:t.mac},on:{input:function(e){e.target.composing||(t.mac=e.target.value)}}}),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{disabled:"1"==t.edidLock},on:{click:t.readEDID}},[t._v("READ")])]):t._e()])},a=[],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),s("99af"),s("a434"),s("d3b7"),s("25f0"),{name:"edidManage",data:function(){return{edid:{val:"passthru",param:[{value:"passthru",label:"Passthrough"},{value:"custom",label:"Custom"},{value:"default",label:"Default EDID"}]},mac:"",edidLock:"0",edidList:[],edidListIndex:-1,isSelectListIndex:-1}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#LOCK-EDID? 1"),this.$socket.sendMsg("#EDID-MODE? 1"),this.$socket.sendMsg("#EDID-NET-SRC? 1"),this.$socket.sendMsg("#EDID-LIST? "),this.$socket.sendMsg("#EDID-ACTIVE? 1 ")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@LOCK-EDID /i)?-1===t.search(/@EDID-MODE /i)?-1===t.search(/@EDID-NET-SRC /i)?-1===t.search(/@EDID-LIST /i)?-1===t.search(/@EDID-ACTIVE /i)?-1!==t.search(/@EDID-RM /i)&&this.handleDeleteEDID(t):this.handleEDIDCustomSelect(t):this.handleEDIDList(t):this.handelEDIDRead(t):this.handleEdidMode(t):this.handleEDIDLock(t)},handleEDIDLock:function(t){this.edidLock=t.split(",")[1]},setEDIDLock:function(t){this.$socket.sendMsg("#LOCK-EDID 1,".concat(t))},handleEdidMode:function(t){var e=t.split(",");this.edid.val=e[1].toLowerCase(),"custom"===this.edid.val&&(this.edidListIndex=parseInt(e[2]),this.isSelectListIndex=parseInt(e[2]))},setEDIDMode:function(t){this.$socket.sendMsg("#EDID-MODE 1,".concat(t,",").concat(-1===this.isSelectListIndex?1:this.edidListIndex))},handelEDIDRead:function(t){this.mac=t.split(",")[1]},readEDID:function(){this.$socket.sendMsg("#EDID-NET-SRC 1,".concat(this.mac))},setEDID:function(){-1!==this.isSelectListIndex&&this.$socket.sendMsg("#EDID-ACTIVE 1,".concat(this.isSelectListIndex))},deleteEDID:function(){-1!==this.isSelectListIndex&&this.isSelectListIndex!==this.edidListIndex&&this.$socket.sendMsg("#EDID-RM ".concat(this.isSelectListIndex))},handleDeleteEDID:function(t){var e=t.split(" ")[1];if(e){e=parseInt(e);for(var s=-1,i=0,a=this.edidList.length;i<a;i++)if(this.edidList[i][0]===e){s=i;break}-1!==s&&(this.edidList.splice(s,1),this.$socket.sendMsg("#EDID-ACTIVE? 1 "))}},handleEDIDCustomSelect:function(t){if(-1===t.search(/error/i)){var e=parseInt(t.split(",")[1]);this.edidListIndex=e,this.isSelectListIndex=e}},handleEDIDList:function(t){var e=t.indexOf("["),s=t.substr(e).split(",");console.log(s.toString()),this.edidList=JSON.parse("["+s.toString()+"]")}}}),d=n,c=(s("6437"),s("2877")),l=Object(c["a"])(d,i,a,!1,null,"005d25fa",null);e["default"]=l.exports},a434:function(t,e,s){"use strict";var i=s("23e7"),a=s("23cb"),n=s("a691"),d=s("50c4"),c=s("7b0b"),l=s("65f0"),o=s("8418"),r=s("1dde"),D=r("splice"),h=Math.max,u=Math.min,I=9007199254740991,f="Maximum allowed length exceeded";i({target:"Array",proto:!0,forced:!D},{splice:function(t,e){var s,i,r,D,E,p,L=c(this),v=d(L.length),g=a(t,v),m=arguments.length;if(0===m?s=i=0:1===m?(s=0,i=v-g):(s=m-2,i=u(h(n(e),0),v-g)),v+s-i>I)throw TypeError(f);for(r=l(L,i),D=0;D<i;D++)E=g+D,E in L&&o(r,D,L[E]);if(r.length=i,s<i){for(D=g;D<v-i;D++)E=D+i,p=D+s,E in L?L[p]=L[E]:delete L[p];for(D=v;D>v-i+s;D--)delete L[D-1]}else if(s>i)for(D=v-i;D>g;D--)E=D+i-1,p=D+s-1,E in L?L[p]=L[E]:delete L[p];for(D=0;D<s;D++)L[D+g]=arguments[D+2];return L.length=v-i+s,r}})}}]);