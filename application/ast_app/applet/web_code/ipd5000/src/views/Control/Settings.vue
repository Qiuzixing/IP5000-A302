<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">CEC Settings</h3>
      <div class="setting">
        <span class="setting-title">Gateway</span>
        <v-switch open-text="Enable"
                  v-model="cecGateWay"
                  close-text="Disable"
                  active-value="1"
                  inactive-value="0"
                  @input="setCECGateway"></v-switch>
      </div>
      <div v-if="$global.deviceType">
        <div class="setting">
          <span class="setting-title">Gateway HDMI Port</span>
          <multiselect :disabled="cecGateWay === '0'"
                       :options="[{value: '1', label: 'HDMI Input'}, {value: '2', label: 'HDMI Output'}]"
                       v-model="cecGateWayPort"
                       @input="setCECPort"></multiselect>
        </div>
        <div class="setting">
          <span class="setting-title">Command </span>
          <div style="position:relative;">
            <input type="text"
                   :disabled="cecGateWay === '0'"
                   class="setting-text"
                   v-model="cecCmd">
            <span v-if="hexError"
                  class="range-alert"
                  style="white-space: nowrap;">Please enter hexadecimal</span>
          </div>

          <button class="btn btn-plain-primary"
                  :disabled="cecGateWay === '0'"
                  style="margin-left: 25px"
                  @click="sendCECCmd">SEND</button>
        </div>
        <div class="radio-setting">
          <span class="setting-title">Responses </span>
          <div>
            <div class="res-title">
              <span>Command</span>
              <!-- <span>Result</span> -->
            </div>
            <div class="res-info">
              <div class="res-info-item"
                   v-for="(item, index) in cecResList"
                   :key="index">
                <span>{{item.cmd}}</span>
                <!-- <span>{{responseType[item.type]}}</span> -->
              </div>
            </div>
          </div>
        </div>
      </div>

    </div>
    <div class="setting-model">
      <h3 class="setting-model-title">RS-232 Settings</h3>
      <div class="setting">
        <span class="setting-title">Gateway</span>
        <v-switch v-model="rs232GW"
                  open-text="Enable"
                  close-text="Disable"
                  @input="setRs232GW"></v-switch>
      </div>
      <div class="setting">
        <span class="setting-title">Gateway Port</span>
        <el-input-number :disabled="!rs232GW"
                         v-model="rs232Port"
                         controls-position="right"
                         :max="65535"
                         :min="5000"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Baud Rate </span>
        <multiselect :options="baudRateParam"
                     v-model="baudRate"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Data Bits </span>
        <multiselect :options="dataBitsParam"
                     v-model="dataBits"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Parity </span>
        <multiselect :options="parityParam"
                     v-model="parity"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Stop Bits </span>
        <multiselect :options="stopBitsParam"
                     v-model="stopBits"></multiselect>
      </div>
      <!-- <div class="setting">
        <span class="setting-title">Connection</span>
        <button :disabled="!rs232GW"
                class="btn btn-plain-primary">CHECK</button>
      </div> -->
      <button class="btn btn-primary"
              @click="saveBaudRate">SAVE</button>

    </div>
    <div class="setting-model"
         v-if="this.$global.deviceType">
      <h3 class="setting-model-title">IR Settings</h3>
      <div class="setting">
        <span class="setting-title">Gateway</span>
        <v-switch v-model="irGW"
                  @input="setIRGateway"
                  open-text="Enable"
                  close-text="Disable"
                  active-value="1"
                  inactive-value="0"></v-switch>
      </div>
      <div class="radio-setting">
        <span class="setting-title">IR Direction IN/OUT</span>
        <div>
          <radio-component v-model="irDirection"
                           :disabled="irGW=='0'"
                           label="in"
                           @change="setIrDirection">IN</radio-component>
          <radio-component v-model="irDirection"
                           :disabled="irGW=='0'"
                           label="out"
                           @change="setIrDirection">OUT</radio-component>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import radioComponent from '@/components/radio.vue'

export default {
  name: 'users',
  components: {
    radioComponent
  },
  data () {
    return {
      cecCmd: '',
      castMode: '1',
      rs232InOut: '1',
      cecGateWay: '0',
      cecGateWayPort: '1',
      baudRate: '115200',
      baudRateParam: [
        {
          value: '115200',
          label: '115200'
        },
        {
          value: '57600',
          label: '57600'
        },
        {
          value: '38400',
          label: '38400'
        },
        {
          value: '19200',
          label: '19200'
        },
        {
          value: '9600',
          label: '9600'
        }
      ],
      dataBits: '8',
      dataBitsParam: [
        {
          value: '8',
          label: '8'
        },
        {
          value: '7',
          label: '7'
        },
        {
          value: '6',
          label: '6'
        },
        {
          value: '5',
          label: '5'
        }
      ],
      parity: 'none',
      parityParam: [
        {
          value: 'none',
          label: 'None'
        },
        {
          value: 'odd',
          label: 'Odd'
        },
        {
          value: 'even',
          label: 'Even'
        }
      ],
      stopBits: '2',
      stopBitsParam: [
        {
          value: '2',
          label: '2'
        },
        {
          value: '1.5',
          label: '1.5'
        },
        {
          value: '1',
          label: '1'
        }
      ],
      cecResList: [],
      responseType: {
        0: 'Sent',
        1: 'Gateway disabled',
        2: 'Inactive CEC-Master',
        3: 'Busy',
        4: 'Illegal Message Parameter',
        5: 'Illegal CEC Address Parameter',
        6: 'Illegal CEC Command',
        7: 'Timeout',
        8: 'Error'
      },
      rs232Port: 5001,
      irDirection: 'in',
      rs232GW: false,
      irGW: '0',
      hexError: false
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#CEC-GW-PORT-ACTIVE? ')
    this.$socket.sendMsg('#UART? 1')
    this.$socket.sendMsg('#PORT-DIRECTION? both.ir.1.ir')
    this.$socket.sendMsg('#COM-ROUTE? *')
    this.$socket.sendMsg('#KDS-IR-GW? ')
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@CEC-GW-PORT-ACTIVE /i) !== -1) {
        this.handleCECPort(msg)
        return
      }
      if (msg.search(/@CEC-SND /i) !== -1) {
        this.handleCECResponse(msg)
        return
      }
      if (msg.search(/@UART /i) !== -1) {
        this.handleRS232Param(msg)
        return
      }
      if (msg.search(/@PORT-DIRECTION /i) !== -1) {
        this.handleIRDirection(msg)
        return
      }
      if (msg.search(/@KDS-IR-GW /i) !== -1) {
        this.handleIRGateway(msg)
        return
      }
      if (msg.search(/@CEC-NTFY /i) !== -1) {
        const cmd = msg.split(',').pop()
        this.cecResList.unshift({ cmd, type: cmd })
        return
      }
      if (msg.search(/@COM-ROUTE /i) !== -1) {
        this.handleRs232Gateway(msg)
      }
    },
    handleCECPort (msg) {
      const port = msg.split(' ')[1]
      if (port === '0') {
        this.cecGateWay = port
      } else {
        this.cecGateWay = '1'
        this.cecGateWayPort = port
      }
    },
    setCECPort () {
      this.$socket.sendMsg('#CEC-GW-PORT-ACTIVE ' + this.cecGateWayPort)
    },
    sendCECCmd () {
      const cmd = this.cecCmd.replace(/\s/g, '')
      if (this.checkHex(cmd)) {
        this.hexError = false
        this.$socket.sendMsg(`#CEC-SND 1,1,1,${cmd.length / 2},${cmd}`)
      } else {
        this.hexError = true
      }
    },
    handleCECResponse (msg) {
      const data = msg.split(',')
      this.cecResList.unshift({ cmd: data[4], type: data[5] })
    },
    checkHex (cmd) {
      return cmd.match(/^([0-9a-fA-F]{2}([0-9a-fA-F]{2}){0,14})$/)
    },
    handleRS232Param (msg) {
      const data = msg.split(',')
      this.baudRate = data[1]
      this.dataBits = data[2]
      this.parity = data[3]
      this.stopBits = data[4]
    },
    handleIRDirection (msg) {
      if (msg.search(/ir/i) !== -1) {
        this.irDirection = msg.split(',')[1].toLowerCase()
      }
    },
    setCECGateway (port) {
      if (port === '0') {
        this.$socket.sendMsg('#CEC-GW-PORT-ACTIVE 0')
      } else {
        this.$socket.sendMsg('#CEC-GW-PORT-ACTIVE ' + this.cecGateWayPort)
      }
    },
    saveBaudRate () {
      if (this.rs232GW) {
        this.$socket.sendMsg(`#COM-ROUTE-ADD 1,1,${this.rs232Port},1,1`)
      }
      this.$socket.sendMsg(`#UART 1,${this.baudRate},${this.dataBits},${this.parity},${this.stopBits}`)
    },
    handleRs232Gateway (msg) {
      const data = msg.split(' ')
      if (data[1].length > 0) {
        const arr = data[1].split(',')
        this.rs232Port = parseInt(arr[2])
        this.rs232GW = true
      } else {
        this.rs232GW = false
      }
    },
    setRs232GW (isOpen) {
      if (isOpen) {
        this.$socket.sendMsg(`#COM-ROUTE-ADD 1,1,${this.rs232Port},1,1`)
      } else {
        this.$socket.sendMsg('#COM-ROUTE-REMOVE 1')
      }
    },
    handleIRGateway (msg) {
      this.irGW = msg.split(' ').pop()
    },
    setIrDirection () {
      this.$socket.sendMsg(`#PORT-DIRECTION both.ir.1.ir,${this.irDirection}`)
    },
    setIRGateway (ctrl) {
      this.$socket.sendMsg(`#KDS-IR-GW ${ctrl}`)
    }
  }
}
</script>
<style lang="less" scoped>
.setting-title {
  width: 200px;
}
.res-title {
  display: flex;
  span {
    padding: 5px;
    font-family: "open sans semiblold";
  }
  span:first-child {
    width: 220px;
    background: #f3f3f3;
    margin-right: 24px;
  }
  span:last-child {
    width: 220px;
    background: #f3f3f3;
  }
}
.res-info {
  max-height: 200px;
  overflow-y: auto;
  .res-info-item {
    display: flex;
    span {
      width: 220px;
      word-wrap: break-word;
      padding: 5px;
    }
    span:first-child {
      margin-right: 24px;
    }
  }
}
</style>
