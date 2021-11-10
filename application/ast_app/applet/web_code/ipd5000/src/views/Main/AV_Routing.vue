<template>
  <div class="main-setting">
    <div class="setting"
         v-if="this.$global.deviceType">
      <span class="setting-title">Input Selection</span>
      <multiselect style="width: 150px"
                   v-model="inputSelect.val"
                   :options="inputSelect.param"
                   @input="setInputSelect"></multiselect>
    </div>
    <div class="setting"
         style="margin-bottom: 0"
         v-if="inputSelect.val === 'stream'">
      <span class="setting-title">Preview</span>
      <span class="setting-title"
            style="width: 180px;">Channel ID</span>
      <span class="setting-title"
            style="width: 180px;">Channel Name</span>
      <!-- <span class="setting-title"
            style="width: 180px;">IP Address</span> -->
    </div>
    <div class="radio-setting"
         style="margin-top: 15px;margin-bottom: 24px;"
         v-if="inputSelect.val === 'stream'">
      <span class="setting-title"><img src="/stream"
             style="width: 180px;"></span>
      <span class="setting-title"
            style="width: 180px;">
        <multiselect :searchable="true"
                     style="width: 150px"
                     :preserveSearch="true"
                     v-model="channel"
                     :options="channelList"
                     @input="setChannel"></multiselect>
      </span>
      <span class="setting-title"
            style="width: 180px;">
        <multiselect :searchable="true"
                     style="width: 150px"
                     :preserveSearch="true"
                     v-model="channel"
                     :options="channelNameList"
                     @input="setChannel"></multiselect>
      </span>
      <!-- <span class="setting-title"
            style="width: 180px;">192.168.1.1</span> -->
    </div>
    <div class="setting"
         style="margin-bottom: 36px">
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
</template>

<script>
import resolutionMap from './resolutionMap'
export default {
  name: 'av_routing',
  data () {
    return {
      inputSelect: {
        val: 'hdmi',
        param: [
          { value: 'hdmi', label: 'HDMI' },
          { value: 'stream', label: 'STREAM' }
        ]
      },
      channel: '',
      channelList: [],
      channelNameList: [],
      marks: {
        0: '0',
        100: '100'
      },
      volume: 0,
      playStop: 'play',
      muteVal: 'mute',
      action: '0',
      aspectRatio: '',
      resolution: '',
      hdcp: '0',
      audioChannel: '',
      audioFormat: '',
      audioRate: ''
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
    } else {
      this.inputSelect.val = 'stream'
    }
    this.$socket.sendMsg('#X-AUD-LVL? out.analog_audio.1.audio.1')
    this.$socket.sendMsg('#KDS-ACTION? ')
    this.$socket.sendMsg('#X-MUTE? out.analog.1.audio.1')
    this.$socket.sendMsg('#HDCP-STAT? 1,1')
    this.$socket.sendMsg('#KDS-RESOL? 1,1,1')
    this.$socket.sendMsg('#KDS-RATIO? 1,1,1')
    this.$socket.sendMsg('#X-AUD-DESC? out.hdmi.1')
    this.getAvChannelMap()
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@X-ROUTE /i) !== -1) {
        this.handleInputSelect(msg)
        return
      }
      if (msg.search(/@X-AUD-LVL /i) !== -1) {
        this.handleVolume(msg)
        return
      }
      if (msg.search(/@X-MUTE /i) !== -1) {
        this.handleAudioMute(msg)
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
      if (msg.search(/@KDS-CHANNEL-SELECT video/i) !== -1) {
        this.handleSelectChannel(msg)
      }
    },
    handleInputSelect (msg) {
      // ~nn@X-ROUTE out.hdmi.1.video.1,in.{stream|hdmi}.1.video.1<CR><LF>
      this.inputSelect.val = msg.split(',')[1].split('.')[1]
    },
    setInputSelect (val) {
      this.$socket.sendMsg(`#X-ROUTE out.hdmi.1.video.1,in.${val}.1.video.1`)
    },
    handleVolume (msg) {
      const data = msg.split(',')
      this.volume = parseInt(data[data.length - 1])
    },
    setVolume () {
      this.$socket.sendMsg(`#X-AUD-LVL out.analog_audio.1.audio.1,${this.volume}`)
    },
    setAudioMute (msg) {
      this.$socket.sendMsg(`#X-MUTE out.analog.1.audio.1,${msg}`)
    },
    handleAudioMute (msg) {
      this.muteVal = msg.split(',').pop()
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
      this.resolution = resolutionMap[data[data.length - 1]]
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
    async getAvChannelMap () {
      await this.$http.get(
        '/device/json?path=/channel/channel_map.json&t=' + Math.random()
      ).then(msg => {
        if (msg.data) {
          const channelList = []
          const channelNameList = []
          msg.data.channels_list.forEach(item => {
            channelList.push({ value: item.id + '', label: '#' + item.id })
            channelNameList.push({ value: item.id + '', label: item.name })
          })
          this.channelList = channelList
          this.channelNameList = channelNameList
        }
      })
      this.$socket.sendMsg('#KDS-CHANNEL-SELECT? video')
    },
    handleSelectChannel (msg) {
      this.channel = msg.split(',')[1]
    },
    setChannel (val) {
      this.$socket.sendMsg('#KDS-CHANNEL-SELECT video,' + val)
    }
  }
}
</script>
<style lang="less" scoped>
</style>
