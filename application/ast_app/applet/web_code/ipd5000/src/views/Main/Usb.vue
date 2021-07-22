<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="radio-setting">
        <span class="setting-title">USB over IP</span>
        <div>
          <radio-component v-model="kvmMode" label="1">Optimized for KVM</radio-component>
          <radio-component v-model="kvmMode" label="2">USB Emulation</radio-component>
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
      <div class="setting">
        <span class="setting-title">Request Time Out (min)</span>
        <el-input-number v-model="timeout" controls-position="right" :min="5" :max="10" @blur="checkBlur"></el-input-number>
      </div>
      <div class="radio-setting">
        <span class="setting-title">Roaming Master/Slaves</span>
        <div>
          <radio-component v-model="roaming" label="1">Master</radio-component>
          <radio-component v-model="roaming" label="0">Slaves</radio-component>
        </div>
      </div>
      <div class="kvm-view" v-if="roaming === '1'">
        <div class="error-input">
          Row:
          <el-input-number style="width: 60px;margin-right: 24px;" controls-position="right"  v-model="row" :max="16" :min="1" @change="handleRowKvmMap" @blur="checkBlur"></el-input-number>
          Col:
          <el-input-number style="width: 60px;margin-right: 24px;" controls-position="right"  v-model="col" :max="16" :min="1" @change="handleColKvmMap" @blur="checkBlur"></el-input-number>
          <!-- <button class="btn btn-primary">CHANGE</button> -->
          <span class="alert-error" style="top: 40px;" v-if="(col * row) > 16">Maximum 16 Slaves</span>
        </div>
        <div v-if="(col * row) <= 16"  class="kvm-layout">
          <div v-for="(rowItem, y) in row" :key="rowItem" >
            <div v-for="(colItem, x) in col" class="kvm-child" :key="colItem">
              <icon-svg icon-class="osd" style="margin: 0 auto; width:60px;height:60px;display: block"/>
              <div style="text-align: center; margin-top: 15px">
                <radio-component v-model="master" @input="resetCoord" :label="kvmMap[y][x].h + ',' + kvmMap[y][x].v" style="margin-bottom: 5px;">Master</radio-component>
                <span style="display: block; margin: 5px 0 0">MAC Address:</span>
                <input type="text" v-model="kvmMap[y][x].mac" class="setting-text" style="width: 130px;font-size: 14px;text-align: center" />
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <footer><button class="btn btn-primary" @click="save">SAVE</button></footer>
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
      kvmMode: '1',
      col: 1,
      row: 1,
      usbOverIp: '1',
      master: '0,0',
      roaming: '2',
      roamingMode: false,
      kvmMap: [[{
        h: 0,
        mac: '',
        v: 0
      }]]
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.getKVMJson()
  },
  methods: {
    handleMsg (msg) {
    },
    checkBlur () {
      this.timeout = this.timeout || 5
      this.col = this.col || this.kvmMap[0].length
      this.row = this.row || this.kvmMap.length
    },
    save () {
      if (this.roaming === '1' && this.col * this.row > 16) return
      this.$http.post('/usb/set_km_usb', {
        'USB-KVM_config': {
          kvm_usb_mode: this.kvmMode,
          kvm_timeout_sec: this.timeout,
          km_roaming: this.roaming === '0' ? [] : this.formatKVMData(this.kvmMap)
        }
      })
    },
    getKVMJson () {
      this.$http.post('/usb/km_usb').then(msg => {
        if (msg.data) {
          const data = msg.data['USB-KVM_config']
          this.timeout = data.kvm_timeout_sec
          this.kvmMode = data.kvm_usb_mode
          this.parseKVM(data.km_roaming)
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
      if (this.col * this.row <= 16) {
        // 非法值到有效值时检查，5 x 4 => 4 * 4
        if (currentVal === this.kvmMap.length) return
        if (currentVal > oldVal) {
          const v = this.kvmMap[this.kvmMap.length - 1][0].v - 1
          const startX = this.kvmMap[0][0].h
          const arr = [...new Array(this.col)].map((item, index) => {
            return {
              mac: '',
              h: startX + index,
              v: v
            }
          })
          this.kvmMap.push(arr)
        } else {
          this.kvmMap.pop()
        }
        this.checkMasterExist()
      }
    },
    handleColKvmMap (currentVal, oldVal) {
      if (this.col * this.row <= 16) {
        // 非法值到有效值时检查，4 x 5 => 4 * 4
        if (currentVal === this.kvmMap[0].length) return
        if (currentVal > oldVal) {
          const h = this.kvmMap[0][this.kvmMap[0].length - 1].h + 1
          this.kvmMap.forEach(item => {
            item.push({
              mac: '',
              h,
              v: item[0].v
            })
          })
        } else {
          this.kvmMap.forEach(item => {
            item.pop()
          })
          this.checkMasterExist()
        }
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
      // 1 , 1
      for (const _row in this.kvmMap) {
        for (const _col in this.kvmMap[_row]) {
          // const pointY = _row <= row ? row - _row : ~(_row - row) + 1
          const pointY = row - _row
          const pointX = _col - col
          this.kvmMap[_row][_col].h = pointX
          this.kvmMap[_row][_col].v = pointY
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
.main-setting{
  display: flex;
  flex-direction: column;
}
.setting-model{
  flex: 1;
}
.main-setting footer {
  flex-shrink: 0;
  margin-top: 15px;
  margin-bottom: 15px;
}
.main-setting {
  //overflow-x: auto !important;
}
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
.kvm-child{
  margin: 15px;
  display: inline-block;
}
.kvm-layout{
  white-space: nowrap;
  max-width: 700px;
  max-height: 590px;
  overflow: scroll;
}
</style>
