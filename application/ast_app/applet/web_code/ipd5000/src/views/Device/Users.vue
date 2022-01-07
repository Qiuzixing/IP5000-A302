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
        <div class="setting">
          <span class="setting-title">New Password</span>
          <input type="password"
                 v-model="newPwd"
                 class="setting-text">
          <button class="btn btn-plain-primary"
                  style="margin-left: 25px;"
                  @click="setPassword">CHANGE</button>
        </div>
        <div class="setting">
          <span class="setting-title">Confirm Password</span>
          <input type="password"
                 v-model="confirmPwd"
                 class="setting-text">
        </div>
      </div>
      <div class="setting-model">
        <div class="setting">
          <span class="setting-model-title">Inactivity auto-logout time</span>
          <el-input-number v-model="logoutTime"
                           controls-position="right"
                           :max="30"
                           :min="0"></el-input-number>
          <button class="btn btn-plain-primary"
                  style="margin-left: 25px;"
                  @click="setLogout">APPLY</button>
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
      confirmPwd: ''
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#SECUR? ')
    this.$socket.sendMsg('#LOGOUT-TIMEOUT? ')
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@LOGOUT-TIMEOUT /i) !== -1) {
        this.handleLogout(msg)
        return
      }
      if (msg.search(/@PASS /i) !== -1) {
        this.handlePwd(msg)
        return
      }
      if (msg.search(/@SECUR /i) !== -1) {
        this.showPwdStatus = msg.trim().split(' ')[1]
      }
    },
    switchPwdStatus (val) {
      if (val === '1') {
        this.verifyShowPwdDialog = true
      } else {
        this.setSecurity('0')
      }
    },
    handleLogout (msg) {
      this.logoutTime = parseInt(msg.split(' ')[1])
    },
    setLogout () {
      this.$socket.sendMsg('#LOGOUT-TIMEOUT ' + this.logoutTime)
    },
    setPassword () {
      if (this.confirmPwd.length === 0 || this.confirmPwd !== this.newPwd) return
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
    }
  }
}
</script>
