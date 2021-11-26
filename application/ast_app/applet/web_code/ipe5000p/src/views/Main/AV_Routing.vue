<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting"
           v-if="this.$global.deviceType === 1">
        <span class="setting-title">Input Selection</span>
        <multiselect v-model="inputSelect.val"
                     :options="inputSelect.param"
                     @input="setInputSelect"></multiselect>
      </div>
      <div class="setting"
           v-if="this.$global.deviceType === 2">
        <span class="setting-title">Input Selection</span>
        <multiselect v-model="inputSelect.val"
                     :options="inputSelect.param2"
                     @input="setInputSelect"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Stream ID</span>
        <el-input-number v-model="channel"
                         controls-position="right"
                         :max="999"
                         :min="1"></el-input-number>
        <button type="button"
                class="btn btn-plain-primary"
                style="margin-left: 24px;"
                @click="setChannel">APPLY</button>
        <!-- <span>{{channel}}</span> -->
      </div>
      <div class="setting">
        <span class="setting-title">Stream Name</span>
        <div style="position: relative;">
          <input type="text"
                 class="setting-text"
                 maxlength="24"
                 v-model="channelName">
          <span class="range-alert"
                v-if="!isChannelName(channelName)"
                style="top:34px;white-space: nowrap;">Alphanumeric and characters within length of 1 to 24 characters, spaces not allowed</span>
        </div>
        <button type="button"
                class="btn btn-plain-primary"
                style="margin-left: 24px;"
                @click="setChannelName">APPLY</button>
      </div>
      <div class="setting" v-if="this.$global.deviceType !== 2">
        <span class="setting-title">Volume</span>
        <el-slider @change="setVolume"
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
           style="margin-top: 36px;">
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
      channelList: []
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    if (this.$global.deviceType) {
      this.$socket.sendMsg('#X-ROUTE? out.hdmi.1.video.1')
    }
    this.$socket.sendMsg('#KDS-DEFINE-CHANNEL? ')
    this.$socket.sendMsg('#KDS-DEFINE-CHANNEL-NAME? ')
    this.$socket.sendMsg('#X-AUD-LVL? out.analog_audio.1.audio.1')
    this.$socket.sendMsg('#KDS-ACTION? ')
    this.$socket.sendMsg('#X-MUTE? out.stream.1.audio.1')
    this.$socket.sendMsg('#HDCP-STAT? 1,1')
    this.$socket.sendMsg('#KDS-RESOL? 1,1,1')
    this.$socket.sendMsg('#KDS-RATIO? 1,1,1')
    this.$socket.sendMsg('#X-AUD-DESC? out.hdmi.1')
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@X-ROUTE /i) !== -1) {
        this.handleInputSelect(msg)
        return
      }
      if (msg.search(/@KDS-DEFINE-CHANNEL /i) !== -1) {
        this.handleChannel(msg)
        return
      }
      if (msg.search(/@KDS-DEFINE-CHANNEL-NAME /i) !== -1) {
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
      if (msg.search(/@X-AUD-DESC /i) !== -1) {
        this.handleAudioChannelDesc(msg)
      }
    },
    handleInputSelect (msg) {
      // ~nn@X-ROUTE out.hdmi.1.video.1,in.stream.{1}.video.1<CR><LF>
      this.inputSelect.val = msg.split(',')[1].split('.')[2]
    },
    setInputSelect (val) {
      this.$socket.sendMsg(`#X-ROUTE out.hdmi.1.video.1,in.hdmi.${val}.video.1`)
    },
    handleChannel (msg) {
      this.channel = msg.split(' ')[1]
    },
    handleChannelName (msg) {
      this.channelName = msg.split(' ')[1]
    },
    setChannelName () {
      if (!this.isChannelName(this.channelName)) return
      this.$socket.sendMsg(`#KDS-DEFINE-CHANNEL-NAME ${this.channelName}`)
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
      this.action = msg.split(' ')[1]
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
      return /^[a-zA-Z0-9][_\-a-zA-Z0-9]{0,23}$/.test(name)
    }
  }
}
</script>
<style lang="less" scoped>
.main-setting .setting-title {
  width: 200px;
}
</style>
