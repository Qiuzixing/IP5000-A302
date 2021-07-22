<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Maximum Resolution</span>
        <multiselect :options="maxResolution.param" v-model="maxResolution.val"></multiselect>
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
<!--        <div class="setting">-->
<!--          <span class="setting-title">Signal Lost Detection(sec)</span>-->
<!--          <el-input-number controls-position="right" :max="90" :min="0"></el-input-number>-->
<!--        </div>-->
      </div>
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
        <span class="setting-title">Force 8-bit Color Depth</span>
        <v-checkbox v-model="avSignal['color depth']" active-value="8-bit" inactive-value="Follow Output"></v-checkbox>
      </div>
      <div class="setting">
        <span class="setting-title">Force RGB</span>
        <v-checkbox v-model="forceRGB" active-value="1" inactive-value="0"></v-checkbox>
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
      showColorDepth: false,
      forceRGB: '0',
      value: 50,
      switchMode: {
        val: 'last_connected',
        param: [
          {
            value: 'last_connected',
            label: 'Last Connected'
          },
          {
            value: 'priority',
            label: 'Priority'
          },
          {
            value: 'manual',
            label: 'Manual'
          }
        ]
      },
      hdcp: 'off',
      hdcp2: 'off',
      hdcp3: 'off',
      test: '0',
      show: false,
      maxResolution: {
        val: '16',
        param: [
          {
            value: '0',
            label: 'Pass Through'
          },
          {
            value: '4',
            label: 'Full HD (720p60)'
          },
          {
            value: '16',
            label: 'Full HD (1080p60)'
          },
          {
            value: '31',
            label: 'Full HD (1080p50)'
          },
          {
            value: '74',
            label: 'Ultra HD 2160p30'
          },
          {
            value: '73',
            label: 'Ultra HD 2160p25, etc.'
          }
        ]
      },
      delay: {
        'sleep delay on signal loss sec': 0,
        'shutdown delay on signal loss sec': 0,
        'wake-up delay on signal detection sec': 0
      },
      avSignal: {
        'input maximum resolution': 'Pass Through',
        'maximum bit rate': 'Best Effort',
        'frame rate percentage': 100,
        'color depth': 'Follow Output',
        'audio connection guard time sec': 0,
        'dante vlan tag': ''
      }
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#KDS-SCALE? ')
    this.$socket.sendMsg('#CS-CONVERT? 1')
    this.getAVSignal()
    this.getDisplayDelay()
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@X-ROUTE /i) !== -1) {
        this.handleInputSelect(msg)
        return
      }
      if (msg.search(/@CS-CONVERT /i) !== -1) {
        this.handleForceRGB(msg)
      }
      if (msg.search(/@KDS-SCALE /i) !== -1) {
        this.handleResolution(msg)
      }

    },
    clickUpload () {
      this.$refs.upload.click()
    },
    handleForceRGB (msg) {
      this.forceRGB = msg.split(',')[1]
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
    handleResolution (data) {
      this.maxResolution.val = data.split(',').pop()
    },
    setDisplayDelay () {
      this.$http.post('/display/set_display_sleep', {
        'Display Delays': this.delay
      })
    },
    save () {
      this.$socket.sendMsg(`#KDS-SCALE ${this.maxResolution.val === '0' ? 0 : 1},${this.maxResolution.val}`)
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
.setting-model{
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
