<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Switching Mode</span>
        <el-select v-model="switchMode.val" @change="setSwitchMode">
          <el-option
            v-for="item in switchMode.param"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <custom-sort :listMap="listMap"
                   v-model="lists"
                   :disabled="switchMode.val !== '1'"></custom-sort>
      <div class="setting"
           style="margin-top: 25px">
        <span class="setting-title">Signal Loss Switching Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '0'"
                         v-model="delay.signal_loss_switching"
                         controls-position="right"
                         :max="90"
                         :min="5"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Signal Detection Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '0'"
                         v-model="delay.signal_detection"
                         controls-position="right"
                         :max="90"
                         :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Cable Plug Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '0'"
                         v-model="delay.cable_plugin"
                         controls-position="right"
                         :max="90"
                         :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Cable Unplug Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '0'"
                         v-model="delay.cable_unplug"
                         controls-position="right"
                         :max="90"
                         :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Signal Loss Switching Power Off Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '0'"
                         v-model="delay.power_off_upon_signal_loss"
                         controls-position="right"
                         :max="60000"
                         :min="5"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Signal Loss Switching Manual Override Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '0'"
                         v-model="delay.manual_override_inactive_signal"
                         controls-position="right"
                         :max="90"
                         :min="5"></el-input-number>
      </div>
    </div>
    <footer>
      <button class="btn btn-primary"
              @click="saveAutoSwitch">SAVE</button>
    </footer>
  </div>
</template>

<script>
import { debounce } from 'lodash'
import customSort from '@/components/custom-sort'
export default {
  name: 'videoPage',
  components: {
    customSort
  },
  data () {
    return {
      lists: ['in.hdmi.1.video', 'in.hdmi.2.video', 'in.usb_c.3.video'],
      listMap: {
        'in.hdmi.1.video': 'HDMI IN1',
        'in.hdmi.2.video': 'HDMI IN2',
        'in.usb_c.3.video': 'USB IN3',
        'in.usb_c.2.video': 'USB IN2'
      },
      switchMode: {
        val: '0',
        param: [
          { value: '2', label: 'Last Connected' },
          { value: '1', label: 'Priority' },
          { value: '0', label: 'Manual' }
        ]
      },
      playStop: 'play',
      delay: {
        cable_plugin: 0,
        cable_unplug: 0,
        manual_override_inactive_signal: 10,
        power_off_upon_signal_loss: 10,
        signal_detection: 0,
        signal_loss_switching: 10
      }
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#X-AV-SW-MODE? out.hdmi.1.video.1')
    this.$socket.sendMsg('#X-PRIORITY? out.stream.1.video')
    this.getAutoSwitchDelay()
  },
  mounted () { },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@X-AV-SW-MODE /i) !== -1) {
        this.handleSwitchMode(msg)
        return
      }
      if (msg.search(/@X-PRIORITY /i) !== -1) {
        this.handleSwitchPriority(msg)
      }
    },
    handleSwitchMode (msg) {
      if (msg.search(/video/g) !== -1) {
        const data = msg.split(',')
        this.switchMode.val = data[data.length - 1]
      }
    },
    setSwitchMode (mode) {
      this.$socket.sendMsg(`#X-AV-SW-MODE out.hdmi.1.video.1,${mode}`)
    },
    getAutoSwitchDelay () {
      this.$http
        .get(
          '/device/json?path=/switch/auto_switch_delays.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.auto_switch_delays) {
            this.delay = msg.data.auto_switch_delays
          }
        })
    },
    setAutoSwitchDelay () {
      this.$http.post('/device/json', {
        path: '/switch/auto_switch_delays.json',
        info: {
          auto_switch_delays: this.delay
        }
      }).then(() => {
        this.$msg.successAlert()
      })
    },
    handleSwitchPriority (msg) {
      if (msg.search(/video/g) !== -1) {
        this.lists = msg
          .match(/[^([]+(?=\])/g)[0]
          .replace(/\s/g, '')
          .split(',')
      }
    },
    saveAutoSwitch: debounce(function () {
      this.$socket.sendMsg(
        `#X-AV-SW-MODE out.hdmi.1.video.1,${this.switchMode.val}`
      )
      if (this.switchMode.val !== '0') {
        this.$socket.sendMsg(
          `#X-PRIORITY out.stream.1.video,[${this.lists.join(',')}]`
        )
        this.setAutoSwitchDelay()
      }
    }, 2000, {
      leading: true,
      trailing: true
    })
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
.main-setting .setting-title {
  width: 400px;
}
.list-group-item {
  display: inline-block;
}
</style>
