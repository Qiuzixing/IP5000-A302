(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-4acb00f1"],{"449c":function(t,e,s){t.exports=s.p+"static/img/warning.svg"},"694a":function(t,e,s){"use strict";s.r(e);var i=function(){var t=this,e=t.$createElement,i=t._self._c||e;return i("div",{staticClass:"main-setting"},[i("div",{staticClass:"setting-model"},[i("h3",{staticClass:"setting-model-title"},[t._v("General Preferences")]),i("div",{staticClass:"setting",staticStyle:{position:"relative"}},[i("span",{staticClass:"setting-title"},[t._v("Host Name")]),i("input",{directives:[{name:"model",rawName:"v-model",value:t.hostname,expression:"hostname"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"24"},domProps:{value:t.hostname},on:{input:function(e){e.target.composing||(t.hostname=e.target.value)}}}),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px"},attrs:{type:"button"},on:{click:t.setHostName}},[t._v("APPLY ")]),t.hostNameError?i("span",{staticClass:"range-alert",staticStyle:{top:"36px","white-space":"nowrap"}},[t._v("Alphanumeric, hyphen and underscore within 24 characters, hyphen and underscore can not at beginning or end")]):t._e()]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Device Model")]),i("span",[t._v(t._s(t.deviceModel))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Device H/W Release")]),i("span",[t._v(t._s(t.HWVer))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("MAC Address")]),i("span",[t._v(t._s(t.macAddr))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Serial Number")]),i("span",[t._v(t._s(t.serialNum))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Front Panel Lock ")]),i("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.setFrontLock},model:{value:t.frontLock,callback:function(e){t.frontLock=e},expression:"frontLock"}})],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Import/Export Device Settings")]),i("el-select",{model:{value:t.exportAndImport.val,callback:function(e){t.$set(t.exportAndImport,"val",e)},expression:"exportAndImport.val"}},t._l(t.exportAndImport.param,(function(t){return i("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px","margin-right":"15px"},attrs:{type:"button"},on:{click:t.importConfig}},[t._v("IMPORT ")]),i("button",{staticClass:"btn btn-plain-primary",attrs:{type:"button"},on:{click:t.exportConfig}},[t._v("EXPORT ")]),i("input",{ref:"uploadConfig",staticStyle:{display:"none"},attrs:{type:"file"},on:{change:t.browseConfig}}),t.uploadComplete?i("span",{staticStyle:{"font-size":"20px","margin-left":"15px",color:"#67c23a"}},[i("i",{staticClass:"el-icon-circle-check"})]):t._e()],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Locate Device")]),i("button",{staticClass:"btn btn-plain-primary",attrs:{type:"button"},on:{click:t.locateDev}},[t._v("APPLY ")])])]),i("div",{staticClass:"setting-model"},[i("h3",{staticClass:"setting-model-title"},[t._v("Version")]),i("div",{staticClass:"radio-setting",staticStyle:{"margin-bottom":"24px"}},[i("span",{staticClass:"setting-title",staticStyle:{"line-height":"36px"}},[t._v("Firmware Version")]),i("span",{staticStyle:{width:"180px","line-height":"36px"}},[t._v(t._s(t.version))]),i("el-upload",{ref:"upload",attrs:{action:"/upload/upgradesoftware","before-upload":t.beforeUpload,"on-progress":t.progressEvent,"on-success":t.upgradeFile,"on-error":t.uploadError,"show-file-list":!1}},[i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px"}},[t._v("UPGRADE ")])])],1),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Last Upgrade Date/Time")]),i("span",[t._v(t._s(t.upgradeTime))])]),i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-title"},[t._v("Firmware Standby Version ")]),i("span",{staticStyle:{width:"180px"}},[t._v(t._s(t.standbyVer))]),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"15px"},on:{click:t.rollBack}},[t._v("ROLLBACK ")])])]),i("div",{staticClass:"setting-model"},[i("div",{staticClass:"setting"},[i("span",{staticClass:"setting-model-title"},[t._v("Device Reset")]),i("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-right":"70px"},attrs:{type:"button"},on:{click:function(e){t.dialogVisibleReset=!0}}},[t._v("RESTART ")]),i("button",{staticClass:"btn btn-plain-primary",attrs:{type:"button"},on:{click:function(e){t.dialogVisibleFactory=!0}}},[i("img",{staticStyle:{"vertical-align":"middle","margin-top":"-5px"},attrs:{src:s("449c"),alt:""}}),t._v("RESET ")])])]),i("el-dialog",{attrs:{title:"RESTART",visible:t.dialogVisibleReset,width:"500px"},on:{"update:visible":function(e){t.dialogVisibleReset=e}}},[i("p",{staticClass:"dialog-second-title"},[t._v("Do you want to restart the device? ")]),i("span",{staticClass:"dialog-footer",attrs:{slot:"footer"},slot:"footer"},[i("button",{staticClass:"btn btn-primary",on:{click:function(e){t.dialogVisibleReset=!1,t.restart()}}},[t._v("PROCEED")]),i("button",{staticClass:"btn btn-primary",on:{click:function(e){t.dialogVisibleReset=!1}}},[t._v("CANCEL")])])]),i("el-dialog",{attrs:{title:"RESET",visible:t.dialogVisibleFactory,width:"500px"},on:{"update:visible":function(e){t.dialogVisibleFactory=e}}},[i("p",{staticClass:"dialog-second-title"},[t._v("Do you want to reset the device? ")]),i("span",{staticClass:"dialog-footer",attrs:{slot:"footer"},slot:"footer"},[i("button",{staticClass:"btn btn-primary",on:{click:function(e){t.dialogVisibleFactory=!1,t.reset()}}},[t._v("PROCEED")]),i("button",{staticClass:"btn btn-primary",on:{click:function(e){t.dialogVisibleFactory=!1}}},[t._v("CANCEL")])])]),i("el-dialog",{attrs:{title:"Upgrade Info","close-on-click-modal":!1,"close-on-press-escape":!1,visible:t.showProgress,width:"500px"},on:{"update:visible":function(e){t.showProgress=e}}},[i("div",[t.fileError?i("p",{staticClass:"upgrade-info"},[t._v("(3/3) Firmware upgrading failed. Error code "+t._s(t.errMsg)+".")]):t._e(),t.upgradeComplete?i("p",{staticClass:"upgrade-info"},[t._v("(3/3) Firmware upgrading completed. Rebooting...")]):t._e(),t.isUpgrade?i("p",{staticClass:"upgrade-info"},[t._v("(2/3) Firmware installing "+t._s(t.upgradeProgress)+"%")]):t._e(),i("p",{staticClass:"upgrade-info"},[t._v("(1/3) Firmware uploading "+t._s(t.uploadProgress)+"%")])])]),t.isExportConfig?i("iframe",{attrs:{src:"/settings/export?method="+t.exportConfigVal,frameborder:"0",width:"0",height:"0"}}):t._e()],1)},a=[],n=(s("ac1f"),s("841c"),s("a15b"),s("fb6a"),s("1276"),s("8a79"),s("b0c0"),{name:"autoSwitch",data:function(){return{hostNameError:!1,show:!1,upgrade:!1,exportConfigVal:"",exportAndImport:{val:"3",param:[{value:"0",label:"All Without IP"},{value:"1",label:"Streams"},{value:"2",label:"AV Settings only"},{value:"3",label:"All including IP"}]},displayOverlay:"off",hostname:"",deviceModel:"",HWVer:"",macAddr:"",serialNum:"",delayDuration:"1",frontLock:"",powerSave:"",upgradeTime:"",standbyVer:"",version:"",dialogVisibleReset:!1,dialogVisibleFactory:!1,fileList:[],progress:0,showProgress:!1,upgradeInfo:[],upgradeComplete:!1,fileError:!1,isUpgrade:!1,upgradeProgress:0,uploadProgress:0,errMsg:"",isExportConfig:!1,uploadComplete:!1}},created:function(){this.$socket.setCallback(this.handleMsg),this.$socket.sendMsg("#NAME? 0"),this.$socket.sendMsg("#MODEL? "),this.$socket.sendMsg("#HW-VERSION? "),this.$socket.sendMsg("#NET-MAC? 0"),this.$socket.sendMsg("#SN? "),this.$socket.sendMsg("#LOCK-FP? "),this.$socket.sendMsg("#UPG-TIME? "),this.$socket.sendMsg("#VERSION? "),this.$socket.sendMsg("#STANDBY-VERSION? ")},methods:{handleMsg:function(t){var e=this;-1===t.search(/@NAME /i)?-1===t.search(/@MODEL /i)?-1===t.search(/@HW-VERSION /i)?-1===t.search(/@NET-MAC /i)?-1===t.search(/@SN /i)?-1===t.search(/@LOCK-FP /i)?-1===t.search(/@STANDBY /i)?-1===t.search(/@UPG-TIME /i)?-1===t.search(/@STANDBY-VERSION /i)?(-1!==t.search(/@VERSION /i)&&this.handleVersion(t),-1===t.search(/@UPGRADE-STATUS /i)?-1!==t.search(/@UPGRADE /i)&&(this.isUpgrade=!0,this.$refs.upload.clearFiles(),setTimeout((function(){e.$socket.sendMsg("#UPGRADE-STATUS? ")}),3e3)):this.handleUpgradeProgress(t)):this.handleStandbyVer(t):this.handleUpgradeTime(t):this.handlePowerSave(t):this.handleFrontLock(t):this.handleSerialNum(t):this.handleMACAddr(t):this.handleHWVersion(t):this.handleDeviceModel(t):this.handleHostname(t)},handleHostname:function(t){this.hostname=t.split(",").slice(1).join(",")},handleDeviceModel:function(t){this.deviceModel=t.split(" ")[1]},handleHWVersion:function(t){this.HWVer=t.split(" ")[1]},handleMACAddr:function(t){this.macAddr=t.split(" ")[1].split(",").pop()},handleSerialNum:function(t){this.serialNum=t.split(" ")[1]},handleFrontLock:function(t){this.frontLock=t.split(" ")[1]},handlePowerSave:function(t){this.powerSave=t.split(" ")[1]},handleUpgradeTime:function(t){this.upgradeTime=t.split(" ")[1]},handleStandbyVer:function(t){this.standbyVer=t.split(" ")[1]},handleVersion:function(t){this.version=t.split(" ")[1]},restart:function(){this.$socket.sendMsg("#RESET"),sessionStorage.removeItem("login")},reset:function(){this.$socket.sendMsg("#FACTORY"),sessionStorage.removeItem("login")},setHostName:function(){this.hostNameError=!this.isValidName(this.hostname),this.hostNameError||this.$socket.sendMsg("#NAME 0,".concat(this.hostname))},setDeviceModel:function(){this.$socket.sendMsg("#MODEL ".concat(this.deviceModel))},locateDev:function(){this.$socket.sendMsg("#IDV")},rollBack:function(){this.$socket.sendMsg("#ROLLBACK")},setFrontLock:function(t){this.$socket.sendMsg("#LOCK-FP ".concat(t))},setPowerSave:function(t){this.$socket.sendMsg("#STANDBY ".concat(t))},upgradeFile:function(){this.uploadProgress=100,this.$socket.sendMsg("#UPGRADE ")},uploadError:function(t){this.showProgress=!1,this.$refs.upload.clearFiles(),setTimeout((function(){-1!==(null===t||void 0===t?void 0:t.message.search("406"))?alert("Invalid File"):alert("Upload failed, please try again")}),200)},beforeUpload:function(t){if(!(t.name.endsWith(".bin")&&t.size<16777216))return alert("Please choose the correct file!"),!1;this.uploadProgress=0,this.upgradeProgress=0,this.isUpgrade=!1,this.upgradeComplete=!1,this.fileError=!1,this.showProgress=!0},progressEvent:function(t){this.uploadProgress=parseInt(t.percent)},handleUpgradeProgress:function(t){var e=this,s=t.split(" ")[1];if(-1!==s.indexOf("err"))return this.fileError=!0,void(this.errMsg=s.split(",")[2]);if(-1!==s.indexOf("ok"))return this.upgradeProgress=100,void(this.upgradeComplete=!0);if(-1!==s.indexOf("ongoing")){var i=parseInt(s.split(",")[1]);this.upgradeProgress=i,setTimeout((function(){e.$socket.sendMsg("#UPGRADE-STATUS? ")}),i>96?1500:3500)}},exportConfig:function(){var t=this;this.isExportConfig=!1,this.exportConfigVal=this.exportAndImport.val,setTimeout((function(){t.isExportConfig=!0}),500)},importConfig:function(){this.$refs.uploadConfig.click()},browseConfig:function(){var t=this,e=this.$refs.uploadConfig.files[0];if(e){var s=new XMLHttpRequest,i=new FormData;i.append("file",e),s.open("POST","/settings/import?method="+this.exportAndImport.val),s.onload=function(e){200===s.status&&(t.$refs.uploadConfig.value="",t.uploadComplete=!0,setTimeout((function(){t.uploadComplete=!1}),2e3))},s.send(i)}},isValidName:function(t){return/^[a-zA-Z0-9]$|^[a-zA-Z0-9][\w-]{0,22}[a-zA-Z0-9]$/.test(t)}}}),o=n,r=(s("f25e"),s("2877")),l=Object(r["a"])(o,i,a,!1,null,"1a14911b",null);e["default"]=l.exports},"8a79":function(t,e,s){"use strict";var i=s("23e7"),a=s("06cf").f,n=s("50c4"),o=s("5a34"),r=s("1d80"),l=s("ab13"),c=s("c430"),d="".endsWith,p=Math.min,g=l("endsWith"),u=!c&&!g&&!!function(){var t=a(String.prototype,"endsWith");return t&&!t.writable}();i({target:"String",proto:!0,forced:!u&&!g},{endsWith:function(t){var e=String(r(this));o(t);var s=arguments.length>1?arguments[1]:void 0,i=n(e.length),a=void 0===s?i:p(n(s),i),l=String(t);return d?d.call(e,l,a):e.slice(a-l.length,a)===l}})},a15b:function(t,e,s){"use strict";var i=s("23e7"),a=s("44ad"),n=s("fc6a"),o=s("a640"),r=[].join,l=a!=Object,c=o("join",",");i({target:"Array",proto:!0,forced:l||!c},{join:function(t){return r.call(n(this),void 0===t?",":t)}})},f25e:function(t,e,s){"use strict";s("f290")},f290:function(t,e,s){}}]);