(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-0d5d8aca"],{1106:function(t,e,n){t.exports=n.p+"static/img/arrow.svg"},"21a6":function(t,e,n){(function(n){var a,i,s;(function(n,o){i=[],a=o,s="function"===typeof a?a.apply(e,i):a,void 0===s||(t.exports=s)})(0,(function(){"use strict";function e(t,e){return"undefined"==typeof e?e={autoBom:!1}:"object"!=typeof e&&(console.warn("Deprecated: Expected third argument to be a object"),e={autoBom:!e}),e.autoBom&&/^\s*(?:text\/\S*|application\/xml|\S*\/\S*\+xml)\s*;.*charset\s*=\s*utf-8/i.test(t.type)?new Blob(["\ufeff",t],{type:t.type}):t}function a(t,e,n){var a=new XMLHttpRequest;a.open("GET",t),a.responseType="blob",a.onload=function(){r(a.response,e,n)},a.onerror=function(){console.error("could not download file")},a.send()}function i(t){var e=new XMLHttpRequest;e.open("HEAD",t,!1);try{e.send()}catch(t){}return 200<=e.status&&299>=e.status}function s(t){try{t.dispatchEvent(new MouseEvent("click"))}catch(a){var e=document.createEvent("MouseEvents");e.initMouseEvent("click",!0,!0,window,0,0,0,80,20,!1,!1,!1,!1,0,null),t.dispatchEvent(e)}}var o="object"==typeof window&&window.window===window?window:"object"==typeof self&&self.self===self?self:"object"==typeof n&&n.global===n?n:void 0,l=o.navigator&&/Macintosh/.test(navigator.userAgent)&&/AppleWebKit/.test(navigator.userAgent)&&!/Safari/.test(navigator.userAgent),r=o.saveAs||("object"!=typeof window||window!==o?function(){}:"download"in HTMLAnchorElement.prototype&&!l?function(t,e,n){var l=o.URL||o.webkitURL,r=document.createElement("a");e=e||t.name||"download",r.download=e,r.rel="noopener","string"==typeof t?(r.href=t,r.origin===location.origin?s(r):i(r.href)?a(t,e,n):s(r,r.target="_blank")):(r.href=l.createObjectURL(t),setTimeout((function(){l.revokeObjectURL(r.href)}),4e4),setTimeout((function(){s(r)}),0))}:"msSaveOrOpenBlob"in navigator?function(t,n,o){if(n=n||t.name||"download","string"!=typeof t)navigator.msSaveOrOpenBlob(e(t,o),n);else if(i(t))a(t,n,o);else{var l=document.createElement("a");l.href=t,l.target="_blank",setTimeout((function(){s(l)}))}}:function(t,e,n,i){if(i=i||open("","_blank"),i&&(i.document.title=i.document.body.innerText="downloading..."),"string"==typeof t)return a(t,e,n);var s="application/octet-stream"===t.type,r=/constructor/i.test(o.HTMLElement)||o.safari,c=/CriOS\/[\d]+/.test(navigator.userAgent);if((c||s&&r||l)&&"undefined"!=typeof FileReader){var d=new FileReader;d.onloadend=function(){var t=d.result;t=c?t:t.replace(/^data:[^;]*;/,"data:attachment/file;"),i?i.location.href=t:location=t,i=null},d.readAsDataURL(t)}else{var h=o.URL||o.webkitURL,u=h.createObjectURL(t);i?i.location=u:location.href=u,i=null,setTimeout((function(){h.revokeObjectURL(u)}),4e4)}});o.saveAs=r.saveAs=r,t.exports=r}))}).call(this,n("c8ba"))},"2ce6":function(t,e,n){"use strict";n.r(e);var a=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-model"},[this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Menu Timeout (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:3600,precision:0,min:0},model:{value:t.osdConfig.timeout_sec,callback:function(e){t.$set(t.osdConfig,"timeout_sec",e)},expression:"osdConfig.timeout_sec"}})],1):t._e(),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Channels per Page")]),a("el-input-number",{attrs:{"controls-position":"right",max:10,min:5},model:{value:t.osdConfig.max_channels_per_page,callback:function(e){t.$set(t.osdConfig,"max_channels_per_page",e)},expression:"osdConfig.max_channels_per_page"}})],1),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Maximum Channels")]),a("el-input-number",{attrs:{"controls-position":"right",max:999,min:0},model:{value:t.osdConfig.max_channels,callback:function(e){t.$set(t.osdConfig,"max_channels",e)},expression:"osdConfig.max_channels"}})],1),a("div",[a("span",{staticClass:"setting-title"},[t._v("Channel List")]),a("ul",{staticClass:"edid-list"},[a("li",[a("button",{staticClass:"btn btn-primary",attrs:{disabled:this.channelList.length>=t.osdConfig.max_channels},on:{click:function(e){t.idRulerError=!1,t.idError=!1,t.nameError=!1,t.addChannel.name="",t.addChannel.id="",t.dialogType=1}}},[t._v("ADD ")]),a("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{type:"button"},on:{click:t.browseChannelList}},[t._v("IMPORT ")]),a("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{type:"button"},on:{click:t.exportChannel}},[t._v(" EXPORT ")]),a("input",{ref:"channelList",staticStyle:{display:"none",width:"0",height:"0"},attrs:{type:"file",accept:"application/json"},on:{change:t.channelFileChange}})]),t.channelError?a("li",{staticStyle:{color:"#d50000","font-size":"12px","font-family":"'open sans bold'"}},[t._v(t._s(t.channelErrorMsg)+" ")]):t._e(),t._m(0),t._l(t.currentData,(function(e,n){return a("li",{key:e.id,class:{active:t.edidListIndex===n}},[a("span",{staticClass:"channel-title",on:{click:function(e){t.edidListIndex=n}}},[t._v(t._s(e.id))]),a("span",{staticClass:"channel-name",on:{click:function(e){t.edidListIndex=n}}},[t._v(t._s(e.name))]),a("span",{staticClass:"channel-icon"},[a("span",{on:{click:function(e){return t.editChannel(n)}}},[a("icon-svg",{staticStyle:{"margin-right":"5px"},attrs:{"icon-class":"edit"}})],1),a("el-popconfirm",{attrs:{title:"Are you sure you want to delete "+e.name+"?"},on:{confirm:function(e){return t.deleteChannel(n)}}},[a("icon-svg",{attrs:{slot:"reference","icon-class":"rubbish"},slot:"reference"})],1)],1)])}))],2),a("div",{staticClass:"channel-list-pagination",staticStyle:{"margin-bottom":"24px"}},[a("span",[t._v("Page "+t._s(t.currentPage)+" of "+t._s(t.countPages()))]),a("div",[a("span",{staticClass:"channel-list-icon",on:{click:function(e){return t.next(t.currentPage-1)}}},[a("img",{attrs:{src:n("1106")}})]),a("span",{staticClass:"channel-list-icon",on:{click:function(e){return t.next(t.currentPage+1)}}},[a("img",{attrs:{src:n("1106")}})])])])]),this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Menu Font Size")]),a("el-select",{model:{value:t.osdConfig.font_size,callback:function(e){t.$set(t.osdConfig,"font_size",e)},expression:"osdConfig.font_size"}},t._l(t.osdSize.param,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1):t._e(),this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Menu Position")]),a("el-select",{model:{value:t.osdConfig.position,callback:function(e){t.$set(t.osdConfig,"position",e)},expression:"osdConfig.position"}},t._l(t.osdPosition.param,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1):t._e(),this.$global.deviceType?a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Display Device Information")]),a("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.setDisplayInfo},model:{value:t.osdInfo,callback:function(e){t.osdInfo=e},expression:"osdInfo"}}),a("button",{staticClass:"btn",class:["1"===t.osdInfo?"btn-plain-primary":"btn-default"],staticStyle:{"margin-left":"24px"},attrs:{disabled:"0"===t.osdInfo,type:"button"},on:{click:function(e){return t.setDisplayInfo("2")}}},[t._v("DISPLAY NOW ")])],1):t._e()]),a("footer",[a("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE ")])]),a("el-dialog",{attrs:{title:"Channel",visible:1==t.dialogType,width:"400px","before-close":t.closeDialog}},[a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("ID")]),a("div",{class:{"error-input":t.idError},staticStyle:{position:"relative",flex:"1"}},[a("input",{directives:[{name:"model",rawName:"v-model",value:t.addChannel.id,expression:"addChannel.id"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"3"},domProps:{value:t.addChannel.id},on:{input:function(e){e.target.composing||t.$set(t.addChannel,"id",e.target.value)}}}),a("span",{staticClass:"alert-error"},[t._v("Numbers only")])])]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("Name")]),a("div",{class:{"error-input":t.nameError},staticStyle:{position:"relative",flex:"1"}},[a("input",{directives:[{name:"model",rawName:"v-model",value:t.addChannel.name,expression:"addChannel.name"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"24"},domProps:{value:t.addChannel.name},on:{input:function(e){e.target.composing||t.$set(t.addChannel,"name",e.target.value)}}}),a("span",{staticClass:"alert-error"},[t._v("Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed.")])])]),t.idRulerError?a("span",{staticStyle:{color:"#d50000","font-size":"14px","font-family":"open sans bold"}},[t._v("This id already exists")]):t._e(),a("span",{staticClass:"dialog-footer",staticStyle:{padding:"0"},attrs:{slot:"footer"},slot:"footer"},[a("button",{staticClass:"btn btn-primary",on:{click:t.handleAddChannel}},[t._v("APPLY")])])]),a("el-dialog",{attrs:{title:"Channel Edit",visible:2==t.dialogType,width:"400px","before-close":t.closeDialog}},[a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("ID")]),a("div",{staticStyle:{position:"relative",flex:"1"}},[t._v(" "+t._s(t.editObj.id)+" ")])]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("Name")]),a("div",{class:{"error-input":t.editError},staticStyle:{position:"relative",flex:"1"}},[a("input",{directives:[{name:"model",rawName:"v-model",value:t.editObj.name,expression:"editObj.name"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"24"},domProps:{value:t.editObj.name},on:{input:function(e){e.target.composing||t.$set(t.editObj,"name",e.target.value)}}}),a("span",{staticClass:"alert-error"},[t._v("Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed.")])])]),a("span",{staticClass:"dialog-footer",staticStyle:{padding:"0"},attrs:{slot:"footer"},slot:"footer"},[a("button",{staticClass:"btn btn-primary",on:{click:t.handleEditChannel}},[t._v("APPLY")])])])],1)},i=[function(){var t=this,e=t.$createElement,n=t._self._c||e;return n("li",[n("span",{staticClass:"channel-title"},[t._v("#ID")]),n("span",[t._v("Name")])])}],s=(n("fb6a"),n("ac1f"),n("841c"),n("b0c0"),n("a434"),n("466d"),n("1276"),n("21a6")),o={name:"osd",data:function(){return{osdInfo:"0",osdConfig:{timeout_sec:100,position:"top_center",font_size:"small",max_channels_per_page:10,max_channels:999},dialogType:0,edidListIndex:0,prePage:5,maxChannels:100,currentPage:1,show:!1,osdSize:{val:"large",param:[{value:"large",label:"Large"},{value:"medium",label:"Medium"},{value:"small",label:"Small"}]},osdPosition:{val:"top_left",param:[{value:"top_left",label:"Top Left"},{value:"top_right",label:"Top Right"},{value:"top_center",label:"Top Center"},{value:"bottom_left",label:"Bottom Left"},{value:"bottom_right",label:"Bottom Right"},{value:"bottom_center",label:"Bottom Center"},{value:"left",label:"Left"},{value:"right",label:"Right"},{value:"center",label:"Center"}]},displayDevice:!1,channelList:[],addChannel:{id:"",name:""},idError:!1,nameError:!1,idRulerError:!1,editError:!1,editObj:{index:0,name:"",id:""},osdJson:{},channelError:!1,channelErrorMsg:""}},computed:{currentData:function(){if(this.channelList.length>0){var t=(this.currentPage-1)*this.osdConfig.max_channels_per_page,e=this.currentPage*this.osdConfig.max_channels_per_page;return this.channelList.slice(t,e)}return[]}},created:function(){this.$socket.setCallback(this.handleMsg),this.$socket.sendMsg("#KDS-OSD-DISPLAY? "),this.getOsdJson(),this.getAvChannelMap()},methods:{handleMsg:function(t){-1!==t.search(/@KDS-OSD-DISPLAY /i)&&this.handleOsdInfo(t)},getOsdJson:function(){var t=this;this.$http.get("/device/json?path=/osd/osd.json&t="+Math.random()).then((function(e){e.data.channel_menu&&(t.osdJson=e.data,t.osdConfig=e.data.channel_menu)}))},getAvChannelMap:function(){var t=this;this.$http.get("/device/json?path=/channel/channel_map.json&t="+Math.random()).then((function(e){e.data.channels_list&&(t.channelList=e.data.channels_list)}))},exportChannel:function(){this.$http.get("/device/json?path=/channel/channel_map.json&t="+Math.random()).then((function(t){if(t.data.channels_list){var e=new Blob([JSON.stringify(t.data)],{type:"text/plain;charset=utf-8"});Object(s["saveAs"])(e,"channel_map.json")}}))},browseChannelList:function(){this.$refs.channelList.click()},channelFileChange:function(t){var e=this,n=t.target.files[0];if(n){if("application/json"!==n.type)return this.channelErrorMsg="File format error",void(this.channelError=!0);if(n.size>131072)return this.channelErrorMsg="The file size is less than 128KB",void(this.channelError=!0);var a=new FileReader;a.readAsText(n,"UTF-8"),a.onload=function(t){try{var n=JSON.parse(t.target.result);Array.isArray(n.channels_list)?(e.channelError=!1,e.channelList=n.channels_list):(e.channelErrorMsg="File format error",e.channelError=!0)}catch(t){e.channelErrorMsg="File format error",e.channelError=!0}}}else this.channelError=!1},next:function(t){var e=this.countPages();t<=0||t>e||(this.currentPage=t,this.edidListIndex=0)},closeDialog:function(){this.dialogType=0},handleAddChannel:function(){if(this.idError=!this.isID(this.addChannel.id),this.nameError=!this.isName(this.addChannel.name),!this.idError&&!this.nameError){var t=!1;for(var e in this.channelList)+this.channelList[e].id===+this.addChannel.id&&(t=!0);this.idRulerError=t,t||(this.channelList.push({id:parseInt(this.addChannel.id),name:this.addChannel.name}),this.channelList=this.sortChannel(this.channelList),this.dialogType=0)}},sortChannel:function(t){return t.sort((function(t,e){return t.id-e.id}))},deleteChannel:function(t){var e=this.osdConfig.max_channels_per_page*(this.currentPage-1)+t;this.channelList.splice(e,1),this.currentPage>this.countPages()&&(this.currentPage=this.countPages(),this.edidListIndex=0)},editChannel:function(t){var e=this.osdConfig.max_channels_per_page*(this.currentPage-1)+t,n=this.channelList.slice(e,e+1)[0];this.editObj={index:e,name:n.name,id:n.id},this.dialogType=2},handleEditChannel:function(){this.editError=!this.isName(this.editObj.name),this.editError||(this.channelList[this.editObj.index].name=this.editObj.name,this.dialogType=0)},countPages:function(){var t=Math.ceil(this.channelList.length/this.osdConfig.max_channels_per_page);return this.currentPage>t&&(this.currentPage=1),t},isID:function(t){return t.match(/^[1-9][0-9]?[0-9]?$/)},isName:function(t){return/^[A-Za-z0-9][A-Za-z0-9\-_]{0,23}$/.test(t)},setDisplayInfo:function(t){this.$socket.sendMsg("#KDS-OSD-DISPLAY "+t)},handleOsdInfo:function(t){this.osdInfo="0"!==t.split(" ")[1]?"1":"0"},save:function(){var t;this.osdJson.channel_menu=this.osdConfig,null!==(t=this.osdJson)&&void 0!==t&&t.device_info&&(this.osdJson.device_info.enabled="1"===this.osdInfo?"on":"off"),this.$http.post("/device/json",{path:"/osd/osd.json",info:this.osdJson}),this.$http.post("/device/json",{path:"/channel/channel_map.json",info:{channels_list:this.channelList}})}}},l=o,r=(n("9ff7"),n("2877")),c=Object(r["a"])(l,a,i,!1,null,"3ae8e92c",null);e["default"]=c.exports},"466d":function(t,e,n){"use strict";var a=n("d784"),i=n("825a"),s=n("50c4"),o=n("1d80"),l=n("8aa5"),r=n("14c3");a("match",1,(function(t,e,n){return[function(e){var n=o(this),a=void 0==e?void 0:e[t];return void 0!==a?a.call(e,n):new RegExp(e)[t](String(n))},function(t){var a=n(e,t,this);if(a.done)return a.value;var o=i(t),c=String(this);if(!o.global)return r(o,c);var d=o.unicode;o.lastIndex=0;var h,u=[],p=0;while(null!==(h=r(o,c))){var f=String(h[0]);u[p]=f,""===f&&(o.lastIndex=l(c,s(o.lastIndex),d)),p++}return 0===p?null:u}]}))},"9ff7":function(t,e,n){"use strict";n("ecd1")},a434:function(t,e,n){"use strict";var a=n("23e7"),i=n("23cb"),s=n("a691"),o=n("50c4"),l=n("7b0b"),r=n("65f0"),c=n("8418"),d=n("1dde"),h=d("splice"),u=Math.max,p=Math.min,f=9007199254740991,g="Maximum allowed length exceeded";a({target:"Array",proto:!0,forced:!h},{splice:function(t,e){var n,a,d,h,v,m,b=l(this),_=o(b.length),C=i(t,_),y=arguments.length;if(0===y?n=a=0:1===y?(n=0,a=_-C):(n=y-2,a=p(u(s(e),0),_-C)),_+n-a>f)throw TypeError(g);for(d=r(b,a),h=0;h<a;h++)v=C+h,v in b&&c(d,h,b[v]);if(d.length=a,n<a){for(h=C;h<_-a;h++)v=h+a,m=h+n,v in b?b[m]=b[v]:delete b[m];for(h=_;h>_-a+n;h--)delete b[h-1]}else if(n>a)for(h=_-a;h>C;h--)v=h+a-1,m=h+n-1,v in b?b[m]=b[v]:delete b[m];for(h=0;h<n;h++)b[h+C]=arguments[h+2];return b.length=_-a+n,d}})},ecd1:function(t,e,n){}}]);