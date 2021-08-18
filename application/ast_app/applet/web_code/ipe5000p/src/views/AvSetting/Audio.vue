<template>
  <div class="main-setting">
    <div class="setting-model" style="margin-bottom: 0">
      <div class="radio-setting">
        <span class="setting-title">Analog Audio Direction IN/OUT</span>
        <div>
          <radio-component v-model="direction" label="in">IN</radio-component>
          <radio-component v-model="direction" label="out">OUT</radio-component>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Audio Source Mode</span>
        <multiselect v-model="audioMode.val" :options="audioMode.param"></multiselect>
      </div>
      <custom-sort v-model="lists" :listMap="listMap" :disabled="audioMode.val !== '1'"></custom-sort>
      <div class="setting" style="margin-top: 25px;">
        <span class="setting-title">Audio Source Selection</span>
        <multiselect :disabled="audioMode.val != '0'" v-model="audioSource.val" :options="audioSource.param"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Audio Connection Guard Time (sec)</span>
        <el-input-number v-model="avSignal['audio connection guard time sec']" controls-position="right" :max="90" :min="0"></el-input-number>
      </div>
      <div class="setting-model">
        <h3 class="setting-title">Audio Destination</h3>
        <div :key="item.name" v-for="(item, index) in audioDestinationDesc" style="margin-bottom: 15px;">
          <checkbox-component :label="item" v-model="audioDestination[index]" :active-value="1" :inactive-value="0"/>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Dante/AES-67 Name</span>
        <input type="text" class="setting-text" maxlength="32" v-model="danteName">
      </div>
    </div>
    <footer><button class="btn btn-primary" @click="save">SAVE</button></footer>
  </div>
</template>

<script>
import customSort from '@/components/custom-sort'
import radioComponent from '@/components/radio.vue'
import checkboxComponent from '@/components/checkbox.vue'
export default {
  name: 'auto-switch',
  components: {
    radioComponent,
    customSort,
    checkboxComponent
  },
  data () {
    return {
      direction: 'in',
      audioMode: {
        val: '2',
        param: [
          { value: '2', label: 'Last Connected' },
          { value: '1', label: 'Priority' },
          { value: '0', label: 'Manual' }
        ]
      },
      audioSource: {
        val: '0',
        param: [
          { value: '0', label: 'HDMI' },
          { value: '1', label: 'Analog' },
          { value: '2', label: 'None' },
          { value: '3', label: 'Dante' }
        ]
      },
      playStop: 'play',
      lists: ['in.dante.1.audio', 'in.analog.2.audio', 'in.hdmi.3.audio'],
      listMap: {
        'in.dante.1.audio': 'Dante',
        'in.analog.2.audio': 'Analog',
        'in.hdmi.3.audio': 'HDMI'
      },
      guardTime: '5',
      audioDestination: [0, 0, 0, 0],
      audioDestinationDesc: [
        'LAN',
        'HDMI',
        'Dante',
        'Analog'
      ],
      avSignal: {
        'input maximum resolution': 'Pass Through',
        'maximum bit rate': 'Best Effort',
        'frame rate percentage': 100,
        'color depth': 'Follow Output',
        'audio connection guard time sec': 0,
        'dante vlan tag': ''
      },
      danteName: ''
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#PORT-DIRECTION? both.analog.1.audio')
    this.$socket.sendMsg('#X-AV-SW-MODE? out.hdmi.1.audio.1')
    this.$socket.sendMsg('#X-PRIORITY? out.hdmi.1.audio')
    this.$socket.sendMsg('#X-ROUTE? ')
    this.$socket.sendMsg('#NAME? ')
    this.$socket.sendMsg('#KDS-AUD? ')
    this.getAVSignal()
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@PORT-DIRECTION /i) !== -1) {
        this.handleDirection(msg)
        return
      }
      if (msg.search(/@X-AV-SW-MODE /i) !== -1) {
        this.audioSwitchMode(msg)
        return
      }
      if (msg.search(/@X-PRIORITY /i) !== -1) {
        this.handleSwitchPriority(msg)
        return
      }
      if (msg.search(/@X-ROUTE /i) !== -1) {
        this.handleAudioDestination(msg)
        return
      }
      if (msg.search(/@NAME /i) !== -1) {
        this.handleDanteName(msg)
        return
      }
      if (msg.search(/@KDS-AUD /i) !== -1) {
        this.audioSource.val = msg.split(' ').pop()
      }
    },
    handleDirection (msg) {
      if (msg.search(/audio/i) !== -1) {
        this.direction = msg.split(',')[1].toLowerCase()
      }
    },
    audioSwitchMode (msg) {
      if (msg.search(/audio/g) !== -1) {
        const data = msg.split(',')
        this.audioMode.val = data[data.length - 1]
      }
    },
    getAVSignal () {
      this.$http.post('/av_signal').then(msg => {
        if (msg.data['AV Signal']) {
          this.avSignal = msg.data['AV Signal']
        }
      })
    },
    handleSwitchPriority (msg) {
      if (msg.search(/audio/g) !== -1) {
        this.lists = msg.match(/[^([]+(?=\])/g)[0].replace(/\s/g, '').split(',')
      }
    },
    handleAudioDestination (msg) {
      console.log(msg)
      const arr = [0, 0, 0, 0]
      const isSelectSource = msg.match(/[^([]+(?=\])/g)[0].replace(/\s/g, '').split(',')
      isSelectSource.forEach(item => {
        arr[parseInt(item)] = 1
      })
      this.audioDestination = arr
    },
    setAudioDestination () {
      const data = []
      this.audioDestination.forEach((item, index) => {
        if (item === 1) data.push(index + 1)
      })
      this.$socket.sendMsg(`#X-ROUTE [${data.join(',')}]`)
    },
    setAVSingle () {
      this.$http.post('/set_av_signal', {
        'AV Signal': this.avSignal
      })
    },
    handleDanteName (msg) {
      this.danteName = msg.split(' ').slice(1).join(' ')
    },
    save () {
      this.$socket.sendMsg(`#PORT-DIRECTION both.analog.1.audio,${this.direction}`)
      this.$socket.sendMsg(`#X-AV-SW-MODE out.hdmi.1.audio.1,${this.audioMode.val}`)
      if (this.audioMode.val === '1') {
        this.$socket.sendMsg(`#X-PRIORITY out.hdmi.1.audio,[${this.lists.join(',')}]`)
      }
      if (this.audioMode.val === '0') {
        this.$socket.sendMsg(`#KDS-AUD ${this.audioSource.val}`)
      }
      this.$socket.sendMsg(`#NAME ${this.danteName}`)
      this.setAVSingle()
      this.setAudioDestination()
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
  width: 350px;
}
.vs__dropdown-toggle{
  width: 180px;
}
.sort-handle{
  cursor: pointer;
}
.sort-seq{
  margin: 0 5px;
}
//.list-group-item{
//  border-top: 1px solid #4D4D4F;
//  margin-bottom: 10px;
//}
</style>
