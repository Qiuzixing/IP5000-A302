(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-2318c270"],{"0b8f":function(t,e,s){"use strict";s("5321")},5321:function(t,e,s){},"8a79":function(t,e,s){"use strict";var i=s("23e7"),a=s("06cf").f,n=s("50c4"),d=s("5a34"),c=s("1d80"),l=s("ab13"),o=s("c430"),r="".endsWith,D=Math.min,h=l("endsWith"),u=!o&&!h&&!!function(){var t=a(String.prototype,"endsWith");return t&&!t.writable}();i({target:"String",proto:!0,forced:!u&&!h},{endsWith:function(t){var e=String(c(this));d(t);var s=arguments.length>1?arguments[1]:void 0,i=n(e.length),a=void 0===s?i:D(n(s),i),l=String(t);return r?r.call(e,l,a):e.slice(a-l.length,a)===l}})},"95f2":function(t,e,s){"use strict";s.r(e);var i=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("EDID Lock")]),s("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.setEDIDLock},model:{value:t.edidLock,callback:function(e){t.edidLock=e},expression:"edidLock"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("EDID Mode")]),s("el-select",{attrs:{disabled:"1"==t.edidLock},on:{change:t.setEDIDMode},model:{value:t.edid.val,callback:function(e){t.$set(t.edid,"val",e)},expression:"edid.val"}},t._l(t.edid.param,(function(t){return s("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1),s("div",{directives:[{name:"show",rawName:"v-show",value:t.edidMode,expression:"edidMode"}]},["custom"===t.edid.val?s("div",{staticClass:"radio-setting"},[s("span",{staticClass:"setting-title"},[t._v("User EDID")]),"1"!=t.edidLock?s("ul",{staticClass:"edid-list"},t._l(t.edidList,(function(e){return s("li",{key:e[0],class:{active:t.isSelectListIndex===e[0]},on:{click:function(s){t.isSelectListIndex=e[0]}}},[t._v(t._s(e[1]))])})),0):s("ul",{staticClass:"edid-list disabled"},t._l(t.edidList,(function(e){return s("li",{key:e[0],class:{active:t.isSelectListIndex===e[0]}},[t._v(t._s(e[1]))])})),0),s("div",{staticStyle:{"margin-left":"24px"}},[s("el-upload",{attrs:{action:"/upload/edid","on-error":t.upgradeFail,"on-success":t.upgradeFile,"before-upload":t.checkEDID,"show-file-list":!1,"auto-upload":!0}},[s("button",{staticClass:"btn btn-plain-primary",attrs:{disabled:"1"===t.edidLock||t.edidList.length>7}},[t._v("UPLOAD")])]),s("br"),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-bottom":"24px"},attrs:{disabled:"1"===t.edidLock||"0"==t.isSelectListIndex},on:{click:t.deleteEDID}},[t._v("REMOVE")]),s("br"),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{width:"97px"},attrs:{disabled:"1"===t.edidLock},on:{click:t.setEDID}},[t._v("APPLY")]),s("input",{staticStyle:{display:"none",width:"0",height:"0"},attrs:{type:"file"}})],1)]):t._e(),"passthru"===t.edid.val?s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Read EDID from Specific Decoder")]),s("input",{directives:[{name:"model",rawName:"v-model",value:t.mac,expression:"mac"}],staticClass:"setting-text",attrs:{type:"text",placeholder:"0.0.0.0",disabled:"1"==t.edidLock},domProps:{value:t.mac},on:{input:function(e){e.target.composing||(t.mac=e.target.value)}}}),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{disabled:"1"==t.edidLock},on:{click:t.readEDID}},[t._v("READ")])]):t._e()])])},a=[],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),s("99af"),s("d3b7"),s("25f0"),s("b0c0"),s("8a79"),{name:"edidManage",data:function(){return{edid:{val:"passthru",param:[{value:"passthru",label:"Passthrough"},{value:"custom",label:"Custom"},{value:"default",label:"Default EDID"}]},mac:"",edidLock:"0",edidList:[],edidListIndex:-1,isSelectListIndex:-1,fileList:[],edidMode:!1}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#LOCK-EDID? 1"),this.$socket.sendMsg("#EDID-MODE? 1"),this.$socket.sendMsg("#EDID-NET-SRC? 1"),this.$socket.sendMsg("#EDID-LIST? "),this.$socket.sendMsg("#EDID-ACTIVE? 1 ")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@LOCK-EDID /i)?-1===t.search(/@EDID-MODE /i)?-1===t.search(/@EDID-NET-SRC /i)?-1===t.search(/@EDID-LIST /i)?-1===t.search(/@EDID-ACTIVE /i)?-1===t.search(/@EDID-ADD /i)?-1!==t.search(/@EDID-RM /i)&&this.handleDeleteEDID(t):this.handleDeleteEDID(t):this.handleEDIDCustomSelect(t):this.handleEDIDList(t):this.handelEDIDRead(t):this.handleEdidMode(t):this.handleEDIDLock(t)},handleEDIDLock:function(t){this.edidLock=t.split(",")[1]},setEDIDLock:function(t){this.$socket.sendMsg("#LOCK-EDID 1,".concat(t))},handleEdidMode:function(t){var e=t.split(",");this.edid.val=e[1].toLowerCase(),"custom"===this.edid.val&&(this.edidListIndex=parseInt(e[2]),this.isSelectListIndex=parseInt(e[2])),this.edidMode=!0},setEDIDMode:function(t){this.$socket.sendMsg("#EDID-MODE 1,".concat(t,",").concat(-1===this.isSelectListIndex?1:this.edidListIndex))},handelEDIDRead:function(t){this.mac=t.split(",")[1]||"0.0.0.0"},readEDID:function(){this.$socket.sendMsg("#EDID-NET-SRC 1,".concat(this.mac))},setEDID:function(){-1!==this.isSelectListIndex&&this.$socket.sendMsg("#EDID-ACTIVE 1,".concat(this.isSelectListIndex))},deleteEDID:function(){-1!==this.isSelectListIndex&&this.isSelectListIndex!==this.edidListIndex&&this.$socket.sendMsg("#EDID-RM ".concat(this.isSelectListIndex))},handleDeleteEDID:function(t){this.$socket.sendMsg("#EDID-LIST? "),this.$socket.sendMsg("#EDID-ACTIVE? 1 ")},handleEDIDCustomSelect:function(t){if(-1===t.search(/error/i)){var e=parseInt(t.split(",")[1]);this.edidListIndex=e,this.isSelectListIndex=e}},handleEDIDList:function(t){var e=t.indexOf("["),s=t.substr(e).split(",");console.log(s.toString()),this.edidList=JSON.parse("["+s.toString()+"]")},upgradeFile:function(t,e){this.edidList.length<8&&this.$socket.sendMsg("#EDID-ADD ".concat(this.edidList.length,",").concat(e.name))},upgradeFail:function(){alert("Please select the correct file")},checkEDID:function(t){return!(256!==t.size||!t.name.endsWith(".bin"))&&this.edidList.every((function(e){return e[1]!==t.name}))}}}),d=n,c=(s("0b8f"),s("2877")),l=Object(c["a"])(d,i,a,!1,null,"2d2030ba",null);e["default"]=l.exports}}]);