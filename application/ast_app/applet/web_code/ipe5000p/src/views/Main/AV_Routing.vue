<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting"
           v-if="this.$global.deviceType === 1">
        <span class="setting-title">Input Selection</span>
        <el-select v-model="inputSelect.val" @change="setInputSelect">
          <el-option
            v-for="item in inputSelect.param"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <div class="setting"
           v-if="this.$global.deviceType === 2">
        <span class="setting-title">Input Selection</span>
        <el-select v-model="inputSelect.val" @change="setInputSelect">
          <el-option
            v-for="item in inputSelect.param2"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <div class="setting">
        <span class="setting-title">Channel ID</span>
        <el-input-number v-model="channel"
                         controls-position="right"
                         :max="999"
                         :min="1"></el-input-number>
        <button type="button"
                class="btn btn-plain-primary"
                style="margin-left: 24px;"
                @click="setChannel">APPLY</button>
      </div>
      <div class="setting" style="position: relative;">
        <span class="setting-title">Channel Name</span>
<!--        <span>{{channelName}}</span>-->
        <div>
          <input type="text"
                 class="setting-text"
                 maxlength="24"
                 v-model="channelName">
        </div>
        <button type="button"
                class="btn btn-plain-primary"
                style="margin-left: 24px;"
                @click="setChannelName">APPLY</button>
        <span class="range-alert"
              v-if="channelNameError"
              style="top:34px;white-space: nowrap;">Alphanumeric, hyphen and underscore within 24 characters, hyphen and underscore can not at beginning or end</span>
      </div>
      <div class="setting" v-if="this.$global.deviceType !== 2">
        <span class="setting-title">Volume</span>
        <el-slider @change="setVolume"
                   :disabled="volumeDisabled"
                   style="width: 200px"
                   :min="0"
                   :max="100"
                   :show-tooltip="false"
                   v-model="volume"
                   :marks="marks">
        </el-slider>
        <span style="margin-left: 15px">{{volume}}</span>
      </div>
      <div class="setting"
           style="margin-top: 24px;">
        <span class="setting-title">Mute</span>
        <v-switch v-model="muteVal"
                  active-value="on"
                  inactive-value="off"
                  @change="setAudioMute"></v-switch>
      </div>
      <div class="setting">
        <span class="setting-title">Play/Stop</span>
        <v-switch v-model="action"
                  open-text="Play"
                  close-text="Stop"
                  active-value="1"
                  inactive-value="0"
                  @change="setAction"></v-switch>
      </div>
      <div class="setting">
        <span class="setting-title">HDCP Encryption</span>
        <span>{{hdcp == '1' ? 'On' : 'Off'}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Resolution</span>
        <span>{{resolution}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Aspect Ratio</span>
        <span>{{aspectRatio}}</span>
      </div>
      <div class="setting">
        <span class="setting-title"> Audio Channels</span>
        <span>{{audioChannel}}</span>
      </div>
      <div class="setting">
        <span class="setting-title"> Audio Rate</span>
        <span>{{audioRate}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Audio Format</span>
        <span>{{audioFormat}}</span>
      </div>
    </div>
  </div>
</template>

<script>
import resolutionMap from './resolutionMap'
export default {
  name: 'av_routing',
  data () {
    return {
      marks: {
        0: '0',
        100: '100'
      },
      volume: 0,
      inputSelect: {
        val: '1',
        param: [
          { value: '1', label: 'HDMI IN1' },
          { value: '2', label: 'HDMI IN2' },
          { value: '3', label: 'USB IN3' }
        ],
        param2: [
          { value: '1', label: 'HDMI IN1' },
          { value: '2', label: 'USB IN2' }
        ]
      },
      channel: '',
      channelName: '',
      action: '0',
      muteVal: '0',
      aspectRatio: '',
      resolution: '',
      hdcp: '0',
      audioChannel: '',
      audioFormat: '',
      audioRate: '',
      channelList: [],
      volumeDisabled: true,
      channelNameError: false
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    if (this.$global.deviceType) {
      this.$socket.sendMsg('#X-ROUTE? out.hdmi.1.video.1')
    }
    this.$socket.sendMsg('#PORT-DIRECTION? both.analog.1.audio')
    this.$socket.sendMsg('#KDS-DEFINE-CHANNEL? ')
    this.$socket.sendMsg('#NAME? 0')
    this.$socket.sendMsg('#X-AUD-LVL? out.analog_audio.1.audio.1')
    this.$socket.sendMsg('#KDS-ACTION? ')
    this.$socket.sendMsg('#X-MUTE? out.stream.1.audio.1')
    this.$socket.sendMsg('#HDCP-STAT? 1,1')
    this.$socket.sendMsg('#KDS-RESOL? 1,1,1')
    this.$socket.sendMsg('#KDS-RATIO? ')
    this.$socket.sendMsg('#X-AUD-DESC? out.hdmi.1')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@X-ROUTE /i) !== -1) {
        this.handleInputSelect(msg)
        return
      }
      if (msg.search(/@KDS-DEFINE-CHANNEL /i) !== -1) {
        this.handleChannel(msg)
        return
      }
      if (msg.search(/@NAME /i) !== -1) {
        this.handleChannelName(msg)
        return
      }
      if (msg.search(/@X-MUTE /i) !== -1) {
        this.handleAudioMute(msg)
        return
      }
      if (msg.search(/@X-AUD-LVL /i) !== -1) {
        this.handleVolume(msg)
        return
      }
      if (msg.search(/@KDS-ACTION /i) !== -1) {
        this.handleAction(msg)
        return
      }
      if (msg.search(/@HDCP-STAT /i) !== -1) {
        this.handleHDCPStatus(msg)
        return
      }
      if (msg.search(/@KDS-RESOL /i) !== -1) {
        this.handleResolution(msg)
        return
      }
      if (msg.search(/@KDS-RATIO /i) !== -1) {
        this.handleRatio(msg)
        return
      }
      if (msg.search(/@PORT-DIRECTION /i) !== -1) {
        this.handleDirection(msg)
        return
      }
      if (msg.search(/@X-AUD-DESC /i) !== -1) {
        this.handleAudioChannelDesc(msg)
      }
    },
    handleInputSelect (msg) {
      // ~nn@X-ROUTE out.hdmi.1.video.1,in.stream.{1}.video.1<CR><LF>
      this.inputSelect.val = msg.split(',')[1].split('.')[2]
    },
    setInputSelect (val) {
      if (this.$global.deviceType === 1) {
        if (val === 3) {
          this.$socket.sendMsg(`#X-ROUTE out.hdmi.1.video.1,in.usb_c.${val}.video.1`)
        } else {
          this.$socket.sendMsg(`#X-ROUTE out.hdmi.1.video.1,in.hdmi.${val}.video.1`)
        }
      }
      if (this.$global.deviceType === 2) {
        if (val === 2) {
          this.$socket.sendMsg(`#X-ROUTE out.hdmi.1.video.1,in.usb_c.${val}.video.1`)
        } else {
          this.$socket.sendMsg(`#X-ROUTE out.hdmi.1.video.1,in.hdmi.${val}.video.1`)
        }
      }
    },
    handleChannel (msg) {
      this.channel = msg.split(' ')[1]
    },
    handleChannelName (msg) {
      this.channelName = msg.split(',').slice(1).join(',')
    },
    setChannelName () {
      this.channelNameError = !this.isChannelName(this.channelName)
      if (this.channelNameError) return
      this.$socket.sendMsg(`#NAME 0,${this.channelName}`)
    },
    handleAudioMute (msg) {
      this.muteVal = msg.split(',').pop()
    },
    setAudioMute (msg) {
      this.$socket.sendMsg(`#X-MUTE out.stream.1.audio.1,${msg}`)
    },
    handleVolume (msg) {
      const data = msg.split(',')
      this.volume = parseInt(data[data.length - 1])
    },
    setVolume () {
      this.$socket.sendMsg(`#X-AUD-LVL out.analog_audio.1.audio.1,${this.volume}`)
    },
    handleAction (msg) {
      const val = msg.split(' ')[1]
      if (val === '0' || val === '1') {
        this.action = val
      }
    },
    setAction (msg) {
      this.$socket.sendMsg(`#KDS-ACTION ${msg}`)
    },
    handleHDCPStatus (msg) {
      const data = msg.split(',')
      this.hdcp = data[data.length - 1]
    },
    handleResolution (msg) {
      const data = msg.split(',')
      this.resolution = resolutionMap[data[data.length - 1]] || 'unknown'
    },
    handleRatio (msg) {
      this.aspectRatio = msg.split(' ')[1]
    },
    handleAudioChannelDesc (msg) {
      const data = msg.split(',')
      this.audioFormat = data[data.length - 1]
      this.audioRate = data[data.length - 2]
      this.audioChannel = data[data.length - 3]
    },
    setChannel () {
      this.$socket.sendMsg('#KDS-DEFINE-CHANNEL ' + (this.channel || 1))
    },
    isChannelName (name) {
      return /^[a-zA-Z0-9]$|^[a-zA-Z0-9][\w-]{0,22}[a-zA-Z0-9]$/.test(name)
    },
    handleDirection (msg) {
      if (msg.search(/audio/i) !== -1) {
        this.volumeDisabled = msg.split(',')[1].toLowerCase() === 'in'
      }
    }
  }
}
</script>
<style lang="less" scoped>
.main-setting .setting-title {
  width: 200px;
}
</style>
