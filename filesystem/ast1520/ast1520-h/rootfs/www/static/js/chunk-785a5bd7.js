(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-785a5bd7"],{"21a6":function(t,e,s){(function(s){var n,i,a;(function(s,o){i=[],n=o,a="function"===typeof n?n.apply(e,i):n,void 0===a||(t.exports=a)})(0,(function(){"use strict";function e(t,e){return"undefined"==typeof e?e={autoBom:!1}:"object"!=typeof e&&(console.warn("Deprecated: Expected third argument to be a object"),e={autoBom:!e}),e.autoBom&&/^\s*(?:text\/\S*|application\/xml|\S*\/\S*\+xml)\s*;.*charset\s*=\s*utf-8/i.test(t.type)?new Blob(["\ufeff",t],{type:t.type}):t}function n(t,e,s){var n=new XMLHttpRequest;n.open("GET",t),n.responseType="blob",n.onload=function(){l(n.response,e,s)},n.onerror=function(){console.error("could not download file")},n.send()}function i(t){var e=new XMLHttpRequest;e.open("HEAD",t,!1);try{e.send()}catch(t){}return 200<=e.status&&299>=e.status}function a(t){try{t.dispatchEvent(new MouseEvent("click"))}catch(n){var e=document.createEvent("MouseEvents");e.initMouseEvent("click",!0,!0,window,0,0,0,80,20,!1,!1,!1,!1,0,null),t.dispatchEvent(e)}}var o="object"==typeof window&&window.window===window?window:"object"==typeof self&&self.self===self?self:"object"==typeof s&&s.global===s?s:void 0,c=o.navigator&&/Macintosh/.test(navigator.userAgent)&&/AppleWebKit/.test(navigator.userAgent)&&!/Safari/.test(navigator.userAgent),l=o.saveAs||("object"!=typeof window||window!==o?function(){}:"download"in HTMLAnchorElement.prototype&&!c?function(t,e,s){var c=o.URL||o.webkitURL,l=document.createElement("a");e=e||t.name||"download",l.download=e,l.rel="noopener","string"==typeof t?(l.href=t,l.origin===location.origin?a(l):i(l.href)?n(t,e,s):a(l,l.target="_blank")):(l.href=c.createObjectURL(t),setTimeout((function(){c.revokeObjectURL(l.href)}),4e4),setTimeout((function(){a(l)}),0))}:"msSaveOrOpenBlob"in navigator?function(t,s,o){if(s=s||t.name||"download","string"!=typeof t)navigator.msSaveOrOpenBlob(e(t,o),s);else if(i(t))n(t,s,o);else{var c=document.createElement("a");c.href=t,c.target="_blank",setTimeout((function(){a(c)}))}}:function(t,e,s,i){if(i=i||open("","_blank"),i&&(i.document.title=i.document.body.innerText="downloading..."),"string"==typeof t)return n(t,e,s);var a="application/octet-stream"===t.type,l=/constructor/i.test(o.HTMLElement)||o.safari,r=/CriOS\/[\d]+/.test(navigator.userAgent);if((r||a&&l||c)&&"undefined"!=typeof FileReader){var d=new FileReader;d.onloadend=function(){var t=d.result;t=r?t:t.replace(/^data:[^;]*;/,"data:attachment/file;"),i?i.location.href=t:location=t,i=null},d.readAsDataURL(t)}else{var g=o.URL||o.webkitURL,u=g.createObjectURL(t);i?i.location=u:location.href=u,i=null,setTimeout((function(){g.revokeObjectURL(u)}),4e4)}});o.saveAs=l.saveAs=l,t.exports=l}))}).call(this,s("c8ba"))},b9a2:function(t,e,s){"use strict";s("cf83")},bf83:function(t,e,s){"use strict";s.r(e);var n=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-model"},[s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Active Syslog")]),s("v-switch",{attrs:{"active-value":"1","inactive-value":"2"},on:{change:t.setActiveLog},model:{value:t.syslog,callback:function(e){t.syslog=e},expression:"syslog"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Log")]),s("button",{staticClass:"btn btn-plain-primary",on:{click:t.viewLog}},[t._v("VIEW ")]),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},on:{click:t.exportLog}},[t._v("EXPORT ")])])]),s("div",{staticClass:"setting-model"},[s("h3",{staticClass:"setting-model-title"},[t._v("Gateway Messages Counter")]),t._m(0),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("RS-232")]),s("span",{staticStyle:{width:"200px"}},[t._v(t._s(t.rs232.send))]),s("span",{staticStyle:{width:"200px"}},[t._v(t._s(t.rs232.recv))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("CEC")]),s("span",{staticStyle:{width:"200px"}},[t._v(t._s(t.cec.send))]),s("span",{staticStyle:{width:"200px"}},[t._v(t._s(t.cec.recv))])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("IR")]),s("span",{staticStyle:{width:"200px"}},[t._v(t._s(t.ir.send))]),s("span",{staticStyle:{width:"200px"}},[t._v(t._s(t.ir.recv))])])]),s("el-dialog",{attrs:{title:"Log",visible:t.showLogDialog,width:"900px"},on:{"update:visible":function(e){t.showLogDialog=e}}},[s("pre",{staticClass:"log-text"},[t._v("      "),s("code",[t._v(t._s("\n"+t.logMsg))]),t._v("\n    ")])])],1)},i=[function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"}),s("span",{staticClass:"setting-title"},[t._v("Send")]),s("span",{staticClass:"setting-title"},[t._v("Receive ")])])}],a=(s("498a"),s("ac1f"),s("841c"),s("1276"),s("99af"),s("21a6")),o={name:"status",data:function(){return{syslog:"1",logResetPolicy:"3",logResetPolicyParam:[{value:"2",label:"Daily"},{value:"3",label:"Weekly"}],rs232:{recv:0,send:0},cec:{recv:0,send:0},ir:{recv:0,send:0},isExport:!1,showLogDialog:!1,logMsg:""}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#LOG-ACTION? "),this.$socket.sendMsg("#GTW-MSG-NUM? 1,"+this.getDay()),this.$socket.sendMsg("#GTW-MSG-NUM? 2"),this.$socket.sendMsg("#GTW-MSG-NUM? 3")},methods:{handleMsg:function(t){-1===t.search(/@LOG-ACTION /i)?-1!==t.search(/@GTW-MSG-NUM /i)&&this.handleSendCounter(t):this.handleLogAction(t)},handleLogAction:function(t){var e=t.split(" ")[1].split(",");this.syslog=e[0],this.logResetPolicy=e[1]},setActiveLog:function(){this.$socket.sendMsg("#LOG-ACTION ".concat(this.syslog,",").concat(this.logResetPolicy))},handleSendCounter:function(t){var e=t.split(" ")[1].split(",");"1"===e?(this.cec.recv=e[2],this.cec.send=e[3]):"2"===e?(this.ir.recv=e[2],this.ir.send=e[3]):"3"===e&&(this.rs232.recv=e[2],this.rs232.send=e[3])},getDay:function(){var t=new Date,e=t.getFullYear(),s=t.getMonth()<8?"0"+(t.getMonth()+1):t.getMonth()+1,n=t.getDate()<9?"0"+t.getDate():t.getDate();return n+"-"+s+"-"+e},exportLog:function(){this.$http.get("/log/log?t="+Math.random()).then((function(t){var e=new Blob([t.data],{type:"text/plain;charset=utf-8"});Object(a["saveAs"])(e,"log.txt")}))},viewLog:function(){var t=this;this.logMsg="",this.showLogDialog=!0,this.$http.get("/log/log?t="+Math.random()).then((function(e){t.logMsg=e.data}))}}},c=o,l=(s("b9a2"),s("2877")),r=Object(l["a"])(c,n,i,!1,null,"629e8492",null);e["default"]=r.exports},cf83:function(t,e,s){}}]);