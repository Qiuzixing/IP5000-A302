(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-45c4470e"],{1106:function(t,e,n){t.exports=n.p+"static/img/arrow.svg"},"2ce6":function(t,e,n){"use strict";n.r(e);var s=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-model"},[s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Menu Timeout (sec)")]),s("el-input-number",{attrs:{"controls-position":"right",max:3600,precision:0,min:0},model:{value:t.osdConfig.timeout_sec,callback:function(e){t.$set(t.osdConfig,"timeout_sec",e)},expression:"osdConfig.timeout_sec"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Channels per Page")]),s("el-input-number",{attrs:{"controls-position":"right",max:10,min:5},model:{value:t.osdConfig.max_channels_per_page,callback:function(e){t.$set(t.osdConfig,"max_channels_per_page",e)},expression:"osdConfig.max_channels_per_page"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Maximum Channels")]),s("el-input-number",{attrs:{"controls-position":"right",max:999,min:0},model:{value:t.osdConfig.max_channels,callback:function(e){t.$set(t.osdConfig,"max_channels",e)},expression:"osdConfig.max_channels"}})],1),s("div",[s("span",{staticClass:"setting-title"},[t._v("Channel List")]),s("ul",{staticClass:"edid-list"},[s("li",[s("button",{staticClass:"btn btn-primary",attrs:{disabled:this.channelList.length>=t.osdConfig.max_channels},on:{click:function(e){t.dialogType=1}}},[t._v("ADD")]),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{type:"button"},on:{click:t.browseChannelList}},[t._v("IMPORT")]),s("button",{staticClass:"btn btn-plain-primary",staticStyle:{"margin-left":"24px"},attrs:{type:"button"}},[t._v(" EXPORT ")]),s("input",{ref:"channelList",staticStyle:{display:"none",width:"0",height:"0"},attrs:{type:"file"},on:{change:t.channelFileChange}})]),t.channelError?s("li",{staticStyle:{color:"#d50000","font-size":"12px","font-family":"'open sans bold'"}},[t._v(t._s(t.channelErrorMsg))]):t._e(),t._m(0),t._l(t.currentData,(function(e,n){return s("li",{key:e.id,class:{active:t.edidListIndex===n}},[s("span",{staticClass:"channel-title",on:{click:function(e){t.edidListIndex=n}}},[t._v(t._s(e.id))]),s("span",{staticClass:"channel-name",on:{click:function(e){t.edidListIndex=n}}},[t._v(t._s(e.name))]),s("span",{staticClass:"channel-icon"},[s("span",{on:{click:function(e){return t.editChannel(n)}}},[s("icon-svg",{staticStyle:{"margin-right":"5px"},attrs:{"icon-class":"edit"}})],1),s("el-popconfirm",{attrs:{title:"Are you sure you want to delete "+e.name+"?"},on:{confirm:function(e){return t.deleteChannel(n)}}},[s("icon-svg",{attrs:{slot:"reference","icon-class":"rubbish"},slot:"reference"})],1)],1)])}))],2),s("div",{staticClass:"channel-list-pagination",staticStyle:{"margin-bottom":"24px"}},[s("span",[t._v("Page "+t._s(t.currentPage)+" of "+t._s(t.countPages()))]),s("div",[s("span",{staticClass:"channel-list-icon",on:{click:function(e){return t.next(t.currentPage-1)}}},[s("img",{attrs:{src:n("1106")}})]),s("span",{staticClass:"channel-list-icon",on:{click:function(e){return t.next(t.currentPage+1)}}},[s("img",{attrs:{src:n("1106")}})])])])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Menu Font Size")]),s("multiselect",{attrs:{options:t.osdSize.param},model:{value:t.osdConfig.font_size,callback:function(e){t.$set(t.osdConfig,"font_size",e)},expression:"osdConfig.font_size"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Menu Position")]),s("multiselect",{attrs:{options:t.osdPosition.param},model:{value:t.osdConfig.position,callback:function(e){t.$set(t.osdConfig,"position",e)},expression:"osdConfig.position"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Display Device Information")]),s("v-switch",{attrs:{"active-value":"1","inactive-value":"0"},on:{change:t.setDisplayInfo},model:{value:t.osdInfo,callback:function(e){t.osdInfo=e},expression:"osdInfo"}}),s("button",{staticClass:"btn",class:["1"===t.osdInfo?"btn-plain-primary":"btn-default"],staticStyle:{"margin-left":"24px"},attrs:{disabled:"0"===t.osdInfo,type:"button"},on:{click:function(e){return t.setDisplayInfo("2")}}},[t._v("DISPLAY NOW")])],1)]),s("footer",[s("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])]),s("el-dialog",{attrs:{title:"Channel",visible:1==t.dialogType,width:"400px","before-close":t.closeDialog}},[s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("ID")]),s("div",{class:{"error-input":t.idError},staticStyle:{position:"relative",flex:"1"}},[s("input",{directives:[{name:"model",rawName:"v-model",value:t.addChannel.id,expression:"addChannel.id"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"3"},domProps:{value:t.addChannel.id},on:{input:function(e){e.target.composing||t.$set(t.addChannel,"id",e.target.value)}}}),s("span",{staticClass:"alert-error"},[t._v("Numbers only")])])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("Name")]),s("div",{class:{"error-input":t.nameError},staticStyle:{position:"relative",flex:"1"}},[s("input",{directives:[{name:"model",rawName:"v-model",value:t.addChannel.name,expression:"addChannel.name"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"24"},domProps:{value:t.addChannel.name},on:{input:function(e){e.target.composing||t.$set(t.addChannel,"name",e.target.value)}}}),s("span",{staticClass:"alert-error"},[t._v("Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed.")])])]),t.idRulerError?s("span",{staticStyle:{color:"#d50000","font-size":"14px","font-family":"open sans bold"}},[t._v("This id already exists")]):t._e(),s("span",{staticClass:"dialog-footer",staticStyle:{padding:"0"},attrs:{slot:"footer"},slot:"footer"},[s("button",{staticClass:"btn btn-primary",on:{click:t.handleAddChannel}},[t._v("APPLY")])])]),s("el-dialog",{attrs:{title:"Channel Edit",visible:2==t.dialogType,width:"400px","before-close":t.closeDialog}},[s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("ID")]),s("div",{staticStyle:{position:"relative",flex:"1"}},[t._v(" "+t._s(t.editObj.id)+" ")])]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title",staticStyle:{width:"80px"}},[t._v("Name")]),s("div",{class:{"error-input":t.editError},staticStyle:{position:"relative",flex:"1"}},[s("input",{directives:[{name:"model",rawName:"v-model",value:t.editObj.name,expression:"editObj.name"}],staticClass:"setting-text",attrs:{type:"text",maxlength:"24"},domProps:{value:t.editObj.name},on:{input:function(e){e.target.composing||t.$set(t.editObj,"name",e.target.value)}}}),s("span",{staticClass:"alert-error"},[t._v("Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed.")])])]),s("span",{staticClass:"dialog-footer",staticStyle:{padding:"0"},attrs:{slot:"footer"},slot:"footer"},[s("button",{staticClass:"btn btn-primary",on:{click:t.handleEditChannel}},[t._v("APPLY")])])])],1)},i=[function(){var t=this,e=t.$createElement,n=t._self._c||e;return n("li",[n("span",{staticClass:"channel-title"},[t._v("#ID")]),n("span",[t._v("Name")])])}],a=(n("fb6a"),n("498a"),n("ac1f"),n("841c"),n("b0c0"),n("a434"),n("466d"),n("1276"),{name:"osd",data:function(){return{osdInfo:"0",osdConfig:{timeout_sec:100,position:"top_center",font_size:"small",max_channels_per_page:10,max_channels:999},dialogType:0,edidListIndex:0,prePage:5,maxChannels:100,currentPage:1,show:!1,osdSize:{val:"large",param:[{value:"large",label:"Large"},{value:"medium",label:"Medium"},{value:"small",label:"Small"}]},osdPosition:{val:"top_left",param:[{value:"top_left",label:"Top Left"},{value:"top_right",label:"Top Right"},{value:"top_center",label:"Top Center"},{value:"bottom_left",label:"Bottom Left"},{value:"bottom_right",label:"Bottom Right"},{value:"bottom_center",label:"Bottom Center"},{value:"left",label:"Left"},{value:"right",label:"Right"},{value:"center",label:"Center"}]},displayDevice:!1,channelList:[],addChannel:{id:"",name:""},idError:!1,nameError:!1,idRulerError:!1,editError:!1,editObj:{index:0,name:"",id:""},osdJson:{},channelError:!1,channelErrorMsg:""}},computed:{currentData:function(){if(this.channelList.length>0){var t=(this.currentPage-1)*this.osdConfig.max_channels_per_page,e=this.currentPage*this.osdConfig.max_channels_per_page;return this.channelList.slice(t,e)}return[]}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#KDS-OSD-DISPLAY? "),this.getOsdJson(),this.getAvChannelMap()},methods:{handleMsg:function(t){console.log(t),-1!==t.search(/@KDS-OSD-DISPLAY /i)&&this.handleOsdInfo(t)},getOsdJson:function(){var t=this;this.$http.get("/device/json?path=/osd/osd.json&t="+Math.random()).then((function(e){e.data.channel_menu&&(t.osdJson=e.data,t.osdConfig=e.data.channel_menu)}))},getAvChannelMap:function(){var t=this;this.$http.get("/device/json?path=/channel/channel_map.json&t="+Math.random()).then((function(e){e.data.channels_list&&(t.channelList=e.data.channels_list)}))},browseChannelList:function(){this.$refs.channelList.click()},channelFileChange:function(t){var e=this,n=t.target.files[0];if(n){if("application/json"!==n.type)return this.channelErrorMsg="File format error",void(this.channelError=!0);if(n.size>1048576)return this.channelErrorMsg="The file size is less than 1MB",void(this.channelError=!0);var s=new FileReader;s.readAsText(n,"UTF-8"),s.onload=function(t){var n=JSON.parse(t.target.result);Array.isArray(n.channels_list)?(e.channelError=!1,e.channelList=n.channels_list):(e.channelErrorMsg="File format error",e.channelError=!0)}}else this.channelError=!1},next:function(t){var e=this.countPages();t<=0||t>e||(this.currentPage=t,this.edidListIndex=0)},closeDialog:function(){this.dialogType=0},handleAddChannel:function(){if(this.idError=!this.isID(this.addChannel.id),this.nameError=!this.isName(this.addChannel.name),!this.idError&&!this.nameError){var t=!1;for(var e in this.channelList)this.channelList[e].id===this.addChannel.id&&(t=!0);this.idRulerError=t,t||(this.channelList.push({id:parseInt(this.addChannel.id),name:this.addChannel.name}),this.channelList=this.sortChannel(this.channelList),this.dialogType=0)}},sortChannel:function(t){return t.sort((function(t,e){return t.id-e.id}))},deleteChannel:function(t){var e=this.osdConfig.max_channels_per_page*(this.currentPage-1)+t;this.channelList.splice(e,1),this.currentPage>this.countPages()&&(this.currentPage=this.countPages(),this.edidListIndex=0)},editChannel:function(t){var e=this.osdConfig.max_channels_per_page*(this.currentPage-1)+t,n=this.channelList.slice(e,e+1)[0];this.editObj={index:e,name:n.name,id:n.id},this.dialogType=2},handleEditChannel:function(){this.editError=!this.isName(this.editObj.name),this.editError||(this.channelList[this.editObj.index].name=this.editObj.name,this.dialogType=0)},countPages:function(){return Math.ceil(this.channelList.length/this.osdConfig.max_channels_per_page)},isID:function(t){return t.match(/^[1-9]?[1-9]?[1-9]$/)},isName:function(t){return/^[A-Za-z0-9][A-Za-z0-9\-_]{0,23}$/.test(t)},setDisplayInfo:function(t){this.$socket.sendMsg("#KDS-OSD-DISPLAY "+t)},handleOsdInfo:function(t){this.osdInfo="0"!==t.split(" ")[1]?"1":"0"},save:function(){var t;this.osdJson.channel_menu=this.osdConfig,null!==(t=this.osdJson)&&void 0!==t&&t.device_info&&(this.osdJson.device_info.enabled="1"===this.osdInfo?"on":"off"),this.$http.post("/device/json",{path:"/osd/osd.json",info:this.osdJson}),this.$http.post("/device/json",{path:"/channel/channel_map.json",info:{channels_list:this.channelList}})}}}),o=a,l=(n("7774"),n("2877")),r=Object(l["a"])(o,s,i,!1,null,"ca4e5490",null);e["default"]=r.exports},"466d":function(t,e,n){"use strict";var s=n("d784"),i=n("825a"),a=n("50c4"),o=n("1d80"),l=n("8aa5"),r=n("14c3");s("match",1,(function(t,e,n){return[function(e){var n=o(this),s=void 0==e?void 0:e[t];return void 0!==s?s.call(e,n):new RegExp(e)[t](String(n))},function(t){var s=n(e,t,this);if(s.done)return s.value;var o=i(t),c=String(this);if(!o.global)return r(o,c);var d=o.unicode;o.lastIndex=0;var h,u=[],p=0;while(null!==(h=r(o,c))){var f=String(h[0]);u[p]=f,""===f&&(o.lastIndex=l(c,a(o.lastIndex),d)),p++}return 0===p?null:u}]}))},7774:function(t,e,n){"use strict";n("d22a")},a434:function(t,e,n){"use strict";var s=n("23e7"),i=n("23cb"),a=n("a691"),o=n("50c4"),l=n("7b0b"),r=n("65f0"),c=n("8418"),d=n("1dde"),h=d("splice"),u=Math.max,p=Math.min,f=9007199254740991,g="Maximum allowed length exceeded";s({target:"Array",proto:!0,forced:!h},{splice:function(t,e){var n,s,d,h,m,v,C=l(this),_=o(C.length),b=i(t,_),x=arguments.length;if(0===x?n=s=0:1===x?(n=0,s=_-b):(n=x-2,s=p(u(a(e),0),_-b)),_+n-s>f)throw TypeError(g);for(d=r(C,s),h=0;h<s;h++)m=b+h,m in C&&c(d,h,C[m]);if(d.length=s,n<s){for(h=b;h<_-s;h++)m=h+s,v=h+n,m in C?C[v]=C[m]:delete C[v];for(h=_;h>_-s+n;h--)delete C[h-1]}else if(n>s)for(h=_-s;h>b;h--)m=h+s-1,v=h+n-1,m in C?C[v]=C[m]:delete C[v];for(h=0;h<n;h++)C[h+b]=arguments[h+2];return C.length=_-s+n,d}})},d22a:function(t,e,n){}}]);