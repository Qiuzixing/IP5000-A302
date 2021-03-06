<template>
  <div class="main-setting">
    <div class="setting-model"
         style="margin-bottom: 0">
      <div class="radio-setting"
           v-if="this.$global.deviceType !==2">
        <span class="setting-title">Analog Audio Direction IN/OUT</span>
        <div>
          <radio-component v-model="direction"
                           label="in"
                           @change="changeAudioDirection">IN
          </radio-component>
          <radio-component v-model="direction"
                           label="out"
                           @change="changeAudioDirection">OUT
          </radio-component>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Audio Source Mode</span>
        <el-select v-model="audioMode.val">
          <el-option
            v-for="item in audioMode.param"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <custom-sort v-model="lists"
                   :listMap="listMap"
                   :disabled="audioMode.val !== '1'"></custom-sort>
      <div class="setting"
           style="margin-top: 25px;"
           v-if="this.$global.deviceType === 1">
        <span class="setting-title">Audio Source Selection</span>
        <el-select v-model="audioSource.val" :disabled="audioMode.val !== '0'">
          <el-option
            v-for="item in audioSource.param"
            :key="item.value"
            :label="item.label"
            :value="item.value"
            :disabled="item.disabled">
          </el-option>
        </el-select>
      </div>
      <div v-else
           class="setting"
           style="margin-top: 25px;">
        <span class="setting-title">Audio Source Selection</span>
        <el-select v-model="audioSource.val" :disabled="audioMode.val !== '0'">
          <el-option
            v-for="item in audioSource.encoderParam"
            :key="item.value"
            :label="item.label"
            :value="item.value"
            :disabled="item.disabled">
          </el-option>
        </el-select>
      </div>
      <div class="setting">
        <span class="setting-title">Audio Connection Guard Time (sec)</span>
        <el-input-number v-model="avSignal.audio_connection_guard_time_sec"
                         controls-position="right"
                         :max="90"
                         :min="0"></el-input-number>
      </div>
      <div class="setting-model" v-if="this.$global.deviceType === 1">
        <h3 class="setting-title">Audio Destination</h3>
        <div :key="item.name"
             v-for="(item, index) in audioDestinationDesc"
             style="margin-bottom: 15px;">
          <checkbox-component :label="item"
                              v-if="!(index ===1 && direction==='in')"
                              v-model="audioDestination[index]"
                              :active-value="1"
                              :inactive-value="0"/>
        </div>
      </div>
      <div class="setting"
           v-if="this.$global.deviceType === 1">
        <span class="setting-title">Dante/AES-67 Name</span>
        <input type="text"
               class="setting-text"
               maxlength="32"
               v-model="danteName">
      </div>
    </div>
    <footer>
      <button class="btn btn-primary"
              @click="save">SAVE
      </button>
    </footer>
  </div>
</template>

<script>
import { debounce } from 'lodash'
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
          {
            value: '2',
            label: 'Last Connected'
          },
          {
            value: '1',
            label: 'Priority'
          },
          {
            value: '0',
            label: 'Manual'
          }
        ]
      },
      audioSource: {
        val: '0',
        param: [
          {
            value: '0',
            label: 'HDMI'
          },
          {
            value: '1',
            label: 'Analog'
          },
          {
            value: '2',
            label: 'None'
          },
          {
            value: '4',
            label: 'Dante'
          }
        ],
        encoderParam: [
          {
            value: '0',
            label: 'HDMI'
          },
          {
            value: '1',
            label: 'Analog'
          },
          {
            value: '2',
            label: 'None'
          }
        ]
      },
      playStop: 'play',
      lists: [],
      lists_bak: ['in.dante.1.audio', 'in.analog_audio.1.audio', 'in.hdmi.1.audio'],
      listMap: {
        'in.dante.1.audio': 'Dante',
        'in.analog_audio.1.audio': 'Analog',
        'in.hdmi.1.audio': 'HDMI'
      },
      guardTime: '5',
      audioDestination: [0, 0, 0, 0],
      audioDestinationDesc: [
        'HDMI',
        'Analog',
        'LAN',
        'Dante'
      ],
      audioDestinationDesc2: [
        'HDMI',
        'Analog',
        'HDMI & LAN',
        'Dante'
      ],
      avSignal: {
        audio_connection_guard_time_sec: 90,
        color_depth: 'bypass',
        force_rgb: 'off',
        frame_rate_percentage: 100,
        input_maximum_resolution: 'pass_through',
        maximum_bit_rate: 'best_effort'
      },
      danteName: ''
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#PORT-DIRECTION? both.analog.1.audio')
    this.$socket.sendMsg('#X-AV-SW-MODE? out.hdmi.1.audio.1')
    this.$socket.sendMsg('#X-PRIORITY? out.hdmi.1.audio')
    this.getAVSignal()
    if (this.$global.deviceType === 1) {
      this.$socket.sendMsg('#KDS-AUD-OUTPUT? ')
    }
    if (this.$global.deviceType) {
      this.$socket.sendMsg('#NAME? 1')
    }
    if (this.$global.deviceType !== 2) {
      this.$socket.sendMsg('#PORT-DIRECTION? both.analog.1.audio')
    }
    this.$socket.sendMsg('#KDS-AUD? ')
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
      if (msg.search(/@KDS-AUD-OUTPUT /i) !== -1) {
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
        this.checkAudioSourceSelect()
      }
    },
    checkAudioSourceSelect () {
      if (this.$global.deviceType) {
        if (this.direction === 'out') {
          if (this.audioSource.val === '1') {
            this.audioSource.val = '2'
          }
          this.audioSource.param = [
            {
              value: '0',
              label: 'HDMI'
            },
            {
              value: '1',
              label: 'Analog',
              disabled: true
            },
            {
              value: '2',
              label: 'None'
            },
            {
              value: '4',
              label: 'Dante'
            }
          ]
        } else {
          this.audioSource.param = [
            {
              value: '0',
              label: 'HDMI'
            },
            {
              value: '1',
              label: 'Analog'
            },
            {
              value: '2',
              label: 'None'
            },
            {
              value: '4',
              label: 'Dante'
            }
          ]
        }
      } else {
        if (this.direction === 'out') {
          if (this.audioSource.val === '1') {
            this.audioSource.val = '2'
          }
          this.audioSource.encoderParam = [
            {
              value: '0',
              label: 'HDMI'
            },
            {
              value: '1',
              label: 'Analog',
              disabled: true
            },
            {
              value: '2',
              label: 'None'
            }
          ]
        } else {
          this.audioSource.encoderParam = [
            {
              value: '0',
              label: 'HDMI'
            },
            {
              value: '1',
              label: 'Analog'
            },
            {
              value: '2',
              label: 'None'
            }
          ]
        }
      }
    },
    audioSwitchMode (msg) {
      if (msg.search(/audio/g) !== -1) {
        const data = msg.split(',')
        this.audioMode.val = data[data.length - 1]
      }
    },
    getAVSignal () {
      this.$http
        .get(
          '/device/json?path=/av_signal/av_signal.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.av_signal) {
            this.avSignal = msg.data.av_signal
          }
        })
    },
    handleSwitchPriority (msg) {
      if (msg.search(/audio/g) !== -1) {
        const arr = msg.match(/[^([]+(?=])/g)[0].replace(/\s/g, '').split(',')
        if (this.$global.deviceType === 2) {
          if (arr.indexOf('in.dante.1.audio') > -1) {
            arr.splice(arr.indexOf('in.dante.1.audio'), 1)
          }
        } else {
          if (this.direction === 'out') {
            if (arr.indexOf('in.analog_audio.1.audio') > -1) {
              arr.splice(arr.indexOf('in.analog_audio.1.audio'), 1)
            }
          }
        }
        this.lists = arr
      }
    },
    handleAudioDestination (msg) {
      const arr = [0, 0, 0, 0]
      const isSelectSource = msg.match(/[^([]+(?=])/g) ? msg.match(/[^([]+(?=])/g)[0].replace(/\s/g, '').split(',') : []
      isSelectSource.forEach(item => {
        arr[parseInt(item)] = 1
      })
      this.audioDestination = arr
    },
    setAudioDestination () {
      const data = []
      this.audioDestination.forEach((item, index) => {
        if (!(this.direction === 'in' && index === 1)) {
          if (item === 1) data.push(index)
        }
      })
      this.$socket.sendMsg(`#KDS-AUD-OUTPUT [${data.join(',')}]`)
    },
    setAVSingle () {
      this.$http.post('/device/json', {
        path: '/av_signal/av_signal.json',
        info: {
          av_signal: this.avSignal
        }
      }).then(() =>{
        this.$msg.successAlert()
      })
    },
    handleDanteName (msg) {
      this.danteName = msg.split(',').slice(1).join(',')
    },
    save: debounce(function () {
      if (this.$global.deviceType !== 2) {
        this.$socket.sendMsg(`#PORT-DIRECTION both.analog.1.audio,${this.direction}`)
      }
      this.$socket.sendMsg(`#X-AV-SW-MODE out.hdmi.1.audio.1,${this.audioMode.val}`)
      this.setAVSingle()
      if (this.$global.deviceType === 1) {
        this.setAudioDestination()
      }

      if (this.audioMode.val === '1') {
        const data = this.lists.slice()
        if (this.direction === 'out' && this.$global.deviceType) {
          data.push('in.analog_audio.1.audio')
        }
        this.$socket.sendMsg(`#X-PRIORITY out.hdmi.1.audio,[${data.join(',')}]`)
      } else if (this.audioMode.val === '0') {
        this.$socket.sendMsg(`#KDS-AUD ${this.audioSource.val}`)
      }
      if (this.$global.deviceType === 1) {
        this.$socket.sendMsg(`#NAME 1,${this.danteName}`)
      }
    }, 2000, {
      leading: true,
      trailing: true
    }),
    changeAudioDirection (val) {
      this.checkAudioSourceSelect()
      if (val === 'out') {
        this.lists.indexOf('in.analog_audio.1.audio') !== -1 && this.lists.splice(this.lists.indexOf('in.analog_audio.1.audio'), 1)
      } else {
        this.lists.indexOf('in.analog_audio.1.audio') === -1 && this.lists.push('in.analog_audio.1.audio')
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

.main-setting .setting-title {
  width: 350px;
}

.vs__dropdown-toggle {
  width: 180px;
}

.sort-handle {
  cursor: pointer;
}

.sort-seq {
  margin: 0 5px;
}

//.list-group-item{
//  border-top: 1px solid #4D4D4F;
//  margin-bottom: 10px;
//}
</style>
