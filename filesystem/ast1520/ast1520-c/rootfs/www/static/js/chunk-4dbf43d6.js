(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-4dbf43d6"],{"04c0":function(t,e,a){"use strict";a("ef68")},1106:function(t,e,a){t.exports=a.p+"static/img/arrow.svg"},"17c1":function(t,e,a){"use strict";var s=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("label",{staticClass:"checkbox",class:{"is-checked":t.checked,isDisabled:t.disabled},on:{click:t.changeVal}},[a("span",{staticClass:"checkbox-input",class:{"is-checked":t.checked}},[a("span",{staticClass:"checkbox-inner"}),a("span",{staticClass:"checkbox-inner-label"},[t._v(t._s(t.label))])])])},i=[],o=(a("a9e3"),{name:"checkBox",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},activeValue:{type:[Boolean,String,Number],default:!0},inactiveValue:{type:[Boolean,String,Number],default:!1},value:{}},data:function(){return{}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}},checked:function(){return this.value===this.activeValue}},methods:{changeVal:function(){if(!this.disabled){var t=this.checked?this.inactiveValue:this.activeValue;this.$emit("input",t),this.$emit("click",t)}}},watch:{checked:function(t){this.isChecked=t}}}),n=o,l=(a("3791"),a("2877")),c=Object(l["a"])(n,s,i,!1,null,null,null);e["a"]=c.exports},3791:function(t,e,a){"use strict";a("69b6")},"57b7":function(t,e,a){"use strict";a.r(e);var s=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-model"},[a("div",{staticClass:"overlay"},[a("div",[a("v-collapse",{attrs:{title:"Image settings"}},[a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Image")]),a("div",{staticClass:"overlay-setting-item overlay-img"},[a("span",{staticClass:"file-name",staticStyle:{display:"inline-block",overflow:"hidden"}},[t._v(t._s(t.imgName))]),a("span",{staticClass:"\n                  upload-icon",on:{click:t.clickUpload}},[a("icon-svg",{attrs:{"icon-class":"upload_img"}})],1),a("input",{ref:"upload",staticStyle:{display:"none"},attrs:{type:"file",accept:"image/png"},on:{change:t.browseImg}}),t.imgError?a("span",{staticClass:"range-alert",staticStyle:{"white-space":"nowrap"}},[t._v(t._s(t.imgErrorMsg))]):t._e(),t.uploadComplete?a("span",{staticStyle:{"font-size":"20px","margin-left":"15px",color:"#67c23a"}},[a("i",{staticClass:"el-icon-circle-check"})]):t._e()])]),a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Align")]),a("div",{staticClass:"overlay-setting-item"},[a("el-select",{model:{value:t.imageInfo.objects[0].position,callback:function(e){t.$set(t.imageInfo.objects[0],"position",e)},expression:"imageInfo.objects[0].position"}},t._l(t.horizontalPosition.param,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1)]),a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Transparency (%)")]),a("div",{staticClass:"overlay-setting-item"},[a("el-slider",{staticStyle:{width:"200px",display:"inline-block"},attrs:{min:0,max:100,"show-tooltip":!1,marks:t.marks},model:{value:t.imageInfo.genral.transparency,callback:function(e){t.$set(t.imageInfo.genral,"transparency",e)},expression:"imageInfo.genral.transparency"}}),a("span",{staticStyle:{"margin-left":"15px"}},[t._v(t._s(t.imageInfo.genral.transparency))])],1)]),a("div",{staticClass:"overlay-setting",staticStyle:{"margin-top":"24px"}},[a("span",{staticClass:"overlay-title"},[t._v("Show")]),a("v-checkbox",{attrs:{"active-value":"on","inactive-value":"off"},on:{click:function(e){return t.setShow(e,1)}},model:{value:t.imageInfo.genral.enable,callback:function(e){t.$set(t.imageInfo.genral,"enable",e)},expression:"imageInfo.genral.enable"}})],1)]),a("v-collapse",{attrs:{title:"Text settings"}},[a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Text")]),a("div",{staticClass:"overlay-setting-item"},[a("input",{directives:[{name:"model",rawName:"v-model",value:t.textInfo.objects[0].caption,expression:"textInfo.objects[0].caption"}],staticClass:"setting-text",staticStyle:{width:"100%"},attrs:{type:"text"},domProps:{value:t.textInfo.objects[0].caption},on:{input:function(e){e.target.composing||t.$set(t.textInfo.objects[0],"caption",e.target.value)}}})])]),a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Size")]),a("div",{staticClass:"fontSize.val"},[a("el-select",{model:{value:t.textInfo.objects[0].size,callback:function(e){t.$set(t.textInfo.objects[0],"size",e)},expression:"textInfo.objects[0].size"}},t._l(t.fontSize.param,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1)]),a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Align")]),a("div",{staticClass:"overlay-setting-item"},[a("el-select",{model:{value:t.textInfo.objects[0].position,callback:function(e){t.$set(t.textInfo.objects[0],"position",e)},expression:"textInfo.objects[0].position"}},t._l(t.horizontalPosition.param,(function(t){return a("el-option",{key:t.value,attrs:{label:t.label,value:t.value}})})),1)],1)]),a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Color")]),a("div",{staticClass:"overlay-setting-item",staticStyle:{position:"relative"}},[a("div",{staticClass:"color-desc"},[a("p",{staticStyle:{margin:"0"}},[a("span",{staticClass:"color-box",style:{background:t.color}}),t._v(" "+t._s(t.color))]),a("color-picker",{attrs:{"color-format":"hex","show-alpha":!1},model:{value:t.color,callback:function(e){t.color=e},expression:"color"}})],1)])]),a("div",{staticClass:"overlay-setting"},[a("span",{staticClass:"overlay-title"},[t._v("Transparency (%)")]),a("div",{staticClass:"overlay-setting-item"},[a("el-slider",{staticStyle:{width:"200px"},attrs:{min:0,max:100,"show-tooltip":!1,marks:t.marks},model:{value:t.textInfo.genral.transparency,callback:function(e){t.$set(t.textInfo.genral,"transparency",e)},expression:"textInfo.genral.transparency"}}),a("span",{staticStyle:{"margin-left":"15px"}},[t._v(t._s(t.textInfo.genral.transparency))])],1)]),a("div",{staticClass:"overlay-setting",staticStyle:{"margin-top":"24px"}},[a("span",{staticClass:"overlay-title"},[t._v("Show")]),a("v-checkbox",{attrs:{"active-value":"on","inactive-value":"off"},on:{click:function(e){return t.setShow(e,2)}},model:{value:t.textInfo.genral.enable,callback:function(e){t.$set(t.textInfo.genral,"enable",e)},expression:"textInfo.genral.enable"}})],1)])],1)])]),a("footer",[a("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE ")])])])},i=[],o=(a("b0c0"),a("ac1f"),a("5319"),a("1276"),function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"v-collapse",class:{open:t.open}},[s("div",{staticClass:"v-collapse-header",on:{click:t.openCollapse}},[s("div",[s("img",{attrs:{src:a("1106"),alt:""}}),t._v(t._s(t.title))])]),s("transition",{attrs:{name:"slide"}},[t.open?s("div",{staticClass:"v-collapse-body"},[t._t("default")],2):t._e()])],1)}),n=[],l={name:"Collapse",props:{title:{required:!0}},data:function(){return{open:!0}},methods:{openCollapse:function(){this.open=!this.open}}},c=l,r=(a("fd3e"),a("2877")),v=Object(r["a"])(c,o,n,!1,null,"0eb75d7c",null),p=v.exports,f=a("17c1"),u=a("2ef0"),g={name:"autoSwitch",components:{vCollapse:p,vCheckbox:f["a"]},data:function(){return{imgError:!1,imgErrorMsg:"",uploadComplete:!1,imgName:"",imageInfo:{genral:{enable:"on",timeout:2,transparency:0},objects:[{type:"image",position:"top_left",path:""}]},textInfo:{genral:{enable:"off",timeout:2,transparency:0},objects:[{type:"text",position:"top_left",caption:"",size:"small",color:"0xffffff"}]},num:61,marks:{0:"0",100:"100"},show:!1,horizontalPosition:{param:[{value:"top_left",label:"Top left"},{value:"top_right",label:"Top right"},{value:"top_center",label:"Top center"},{value:"bottom_left",label:"Bottom left"},{value:"bottom_right",label:"Bottom right"},{value:"bottom_center",label:"Bottom center"},{value:"left",label:"Left"},{value:"right",label:"Right"},{value:"center",label:"Center"}]},fontSize:{val:"small",param:[{value:"small",label:"Small"},{value:"medium",label:"Medium"},{value:"large",label:"Large"}]},color:"#FFFFFF"}},created:function(){this.getTextInfo(),this.getImgInfo()},methods:{showOverlay:function(t){if(t){var e="on"===this.imageInfo.genral.enable?"overlay2_setting":"overlay1_setting";this.$socket.sendMsg("#KDS-START-OVERLAY ".concat(e,",0"))}else this.$socket.sendMsg("#KDS-STOP-OVERLAY ")},clickUpload:function(){this.$refs.upload.click()},browseImg:function(t){var e,a=this;if(this.imgName=(null===(e=t.target.files[0])||void 0===e?void 0:e.name)||"",this.imgName){var s=t.target.files[0];if(s.size>1048576)return this.imgErrorMsg="Maximum file size should be less than 1MB",void(this.imgError=!0);if("image/png"!==s.type)return this.imgErrorMsg="The image format must be PNG (resolution less than 640 x 360)",void(this.imgError=!0);var i=new FileReader;i.onload=function(t){var e=t.target.result,s=new Image;s.onload=function(){var t=s.width,e=s.height;t<=640&&e<=360?a.imgError=!1:(a.imgErrorMsg="The image format must be PNG (640 x 360)",a.imgError=!0)},s.src=e},i.readAsDataURL(s)}else this.imgError=!1},setShow:function(t,e){1===e?"on"===t&&"on"===this.textInfo.genral.enable&&(this.textInfo.genral.enable="off"):"on"===t&&"on"===this.imageInfo.genral.enable&&(this.imageInfo.genral.enable="off")},getTextInfo:function(){var t=this;this.$http.get("/device/json?path=/overlay/overlay1_setting.json&t="+Math.random()).then((function(e){t.textInfo=e.data,t.color=e.data.objects[0].color.replace("0x","#")}))},getImgInfo:function(){var t=this;this.$http.get("/device/json?path=/overlay/overlay2_setting.json&t="+Math.random()).then((function(e){t.imageInfo=e.data,t.imgName=e.data.objects[0].path.split("/").pop()||""}))},save:Object(u["debounce"])((function(){this.saveTextInfo(),this.saveImgInfo(),this.saveImg()}),2e3,{leading:!0,trailing:!0}),saveTextInfo:function(){this.textInfo.objects[0].color=this.color.replace("#","0x"),this.$http.post("/device/json",{path:"/overlay/overlay1_setting.json",info:this.textInfo})},saveImgInfo:function(){var t=this;this.$http.post("/device/json",{path:"/overlay/overlay2_setting.json",info:this.imageInfo}).then((function(){t.$msg.successAlert()}))},saveImg:function(){var t=this;if(!this.imgError&&this.$refs.upload.files[0]){var e=this.$refs.upload.files[0],a=new XMLHttpRequest,s=new FormData;s.append("file",e),a.open("POST","/upload/overlayimage"),a.onload=function(){200===a.status&&(t.$refs.upload.value="",t.uploadComplete=!0,setTimeout((function(){t.getImgInfo(),t.uploadComplete=!1}),1e3))},a.send(s)}}}},m=g,d=(a("04c0"),Object(r["a"])(m,s,i,!1,null,"af3aeb38",null));e["default"]=d.exports},"69b6":function(t,e,a){},b5f0:function(t,e,a){},ef68:function(t,e,a){},fd3e:function(t,e,a){"use strict";a("b5f0")}}]);