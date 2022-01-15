<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">HTTPS</h3>
      <div class="setting">
        <span class="setting-title">Server</span>
        <v-switch v-model="httpsServer"
                  active-value="on"
                  inactive-value="off"></v-switch>
      </div>
      <div>
        <radio-component v-model="https"
                         :disabled="httpsServer === 'off'"
                         @change="notHttpsFile=false"
                         label="in">Internal Certificate
        </radio-component>
        <radio-component v-model="https"
                         :disabled="httpsServer === 'off'"
                         label="out"
                         @change="notHttpsFile=false">Server Certificate
        </radio-component>
        <div v-if="https === 'out'"
             style="margin-left: 30px">
          <div class="setting">
            <span class="setting-title"
                  style="width: 180px;">Upload Certificate</span>
            <input type="text"
                   class="setting-text"
                   v-model="httpsFileName"
                   :disabled="httpsServer === 'off'"
                   readonly>
            <input type="file"
                   ref="https"
                   @change="httpsFileChange"
                   style="display:none;">
            <img class="upload-icon"
                 @click="browseFile('https', httpsServer === 'on')"
                 src="../../assets/img/Upload.svg"
                 alt="">
          </div>
          <div class="setting">
            <span class="setting-title"
                  style="width: 180px;">Private Key Password</span>
            <input type="password"
                   maxLength="24"
                   class="setting-text"
                   :disabled="httpsServer === 'off'"
                   v-model="httpPrivatePwd">
          </div>
        </div>
        <button class="btn btn-primary"
                style="margin-left: 30px"
                @click="setHTTPS">APPLY & REBOOT
        </button>
        <p class="error"
           style="margin-left: 30px;"
           v-if="notHttpsFile">{{ httpsErrorAlert }}</p>
      </div>
    </div>
    <div class="setting-model">
      <h3 class="setting-model-title">802.1x</h3>
      <div class="setting">
        <span class="setting-title">IEEE 802.1x Authentication</span>
        <v-switch v-model="server8021x"
                  active-value="on"
                  inactive-value="off"></v-switch>
      </div>
      <div>
        <radio-component v-model="security801"
                         :disabled="server8021x === 'off'"
                         label="eap_mschap"
                         @change="server8021error=false">PEAP-MSCHAP V2
        </radio-component>
        <div v-if="security801 === 'eap_mschap'"
             style="margin-left: 30px">
          <div class="setting" style="position: relative;">
            <span class="setting-title"
                  style="width: 180px;">Username</span>
            <input type="text"
                   maxLength="24"
                   class="setting-text"
                   :disabled="server8021x === 'off'"
                   v-model="mschap_username">
            <span class="range-alert"
                  v-if="mschap_username && !isValidName(mschap_username)"
                  style="top:36px;white-space: nowrap;">Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed</span>
          </div>
          <div class="setting">
            <span class="setting-title"
                  style="width: 180px;">Password</span>
            <input type="password"
                   maxLength="24"
                   class="setting-text"
                   :disabled="server8021x === 'off'"
                   v-model="mschap_password">
          </div>
        </div>
        <radio-component v-model="security801"
                         :disabled="server8021x === 'off'"
                         @change="server8021error=false"
                         label="eap_tls">EAP-TLS
        </radio-component>
        <div v-if="security801 === 'eap_tls'"
             style="margin-left: 30px">
          <div class="setting" style="position: relative;">
            <span class="setting-title"
                  style="width: 180px;">Username</span>
            <input type="text"
                   maxLength="24"
                   class="setting-text"
                   :disabled="server8021x === 'off'"
                   v-model="tls_username">
            <span class="range-alert"
                  v-if="tls_username && !isValidName(tls_username)"
                  style="top:36px;white-space: nowrap;">Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed</span>
          </div>
          <div class="setting">
            <span class="setting-title"
                  style="width: 180px;">CA Certificate</span>
            <input type="text"
                   class="setting-text"
                   v-model="tls_ca_certificate"
                   :disabled="server8021x === 'off'"
                   readonly>
            <input type="file"
                   ref="tls_ca_certificate"
                   @change="serve8021FileChange($event, 'tls_ca_certificate')"
                   style="display:none;">
            <img class="upload-icon"
                 @click="browseFile('tls_ca_certificate', server8021x === 'on')"
                 src="../../assets/img/Upload.svg"
                 alt="">
          </div>
          <div class="setting">
            <span class="setting-title"
                  style="width: 180px;">Client Certificate</span>
            <input type="text"
                   v-model="tls_client_certificate"
                   class="setting-text"
                   :disabled="server8021x === 'off'"
                   readonly>
            <input type="file"
                   ref="tls_client_certificate"
                   @change="serve8021FileChange($event, 'tls_client_certificate')"
                   style="display:none;">
            <img class="upload-icon"
                 @click="browseFile('tls_client_certificate',server8021x === 'on')"
                 src="../../assets/img/Upload.svg"
                 alt="">
          </div>
          <div class="setting">
            <span class="setting-title"
                  style="width: 180px;">Private Key</span>
            <input type="text"
                   v-model="tls_private_key"
                   class="setting-text"
                   :disabled="server8021x === 'off'"
                   readonly>
            <input type="file"
                   ref="tls_private_key"
                   @change="serve8021FileChange($event, 'tls_private_key')"
                   style="display:none;">
            <img class="upload-icon"
                 @click="browseFile('tls_private_key',server8021x === 'on')"
                 src="../../assets/img/Upload.svg"
                 alt="">
          </div>
          <div class="setting">
            <span class="setting-title"
                  style="width: 180px;">Private Key Password</span>
            <input type="password"
                   maxLength="24"
                   class="setting-text"
                   :disabled="server8021x === 'off'"
                   v-model="tls_private_password">
          </div>
        </div>
        <button class="btn btn-primary"
                style="margin-left: 30px"
                @click="set8021x">APPLY
        </button>
        <p class="error"
           style="margin-left: 30px;"
           v-if="server8021error">{{ server8021Alert }}</p>
      </div>
    </div>
  </div>
</template>

<script>
import radioComponent from '@/components/radio.vue'

export default {
  name: 'autoSwitch',
  components: {
    radioComponent
  },
  data () {
    return {
      https: 'in',
      httpsServer: 'off',
      httpsFileName: '',
      httpPrivatePwd: '',
      notHttpsFile: false,
      httpsErrorAlert: '',
      server8021x: 'off',
      security801: 'eap_mschap',
      mschap_username: '',
      mschap_password: '',
      tls_username: '',
      tls_ca_certificate: '',
      tls_client_certificate: '',
      tls_private_key: '',
      tls_private_password: '',
      server8021error: false,
      server8021Alert: ''
    }
  },
  created () {
    this.getHTTPS()
    this.get801x()
  },
  methods: {
    getHTTPS () {
      this.$http
        .get(
          '/device/json?path=/secure/https_setting.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.https_setting) {
            this.httpsServer = msg.data.https_setting.mode
            this.https = msg.data.https_setting.method
          }
        })
    },
    get801x () {
      this.$http
        .get(
          '/device/json?path=/secure/ieee802_1x_setting.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.ieee802_1x_setting) {
            this.server8021x = msg.data.ieee802_1x_setting.mode
            this.security801 = msg.data.ieee802_1x_setting.default_authentication || 'eap_mschap'
          }
        })
    },
    setHTTPS () {
      const formData = new FormData()
      formData.append('mode', this.httpsServer)
      if (this.httpsServer === 'on') {
        formData.append('method', this.https)
        if (this.https === 'out') {
          if (!this.httpsFileName) {
            this.httpsErrorAlert = 'Please select file'
            this.notHttpsFile = true
            return
          }
          if (!(this.httpsFileName.endsWith('.pem') || this.httpsFileName.endsWith('.key'))) {
            this.httpsErrorAlert = 'File format error'
            this.notHttpsFile = true
            return
          }
          formData.append('certificate', this.$refs.https.files[0])
          formData.append('password', this.httpPrivatePwd)
        }
      }
      this.notHttpsFile = false
      const xhr = new XMLHttpRequest()
      xhr.open('POST', '/security/https')
      xhr.onload = oevent => {
        if (xhr.status === 200) {
          this.httpsFileName = ''
          this.httpPrivatePwd = ''
          console.log('upload success')
        }
      }
      xhr.send(formData)
    },
    browseFile (ref, isBrowse) {
      if (!isBrowse) return
      this.$refs[ref].click()
    },
    httpsFileChange (event) {
      this.httpsFileName = event.target.files[0]?.name || ''
    },
    serve8021FileChange (event, attr) {
      this[attr] = event.target.files[0]?.name || ''
    },
    set8021x () {
      const formData = new FormData()
      formData.append('mode', this.server8021x)
      if (this.server8021x === 'on') {
        formData.append('default_authentication', this.security801)
        if (this.security801 === 'eap_mschap') {
          if (!this.mschap_username) {
            this.server8021Alert = 'Please enter username'
            this.server8021error = true
            return
          }
          if (!this.mschap_password) {
            this.server8021Alert = 'Please enter password'
            this.server8021error = true
            return
          }
          if (!this.isValidName(this.mschap_username) || !this.isValidName(this.mschap_password)) return
          formData.append('mschap_username', this.mschap_username)
          formData.append('mschap_password', this.mschap_password)
        } else {
          if (!this.tls_username) {
            this.server8021Alert = 'Please enter username'
            this.server8021error = true
            return
          }
          if (!this.tls_ca_certificate) {
            this.server8021Alert = 'Please select CA certifivate file'
            this.server8021error = true
            return
          }
          if (!this.tls_client_certificate) {
            this.server8021Alert = 'Please select client certifivate file'
            this.server8021error = true
            return
          }
          if (!this.tls_private_key) {
            this.server8021Alert = 'Please select private key file'
            this.server8021error = true
            return
          }
          if (!this.tls_private_password) {
            this.server8021Alert = 'Please enter password'
            this.server8021error = true
            return
          }
          if (!this.isValidName(this.tls_username) || !this.isValidName(this.tls_private_password)) return
          formData.append('tls_username', this.tls_username)
          formData.append('tls_ca_certificate', this.$refs.tls_ca_certificate.files[0])
          formData.append('tls_client_certificate', this.$refs.tls_client_certificate.files[0])
          formData.append('tls_private_key', this.$refs.tls_private_key.files[0])
          formData.append('tls_private_password', this.tls_private_password)
        }
      }
      this.server8021error = false
      const xhr = new XMLHttpRequest()
      xhr.open('POST', '/security/802_1x')
      xhr.onload = oevent => {
        if (xhr.status === 200) {
          this.httpsFileName = ''
          this.httpPrivatePwd = ''
          console.log('upload success')
        }
      }
      xhr.send(formData)
    },
    isValidName (name) {
      return /^[a-zA-Z0-9][_\-a-zA-Z0-9]{0,23}$/.test(name)
    }
  }
}
</script>
<style lang="less" scoped>
.setting-title {
  width: 220px;
}

.upload-icon {
  cursor: pointer;
  width: 26px;
}

.error {
  color: #d50000;
  font-size: 12px;
  font-family: "open sans bold";
}
</style>
