<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Switching Mode</span>
        <multiselect v-model="switchMode.val" :options="switchMode.param" @input="setSwitchMode"></multiselect>
      </div>
        <custom-sort :listMap="listMap" v-model="lists" :disabled="switchMode.val !== '1'"></custom-sort>
      <div class="setting" style="margin-top: 25px;">
        <span class="setting-title">Signal Loss Switching Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '2'" v-model="delay['signal loss switching']" controls-position="right" :max="90" :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Signal Detection Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '2'" v-model="delay['signal detection']" controls-position="right" :max="90" :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Cable Plug Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '2'" v-model="delay['cable plugin']" controls-position="right" :max="90" :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Cable Unplug Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '2'" v-model="delay['cable unplug']" controls-position="right" :max="90" :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Signal Loss Switching Power Off Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '2'" v-model="delay['power off upon signal loss']" controls-position="right" :max="90" :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Signal Loss Switching Manual Override Delay (sec)</span>
        <el-input-number :disabled="switchMode.val === '2'" v-model="delay['manual-override inactive-signal']" controls-position="right" :max="90" :min="0"></el-input-number>
      </div>
    </div>
    <footer><button class="btn btn-primary" @click="saveAutoSwitch">SAVE</button></footer>
  </div>
</template>

<script>
// import draggable from 'vuedraggable'
import customSort from '@/components/custom-sort'
export default {
  name: 'videoPage',
  components: {
    customSort
    // draggable
  },
  data () {
    return {
      lists: ['in.hdmi.1.video', 'in.hdmi.2.video', 'in.usb_c.3.video'],
      listMap: {
        'in.hdmi.1.video': 'HDMI IN1',
        'in.hdmi.2.video': 'HDMI IN2',
        'in.usb_c.3.video': 'USB IN3'
      },
      switchMode: {
        val: '0',
        param: [
          { value: '0', label: 'Last Connected' },
          { value: '1', label: 'Priority' },
          { value: '2', label: 'Manual' }
        ]
      },
      playStop: 'play',
      delay: {
        'signal loss switching': 0,
        'signal detection': 0,
        'cable unplug': 0,
        'cable plugin': 0,
        'power off upon signal loss': 0,
        'manual override upon signal loss': 0
      }
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#X-AV-SW-MODE? out.hdmi.1.video.1')
    this.$socket.sendMsg('#X-PRIORITY? out.stream.1.video')
    this.getAutoSwitchDelay()
  },
  mounted () {
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
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
      this.$http.post('/switch/auto_switch_delays').then(msg => {
        if (msg.data['Auto Switch Delays']) {
          this.delay = msg.data['Auto Switch Delays']
        }
      })
    },
    handleSwitchPriority (msg) {
      if (msg.search(/video/g) !== -1) {
        this.lists = msg.match(/[^([]+(?=\])/g)[0].replace(/\s/g, '').split(',')
      }
    },
    saveAutoSwitch () {
      this.$socket.sendMsg(`#X-AV-SW-MODE out.hdmi.1.video.1,${this.switchMode.val}`)
      if (this.switchMode.val !== '2') {
        this.$socket.sendMsg(`#X-PRIORITY out.stream.1.video,  [${this.lists.join(',')}]`)
        this.setAutoSwitchDelay()
      }
    },
    setAutoSwitchDelay () {
      this.$http.post('/switch/set_auto_switch_delays', {
        'Auto Switch Delays': this.delay
      })
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
.main-setting .setting-title {
  width: 400px;
}
.list-group-item{

  display: inline-block;
}

</style>
