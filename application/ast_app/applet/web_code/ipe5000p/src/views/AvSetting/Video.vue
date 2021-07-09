<template>
  <div class="main-setting">
    <div class="setting-content">
      <div class="setting-model" style="margin-bottom: 0">
        <h3 class="setting-model-title">Input HDCP Appearance</h3>
        <div class="setting">
          <span class="setting-title">Input 1</span>
          <v-switch v-model="hdcp[0]" active-value="1" inactive-value="0" @change="setHDCP($event, 1)"></v-switch>
        </div>
        <div class="setting">
          <span class="setting-title">Input 2</span>
          <v-switch v-model="hdcp[1]" active-value="1" inactive-value="0" @change="setHDCP($event, 2)"></v-switch>
        </div>
        <div class="setting">
          <span class="setting-title">Input 3</span>
          <v-switch v-model="hdcp[2]" active-value="1" inactive-value="0" @change="setHDCP($event, 3)"></v-switch>
        </div>
        <div class="setting">
          <span class="setting-title">ALL</span>
          <button class="btn btn-plain-primary" @click="setAllHDCP('1')">ON</button>
          <button class="btn btn-plain-primary" @click="setAllHDCP('0')" style="margin-left: 25px;">OFF</button>
        </div>
      </div>
      <div class="setting-model">
        <h3 class="setting-model-title">Display</h3>
        <div class="setting">
          <span class="setting-title">Sleep (5V-off) Delay On Video Signal Loss (sec)</span>
          <el-input-number v-model="delay['sleep delay on signal loss sec']" controls-position="right" :max="90" :min="0"></el-input-number>
        </div>
        <div class="setting">
          <span class="setting-title">Shutdown (CEC) Delay On Video Signal Loss (sec)</span>
          <el-input-number v-model="delay['shutdown delay on signal loss sec']" controls-position="right" :max="90" :min="0"></el-input-number>
        </div>
        <div class="setting">
          <span class="setting-title">Wake-up (CEC) Delay On Video Signal Detection (sec)</span>
          <el-input-number v-model="delay['wake-up delay on signal detection sec']" controls-position="right" :max="90" :min="0"></el-input-number>
<!--          <button class="btn btn-plain-primary" style="margin-left: 24px;" @click="setDisplayDelay">APPLY</button>-->
        </div>
      </div>
      <div class="setting-model">
        <div class="radio-setting">
          <span class="setting-title">Sleep Image</span>
          <div class="overlay-setting">
            <div class="overlay-setting-item overlay-img">
              <span class="file-name" style="display: inline-block;">sleep.png</span>
              <span class="upload-icon" @click="clickUpload">
                <icon-svg icon-class="upload_img"/>
              </span>
              <input type="file" accept="image/jpeg" ref="upload" style="display: none;">
              <br>
              <button class="btn btn-plain-primary" style="margin-top: 15px;margin-bottom: 15px;">SLEEP</button>
            </div>
          </div>
        </div>
        <div class="radio-setting" style="margin-bottom: 24px;">
          <span class="setting-title">Image Preview</span>
          <img src="/preview" alt="" style="max-width: 320px;">
        </div>
        <div class="setting">
          <span class="setting-title">Maximum Bit Rate</span>
          <multiselect v-model="avSignal['maximum bit rate']" :options="bitRateParam"></multiselect>
        </div>
        <div class="setting">
          <span class="setting-title">Maximum Video Frame Rate (%)</span>
          <el-slider
            style="width: 200px"
            :min="0" :max="100"
            :show-tooltip="false"
            v-model="avSignal['frame rate percentage']"
            :marks="marks">
          </el-slider>
          <span style="margin-left: 15px">{{avSignal['frame rate percentage']}}</span>
        </div>
        <div class="setting">
          <span class="setting-title">Force 8-bit Color Depth</span>
          <v-checkbox v-model="avSignal['color depth']" active-value="8-bit" inactive-value="Follow Output"></v-checkbox>
        </div>
        <div class="setting">
          <span class="setting-title">Force RGB</span>
          <v-checkbox v-model="forceRGB" active-value="1" inactive-value="0"></v-checkbox>
        </div>
      </div>
    </div>
    <footer><button class="btn btn-primary" @click="save">SAVE</button></footer>
  </div>
</template>

<script>
import vCheckbox from '@/components/checkbox.vue'

export default {
  name: 'audioPage',
  components: {
    vCheckbox
  },
  data () {
    return {
      forceRGB: '0',
      maxBitRate: 50,
      marks: {
        0: '0',
        100: '100'
      },
      avSignal: {
        'input maximum resolution': 'Pass Through',
        'maximum bit rate': 'Best Effort',
        'frame rate percentage': 100,
        'color depth': 'Follow Output',
        'audio connection guard time sec': 0,
        'dante vlan tag': ''
      },
      delay: {
        'sleep delay on signal loss sec': 0,
        'shutdown delay on signal loss sec': 0,
        'wake-up delay on signal detection sec': 0
      },
      hdcp: ['0', '0', '0'],
      bitRateParam: [
        { value: 'Best Effort', label: 'Best Effort' },
        { value: '200 Mbps', label: '200 Mbps' },
        { value: '150 Mbps', label: '150 Mbps' },
        { value: '100 Mbps', label: '100 Mbps' },
        { value: '50 Mbps', label: '50 Mbps' },
        { value: '10 Mbps', label: '10 Mbps' }
      ]
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#HDCP-MOD? 1')
    this.$socket.sendMsg('#HDCP-MOD? 2')
    this.$socket.sendMsg('#HDCP-MOD? 3')
    this.$socket.sendMsg('#CS-CONVERT? 1')
    this.getAVSignal()
    this.getDisplayDelay()
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@HDCP-MOD /i) !== -1) {
        this.handleHDCP(msg)
        return
      }
      if (msg.search(/@CS-CONVERT /i) !== -1) {
        this.handleForceRGB(msg)
      }
    },
    handleHDCP (msg) {
      const data = msg.split(' ')[1].split(',')
      this.hdcp[parseInt(data[0]) - 1] = data[1]
    },
    setAllHDCP (val) {
      this.$socket.sendMsg(`#HDCP-MOD 1,${val}`)
      this.$socket.sendMsg(`#HDCP-MOD 2,${val}`)
      this.$socket.sendMsg(`#HDCP-MOD 3,${val}`)
    },
    setHDCP (val, index) {
      this.$socket.sendMsg(`#HDCP-MOD ${index},${val}`)
    },
    clickUpload () {
      this.$refs.upload.click()
    },
    getAVSignal () {
      this.$http.post('/av_signal').then(msg => {
        if (msg.data['AV Signal']) {
          this.avSignal = msg.data['AV Signal']
        }
      })
    },
    setAVSingle () {
      this.$http.post('/set_av_signal', {
        'AV Signal': this.avSignal
      })
    },
    getDisplayDelay () {
      this.$http.post('/display/display_sleep').then(msg => {
        if (msg.data['Display Delays']) {
          this.delay = msg.data['Display Delays']
        }
      })
    },
    setDisplayDelay () {
      this.$http.post('/display/set_display_sleep', {
        'Display Delays': this.delay
      })
    },
    handleForceRGB (msg) {
      this.forceRGB = msg.split(',')[1]
    },
    save () {
      this.setDisplayDelay()
      this.setAVSingle()
      this.$socket.sendMsg(`#CS-CONVERT 1,${this.forceRGB}`)
    }
  }
}
</script>
<style lang="less" scoped>
.main-setting{
  display: flex;
  flex-direction: column;
}
.setting-content{
  flex: 1;
}
.main-setting footer {
  flex-shrink: 0;
  margin-top: 15px;
  margin-bottom: 15px;
}
.main-setting .setting-title {
  width: 420px;
}
.overlay-setting {
  display: flex;
  margin-bottom: 15px;
  align-items: center;

  .overlay-setting-item {
    flex: 1
  }
  .overlay-title {
    width: 176px;
    font-family: 'open sans semiblold', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
  }

  .overlay-img {
    position: relative;

    .file-name {
      width: 180px;
      height: 30px;
      line-height: 30px;
      padding-right: 25px;
      box-sizing: border-box;
      border-bottom: 1px solid #4D4D4F;
    }

    //
    .upload-icon {
      position: absolute;
      top: 6px;
      left: 158px;
    }

    svg {
      cursor: pointer;
    }
  }
}
</style>
