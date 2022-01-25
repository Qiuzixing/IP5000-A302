<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">Interface Settings</h3>
      <table class="table">
        <thead>
        <tr>
          <th>Service Name</th>
          <th>Port</th>
          <th>802.1Q</th>
          <th>VLAN Tag</th>
          <th>DHCP</th>
          <th>IP Address</th>
          <th>Mask Address</th>
          <th>Gateway Address</th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <th>Stream</th>
          <th>Media</th>
          <th>N/A</th>
          <th>N/A</th>
          <th>
            <v-switch v-model="ipMode0"
                      style="width: 120px"
                      active-value="1"
                      inactive-value="0"></v-switch>
          </th>
          <th>
            <div style="position:relative;">
              <input type="text"
                     :disabled="ipMode0 === '1'"
                     class="setting-text"
                     style="width: 140px"
                     v-model="ipInfo0[0]">
              <span class="range-alert"
                    v-if="ipInfo0Error === 2"
                    style="top:36px;white-space: nowrap;">Please enter a valid address</span>
            </div>
          </th>
          <th>
            <div style="position:relative;">
              <input type="text"
                     :disabled="ipMode0 === '1'"
                     class="setting-text"
                     style="width: 140px"
                     v-model="ipInfo0[1]">
              <span class="range-alert"
                    v-if="ipInfo0Error === 1"
                    style="top:36px;white-space: nowrap;">Please enter a valid address</span>
            </div>
          </th>
          <th>
            <div style="position:relative;">
              <input type="text"
                     :disabled="ipMode0 === '1'"
                     class="setting-text"
                     style="width: 140px"
                     v-model="ipInfo0[2]">
              <span class="range-alert"
                    v-if="ipInfo0Error === 3"
                    style="top:36px;white-space: nowrap;">Please enter a valid address</span>
            </div>
          </th>
        </tr>
        <tr>
          <th>P3K & Gateway</th>
          <th>
            <el-select style="width: 100px" v-model="configPort0">
              <el-option value="0" label="Media"></el-option>
              <el-option value="1" label="Service"></el-option>
            </el-select>
          </th>
          <th>
            <v-switch v-model="p3k802Q"
                      style="width: 120px"></v-switch>
          </th>
          <th>
            <el-input-number v-model="danteTag1"
                             style="width: 80px"
                             :disabled="!p3k802Q"
                             controls-position="right"
                             :max="4093"
                             :min="2"></el-input-number>
          </th>
          <th>
            <v-switch v-model="ipMode1"
                      style="width: 120px"
                      active-value="1"
                      :disabled="configPort0 === '0' && !p3k802Q"
                      inactive-value="0"></v-switch>
          </th>
          <th>
            <div style="position: relative;">
              <input type="text"
                     style="width: 140px"
                     :disabled="ipMode1 === '1' || (configPort0 === '0' && !p3k802Q)"
                     class="setting-text"
                     v-model="ipInfo1[0]">
              <span class="range-alert"
                    v-if="ipInfo1Error ===2 && !(this.configPort0 === '0' && !this.p3k802Q)"
                    style="top:36px;white-space: nowrap;">Please enter a valid address</span>
            </div>
          </th>
          <th>
            <div style="position: relative;">
              <input type="text"
                     :disabled="ipMode1 === '1' || (configPort0 === '0' && !p3k802Q)"
                     class="setting-text"
                     style="width: 140px"
                     v-model="ipInfo1[1]">
              <span class="range-alert"
                    v-if="ipInfo1Error ===1 && !(this.configPort0 === '0' && !this.p3k802Q)"
                    style="top:36px;white-space: nowrap;">Please enter a valid address</span>
            </div>
          </th>
          <th>
            <div style="position: relative;">
            <input type="text"
                   :disabled="ipMode1 === '1' || (configPort0 === '0' && !p3k802Q)"
                   class="setting-text"
                   style="width: 140px"
                   v-model="ipInfo1[2]">
              <span class="range-alert"
                    v-if="ipInfo1Error === 3 && !(this.configPort0 === '0' && !this.p3k802Q)"
                    style="top:36px;white-space: nowrap;">Please enter a valid address</span>
            </div>
          </th>
        </tr>
        <tr v-if="this.$global.deviceType === 1">
          <th>Dante</th>
          <th>
            <el-select style="width: 100px" v-model="configPort1">
              <el-option value="0" label="Media"></el-option>
              <el-option value="1" label="Service"></el-option>
            </el-select>
          </th>
          <th>
            <v-switch
              v-model="dante802Q"
              style="width: 120px"></v-switch>
          </th>
          <th>
            <el-input-number v-model="danteTag2"
                             :disabled="!dante802Q"
                             style="width: 80px"
                             controls-position="right"
                             :max="4093"
                             :min="2"></el-input-number>
          </th>
        </tr>
        </tbody>
      </table>
    </div>
    <div class="setting">
      <div class="radio-setting"
           style="margin-bottom: 0">
        <span class="setting-model-title">IP Casting Mode</span>
        <div>
          <radio-component v-model="castMode"
                           label="1">Unicast
          </radio-component>
          <radio-component v-model="castMode"
                           label="2">Multicast
          </radio-component>
        </div>
      </div>
    </div>
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
        <div style="position: relative">
          <el-input-number v-model="tcp"
                           controls-position="right"
                           :max="5099"
                           :min="5000"></el-input-number>
          <span class="range-alert"
                v-if="portUse"
                style="top:36px;white-space: nowrap;">Port already in use</span>
        </div>
      </div>
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
              @click="save">SAVE
      </button>
    </footer>
    <el-dialog title="RESTART"
               :visible.sync="dialogVisibleReset"
               width="500px">
      <p class="dialog-second-title">Restart for new settings to take effect. </p>
      <p class="dialog-second-title">Do you want to restart now?</p>
      <span slot="footer"
            class="dialog-footer">
        <button class="btn btn-primary"
                @click="dialogVisibleReset = false, restart()">RESTART</button>
        <button class="btn btn-primary"
                @click="dialogVisibleReset = false">LATER</button>
      </span>
    </el-dialog>
  </div>
</template>

<script>
import radioComponent from '@/components/radio.vue'
import { debounce } from 'lodash'
import checkLanSettings from '../../util/util'

export default {
  name: 'autoSwitch',
  components: {
    radioComponent
  },
  data () {
    return {
      dialogVisibleReset: false,
      port1: '1',
      port2: '1',
      ipMode0: '0',
      ipMode1: '0',
      ipInfo0: [],
      ipInfo1: [],
      configPort0: '0',
      configPort1: '0',
      danteTag: '',
      tcp: 5000,
      serverTcpPort: '5001',
      udp: 50000,
      castMode: '1',
      multicastAddress: '0,0,0,0',
      ttl: 64,
      danteTag1: 2,
      danteTag2: 2,
      p3k802Q: false,
      dante802Q: false,
      ipInfo0Error: 0,
      ipInfo1Error: 0,
      setIpFlag: false,
      rs232Port: 0,
      portUse: false
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#NET-DHCP? 0')
    this.$socket.sendMsg('#NET-DHCP? 1')
    this.$socket.sendMsg('#NET-CONFIG? 0')
    this.$socket.sendMsg('#NET-CONFIG? 1')
    this.$socket.sendMsg('#KDS-GW-ETH? 0')
    this.$socket.sendMsg('#KDS-VLAN-TAG? 0')
    this.$socket.sendMsg('#KDS-METHOD? ')
    this.$socket.sendMsg('#KDS-MULTICAST? ')
    this.$socket.sendMsg('#ETH-PORT? TCP')
    this.$socket.sendMsg('#ETH-PORT? UDP')
    this.$socket.sendMsg('#COM-ROUTE? *')
    if (this.$global.deviceType === 1) {
      this.$socket.sendMsg('#KDS-GW-ETH? 1')
      this.$socket.sendMsg('#KDS-VLAN-TAG? 1')
    }
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@NET-DHCP /i) !== -1) {
        this.handleIPMode(msg)
        return
      }
      if (msg.search(/@NET-CONFIG /i) !== -1) {
        this.handleIP(msg)
        return
      }
      if (msg.search(/@KDS-VLAN-TAG /i) !== -1) {
        this.handleDanteTag(msg)
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
      if (msg.search(/@COM-ROUTE /i) !== -1) {
        this.handleRs232Gateway(msg)
        return
      }
      if (msg.search(/@ETH-PORT /i) !== -1) {
        this.handleETHPort(msg)
      }
    },
    handleIPMode (msg) {
      const data = msg.split(' ')[1].split(',')
      const isDHCP = data[1] === '1' ? data[1] : '0'
      this['ipMode' + data[0]] = isDHCP
      if (this.setIpFlag) {
        this.setIpFlag = false
        this.dialogVisibleReset = true
      }
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
      this[ipIndex] = ipArr
    },
    handleDanteTag (msg) {
      const data = msg.split(' ')[1].split(',')
      if (data[0] === '0') {
        const tag = parseInt(data[1])
        this.p3k802Q = tag !== 1
        if (tag !== 1) {
          this.danteTag1 = tag
        }
      } else if (data[0] === '1') {
        const tag = parseInt(data[1])
        this.dante802Q = tag !== 1
        if (tag !== 1) {
          this.danteTag2 = tag
        }
      }
    },
    handlePortConfig (msg) {
      const data = msg.split(' ')[1].split(',')
      this['configPort' + data[0]] = data[1]
    },
    handleETHPort (msg) {
      if (msg.search(/err/gi) === -1) {
        const data = msg.toLowerCase().split(' ')[1].split(',')
        this[data[0]] = parseInt(data[1])
      }
    },
    handleIpCastMode (msg) {
      this.castMode = msg.split(' ')[1]
    },
    handleMulticast (msg) {
      const data = msg.split(' ')[1].split(',')
      // this.multicastAddress = data[0]
      this.ttl = parseInt(data[1])
    },
    save: debounce(function () {
      this.portUse = false
      if(!this.validIP()) return
      if (this.tcp == this.rs232Port) {
        this.portUse = true
        return
      }
      this.setPortConfig()
      this.setIpCastingMode()
      this.setTcpUDP()
      this.setIp()
    }, 2000, {
      leading: true,
      trailing: true
    }),
    setPortConfig () {
      this.$socket.sendMsg('#KDS-GW-ETH 0,' + this.configPort0)
      this.$socket.sendMsg('#KDS-VLAN-TAG 0,' + (this.p3k802Q ? this.danteTag1 : 1))
      if (this.$global.deviceType === 1) {
        this.$socket.sendMsg('#KDS-GW-ETH 1,' + this.configPort1)
        this.$socket.sendMsg('#KDS-VLAN-TAG 1,' + (this.dante802Q ? this.danteTag2 : 1))
      }
    },
    setIpCastingMode () {
      this.$socket.sendMsg('#KDS-METHOD ' + this.castMode)
      if (this.castMode === '2') {
        this.$socket.sendMsg(`#KDS-MULTICAST 225.225.0.10,${this.ttl}`)
      }
    },
    setTcpUDP () {
      this.$socket.sendMsg('#ETH-PORT TCP,' + this.tcp)
      this.$socket.sendMsg('#ETH-PORT UDP,' + this.udp)
    },
    setIp () {
      this.setIpFlag = true
      if (this.ipMode0 !== '1') {
        this.$socket.sendMsg('#NET-DHCP 0,0')
        this.$socket.sendMsg('#NET-CONFIG 0,' + this.ipInfo0.join(','))
      } else {
        this.$socket.sendMsg('#NET-DHCP 0,1')
      }
      if (this.configPort0 === '0' && !this.p3k802Q) return
      if (this.ipMode1 !== '1') {
        this.$socket.sendMsg('#NET-DHCP 1,0')
        this.$socket.sendMsg('#NET-CONFIG 1,' + this.ipInfo1.join(','))
      } else {
        this.$socket.sendMsg('#NET-DHCP 1,1')
      }
    },
    validIP () {
      this.ipInfo0Error = 0
      this.ipInfo1Error = 0
      if (this.ipMode0 !== '1') {
        this.ipInfo0Error = this.isValidIP(this.ipInfo0[0], this.ipInfo0[1], this.ipInfo0[2])
        if (this.ipInfo0Error === 4) {
          alert('The IP address of Eth0 cannot be the same as the gateway address')
        }
        if (this.ipInfo0Error === 5) {
          alert('The gateway address of Eth0 is not at the same network(subnet), which is defined on basis of IP address and mask address.')
        }
        if (this.ipInfo0Error !== 0) {
          return false
        }
      }
      if (!(this.configPort0 === '0' && !this.p3k802Q)) {
        if (this.ipMode1 !== '1') {
          this.ipInfo1Error = this.isValidIP(this.ipInfo1[0], this.ipInfo1[1], this.ipInfo1[2])
          if (this.ipInfo1Error === 4) {
            alert('The IP address of Eth1 cannot be the same as the gateway address')
          }
          if (this.ipInfo1Error === 5) {
            alert('The gateway address of Eth1 is not at the same network(subnet), which is defined on basis of IP address and mask address.')
          }
          if (this.ipInfo1Error !== 0) {
            return false
          }
        }
      }
      return true
    },
    isValidIP (ipAddr, netmask, gateway) {
      return checkLanSettings(ipAddr, netmask, gateway)
    },
    restart () {
      this.$socket.sendMsg('#RESET')
      this.dialogVisibleReset = false
      sessionStorage.removeItem('login')
      setTimeout(() => {
        this.$msg.successAlert('Network configuration changed, please reopen the web page with the new network settings', 8000)
      }, 1000)
    },
    handleRs232Gateway (msg) {
      const data = msg.split(' ')
      if (data[1].length > 0) {
        const arr = data[1].split(',')
        this.rs232Port = parseInt(arr[2])
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

.table {
  width: 100%;
  max-width: 1280px;
  table-layout: auto;
  border-spacing: 0;

  thead th {
    color: #35acf8;
  }

  thead th,
  tbody th {
    height: 50px;
    font-family: "open sans semiblold";
    text-align: left;
  }
}
</style>
