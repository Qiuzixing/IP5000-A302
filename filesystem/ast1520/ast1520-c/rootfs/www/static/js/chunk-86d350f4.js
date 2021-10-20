(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-86d350f4"],{"17d0":function(t,e,s){},4275:function(t,e,s){"use strict";s("17d0")},"9f7c":function(t,e,s){"use strict";s("e075")},b2c7:function(t,e,s){"use strict";var i=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[s("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[s("span",{staticClass:"radio__inner"}),s("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(e){t.model=t.label},t.handleChange]}})]),s("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},a=[],n={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;this.$nextTick((function(){t.$emit("change",t.model)}))}}},l=n,o=(s("9f7c"),s("2877")),c=Object(o["a"])(l,i,a,!1,null,null,null);e["a"]=c.exports},de99:function(t,e,s){"use strict";s.r(e);var i=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("div",{staticClass:"main-setting"},[s("div",{staticClass:"setting-model"},[s("div",[s("span",{staticClass:"setting-title",staticStyle:{"margin-right":"15px"}},[t._v("Horizontal")]),s("el-input-number",{staticClass:"video-wall-child",staticStyle:{width:"80px","margin-right":"30px"},attrs:{max:16,min:1,"controls-position":"right"},model:{value:t.col,callback:function(e){t.col=e},expression:"col"}}),s("span",{staticClass:"setting-title",staticStyle:{"margin-right":"15px"}},[t._v("Vertical")]),s("el-input-number",{staticClass:"video-wall-child",staticStyle:{width:"80px"},attrs:{max:16,min:1,"controls-position":"right"},on:{change:t.checkSelectId},model:{value:t.row,callback:function(e){t.row=e},expression:"row"}})],1),s("p",{staticClass:"setting-title",staticStyle:{margin:"15px 0 0"}},[t._v("Layout")]),s("div",{staticClass:"video-wall"},t._l(t.row,(function(e){return s("div",{key:e,staticClass:"video-wall-row"},t._l(t.col,(function(i){return s("div",{key:i,staticClass:"video-wall-col",class:{active:(e-1)*t.col+i===t.selectedId},on:{click:function(s){t.selectedId=(e-1)*t.col+i}}},[s("span",[t._v("Screen "+t._s((e-1)*t.col+i))])])})),0)})),0),s("div",{staticClass:"radio-setting"},[s("span",{staticClass:"setting-title"},[t._v("Stretch Type")]),s("div",[s("radio-component",{attrs:{label:"0"},model:{value:t.stretchType,callback:function(e){t.stretchType=e},expression:"stretchType"}},[t._v("Fit In")]),s("radio-component",{attrs:{label:"1"},model:{value:t.stretchType,callback:function(e){t.stretchType=e},expression:"stretchType"}},[t._v("Fit Out")])],1)]),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Video Rotation")]),s("multiselect",{attrs:{options:t.rotationParam},model:{value:t.videoRotation,callback:function(e){t.videoRotation=e},expression:"videoRotation"}})],1),s("div",{staticClass:"setting"},[s("span",{staticClass:"setting-title"},[t._v("Video Wall Test")]),s("v-switch",{attrs:{"open-text":"Start","close-text":"Stop"}})],1)]),s("footer",[s("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},a=[],n=(s("498a"),s("ac1f"),s("841c"),s("1276"),s("99af"),s("b2c7")),l={name:"videoWall",components:{radioComponent:n["a"]},data:function(){return{stretchType:"0",roaming:"0",bezel:"0",roamingMode:!1,videoRotation:"0",col:2,row:2,selectedId:1,rotationParam:[{value:"0",label:"0"},{value:"1",label:"90"},{value:"2",label:"180"},{value:"3",label:"270"}]}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#VIEW-MOD? "),this.$socket.sendMsg("#VIDEO-WALL-SETUP? ")},methods:{handleMsg:function(t){-1===t.search(/@VIEW-MOD /i)?-1===t.search(/@VIDEO-WALL-SETUP /i)?-1===t.search(/@WND-STRETCH /i)?-1!==t.search(/@WND-BEZEL /i)&&this.handleBEZEL(t):this.handleStretch(t):this.handleSelectedLayoutRotation(t):this.handleViewMode(t)},handleViewMode:function(t){var e=t.split(",");this.col=+e[1],this.row=+e[2]},handleSelectedLayoutRotation:function(t){var e=t.split(" ")[1].split(",");this.selectedId=+e[0],this.videoRotation=e[1],this.$socket.sendMsg("#WND-STRETCH? "+this.selectedId)},handleStretch:function(t){this.stretchType=t.split(" ")[1].split(",")[1]},handleBEZEL:function(t){this.bezel=+t.split(",")[2]},checkBlur:function(){this.bezel=this.bezel||0},checkSelectId:function(){this.selectedId>this.col*this.row&&(this.selectedId=1)},save:function(){this.$socket.sendMsg("#VIEW-MOD 15,".concat(this.col,",").concat(this.row)),this.$socket.sendMsg("#VIDEO-WALL-SETUP ".concat(this.selectedId,",").concat(this.videoRotation)),this.$socket.sendMsg("#WND-STRETCH ".concat(this.selectedId,",").concat(this.stretchType))}}},o=l,c=(s("4275"),s("2877")),d=Object(c["a"])(o,i,a,!1,null,"4ab21daa",null);e["default"]=d.exports},e075:function(t,e,s){}}]);