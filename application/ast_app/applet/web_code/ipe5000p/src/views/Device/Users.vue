<template>
  <div class="main-setting">
    <div class="setting-content">
      <div class="setting-model">
        <h3 class="setting-model-title">Password</h3>
        <div class="setting">
          <span class="setting-title">Security Status</span>
          <v-switch v-model="showPwdStatus"
                    active-value="1"
                    inactive-value="0"
                    @change="switchPwdStatus"></v-switch>
        </div>
        <div class="setting" style="position: relative;">
          <span class="setting-title">New Password</span>
          <input type="password"
                 v-model="newPwd"
                 maxLength="24"
                 class="setting-text">
          <button class="btn btn-plain-primary"
                  style="margin-left: 25px;"
                  @click="setPassword">CHANGE
          </button>
          <span v-if="pwdError" class="range-alert" style="top: 36px; white-space: nowrap;">Alphanumeric within 24 characters, spaces not allowed</span>
        </div>
        <div class="setting" style="position: relative;">
          <span class="setting-title">Confirm Password</span>
          <input type="password"
                 maxLength="24"
                 v-model="confirmPwd"
                 class="setting-text">
          <span v-if="diffPwdError" class="range-alert" style="top: 36px; white-space: nowrap;">New password and confirm password do not match</span>
        </div>
      </div>
      <div class="setting-model">
        <div class="setting">
          <span class="setting-model-title">Inactivity auto-logout time (min)</span>
          <el-input-number v-model="logoutTime"
                           controls-position="right"
                           :max="30"
                           :min="0"></el-input-number>
          <button class="btn btn-plain-primary"
                  style="margin-left: 25px;"
                  @click="setLogout">APPLY
          </button>
        </div>
      </div>
    </div>
    <el-dialog title="Security Status"
               :visible.sync="verifyShowPwdDialog"
               width="550px"
               :show-close="false"
               :close-on-click-modal="false">
      <p class="dialog-second-title">Would you like to enable security?</p>
      <p>This action will enable the authentication.</p>
      <p>Do you want to proceed?</p>
      <span slot="footer"
            class="dialog-footer">
        <button class="btn btn-primary"
                @click="verifyShowPwdDialog = false, setSecurity('1') ">PROCEED</button>
        <button class="btn btn-primary"
                @click="verifyShowPwdDialog = false,showPwdStatus = '0'">CANCEL</button>
      </span>
    </el-dialog>
    <el-dialog title="Security Status"
               :visible.sync="disabledSecurityDialog"
               width="550px"
               :show-close="false"
               :close-on-click-modal="false">
      <p class="dialog-second-title">Would you like to disable security?</p>
      <p>This action will disable the authentication.</p>
      <p>Do you want to proceed?</p>
      <p><span style="font-family: 'open sans semiblold';margin-right: 15px;">Current password</span>
        <input type="password"
               maxLength="24"
               v-model="securityPwd"
               class="setting-text"></p>
      <p style="margin: 5px 0;color: #D50000;"
         v-if="securityPwdError">Incorrect password</p>
      <span slot="footer"
            class="dialog-footer">
        <button class="btn btn-primary"
                @click="verifyPwd">PROCEED</button>
        <button class="btn btn-primary"
                @click="disabledSecurityDialog = false,showPwdStatus = '1'">CANCEL</button>
      </span>
    </el-dialog>
  </div>
</template>

<script>

export default {
  name: 'users',
  data () {
    return {
      logoutTime: 0,
      showPassword: false,
      showPwdStatus: '0',
      verifyShowPwdDialog: false,
      pwd: '',
      newPwd: '',
      confirmPwd: '',
      pwdError: false,
      diffPwdError: false,
      disabledSecurityDialog: false,
      securityPwd: '',
      securityPwdError: false,
      setSecurityFlag: false
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#SECUR? ')
    this.$socket.sendMsg('#LOGOUT-TIMEOUT? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@LOGOUT-TIMEOUT /i) !== -1) {
        this.handleLogout(msg)
        return
      }
      if (msg.search(/@PASS /i) !== -1) {
        this.handlePwd(msg)
        return
      }
      if (msg.search(/@LOGIN /i) !== -1) {
        this.handleLogin(msg)
        return
      }
      if (msg.search(/@SECUR /i) !== -1) {
        this.showPwdStatus = msg.trim().split(' ')[1]
      }
    },
    switchPwdStatus (val) {
      if (val === '1') {
        this.verifyShowPwdDialog = true
      } else if (val === '0') {
        this.securityPwd = ''
        this.securityPwdError = false
        this.disabledSecurityDialog = true
        // this.setSecurity('0')
      }
    },
    handleLogout (msg) {
      this.logoutTime = parseInt(msg.split(' ')[1])
    },
    setLogout () {
      this.$socket.sendMsg('#LOGOUT-TIMEOUT ' + this.logoutTime)
    },
    setPassword () {
      this.pwdError = !this.isPwd(this.newPwd)
      if (this.pwdError) return
      this.diffPwdError = !(this.newPwd === this.confirmPwd)
      if (this.diffPwdError) return
      this.$socket.sendMsg(`#PASS admin,${this.newPwd}`)
    },
    setSecurity (ctrl) {
      this.$socket.sendMsg('#SECUR ' + ctrl)
    },
    handlePwd (msg) {
      if (msg.toLowerCase().endsWith('ok')) {
        this.newPwd = ''
        this.confirmPwd = ''
        this.pwd = ''
      }
    },
    isPwd (name) {
      return /^[\x21-\x7e]{1,24}$/.test(name)
    },
    verifyPwd () {
      if (!this.isPwd(this.securityPwd)) {
        this.securityPwdError = true
        return
      }
      this.setSecurityFlag = true
      this.$socket.sendMsg(`#LOGIN admin,${this.securityPwd}`)
    },
    handleLogin (msg) {
      if (this.setSecurityFlag) {
        this.setSecurityFlag = false
        if (msg.split(',').pop().trim().toLowerCase() === 'ok') {
          this.setSecurity('0')
          this.disabledSecurityDialog = false
        } else {
          this.securityPwdError = true
        }
      }
    }
  }
}
</script>
