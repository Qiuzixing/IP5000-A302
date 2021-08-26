(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-cc49b1c2"],{"06c5":function(t,e,a){"use strict";a.d(e,"a",(function(){return n}));a("fb6a"),a("d3b7"),a("b0c0"),a("a630"),a("3ca3");var i=a("6b75");function n(t,e){if(t){if("string"===typeof t)return Object(i["a"])(t,e);var a=Object.prototype.toString.call(t).slice(8,-1);return"Object"===a&&t.constructor&&(a=t.constructor.name),"Map"===a||"Set"===a?Array.from(t):"Arguments"===a||/^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(a)?Object(i["a"])(t,e):void 0}}},1554:function(t,e,a){},"17c1":function(t,e,a){"use strict";var i=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("label",{staticClass:"checkbox",class:{"is-checked":t.checked,isDisabled:t.disabled},on:{click:t.changeVal}},[a("span",{staticClass:"checkbox-input",class:{"is-checked":t.checked}},[a("span",{staticClass:"checkbox-inner"}),a("span",{staticClass:"checkbox-inner-label"},[t._v(t._s(t.label))])])])},n=[],o=(a("a9e3"),{name:"checkBox",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},activeValue:{type:[Boolean,String,Number],default:!0},inactiveValue:{type:[Boolean,String,Number],default:!1},value:{}},data:function(){return{}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}},checked:function(){return this.value===this.activeValue}},methods:{changeVal:function(){if(!this.disabled){var t=this.checked?this.inactiveValue:this.activeValue;this.$emit("input",t),this.$emit("click",t)}}},watch:{checked:function(t){this.isChecked=t}}}),s=o,c=(a("3791"),a("2877")),l=Object(c["a"])(s,i,n,!1,null,null,null);e["a"]=l.exports},2909:function(t,e,a){"use strict";a.d(e,"a",(function(){return l}));var i=a("6b75");function n(t){if(Array.isArray(t))return Object(i["a"])(t)}a("a4d3"),a("e01a"),a("d3b7"),a("d28b"),a("3ca3"),a("ddb0"),a("a630");function o(t){if("undefined"!==typeof Symbol&&Symbol.iterator in Object(t))return Array.from(t)}var s=a("06c5");function c(){throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}function l(t){return n(t)||o(t)||Object(s["a"])(t)||c()}},3791:function(t,e,a){"use strict";a("69b6")},"3b5c":function(t,e,a){},"3de2":function(t,e,a){"use strict";a.r(e);var i=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-model",staticStyle:{"margin-bottom":"0"}},[a("div",{staticClass:"radio-setting"},[a("span",{staticClass:"setting-title"},[t._v("Analog Audio Direction IN/OUT")]),a("div",[a("radio-component",{attrs:{label:"in"},model:{value:t.direction,callback:function(e){t.direction=e},expression:"direction"}},[t._v("IN")]),a("radio-component",{attrs:{label:"out"},model:{value:t.direction,callback:function(e){t.direction=e},expression:"direction"}},[t._v("OUT")])],1)]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Audio Source Mode")]),a("multiselect",{attrs:{options:t.audioMode.param},model:{value:t.audioMode.val,callback:function(e){t.$set(t.audioMode,"val",e)},expression:"audioMode.val"}})],1),a("custom-sort",{attrs:{listMap:t.listMap,disabled:"1"!==t.audioMode.val},model:{value:t.lists,callback:function(e){t.lists=e},expression:"lists"}}),a("div",{staticClass:"setting",staticStyle:{"margin-top":"25px"}},[a("span",{staticClass:"setting-title"},[t._v("Audio Source Selection")]),a("multiselect",{attrs:{disabled:"0"!=t.audioMode.val,options:t.audioSource.param},model:{value:t.audioSource.val,callback:function(e){t.$set(t.audioSource,"val",e)},expression:"audioSource.val"}})],1),a("div",{staticClass:"setting",staticStyle:{"margin-top":"24px"}},[a("span",{staticClass:"setting-title"},[t._v("Audio Connection Guard Time (sec)")]),a("el-input-number",{attrs:{"controls-position":"right",max:90,min:0},model:{value:t.avSignal["audio connection guard time sec"],callback:function(e){t.$set(t.avSignal,"audio connection guard time sec",e)},expression:"avSignal['audio connection guard time sec']"}})],1),a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-title"},[t._v("Audio Destination")]),t._l(t.audioDestinationDesc,(function(e,i){return a("div",{key:e.name,staticStyle:{"margin-bottom":"15px"}},[a("checkbox-component",{attrs:{label:e,"active-value":1,"inactive-value":0},model:{value:t.audioDestination[i],callback:function(e){t.$set(t.audioDestination,i,e)},expression:"audioDestination[index]"}})],1)}))],2)],1),a("footer",[a("button",{staticClass:"btn btn-primary",on:{click:t.save}},[t._v("SAVE")])])])},n=[],o=(a("498a"),a("ac1f"),a("841c"),a("1276"),a("5319"),a("466d"),a("159b"),a("a15b"),function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"sort-model"},t._l(t.model,(function(e,i){return a("div",{key:e,staticClass:"list-group-item",class:[{disabled:t.disabled}]},[0===i?a("span",{staticClass:"sort-title"},[t._v("High")]):t._e(),i===t.model.length-1?a("span",{staticClass:"sort-title"},[t._v("Low")]):t._e(),t.disabled?a("div",{staticClass:"sort-source",attrs:{draggable:"false"}},[t._v(" "+t._s(t.listMap[e])+" ")]):a("div",{staticClass:"sort-source",class:[{active:t.dragIndex===i},{"drag-enter":t.dragEnterIndex===i}],attrs:{draggable:"true"},on:{dragend:function(e){t.dragEnterIndex=-1,t.dragIndex=-1},dragenter:function(e){return t.dragEnter(i)},dragleave:function(e){t.dragEnterIndex=-1},dragstart:function(e){return t.dragStart(e,i)},dragover:t.allowDrop,drop:function(e){return t.drop(e,i)}}},[t._v(" "+t._s(t.listMap[e])+" ")])])})),0)}),s=[],c=a("2909"),l=(a("a434"),a("99af"),{name:"customSort",props:{disabled:{type:Boolean,default:!1},listMap:{type:Object,default:{}},value:{}},data:function(){return{dragIndex:-1,dragEnterIndex:-1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{dragEnter:function(t){t!==this.dragIndex&&(this.dragEnterIndex=t)},allowDrop:function(t){t.preventDefault()},drop:function(t,e){this.allowDrop(t);var a=this.dragIndex;if(this.dragIndex=-1,this.dragEnterIndex=-1,e!==this.dragIndex){var i=this.value;i.splice.apply(i,[a,1].concat(Object(c["a"])(i.splice(e,1,i[a])))),this.$emit("input",i)}},dragStart:function(t,e){this.dragIndex=e}}}),r=l,d=(a("6247"),a("2877")),u=Object(d["a"])(r,o,s,!1,null,"92982cb6",null),h=u.exports,f=a("b2c7"),v=a("17c1"),p={name:"auto-switch",components:{customSort:h,radioComponent:f["a"],checkboxComponent:v["a"]},data:function(){return{direction:"in",audioMode:{val:"2",param:[{value:"2",label:"Last Connected"},{value:"1",label:"Priority"},{value:"0",label:"Manual"}]},audioSource:{val:"0",param:[{value:"0",label:"HDMI"},{value:"1",label:"Analog"},{value:"2",label:"None"},{value:"3",label:"Dante"}]},playStop:"play",listMap:{"in.hdmi.1.audio":"HDMI","in.analog.1.audio":"Analog"},lists:["in.hdmi.1.audio","in.analog.1.audio"],avSignal:{"input maximum resolution":"Pass Through","maximum bit rate":"Best Effort","frame rate percentage":100,"color depth":"Follow Output","audio connection guard time sec":0,"dante vlan tag":""},audioDestination:[0,0],audioDestinationDesc:["HDMI","Analog"]}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(e){t.handleMsg(e.data.trim())}},created:function(){this.$socket.sendMsg("#PORT-DIRECTION? both.analog.1.audio"),this.$socket.sendMsg("#X-AV-SW-MODE? out.hdmi.1.audio.2"),this.$socket.sendMsg("#X-PRIORITY? out.hdmi.1.audio"),this.$socket.sendMsg("#KDS-AUD-OUTPUT? "),this.$socket.sendMsg("#KDS-AUD? "),this.getAVSignal()},methods:{handleMsg:function(t){console.log(t),-1===t.search(/@PORT-DIRECTION /i)?-1===t.search(/@X-AV-SW-MODE /i)?-1===t.search(/@X-PRIORITY /i)?-1===t.search(/@KDS-AUD-OUTPUT /i)?-1!==t.search(/@KDS-AUD /i)&&(this.audioSource.val=t.split(" ").pop()):this.handleAudioDestination(t):this.handleSwitchPriority(t):this.audioSwitchMode(t):this.handleDirection(t)},handleDirection:function(t){-1!==t.search(/audio/i)&&(this.direction=t.split(",")[1].toLowerCase())},audioSwitchMode:function(t){if(-1!==t.search(/audio/g)){var e=t.split(",");this.audioMode.val=e[e.length-1]}},getAVSignal:function(){var t=this;this.$http.post("/av_signal").then((function(e){e.data["AV Signal"]&&(t.avSignal=e.data["AV Signal"])}))},handleSwitchPriority:function(t){-1!==t.search(/audio/g)&&(this.lists=t.match(/[^([]+(?=\])/g)[0].replace(/\s/g,"").split(","))},handleAudioDestination:function(t){var e=[0,0,0,0],a=t.match(/[^([]+(?=\])/g)[0].replace(/\s/g,"").split(",");a.forEach((function(t){e[parseInt(t)]=1})),this.audioDestination=e},setAudioDestination:function(){var t=[];this.audioDestination.forEach((function(e,a){1===e&&t.push(a+1)})),this.$socket.sendMsg("#KDS-AUD-OUTPUT [".concat(t.join(","),"]"))},setAVSingle:function(){this.$http.post("/set_av_signal",{"AV Signal":this.avSignal})},save:function(){this.$socket.sendMsg("#PORT-DIRECTION both.analog.1.audio,".concat(this.direction)),this.$socket.sendMsg("#X-AV-SW-MODE out.hdmi.1.audio.2,".concat(this.audioMode.val)),"1"===this.audioMode.val&&this.$socket.sendMsg("#X-PRIORITY out.hdmi.1.audio,[".concat(this.lists.join(","),"]")),"0"===this.audioMode.val&&this.$socket.sendMsg("#KDS-AUD ".concat(this.audioSource.val)),this.setAVSingle(),this.setAudioDestination()}}},g=p,m=(a("e904"),Object(d["a"])(g,i,n,!1,null,"4e8ddbea",null));e["default"]=m.exports},"466d":function(t,e,a){"use strict";var i=a("d784"),n=a("825a"),o=a("50c4"),s=a("1d80"),c=a("8aa5"),l=a("14c3");i("match",1,(function(t,e,a){return[function(e){var a=s(this),i=void 0==e?void 0:e[t];return void 0!==i?i.call(e,a):new RegExp(e)[t](String(a))},function(t){var i=a(e,t,this);if(i.done)return i.value;var s=n(t),r=String(this);if(!s.global)return l(s,r);var d=s.unicode;s.lastIndex=0;var u,h=[],f=0;while(null!==(u=l(s,r))){var v=String(u[0]);h[f]=v,""===v&&(s.lastIndex=c(r,o(s.lastIndex),d)),f++}return 0===f?null:h}]}))},"4df4":function(t,e,a){"use strict";var i=a("0366"),n=a("7b0b"),o=a("9bdd"),s=a("e95a"),c=a("50c4"),l=a("8418"),r=a("35a1");t.exports=function(t){var e,a,d,u,h,f,v=n(t),p="function"==typeof this?this:Array,g=arguments.length,m=g>1?arguments[1]:void 0,b=void 0!==m,S=r(v),x=0;if(b&&(m=i(m,g>2?arguments[2]:void 0,2)),void 0==S||p==Array&&s(S))for(e=c(v.length),a=new p(e);e>x;x++)f=b?m(v[x],x):v[x],l(a,x,f);else for(u=S.call(v),h=u.next,a=new p;!(d=h.call(u)).done;x++)f=b?o(u,m,[d.value,x],!0):d.value,l(a,x,f);return a.length=x,a}},6247:function(t,e,a){"use strict";a("1554")},"69b6":function(t,e,a){},"6b75":function(t,e,a){"use strict";function i(t,e){(null==e||e>t.length)&&(e=t.length);for(var a=0,i=new Array(e);a<e;a++)i[a]=t[a];return i}a.d(e,"a",(function(){return i}))},"9bdd":function(t,e,a){var i=a("825a"),n=a("2a62");t.exports=function(t,e,a,o){try{return o?e(i(a)[0],a[1]):e(a)}catch(s){throw n(t),s}}},"9f7c":function(t,e,a){"use strict";a("e075")},a15b:function(t,e,a){"use strict";var i=a("23e7"),n=a("44ad"),o=a("fc6a"),s=a("a640"),c=[].join,l=n!=Object,r=s("join",",");i({target:"Array",proto:!0,forced:l||!r},{join:function(t){return c.call(o(this),void 0===t?",":t)}})},a434:function(t,e,a){"use strict";var i=a("23e7"),n=a("23cb"),o=a("a691"),s=a("50c4"),c=a("7b0b"),l=a("65f0"),r=a("8418"),d=a("1dde"),u=d("splice"),h=Math.max,f=Math.min,v=9007199254740991,p="Maximum allowed length exceeded";i({target:"Array",proto:!0,forced:!u},{splice:function(t,e){var a,i,d,u,g,m,b=c(this),S=s(b.length),x=n(t,S),y=arguments.length;if(0===y?a=i=0:1===y?(a=0,i=S-x):(a=y-2,i=f(h(o(e),0),S-x)),S+a-i>v)throw TypeError(p);for(d=l(b,i),u=0;u<i;u++)g=x+u,g in b&&r(d,u,b[g]);if(d.length=i,a<i){for(u=x;u<S-i;u++)g=u+i,m=u+a,g in b?b[m]=b[g]:delete b[m];for(u=S;u>S-i+a;u--)delete b[u-1]}else if(a>i)for(u=S-i;u>x;u--)g=u+i-1,m=u+a-1,g in b?b[m]=b[g]:delete b[m];for(u=0;u<a;u++)b[u+x]=arguments[u+2];return b.length=S-i+a,d}})},a630:function(t,e,a){var i=a("23e7"),n=a("4df4"),o=a("1c7e"),s=!o((function(t){Array.from(t)}));i({target:"Array",stat:!0,forced:s},{from:n})},b2c7:function(t,e,a){"use strict";var i=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[a("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[a("span",{staticClass:"radio__inner"}),a("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(e){t.model=t.label},t.handleChange]}})]),a("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},n=[],o={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;console.log(this.disabled),this.$nextTick((function(){t.$emit("change",t.model)}))}}},s=o,c=(a("9f7c"),a("2877")),l=Object(c["a"])(s,i,n,!1,null,null,null);e["a"]=l.exports},e075:function(t,e,a){},e904:function(t,e,a){"use strict";a("3b5c")}}]);