<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Active Syslog</span>
        <v-switch v-model="syslog"
                  active-value="1"
                  inactive-value="2"
                  @change="setActiveLog"></v-switch>
      </div>
      <div class="setting">
        <span class="setting-title">Log Reset Policy</span>
        <multiselect v-model="logResetPolicy"
                     :options="logResetPolicyParam"
                     @input="setActiveLog"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Log</span>
        <button class="btn btn-plain-primary"
                @click="viewLog">VIEW</button>
        <button class="btn btn-plain-primary"
                style="margin-left: 25px"
                @click="exportLog">EXPORT</button>
      </div>
    </div>
    <div class="setting-model">
      <h3 class="setting-model-title">Gateway Messages Counter</h3>
      <div class="setting">
        <span class="setting-title"></span>
        <span class="setting-title">Send</span>
        <span class="setting-title">Receive </span>
      </div>
      <div class="setting">
        <span class="setting-title">RS-232</span>
        <span style="width: 200px;">{{rs232.send}}</span>
        <span style="width: 200px;">{{rs232.recv}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">CEC</span>
        <span style="width: 200px;">{{cec.send}}</span>
        <span style="width: 200px;">{{cec.recv}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">IR</span>
        <span style="width: 200px;">{{ir.send}}</span>
        <span style="width: 200px;">{{ir.recv}}</span>
      </div>
    </div>
    <iframe v-if="isExport"
            src="/log/log"
            frameborder="0"
            width="0"
            height="0"></iframe>
    <el-dialog title="Log"
               :visible.sync="showLogDialog"
               width="800px">
      <p>
        {{logMsg}}
      </p>
    </el-dialog>
  </div>
</template>

<script>

export default {
  name: 'status',
  data () {
    return {
      syslog: '1',
      logResetPolicy: '3',
      logResetPolicyParam: [{
        value: '2',
        label: 'Daily'
      }, {
        value: '3',
        label: 'Weekly'
      }],
      rs232: {
        recv: 0,
        send: 0
      },
      cec: {
        recv: 0,
        send: 0
      },
      ir: {
        recv: 0,
        send: 0
      },
      isExport: false,
      showLogDialog: false,
      logMsg: ''
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#LOG-ACTION? ')
    this.$socket.sendMsg('#GTW-MSG-NUM? 1,' + this.getDay())
    this.$socket.sendMsg('#GTW-MSG-NUM? 2')
    this.$socket.sendMsg('#GTW-MSG-NUM? 3')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@LOG-ACTION /i) !== -1) {
        this.handleLogAction(msg)
        return
      }
      if (msg.search(/@GTW-MSG-NUM /i) !== -1) {
        this.handleSendCounter(msg)
      }
    },
    handleLogAction (msg) {
      const data = msg.split(' ')[1].split(',')
      this.syslog = data[0]
      this.logResetPolicy = data[1]
    },
    setActiveLog () {
      this.$socket.sendMsg(`#LOG-ACTION ${this.syslog},${this.logResetPolicy}`)
    },
    handleSendCounter (msg) {
      const data = msg.split(' ')[1].split(',')
      if (data === '1') {
        // cec
        this.cec.recv = data[2]
        this.cec.send = data[3]
      } else if (data === '2') {
        // IR
        this.ir.recv = data[2]
        this.ir.send = data[3]
      } else if (data === '3') {
        // rs232
        this.rs232.recv = data[2]
        this.rs232.send = data[3]
      }
    },
    getDay () {
      const date = new Date()
      const year = date.getFullYear()
      const month = date.getMonth() < 8 ? '0' + (date.getMonth() + 1) : date.getMonth() + 1
      const day = date.getDate() < 9 ? '0' + date.getDate() : date.getDate()
      return day + '-' + month + '-' + year
    },
    exportLog () {
      this.isExport = false
      setTimeout(() => {
        this.isExport = true
      }, 500)
    },
    viewLog () {
      this.logMsg = ''
      this.showLogDialog = true
      this.$http
        .get(
          '/log/log?t=' + Math.random()
        )
        .then(msg => {
          this.logMsg = msg.data
        })
    }
  }
}
</script>
<style lang="less" scoped>
.text-center {
  text-align: center;
}
.setting-title {
  width: 200px;
}
</style>
