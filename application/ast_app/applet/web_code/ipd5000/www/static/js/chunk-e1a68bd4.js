(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-e1a68bd4"],{"2d62":function(t,a,e){"use strict";e("d880")},"4df4":function(t,a,e){"use strict";var i=e("0366"),n=e("7b0b"),o=e("9bdd"),s=e("e95a"),r=e("50c4"),c=e("8418"),l=e("35a1");t.exports=function(t){var a,e,m,v,d,u,h=n(t),p="function"==typeof this?this:Array,f=arguments.length,k=f>1?arguments[1]:void 0,b=void 0!==k,M=l(h),g=0;if(b&&(k=i(k,f>2?arguments[2]:void 0,2)),void 0==M||p==Array&&s(M))for(a=r(h.length),e=new p(a);a>g;g++)u=b?k(h[g],g):h[g],c(e,g,u);else for(v=M.call(h),d=v.next,e=new p;!(m=d.call(v)).done;g++)u=b?o(v,k,[m.value,g],!0):m.value,c(e,g,u);return e.length=g,e}},"63a5":function(t,a,e){"use strict";e.r(a);var i=function(){var t=this,a=t.$createElement,e=t._self._c||a;return e("div",{staticClass:"main-setting"},[e("div",{staticClass:"setting-model"},[e("div",{staticClass:"radio-setting"},[e("span",{staticClass:"setting-title"},[t._v("USB over IP")]),e("div",[e("radio-component",{attrs:{disabled:"1"===t.castMode,label:"km"},model:{value:t.kvmMode,callback:function(a){t.kvmMode=a},expression:"kvmMode"}},[t._v("Optimized for KVM")]),e("radio-component",{attrs:{disabled:"1"===t.castMode,label:"usb"},model:{value:t.kvmMode,callback:function(a){t.kvmMode=a},expression:"kvmMode"}},[t._v("USB Emulation")])],1)]),e("div",{staticClass:"setting"},[e("span",{staticClass:"setting-title"},[t._v("Request Time Out (min)")]),e("el-input-number",{attrs:{"controls-position":"right",min:5,max:10,disabled:"1"===t.castMode},model:{value:t.timeout,callback:function(a){t.timeout=a},expression:"timeout"}})],1),"km"===t.kvmMode?e("div",{staticClass:"radio-setting"},[e("span",{staticClass:"setting-title"},[t._v("Roaming Master/Slaves")]),e("div",[e("radio-component",{attrs:{label:"1",disabled:"1"===t.castMode},model:{value:t.roaming,callback:function(a){t.roaming=a},expression:"roaming"}},[t._v("Master")]),e("radio-component",{attrs:{label:"0",disabled:"1"===t.castMode},model:{value:t.roaming,callback:function(a){t.roaming=a},expression:"roaming"}},[t._v("Slaves")])],1)]):t._e(),"1"===t.roaming&&"km"===t.kvmMode?e("div",{staticClass:"kvm-view"},[e("div",{staticClass:"error-input"},[t._v(" Row: "),e("el-input-number",{staticStyle:{width:"80px","margin-right":"24px"},attrs:{"controls-position":"right",max:16,min:1},on:{change:t.handleRowKvmMap},model:{value:t.row,callback:function(a){t.row=a},expression:"row"}}),t._v(" Col: "),e("el-input-number",{staticStyle:{width:"80px","margin-right":"24px"},attrs:{"controls-position":"right",max:16,min:1},on:{change:t.handleColKvmMap},model:{value:t.col,callback:function(a){t.col=a},expression:"col"}}),t.col*t.row>16?e("span",{staticClass:"alert-error",staticStyle:{top:"60px"}},[t._v("Maximum 16 Slaves")]):t._e()],1),t.col*t.row<=16?e("div",{staticClass:"kvm-layout"},t._l(t.row,(function(a,i){return e("div",{key:a},t._l(t.col,(function(a,n){return e("div",{key:a,staticClass:"kvm-child"},[e("icon-svg",{staticStyle:{margin:"0 auto",width:"60px",height:"60px",display:"block"},attrs:{"icon-class":"osd"}}),e("div",{staticStyle:{"text-align":"center","margin-top":"15px"}},[e("radio-component",{staticStyle:{"margin-bottom":"5px"},attrs:{label:t.kvmMap[i][n].h+","+t.kvmMap[i][n].v},on:{input:t.resetCoord},model:{value:t.master,callback:function(a){t.master=a},expression:"master"}},[t._v("Master")]),e("span",{staticStyle:{display:"block",margin:"5px 0 0"}},[t._v("MAC Address:")]),e("input",{directives:[{name:"model",rawName:"v-model",value:t.kvmMap[i][n].mac,expression:"kvmMap[y][x].mac"}],staticClass:"setting-text",staticStyle:{width:"130px","font-size":"14px","text-align":"center"},attrs:{type:"text",disabled:t.master===t.kvmMap[i][n].h+","+t.kvmMap[i][n].v},domProps:{value:t.kvmMap[i][n].mac},on:{input:function(a){a.target.composing||t.$set(t.kvmMap[i][n],"mac",a.target.value)}}})],1)],1)})),0)})),0):t._e()]):t._e()]),e("footer",[e("button",{staticClass:"btn btn-primary",attrs:{disabled:"1"===t.castMode},on:{click:t.save}},[t._v("SAVE")])])])},n=[];function o(t){if(Array.isArray(t))return t}e("a4d3"),e("e01a"),e("d3b7"),e("d28b"),e("3ca3"),e("ddb0");function s(t,a){if("undefined"!==typeof Symbol&&Symbol.iterator in Object(t)){var e=[],i=!0,n=!1,o=void 0;try{for(var s,r=t[Symbol.iterator]();!(i=(s=r.next()).done);i=!0)if(e.push(s.value),a&&e.length===a)break}catch(c){n=!0,o=c}finally{try{i||null==r["return"]||r["return"]()}finally{if(n)throw o}}return e}}e("fb6a"),e("b0c0"),e("a630");function r(t,a){(null==a||a>t.length)&&(a=t.length);for(var e=0,i=new Array(a);e<a;e++)i[e]=t[e];return i}function c(t,a){if(t){if("string"===typeof t)return r(t,a);var e=Object.prototype.toString.call(t).slice(8,-1);return"Object"===e&&t.constructor&&(e=t.constructor.name),"Map"===e||"Set"===e?Array.from(t):"Arguments"===e||/^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(e)?r(t,a):void 0}}function l(){throw new TypeError("Invalid attempt to destructure non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}function m(t,a){return o(t)||s(t,a)||c(t,a)||l()}function v(t){if(Array.isArray(t))return r(t)}function d(t){if("undefined"!==typeof Symbol&&Symbol.iterator in Object(t))return Array.from(t)}function u(){throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}function h(t){return v(t)||d(t)||c(t)||u()}e("498a"),e("ac1f"),e("841c"),e("5319"),e("1276"),e("159b"),e("d81d"),e("13d5"),e("99af");var p=e("b2c7"),f={name:"usb",components:{radioComponent:p["a"]},data:function(){return{timeout:5,kvmMode:"km",col:1,row:1,usbOverIp:"1",master:"0,0",roaming:"2",kvmMap:[[{h:0,mac:"",v:0}]],mac:"",kvm:{},castMode:"0"}},beforeCreate:function(){var t=this;this.$socket.ws.onmessage=function(a){t.handleMsg(a.data.trim())}},created:function(){this.$socket.sendMsg("#NET-MAC? 0"),this.$socket.sendMsg("#KDS-METHOD? ")},methods:{handleMsg:function(t){-1===t.search(/@KDS-METHOD /i)?-1!==t.search(/@NET-MAC /i)&&this.handleMACAddr(t):this.handleIpCastMode(t)},handleMACAddr:function(t){this.mac=t.split(" ")[1].split(",").pop().replace(/:|-/gi,""),this.getKVMJson()},handleIpCastMode:function(t){this.castMode=t.split(" ")[1]},save:function(){"1"===this.roaming&&this.col*this.row>16||(this.kvm.kvm_timeout_sec=this.timeout,this.kvm.kvm_usb_mode=this.kvmMode,this.kvm.kvm_col=this.col,this.kvm.kvm_row=this.row,this.kvm.km_roaming="0"===this.roaming?[]:this.formatKVMData(this.kvmMap),this.$http.post("/device/json",{path:"/usb/km_usb.json",info:{usb_kvm_config:this.kvm}}))},getKVMJson:function(){var t=this;this.$http.get("/device/json?path=/usb/km_usb.json&t="+Math.random()).then((function(a){a.data.usb_kvm_config&&(t.kvm=a.data.usb_kvm_config,t.timeout=t.kvm.kvm_timeout_sec,t.kvmMode=t.kvm.kvm_usb_mode,t.roaming=t.kvm.km_roaming.length?"1":"0",t.parseKVM(t.kvm.km_roaming))}))},parseKVM:function(t){var a=this;if(this.roaming=t.length>0?"1":"0",t.length>0){var e=[],i={};t.forEach((function(t){t.v===t.h&&0===t.v&&(t.mac=a.mac),i[t.v]?(e[i[t.v]-1].push(t),e[i[t.v]-1].sort((function(t,a){return t.h-a.h}))):(e.push([t]),i[t.v]=e.length)})),this.kvmMap=e.sort((function(t,a){return a[0].v-t[0].v})),this.row=e.length,this.col=e[0].length}},handleRowKvmMap:function(t,a){var e=this;if(t>a)(function(){for(var i=e.kvmMap[e.kvmMap.length-1][0].v-1,n=e.kvmMap[0][0].h,o=0;o<t-a;o++){var s=h(new Array(e.col)).map((function(t,a){return{mac:"",h:n+a,v:i}}));e.kvmMap.push(s)}})();else for(var i=0;i<a-t;i++)this.kvmMap.pop();this.checkMasterExist()},handleColKvmMap:function(t,a){var e=this;t!==this.kvmMap[0].length&&(t>a?this.kvmMap.forEach((function(i){for(var n=0;n<t-a;n++){var o=e.kvmMap[0][e.kvmMap[0].length-1].h+1;i.push({mac:"",h:o,v:i[0].v})}})):(this.kvmMap.forEach((function(e){for(var i=0;i<a-t;i++)e.pop()})),this.checkMasterExist()))},checkMasterExist:function(){var t=this.master.split(","),a=m(t,2),e=a[0],i=a[1],n=this.kvmMap[0][0].h,o=this.col+n-1,s=this.kvmMap[0][0].v,r=s-this.row+1;(e<n||e>o||i<r||i>s)&&(this.master=this.kvmMap[0][0].h+","+this.kvmMap[0][0].v,this.resetCoord())},resetCoord:function(){var t=this.masterCoord(),a=t.col,e=t.row;for(var i in this.kvmMap)for(var n in this.kvmMap[i]){var o=e-i,s=n-a;this.kvmMap[i][n].h=s,this.kvmMap[i][n].v=o,this.kvmMap[i][n].mac===this.mac&&(this.kvmMap[i][n].mac=""),s===o&&0===s&&(this.kvmMap[i][n].mac=this.mac)}this.master="0,0"},masterCoord:function(){var t=this.master.split(","),a=m(t,2),e=a[0],i=a[1];for(var n in e=+e,i=+i,this.kvmMap)for(var o in this.kvmMap[n])if(this.kvmMap[n][o].h===e&&this.kvmMap[n][o].v===i)return{row:n,col:o}},formatKVMData:function(t){return t.reduce((function(t,a){return t.concat(a)}))}}},k=f,b=(e("2d62"),e("2877")),M=Object(b["a"])(k,i,n,!1,null,"370b42fc",null);a["default"]=M.exports},"9bdd":function(t,a,e){var i=e("825a"),n=e("2a62");t.exports=function(t,a,e,o){try{return o?a(i(e)[0],e[1]):a(e)}catch(s){throw n(t),s}}},"9f7c":function(t,a,e){"use strict";e("e075")},a630:function(t,a,e){var i=e("23e7"),n=e("4df4"),o=e("1c7e"),s=!o((function(t){Array.from(t)}));i({target:"Array",stat:!0,forced:s},{from:n})},b2c7:function(t,a,e){"use strict";var i=function(){var t=this,a=t.$createElement,e=t._self._c||a;return e("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[e("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[e("span",{staticClass:"radio__inner"}),e("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(a){t.model=t.label},t.handleChange]}})]),e("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},n=[],o={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;this.$nextTick((function(){t.$emit("change",t.model)}))}}},s=o,r=(e("9f7c"),e("2877")),c=Object(r["a"])(s,i,n,!1,null,null,null);a["a"]=c.exports},d880:function(t,a,e){},e075:function(t,a,e){}}]);