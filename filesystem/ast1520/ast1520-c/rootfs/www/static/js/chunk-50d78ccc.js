(window["webpackJsonp"]=window["webpackJsonp"]||[]).push([["chunk-50d78ccc"],{"21c4":function(t,e,s){},5883:function(t,e,s){"use strict";s("21c4")},"8a79":function(t,e,s){"use strict";var a=s("23e7"),i=s("06cf").f,r=s("50c4"),n=s("5a34"),l=s("1d80"),o=s("ab13"),c=s("c430"),p="".endsWith,d=Math.min,v=o("endsWith"),u=!c&&!v&&!!function(){var t=i(String.prototype,"endsWith");return t&&!t.writable}();a({target:"String",proto:!0,forced:!u&&!v},{endsWith:function(t){var e=String(l(this));n(t);var s=arguments.length>1?arguments[1]:void 0,a=r(e.length),i=void 0===s?a:d(r(s),a),o=String(t);return p?p.call(e,o,i):e.slice(i-o.length,i)===o}})},"9ba2":function(t,e,s){t.exports=s.p+"static/img/Upload.svg"},"9e33":function(t,e,s){"use strict";s.r(e);var a=function(){var t=this,e=t.$createElement,a=t._self._c||e;return a("div",{staticClass:"main-setting"},[a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-model-title"},[t._v("HTTPS")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("Server")]),a("v-switch",{attrs:{"active-value":"on","inactive-value":"off"},model:{value:t.httpsServer,callback:function(e){t.httpsServer=e},expression:"httpsServer"}})],1),a("div",[a("radio-component",{attrs:{disabled:"off"===t.httpsServer,label:"in"},on:{change:function(e){t.notHttpsFile=!1}},model:{value:t.https,callback:function(e){t.https=e},expression:"https"}},[t._v("Internal Certificate ")]),a("radio-component",{attrs:{disabled:"off"===t.httpsServer,label:"out"},on:{change:function(e){t.notHttpsFile=!1}},model:{value:t.https,callback:function(e){t.https=e},expression:"https"}},[t._v("Server Certificate ")]),"out"===t.https?a("div",{staticStyle:{"margin-left":"30px"}},[a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Upload Certificate")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.httpsFileName,expression:"httpsFileName"}],staticClass:"setting-text",attrs:{type:"text",disabled:"off"===t.httpsServer,readonly:""},domProps:{value:t.httpsFileName},on:{input:function(e){e.target.composing||(t.httpsFileName=e.target.value)}}}),a("input",{ref:"https",staticStyle:{display:"none"},attrs:{type:"file"},on:{change:t.httpsFileChange}}),a("img",{staticClass:"upload-icon",attrs:{src:s("9ba2"),alt:""},on:{click:function(e){return t.browseFile("https","on"===t.httpsServer)}}})]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Private Key Password")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.httpPrivatePwd,expression:"httpPrivatePwd"}],staticClass:"setting-text",attrs:{type:"password",maxLength:"24",disabled:"off"===t.httpsServer},domProps:{value:t.httpPrivatePwd},on:{input:function(e){e.target.composing||(t.httpPrivatePwd=e.target.value)}}})])]):t._e(),a("button",{staticClass:"btn btn-primary",staticStyle:{"margin-left":"30px"},on:{click:t.setHTTPS}},[t._v("APPLY & REBOOT ")]),t.notHttpsFile?a("p",{staticClass:"error",staticStyle:{"margin-left":"30px"}},[t._v(t._s(t.httpsErrorAlert))]):t._e()],1)]),a("div",{staticClass:"setting-model"},[a("h3",{staticClass:"setting-model-title"},[t._v("802.1x")]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title"},[t._v("IEE 802.1x Authentication")]),a("v-switch",{attrs:{"active-value":"on","inactive-value":"off"},model:{value:t.server8021x,callback:function(e){t.server8021x=e},expression:"server8021x"}})],1),a("div",[a("radio-component",{attrs:{disabled:"off"===t.server8021x,label:"eap_mschap"},on:{change:function(e){t.server8021error=!1}},model:{value:t.security801,callback:function(e){t.security801=e},expression:"security801"}},[t._v("PEAP-MSCHAP V2 ")]),"eap_mschap"===t.security801?a("div",{staticStyle:{"margin-left":"30px"}},[a("div",{staticClass:"setting",staticStyle:{position:"relative"}},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Username")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.mschap_username,expression:"mschap_username"}],staticClass:"setting-text",attrs:{type:"text",maxLength:"24",disabled:"off"===t.server8021x},domProps:{value:t.mschap_username},on:{input:function(e){e.target.composing||(t.mschap_username=e.target.value)}}}),t.mschap_username&&!t.isValidName(t.mschap_username)?a("span",{staticClass:"range-alert",staticStyle:{top:"36px","white-space":"nowrap"}},[t._v("Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed")]):t._e()]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Password")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.mschap_password,expression:"mschap_password"}],staticClass:"setting-text",attrs:{type:"password",maxLength:"24",disabled:"off"===t.server8021x},domProps:{value:t.mschap_password},on:{input:function(e){e.target.composing||(t.mschap_password=e.target.value)}}})])]):t._e(),a("radio-component",{attrs:{disabled:"off"===t.server8021x,label:"eap_tls"},on:{change:function(e){t.server8021error=!1}},model:{value:t.security801,callback:function(e){t.security801=e},expression:"security801"}},[t._v("EAP-TLS ")]),"eap_tls"===t.security801?a("div",{staticStyle:{"margin-left":"30px"}},[a("div",{staticClass:"setting",staticStyle:{position:"relative"}},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Username")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.tls_username,expression:"tls_username"}],staticClass:"setting-text",attrs:{type:"text",maxLength:"24",disabled:"off"===t.server8021x},domProps:{value:t.tls_username},on:{input:function(e){e.target.composing||(t.tls_username=e.target.value)}}}),t.tls_username&&!t.isValidName(t.tls_username)?a("span",{staticClass:"range-alert",staticStyle:{top:"36px","white-space":"nowrap"}},[t._v("Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed")]):t._e()]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("CA Certificate")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.tls_ca_certificate,expression:"tls_ca_certificate"}],staticClass:"setting-text",attrs:{type:"text",disabled:"off"===t.server8021x,readonly:""},domProps:{value:t.tls_ca_certificate},on:{input:function(e){e.target.composing||(t.tls_ca_certificate=e.target.value)}}}),a("input",{ref:"tls_ca_certificate",staticStyle:{display:"none"},attrs:{type:"file"},on:{change:function(e){return t.serve8021FileChange(e,"tls_ca_certificate")}}}),a("img",{staticClass:"upload-icon",attrs:{src:s("9ba2"),alt:""},on:{click:function(e){return t.browseFile("tls_ca_certificate","on"===t.server8021x)}}})]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Client Certificate")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.tls_client_certificate,expression:"tls_client_certificate"}],staticClass:"setting-text",attrs:{type:"text",disabled:"off"===t.server8021x,readonly:""},domProps:{value:t.tls_client_certificate},on:{input:function(e){e.target.composing||(t.tls_client_certificate=e.target.value)}}}),a("input",{ref:"tls_client_certificate",staticStyle:{display:"none"},attrs:{type:"file"},on:{change:function(e){return t.serve8021FileChange(e,"tls_client_certificate")}}}),a("img",{staticClass:"upload-icon",attrs:{src:s("9ba2"),alt:""},on:{click:function(e){return t.browseFile("tls_client_certificate","on"===t.server8021x)}}})]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Private Key")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.tls_private_key,expression:"tls_private_key"}],staticClass:"setting-text",attrs:{type:"text",disabled:"off"===t.server8021x,readonly:""},domProps:{value:t.tls_private_key},on:{input:function(e){e.target.composing||(t.tls_private_key=e.target.value)}}}),a("input",{ref:"tls_private_key",staticStyle:{display:"none"},attrs:{type:"file"},on:{change:function(e){return t.serve8021FileChange(e,"tls_private_key")}}}),a("img",{staticClass:"upload-icon",attrs:{src:s("9ba2"),alt:""},on:{click:function(e){return t.browseFile("tls_private_key","on"===t.server8021x)}}})]),a("div",{staticClass:"setting"},[a("span",{staticClass:"setting-title",staticStyle:{width:"180px"}},[t._v("Private Key Password")]),a("input",{directives:[{name:"model",rawName:"v-model",value:t.tls_private_password,expression:"tls_private_password"}],staticClass:"setting-text",attrs:{type:"password",maxLength:"24",disabled:"off"===t.server8021x},domProps:{value:t.tls_private_password},on:{input:function(e){e.target.composing||(t.tls_private_password=e.target.value)}}})])]):t._e(),a("button",{staticClass:"btn btn-primary",staticStyle:{"margin-left":"30px"},on:{click:t.set8021x}},[t._v("APPLY ")]),t.server8021error?a("p",{staticClass:"error",staticStyle:{"margin-left":"30px"}},[t._v(t._s(t.server8021Alert))]):t._e()],1)])])},i=[],r=(s("8a79"),s("b0c0"),s("b2c7")),n={name:"autoSwitch",components:{radioComponent:r["a"]},data:function(){return{https:"in",httpsServer:"off",httpsFileName:"",httpPrivatePwd:"",notHttpsFile:!1,httpsErrorAlert:"",server8021x:"off",security801:"eap_mschap",mschap_username:"",mschap_password:"",tls_username:"",tls_ca_certificate:"",tls_client_certificate:"",tls_private_key:"",tls_private_password:"",server8021error:!1,server8021Alert:""}},created:function(){this.getHTTPS(),this.get801x()},methods:{getHTTPS:function(){var t=this;this.$http.get("/device/json?path=/secure/https_setting.json&t="+Math.random()).then((function(e){e.data.https_setting&&(t.httpsServer=e.data.https_setting.mode,t.https=e.data.https_setting.method)}))},get801x:function(){var t=this;this.$http.get("/device/json?path=/secure/ieee802_1x_setting.json&t="+Math.random()).then((function(e){e.data.ieee802_1x_setting&&(t.server8021x=e.data.ieee802_1x_setting.mode,t.security801=e.data.ieee802_1x_setting.default_authentication||"eap_mschap")}))},setHTTPS:function(){var t=this,e=new FormData;if(e.append("mode",this.httpsServer),"on"===this.httpsServer&&(e.append("method",this.https),"out"===this.https)){if(!this.httpsFileName)return this.httpsErrorAlert="Please select file",void(this.notHttpsFile=!0);if(!this.httpsFileName.endsWith(".pem")&&!this.httpsFileName.endsWith(".key"))return this.httpsErrorAlert="File format error",void(this.notHttpsFile=!0);e.append("certificate",this.$refs.https.files[0]),e.append("password",this.httpPrivatePwd)}this.notHttpsFile=!1;var s=new XMLHttpRequest;s.open("POST","/security/https"),s.onload=function(e){200===s.status&&(t.httpsFileName="",t.httpPrivatePwd="",console.log("upload success"))},s.send(e)},browseFile:function(t,e){e&&this.$refs[t].click()},httpsFileChange:function(t){var e;this.httpsFileName=(null===(e=t.target.files[0])||void 0===e?void 0:e.name)||""},serve8021FileChange:function(t,e){var s;this[e]=(null===(s=t.target.files[0])||void 0===s?void 0:s.name)||""},set8021x:function(){var t=this,e=new FormData;if(e.append("mode",this.server8021x),"on"===this.server8021x)if(e.append("default_authentication",this.security801),"eap_mschap"===this.security801){if(!this.mschap_username)return this.server8021Alert="Please enter username",void(this.server8021error=!0);if(!this.mschap_password)return this.server8021Alert="Please enter password",void(this.server8021error=!0);if(!this.isValidName(this.mschap_username)||!this.isValidName(this.mschap_password))return;e.append("mschap_username",this.mschap_username),e.append("mschap_password",this.mschap_password)}else{if(!this.tls_username)return this.server8021Alert="Please enter username",void(this.server8021error=!0);if(!this.tls_ca_certificate)return this.server8021Alert="Please select CA certifivate file",void(this.server8021error=!0);if(!this.tls_client_certificate)return this.server8021Alert="Please select client certifivate file",void(this.server8021error=!0);if(!this.tls_private_key)return this.server8021Alert="Please select private key file",void(this.server8021error=!0);if(!this.tls_private_password)return this.server8021Alert="Please enter password",void(this.server8021error=!0);if(!this.isValidName(this.tls_username)||!this.isValidName(this.tls_private_password))return;e.append("tls_username",this.tls_username),e.append("tls_ca_certificate",this.$refs.tls_ca_certificate.files[0]),e.append("tls_client_certificate",this.$refs.tls_client_certificate.files[0]),e.append("tls_private_key",this.$refs.tls_private_key.files[0]),e.append("tls_private_password",this.tls_private_password)}this.server8021error=!1;var s=new XMLHttpRequest;s.open("POST","/security/802_1x"),s.onload=function(e){200===s.status&&(t.httpsFileName="",t.httpPrivatePwd="",console.log("upload success"))},s.send(e)},isValidName:function(t){return/^[a-zA-Z0-9][_\-a-zA-Z0-9]{0,23}$/.test(t)}}},l=n,o=(s("5883"),s("2877")),c=Object(o["a"])(l,a,i,!1,null,"9c94dd24",null);e["default"]=c.exports},"9f7c":function(t,e,s){"use strict";s("e075")},b2c7:function(t,e,s){"use strict";var a=function(){var t=this,e=t.$createElement,s=t._self._c||e;return s("label",{staticClass:"radio-model",class:{"is-disabled":t.disabled}},[s("span",{staticClass:"radio_input",class:{"is-checked":t.model===t.label}},[s("span",{staticClass:"radio__inner"}),s("input",{directives:[{name:"model",rawName:"v-model",value:t.model,expression:"model"}],attrs:{type:"radio",disabled:t.disabled,name:t.name},domProps:{value:t.label,checked:t._q(t.model,t.label)},on:{change:[function(e){t.model=t.label},t.handleChange]}})]),s("span",{staticClass:"radio__label",on:{keydown:function(t){t.stopPropagation()}}},[t._t("default"),t.$slots.default?t._e():[t._v(t._s(t.isEmpty?"":t.label))]],2)])},i=[],r={name:"radioComponent",props:{label:{type:String,default:""},disabled:{type:Boolean,default:!1},name:String,value:{},isEmpty:{type:Boolean,default:!1}},computed:{model:{get:function(){return this.value},set:function(t){this.$emit("input",t)}}},methods:{handleChange:function(){var t=this;this.$nextTick((function(){t.$emit("change",t.model)}))}}},n=r,l=(s("9f7c"),s("2877")),o=Object(l["a"])(n,a,i,!1,null,null,null);e["a"]=o.exports},e075:function(t,e,s){}}]);