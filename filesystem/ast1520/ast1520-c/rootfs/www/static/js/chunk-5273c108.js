(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-5273c108"],{"41e1":function(t,s,e){"use strict";e("9ed1")},"449c":function(t,s,e){t.exports=e.p+"static/img/warning.svg"},"694a":function(t,s,e){"use strict";e.r(s);var i=function(){var t=this,s=t.$createElement,i=t._self._c||s;return i("div",{staticClass:"main-setting"},[i("div",{staticClass:"setting-model"},[i("h3",{staticClass:"setting-model-title"},[t._v("General Preferences")]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Host Name")]),i("input",{directives:[{name:"model",rawName:"v-model",value:t.hostname,expression:"hostname"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"63"},domProps:{value:t.hostname},on:{input:function(s){s.target.composing||(t.hostname=s.target.value)}}}),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px"},attrs:{type:"button"},on:{click:t.setHostName}},[t._v("APPLY")])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Device Model")]),i("span",[t._v(t._s(t.deviceModel))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Device H/W Release")]),i("span",[t._v(t._s(t.HWVer))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("MAC Address")]),i("span",[t._v(t._s(t.macAddr))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Serial Number")]),i("span",[t._v(t._s(t.serialNum))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Front Panel Lock ")]),i("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.setFrontLock},model:{value:t.frontLock,callback:function(s){t.frontLock=s},expression:"frontLock"}})],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Import/Export Device Setting")]),i("multiselect",{attrs:{options:t.exportAndImport.param},model:{value:t.exportAndImport.val,callback:function(s){t.$set(t.exportAndImport,"val",s)},expression:"exportAndImport.val"}}),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px","margin-right":"15px"},attrs:{type:"button"}},[t._v("IMPORT")]),i("button",{staticClass:"btn btn-plain-primary",attrs:{type:"button"}},[t._v("EXPORT")])],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Locate Device")]),i("button",{staticClass:"btn btn-plain-primary",attrs:{type:"button"},on:{click:t.locateDev}},[t._v("APPLY")])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Power Save")]),i("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.setPowerSave},model:{value:t.powerSave,callback:function(s){t.powerSave=s},expression:"powerSave"}})],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Inactivity Auto-standby Delay Duration")]),i("el-input-number",{attrs:{"controls-position":"right",max:30,min:0},model:{value:t.autoStandbyTime,callback:function(s){t.autoStandbyTime=s},expression:"autoStandbyTime"}}),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px"},on:{click:t.setAutoStandbyTime}},[t._v("APPLY")])],1)]),i("div",{staticClass:"setting-model"},[i("h3",{staticClass:"setting-model-title"},[t._v("Version")]),i("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"24px"}},[i("span",{staticClass:"setting-title",staticStyle:{"line-height":"36px"}},[t._v("Firmware Version")]),i("span",{staticStyle:{width:"180px","line-height":"36px"}},[t._v(t._s(t.version))]),i("el-upload",{attrs:{action:"/upload/upgradesoftware","on-success":t.upgradeFile,"file-list":t.fileList}},[i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px"}},[t._v("UPGRADE")])])],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Last Upgrade Date/Time")]),i("span",[t._v(t._s(t.upgradeTime))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Firmware Standby Version ")]),i("span",{staticStyle:{width:"180px"}},[t._v(t._s(t.standbyVer))]),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px"},on:{click:t.rollBack}},[t._v("ROLLBACK")])])]),i("div",{staticClass:"setting-model"},[i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-model-title"},[t._v("Device Reset")]),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-right":"70px"},attrs:{type:"button"},on:{click:function(s){t.dialogVisibleReset=!0}}},[t._v("RESTART")]),i("button",{staticClass:"btn btn-plain-primary",attrs:{type:"button"},on:{click:function(s){t.dialogVisibleFactory=!0}}},[i("img",{staticStyle:{"vertical-align":"middle","margin-top":"-5px"},attrs:{src:e("449c"),alt:""}}),t._v("RESET")])])]),i("el-dialog",{attrs:{title:"RESET",visible:t.dialogVisibleReset,width:"500px"},on:{"update:visible":function(s){t.dialogVisibleReset=s}}},[i("p",{staticClass:"dialog-second-title"},[t._v("Do you want to restart the device? ")]),i("span",{staticClass:"dialog-footer",attrs:{slot:"footer"},slot:"footer"},[i("button",{staticClass:"btn btn-primary",on:{click:function(s){t.dialogVisibleReset=!1,t.restart()}}},[t._v("PROCEED")]),i("button",{staticClass:"btn btn-primary",on:{click:function(s){t.dialogVisibleReset=!1}}},[t._v("CANCEL")])])]),i("el-dialog",{attrs:{title:"RESET",visible:t.dialogVisibleFactory,width:"500px"},on:{"update:visible":function(s){t.dialogVisibleFactory=s}}},[i("p",{staticClass:"dialog-second-title"},[t._v("Do you want to reset the device? ")]),i("span",{staticClass:"dialog-footer",attrs:{slot:"footer"},slot:"footer"},[i("button",{staticClass:"btn btn-primary",on:{click:function(s){t.dialogVisibleFactory=!1,t.reset()}}},[t._v("PROCEED")]),i("button",{staticClass:"btn btn-primary",on:{click:function(s){t.dialogVisibleFactory=!1}}},[t._v("CANCEL")])])])],1)},a=[],n=(e("498a"),e("ac1f"),e("841c"),e("a15b"),e("fb6a"),e("1276"),{name:"autoSwitch",data:function(){return{show:!1,upgrade:!1,exportAndImport:{val:"all",param:[{value:"all",label:"All"},{value:"ip",label:"Without IP"},{value:"stream",label:"Streams"},{value:"av",label:"AV Setting Only"}]},displayOverlay:"off",hostname:"",deviceModel:"",HWVer:"",macAddr:"",serialNum:"",delayDuration:"1",frontLock:"",powerSave:"",upgradeTime:"",standbyVer:"",version:"",dialogVisibleReset:!1,dialogVisibleFactory:!1,autoStandbyTime:30,fileList:[]}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(s){t.handleMsg(s.data.trim())}},created:function(){this.$socket.sendMsg("#NAME? 0"),this.$socket.sendMsg("#MODEL? "),this.$socket.sendMsg("#HW-VERSION? "),this.$socket.sendMsg("#NET-MAC? 0"),this.$socket.sendMsg("#SN? "),this.$socket.sendMsg("#LOCK-FP? "),this.$socket.sendMsg("#STANDBY? "),this.$socket.sendMsg("#UPG-TIME? "),this.$socket.sendMsg("#VERSION? "),this.$socket.sendMsg("#STANDBY-VERSION? ")},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@NAME /i)?-1===t.search(/@MODEL /i)?-1===t.search(/@HW-VERSION /i)?-1===t.search(/@NET-MAC /i)?-1===t.search(/@SN /i)?-1===t.search(/@LOCK-FP /i)?-1===t.search(/@STANDBY /i)?-1===t.search(/@UPG-TIME /i)?-1===t.search(/@STANDBY-VERSION /i)?-1===t.search(/@STANDBY-TIMEOUT /i)?-1!==t.search(/@VERSION /i)&&this.handleVersion(t):this.handleAutoStandbyTime(t):this.handleStandbyVer(t):this.handleUpgradeTime(t):this.handlePowerSave(t):this.handleFrontLock(t):this.handleSerialNum(t):this.handleMACAddr(t):this.handleHWVersion(t):this.handleDeviceModel(t):this.handleHostname(t)},handleHostname:function(t){this.hostname=t.split(",").slice(1).join(",")},handleDeviceModel:function(t){this.deviceModel=t.split(" ")[1]},handleHWVersion:function(t){this.HWVer=t.split(" ")[1]},handleMACAddr:function(t){this.macAddr=t.split(" ")[1].split(",").pop()},handleSerialNum:function(t){this.serialNum=t.split(" ")[1]},handleFrontLock:function(t){this.frontLock=t.split(" ")[1]},handlePowerSave:function(t){this.powerSave=t.split(" ")[1]},handleUpgradeTime:function(t){this.upgradeTime=t.split(" ")[1]},handleStandbyVer:function(t){this.standbyVer=t.split(" ")[1]},handleVersion:function(t){this.version=t.split(" ")[1]},restart:function(){this.$socket.sendMsg("#RESET")},reset:function(){this.$socket.sendMsg("#FACTORY")},setHostName:function(){this.$socket.sendMsg("#NAME 0,".concat(this.hostname))},setDeviceModel:function(){this.$socket.sendMsg("#MODEL ".concat(this.deviceModel))},locateDev:function(){this.$socket.sendMsg("#IDV")},setAutoStandbyTime:function(){this.$socket.sendMsg("#STANDBY-TIMEOUT ".concat(this.autoStandbyTime))},handleAutoStandbyTime:function(t){this.autoStandbyTime=parseInt(t.split(" ")[1])},rollBack:function(){this.$socket.sendMsg("#ROLLBACK")},setFrontLock:function(t){this.$socket.sendMsg("#LOCK-FP ".concat(t))},setPowerSave:function(t){this.$socket.sendMsg("#STANDBY ".concat(t))},upgradeFile:function(){this.$socket.sendMsg("#UPGRADE ")}}}),o=n,l=(e("41e1"),e("2877")),c=Object(l["a"])(o,i,a,!1,null,"fbc74bb0",null);s["default"]=c.exports},"9ed1":function(t,s,e){},a15b:function(t,s,e){"use strict";var i=e("23e7"),a=e("44ad"),n=e("fc6a"),o=e("a640"),l=[].join,c=a!=Object,r=o("join",",");i({target:"Array",proto:!0,forced:c||!r},{join:function(t){return l.call(n(this),void 0===t?",":t)}})}}]);