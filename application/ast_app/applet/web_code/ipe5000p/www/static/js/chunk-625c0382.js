(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-625c0382"],{"5d14":function(t,s,i){"use strict";i.r(s);var a=function(){var t=this,s=t.$createElement,i=t._self._c||s;return i("div",{staticClass:"main-setting"},[i("div",{staticClass:"setting-content"},[i("div",{staticClass:"setting-model"},[i("h3",{staticClass:"setting-model-title"},[t._v("Password")]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Security Status")]),i("v-switch",{on:{change:t.switchPwdStatus},model:{value:t.showPwdStatus,callback:function(s){t.showPwdStatus=s},expression:"showPwdStatus"}})],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Current Password")]),i("input",{staticClass:"setting-text",attrs:{type:t.showPassword?"text":"password"}}),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},on:{change:t.setPassword}},[t._v("CHANGE")])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("New Password")]),i("input",{staticClass:"setting-text",attrs:{type:t.showPassword?"text":"password"}})]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Confirm Password")]),i("input",{staticClass:"setting-text",attrs:{type:t.showPassword?"text":"password"}})])]),i("div",{staticClass:"setting-model"},[i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-model-title"},[t._v("Inactivity auto-logout time")]),i("el-input-number",{attrs:{"controls-position":"right",max:30,min:0},model:{value:t.logoutTime,callback:function(s){t.logoutTime=s},expression:"logoutTime"}}),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"25px"},on:{click:t.setLogout}},[t._v("APPLY")])],1)])]),i("el-dialog",{attrs:{title:"Security Status",visible:t.verifyShowPwdDialog,width:"550px","show-close":!1,"close-on-click-modal":!1},on:{"update:visible":function(s){t.verifyShowPwdDialog=s}}},[i("p",{staticClass:"dialog-second-title"},[t._v("Would you like to enable security?")]),i("p",[t._v("This action will enable the authentication.")]),i("p",[t._v("Do you want to proceed?")]),i("span",{staticClass:"dialog-footer",attrs:{slot:"footer"},slot:"footer"},[i("button",{staticClass:"btn btn-primary",on:{click:function(s){t.verifyShowPwdDialog=!1,t.showPassword=!0}}},[t._v("PROCEED")]),i("button",{staticClass:"btn btn-primary",on:{click:function(s){t.verifyShowPwdDialog=!1,t.showPwdStatus=!1}}},[t._v("CANCEL")])])])],1)},e=[],n=(i("498a"),i("ac1f"),i("841c"),i("1276"),i("99af"),i("8a79"),{name:"users",data:function(){return{logoutTime:0,showPassword:!1,showPwdStatus:!1,verifyShowPwdDialog:!1,pwd:"",newPwd:"",confirmPwd:""}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(s){t.handleMsg(s.data.trim())}},created:function(){this.$socket.sendMsg("#LOGOUT-TIMEOUT? ")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@LOGOUT-TIMEOUT /i)?-1!==t.search(/@PASS /i)&&this.handlePwd(t):this.handleLogout(t)},switchPwdStatus:function(t){t?this.verifyShowPwdDialog=!0:this.showPassword=!1},handleLogout:function(t){this.logoutTime=parseInt(t.split(" ")[1])},setLogout:function(){this.$socket.sendMsg("#LOGOUT-TIMEOUT "+this.logoutTime)},setPassword:function(){0!==this.confirmPwd.length&&this.confirmPwd===this.newPwd&&this.$socket.sendMsg("#PASS ".concat(this.pwd,",").concat(this.newPwd))},handlePwd:function(t){t.toLowerCase().endsWith("ok")&&(this.newPwd="",this.confirmPwd="",this.pwd="")}}}),o=n,l=i("2877"),c=Object(l["a"])(o,a,e,!1,null,null,null);s["default"]=c.exports},"8a79":function(t,s,i){"use strict";var a=i("23e7"),e=i("06cf").f,n=i("50c4"),o=i("5a34"),l=i("1d80"),c=i("ab13"),r=i("c430"),d="".endsWith,w=Math.min,u=c("endsWith"),h=!r&&!u&&!!function(){var t=e(String.prototype,"endsWith");return t&&!t.writable}();a({target:"String",proto:!0,forced:!h&&!u},{endsWith:function(t){var s=String(l(this));o(t);var i=arguments.length>1?arguments[1]:void 0,a=n(s.length),e=void 0===i?a:w(n(i),a),c=String(t);return d?d.call(s,c,e):s.slice(e-c.length,e)===c}})}}]);