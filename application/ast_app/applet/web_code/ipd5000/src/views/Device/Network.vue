<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting"
           style="margin-bottom: 0">
        <h3 class="setting-model-title">IP Settings</h3>
        <h3 class="setting-model-title">Eth 0</h3>
        <h3 class="setting-model-title"
            v-if="daisyChain === '0'">Eth 1</h3>
      </div>
      <div class="setting">
        <span class="setting-title">DHCP</span>
        <div class="setting-title">
          <v-switch v-model="ipMode0"
                    style="width: 120px"
                    active-value="1"
                    inactive-value="0"></v-switch>
        </div>
        <div class="setting-title"
             v-if="daisyChain === '0'">
          <v-switch v-model="ipMode1"
                    style="width: 120px"
                    active-value="1"
                    inactive-value="0"></v-switch>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">IP Address</span>
        <div class="setting-title">
          <input type="text"
                 :disabled="ipMode0 === '1'"
                 class="setting-text"
                 v-model="ipInfo0[0]">
        </div>
        <div class="setting-title"
             v-if="daisyChain === '0'">
          <input type="text"
                 :disabled="ipMode1 === '1'"
                 class="setting-text"
                 v-model="ipInfo1[0]">
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Mask Address</span>
        <div class="setting-title">
          <input type="text"
                 :disabled="ipMode0 === '1'"
                 class="setting-text"
                 v-model="ipInfo0[1]">
        </div>
        <div class="setting-title"
             v-if="daisyChain === '0'">
          <input type="text"
                 :disabled="ipMode1 === '1'"
                 class="setting-text"
                 v-model="ipInfo1[1]">
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Gateway Address</span>
        <div class="setting-title">
          <input type="text"
                 :disabled="ipMode0 === '1'"
                 class="setting-text"
                 v-model="ipInfo0[2]">
        </div>
        <div class="setting-title"
             v-if="daisyChain === '0'">
          <input type="text"
                 :disabled="ipMode1 === '1'"
                 class="setting-text"
                 v-model="ipInfo1[2]">
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Primary DNS</span>
        <div class="setting-title">
          <input type="text"
                 :disabled="ipMode0 === '1'"
                 class="setting-text"
                 v-model="ipInfo0[3]">
        </div>
        <div class="setting-title"
             v-if="daisyChain === '0'">
          <input type="text"
                 :disabled="ipMode1 === '1'"
                 class="setting-text"
                 v-model="ipInfo1[3]">
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Secondary DNS</span>
        <div class="setting-title">
          <input type="text"
                 :disabled="ipMode0 === '1'"
                 class="setting-text"
                 v-model="ipInfo0[4]">
        </div>
        <div class="setting-title"
             v-if="daisyChain === '0'">
          <input type="text"
                 :disabled="ipMode1 === '1'"
                 class="setting-text"
                 v-model="ipInfo1[4]">
        </div>
      </div>
    </div>

    <div class="setting">
      <div class="radio-setting"
           style="margin-bottom: 0">
        <span class="setting-model-title">Daisy Chain</span>
        <v-switch v-model="daisyChain"
                  @change="daisyChainChange"
                  active-value="1"
                  inactive-value="0"></v-switch>
      </div>
    </div>
    <div class="setting-model">
      <div class="setting"
           style="margin-bottom: 0">
        <h3 class="setting-model-title">Port Configuration</h3>
        <h3 class="setting-model-title"
            style="width: 80px;">Eth 0</h3>
        <h3 class="setting-model-title"
            v-if="daisyChain === '0'"
            style="width: 80px;">Eth 1</h3>
      </div>
      <div class="setting">
        <span class="setting-title">P3K Port</span>
        <div class="setting-title"
             style="width: 80px;">
          <radio-component v-model="configPort0"
                           label="0"
                           :isEmpty="true"
                           style="margin-bottom: 0;"></radio-component>
        </div>
        <div class="setting-title"
             v-if="daisyChain === '0'"
             style="width: 80px;">
          <radio-component v-model="configPort0"
                           label="1"
                           :isEmpty="true"
                           style="margin-bottom: 0;"></radio-component>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">RS232 Gateway Port</span>
        <div class="setting-title"
             style="width: 80px;">
          <radio-component v-model="configPort1"
                           label="0"
                           :isEmpty="true"
                           style="margin-bottom: 0;"></radio-component>
        </div>
        <div class="setting-title"
             style="width: 80px;">
          <radio-component v-model="configPort1"
                           v-if="daisyChain === '0'"
                           label="1"
                           :isEmpty="true"
                           style="margin-bottom: 0;"></radio-component>
        </div>
      </div>
    </div>
    <div class="setting">
      <div class="radio-setting"
           style="margin-bottom: 0">
        <span class="setting-model-title">IP Casting Mode</span>
        <div>
          <radio-component v-model="castMode"
                           label="1">Unicast</radio-component>
          <radio-component v-model="castMode"
                           label="2">Multicast</radio-component>
        </div>
      </div>
    </div>
    <!-- <div class="setting">
      <span class="setting-title">IP Multicast Address</span>
      <div class="setting-title">
        <input type="text"
               v-model="multicastAddress"
               class="setting-text"
               :disabled="castMode == '1'">
      </div>
    </div> -->
    <div class="setting">
      <span class="setting-title">TTL</span>
      <div class="setting-title">
        <el-input-number v-model="ttl"
                         controls-position="right"
                         :max="255"
                         :min="1"
                         :disabled="castMode == '1'"></el-input-number>
      </div>
    </div>
    <div class="setting-model">
      <h3 class="setting-model-title">TCP/UDP Management</h3>
      <div class="setting">
        <span class="setting-title">TCP Port</span>
        <el-input-number v-model="tcp"
                         controls-position="right"
                         :max="5099"
                         :min="5000"></el-input-number>
      </div>
      <!--      <div class="setting">-->
      <!--        <span class="setting-title">Service TCP Ports</span>-->
      <!--        <el-input-number v-model="serverTcpPort" controls-position="right" :max="5099" :min="5000"></el-input-number>-->
      <!--      </div>-->
      <div class="setting">
        <span class="setting-title">UDP Port</span>
        <el-input-number v-model="udp"
                         controls-position="right"
                         :min="50000"
                         :max="50999"></el-input-number>
      </div>
    </div>
    <footer>
      <button class="btn btn-primary"
              @click="save">SAVE</button>
    </footer>
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
      port1: '1',
      port2: '1',
      ipMode0: '0',
      ipMode1: '0',
      ipInfo0: [],
      ipInfo1: [],
      configPort0: '0',
      configPort1: '0',
      configPort2: '0',
      danteTag: '',
      tcp: '5000',
      serverTcpPort: 5000,
      udp: 50000,
      castMode: '1',
      multicastAddress: '0,0,0,0',
      ttl: 64,
      danteTag1: 1,
      danteTag2: 1,
      danteTag3: 1,
      daisyChain: '0'
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#KDS-DAISY-CHAIN? ')
    this.$socket.sendMsg('#NET-DHCP? 0')
    this.$socket.sendMsg('#NET-DHCP? 1')
    this.$socket.sendMsg('#NET-CONFIG? 0')
    this.$socket.sendMsg('#NET-CONFIG? 1')
    this.$socket.sendMsg('#KDS-GW-ETH? 0')
    this.$socket.sendMsg('#KDS-GW-ETH? 1')
    this.$socket.sendMsg('#KDS-METHOD? ')
    this.$socket.sendMsg('#KDS-MULTICAST? ')
    this.$socket.sendMsg('#ETH-PORT? TCP')
    this.$socket.sendMsg('#ETH-PORT? UDP')
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@NET-DHCP /i) !== -1) {
        this.handleIPMode(msg)
        return
      }
      if (msg.search(/@NET-CONFIG /i) !== -1) {
        this.handleIP(msg)
        return
      }
      if (msg.search(/@KDS-GW-ETH /i) !== -1) {
        this.handlePortConfig(msg)
        return
      }
      if (msg.search(/@KDS-METHOD /i) !== -1) {
        this.handleIpCastMode(msg)
        return
      }
      if (msg.search(/@KDS-MULTICAST /i) !== -1) {
        this.handleMulticast(msg)
        return
      }
      if (msg.search(/@ETH-PORT /i) !== -1) {
        this.handleETHPort(msg)
        return
      }
      if (msg.search(/@KDS-DAISY-CHAIN /i) !== -1) {
        this.daisyChain = msg.split(' ').pop()
      }
    },
    handleIPMode (msg) {
      const data = msg.split(' ')[1].split(',')
      const isDHCP = data[1] === '1' ? data[1] : '0'
      this['ipMode' + data[0]] = isDHCP
    },
    handleIP (msg) {
      const data = msg.split(' ')[1]
      const ipIndex = 'ipInfo' + data[0]
      const ipArr = []
      const reg = /(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/g
      const ipParse = data.match(reg)
      ipArr.push(ipParse[0] || '')
      ipArr.push(ipParse[1] || '')
      ipArr.push(ipParse[2] || '')
      ipArr.push(ipParse[3] || '')
      ipArr.push(ipParse[4] || '')
      this[ipIndex] = ipArr
    },

    handlePortConfig (msg) {
      const data = msg.split(' ')[1].split(',')
      this['configPort' + data[0]] = data[1]
    },
    handleETHPort (msg) {
      const data = msg.toLowerCase().split(' ')[1].split(',')
      this[data[0]] = parseInt(data[1])
    },
    handleIpCastMode (msg) {
      this.castMode = msg.split(' ')[1]
    },
    handleMulticast (msg) {
      const data = msg.split(' ')[1].split(',')
      this.multicastAddress = parseInt(data[0])
      this.ttl = parseInt(data[1])
    },
    save () {
      this.$socket.sendMsg('#KDS-DAISY-CHAIN ' + this.daisyChain)
      this.setPortConfig()
      this.setIpCastingMode()
      this.setTcpUDP()
      this.setIp()
    },
    setPortConfig () {
      this.$socket.sendMsg('#KDS-GW-ETH 0,' + this.configPort0)
      this.$socket.sendMsg('#KDS-GW-ETH 1,' + this.configPort1)
    },
    setIpCastingMode () {
      this.$socket.sendMsg('#KDS-METHOD ' + this.castMode)
      if (this.castMode === '2') {
        this.$socket.sendMsg(`#KDS-MULTICAST ${this.multicastAddress},${this.ttl}`)
      }
    },
    setTcpUDP () {
      this.$socket.sendMsg('#ETH-PORT TCP,' + this.tcp)
      this.$socket.sendMsg('#ETH-PORT UDP,' + this.udp)
    },
    setIp () {
      if (this.ipMode0 !== '1') {
        this.$socket.sendMsg('#NET-DHCP 0,0')
        this.$socket.sendMsg('#NET-CONFIG 0,' + this.ipInfo0.join(','))
      } else {
        this.$socket.sendMsg('#NET-DHCP 0,1')
      }
      if (this.daisyChain === '0') {
        if (this.ipMode1 !== '1') {
          this.$socket.sendMsg('#NET-DHCP 1,0')
          this.$socket.sendMsg('#NET-CONFIG 1,' + this.ipInfo1.join(','))
        } else {
          this.$socket.sendMsg('#NET-DHCP 1,1')
        }
      }
    },
    daisyChainChange (val) {
      if (val === '1') {
        this.configPort0 = '0'
        this.configPort1 = '0'
      }
    }
  }
}
</script>
<style lang="less" scoped>
.main-setting {
  display: flex;
  flex-direction: column;
}

.setting-model {
  flex: 1;
}

.main-setting footer {
  flex-shrink: 0;
  margin-top: 15px;
  margin-bottom: 15px;
}

.setting-title {
  width: 200px;
}

.setting-model-title {
  width: 200px;
}
</style>
