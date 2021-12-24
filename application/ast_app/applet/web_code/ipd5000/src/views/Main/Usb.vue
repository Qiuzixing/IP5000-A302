<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="radio-setting">
        <span class="setting-title">USB over IP</span>
        <div>
          <radio-component v-model="kvmMode"
                           :disabled="castMode === '1'"
                           label="km">Optimized for KVM</radio-component>
          <radio-component v-model="kvmMode"
                           :disabled="castMode === '1'"
                           label="usb">USB Emulation</radio-component>
        </div>
      </div>
      <!--      <div class="setting">-->
      <!--        <span class="setting-title">Active KVM </span>-->
      <!--        <v-switch open-text="Enable" close-text="Disable"></v-switch>-->
      <!--      </div>-->
      <!--      <div class="setting">-->
      <!--        <span class="setting-title">Active per Request</span>-->
      <!--        <v-switch open-text="Enable" close-text="Disable"></v-switch>-->
      <!--      </div>-->
      <div class="setting"
           v-if="kvmMode === 'km'">
        <span class="setting-title">Request Time Out (min)</span>
        <el-input-number v-model="timeout"
                         controls-position="right"
                         :min="5"
                         :max="10"
                         :disabled="castMode === '1'"></el-input-number>
      </div>
      <div class="radio-setting"
           v-if="kvmMode === 'km'">
        <span class="setting-title">Roaming Master/Slave</span>
        <div>
          <radio-component v-model="roaming"
                           label="1"
                           @change="resetCoord"
                           :disabled="castMode === '1'">Master</radio-component>
          <radio-component v-model="roaming"
                           label="0"
                           :disabled="castMode === '1'">Slave</radio-component>
        </div>
      </div>
      <div class="kvm-view"
           v-if="roaming === '1' && kvmMode === 'km'">
        <div class="error-input">
          Row:
          <el-input-number style="width: 80px;margin-right: 24px;"
                           controls-position="right"
                           v-model="row"
                           :max="16"
                           :min="1"
                           @change="handleRowKvmMap"></el-input-number>
          Col:
          <el-input-number style="width: 80px;margin-right: 24px;"
                           controls-position="right"
                           v-model="col"
                           :max="16"
                           :min="1"
                           @change="handleColKvmMap"></el-input-number>
          <!-- <button class="btn btn-primary">CHANGE</button> -->
          <span class="alert-error"
                style="top: 60px;"
                v-if="(col * row) > 16">Maximum 16 Slaves</span>
        </div>
        <div v-if="(col * row) <= 16"
             class="kvm-layout">
          <div v-for="(rowItem, y) in row"
               :key="rowItem">
            <div v-for="(colItem, x) in col"
                 class="kvm-child"
                 :key="colItem">
              <icon-svg icon-class="osd"
                        style="margin: 0 auto; width:60px;height:60px;display: block" />
              <div style="text-align: center; margin-top: 15px">
                <radio-component v-model="master"
                                 @input="resetCoord"
                                 :label="kvmMap[y][x].h + ',' + kvmMap[y][x].v"
                                 style="margin-bottom: 5px;">Master</radio-component>
                <span style="display: block; margin: 5px 0 0">MAC Address:</span>
                <div style="position: relative;">
                  <input type="text"
                         :disabled="master===kvmMap[y][x].h+ ','+kvmMap[y][x].v"
                         v-model="kvmMap[y][x].mac"
                         class="setting-text"
                         style="width: 130px;font-size: 14px;text-align: center" />
                  <span class="range-alert"
                        v-if="macError === (rowItem-1)*row + colItem">MAC address error</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <footer>
      <p class="error-msg"
         v-if="sameMacError">You can not define the same MAC as a Master and a slave</p>
      <button class="btn btn-primary"
              :disabled="castMode === '1'"
              @click="save">SAVE</button>
    </footer>
  </div>
</template>

<script>
import radioComponent from '@/components/radio.vue'
// import checkboxComponent from '@/components/checkbox.vue'
export default {
  name: 'usb',
  components: {
    radioComponent
  },
  data () {
    return {
      timeout: 5,
      kvmMode: 'km',
      col: 1,
      row: 1,
      usbOverIp: '1',
      master: '0,0',
      roaming: '2',
      kvmMap: [[{
        h: 0,
        mac: '',
        v: 0
      }]],
      mac: '',
      kvm: {},
      macError: -1,
      castMode: '0', // 2: Multicast才可以设置KVM
      sameMacError: false
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#NET-MAC? 0')
    this.$socket.sendMsg('#KDS-METHOD? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@KDS-METHOD /i) !== -1) {
        this.handleIpCastMode(msg)
        return
      }
      if (msg.search(/@NET-MAC /i) !== -1) {
        this.handleMACAddr(msg)
      }
    },
    handleMACAddr (msg) {
      this.mac = msg.split(' ')[1].split(',').pop().replace(/:|-/ig, '')
      this.getKVMJson()
    },
    handleIpCastMode (msg) {
      this.castMode = msg.split(' ')[1]
    },
    checkMAC (data) {
      for (let i = 0; i < data.length; i++) {
        for (let j = 0; j < data[i].length; j++) {
          if (data[i][j].mac && !data[i][j].mac.match(/^[a-f0-9]{12}$/i)) {
            return data[i].length * i + j + 1
          }
        }
      }
      return -1
    },
    checkSameMac (data) {
      let count = 0
      for (let i = 0; i < data.length; i++) {
        for (let j = 0; j < data[i].length; j++) {
          if (data[i][j].mac === this.mac) {
            count++
          }
        }
      }
      return count
    },
    save () {
      if (this.roaming === '1' && this.col * this.row > 16) return
      this.macError = this.checkMAC(this.kvmMap)
      if (this.macError !== -1) return
      if (this.checkSameMac(this.kvmMap) > 1) {
        this.sameMacError = true
        return
      }
      this.sameMacError = false
      this.kvm.kvm_timeout_sec = this.timeout
      this.kvm.kvm_usb_mode = this.kvmMode
      this.kvm.kvm_col = this.col
      this.kvm.kvm_row = this.row
      this.kvm.km_roaming = this.roaming === '0' ? [] : this.formatKVMData(this.kvmMap)
      this.$http.post('/device/json', {
        path: '/usb/km_usb.json',
        info: {
          usb_kvm_config: this.kvm
        }
      })
    },
    getKVMJson () {
      this.$http
        .get(
          '/device/json?path=/usb/km_usb.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.usb_kvm_config) {
            this.kvm = msg.data.usb_kvm_config
            this.timeout = this.kvm.kvm_timeout_sec
            this.kvmMode = this.kvm.kvm_usb_mode
            this.roaming = this.kvm.km_roaming.length ? '1' : '0'
            this.parseKVM(this.kvm.km_roaming)
          }
        })
    },
    parseKVM (data) {
      this.roaming = data.length > 0 ? '1' : '0'
      if (data.length > 0) {
        //  将数据转为二维数组
        const _2dArray = []
        const yMap = {} // 记录Y轴对应的二维数组下标
        data.forEach((item) => {
          if (item.v === item.h && item.v === 0) {
            item.mac = this.mac
          }
          if (yMap[item.v]) {
            _2dArray[yMap[item.v] - 1].push(item)
            _2dArray[yMap[item.v] - 1].sort((a, b) => a.h - b.h)
          } else {
            _2dArray.push([item])
            yMap[item.v] = _2dArray.length
          }
        })
        this.kvmMap = _2dArray.sort((a, b) => b[0].v - a[0].v)
        this.row = _2dArray.length
        this.col = _2dArray[0].length
      }
    },
    handleRowKvmMap (currentVal, oldVal) {
      this.macError = -1
      // 非法值到有效值时检查，5 x 4 => 4 * 4
      if (currentVal > oldVal) {
        const v = this.kvmMap[this.kvmMap.length - 1][0].v - 1
        const startX = this.kvmMap[0][0].h
        for (let i = 0; i < currentVal - oldVal; i++) {
          const arr = [...new Array(this.col)].map((item, index) => {
            return {
              mac: '',
              h: startX + index,
              v: v
            }
          })
          this.kvmMap.push(arr)
        }
      } else {
        for (let i = 0; i < oldVal - currentVal; i++) {
          this.kvmMap.pop()
        }
      }
      this.checkMasterExist()
    },
    handleColKvmMap (currentVal, oldVal) {
      this.macError = -1
      // 非法值到有效值时检查，4 x 5 => 4 * 4
      if (currentVal === this.kvmMap[0].length) return
      if (currentVal > oldVal) {
        this.kvmMap.forEach(item => {
          for (let i = 0; i < currentVal - oldVal; i++) {
            // const h = this.kvmMap[0][this.kvmMap[0].length - 1].h + 1
            const h = this.kvmMap[i][i].h + 1
            item.push({
              mac: '',
              h,
              v: item[0].v
            })
          }
        })
      } else {
        this.kvmMap.forEach(item => {
          for (let i = 0; i < oldVal - currentVal; i++) {
            item.pop()
          }
        })
        this.checkMasterExist()
      }
    },
    // 检查master是否存在， 不存在重置为第一个。
    checkMasterExist () {
      const [x, y] = this.master.split(',')
      const minX = this.kvmMap[0][0].h
      const maxX = this.col + minX - 1
      const maxY = this.kvmMap[0][0].v
      const minY = maxY - this.row + 1
      if (x < minX || x > maxX || y < minY || y > maxY) {
        this.master = this.kvmMap[0][0].h + ',' + this.kvmMap[0][0].v
        this.resetCoord()
      }
    },
    // 重新排序坐标
    resetCoord () {
      const { col, row } = this.masterCoord()
      for (const _row in this.kvmMap) {
        for (const _col in this.kvmMap[_row]) {
          // const pointY = _row <= row ? row - _row : ~(_row - row) + 1
          const pointY = row - _row
          const pointX = _col - col
          this.kvmMap[_row][_col].h = pointX
          this.kvmMap[_row][_col].v = pointY
          if (this.kvmMap[_row][_col].mac === this.mac) {
            this.kvmMap[_row][_col].mac = ''
          }
          if (pointX === pointY && pointX === 0) { this.kvmMap[_row][_col].mac = this.mac }
        }
      }
      this.master = '0,0'
    },
    // 找到master 行列位置
    masterCoord () {
      let [x, y] = this.master.split(',')
      x = +x
      y = +y
      for (const row in this.kvmMap) {
        for (const col in this.kvmMap[row]) {
          if (this.kvmMap[row][col].h === x && this.kvmMap[row][col].v === y) {
            return {
              row: row,
              col: col
            }
          }
        }
      }
    },
    formatKVMData (data) {
      return data.reduce(function (a, b) { return a.concat(b) })
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
// .main-setting {
//   //overflow-x: auto !important;
// }
.setting-title {
  width: 220px;
}
// .kvm-view{
//   width: 600px;
//   &.kvm2-2{
//     width: 300px;
//     .kvm-child{
//       margin-bottom: 15px;
//     }
//   }
//   display: flex;
//   flex-wrap: wrap;
//   .kvm-child{
//     width: 150px;
//   }
// }
.kvm-child {
  margin: 15px;
  display: inline-block;
}
.kvm-layout {
  white-space: nowrap;
  max-width: 700px;
  max-height: 590px;
  overflow: scroll;
}
</style>
