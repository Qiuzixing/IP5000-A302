<template>
  <div class="main-setting">
    <div class="setting-content">
      <div class="setting-model"
           style="margin-bottom: 0">
        <h3 class="setting-model-title">Input HDCP Appearance</h3>
        <div class="setting">
          <span class="setting-title">Input 1</span>
          <v-switch v-model="hdcp[0]"
                    active-value="1"
                    inactive-value="0"
                    @change="setHDCP($event, 1)"></v-switch>
        </div>
        <div class="setting"
             v-if="this.$global.deviceType">
          <span class="setting-title">Input 2</span>
          <v-switch v-model="hdcp[1]"
                    active-value="1"
                    inactive-value="0"
                    @change="setHDCP($event, 2)"></v-switch>
        </div>
        <div class="setting"
             v-if="this.$global.deviceType === 1">
          <span class="setting-title">Input 3</span>
          <v-switch v-model="hdcp[2]"
                    active-value="1"
                    inactive-value="0"
                    @change="setHDCP($event, 3)"></v-switch>
        </div>
        <div class="setting"
             v-if="this.$global.deviceType">
          <span class="setting-title">ALL</span>
          <button class="btn btn-plain-primary"
                  @click="setAllHDCP('1')">ON</button>
          <button class="btn btn-plain-primary"
                  @click="setAllHDCP('0')"
                  style="margin-left: 25px;">OFF</button>
        </div>
      </div>
      <div class="setting-model" style="margin-top: 50px;">
        <div class="radio-setting"
             style="margin-bottom: 24px;">
          <span class="setting-title">Image Preview</span>
          <img src="/stream"
               alt=""
               style="max-width: 320px;">
        </div>
        <div class="setting">
          <span class="setting-title">Maximum Bit Rate</span>
          <el-select v-model="avSignal.maximum_bit_rate">
            <el-option
              v-for="item in bitRateParam"
              :key="item.value"
              :label="item.label"
              :value="item.value">
            </el-option>
          </el-select>
        </div>
        <div class="setting">
          <span class="setting-title">Maximum Video Frame Rate (%)</span>
          <el-slider style="width: 200px"
                     :min="0"
                     :max="100"
                     :show-tooltip="false"
                     v-model="avSignal.frame_rate_percentage"
                     :marks="marks">
          </el-slider>
          <span style="margin-left: 15px">{{avSignal.frame_rate_percentage}}</span>
        </div>
      </div>
    </div>
    <footer><button class="btn btn-primary"
              @click="save">SAVE</button></footer>
  </div>
</template>

<script>
import { debounce } from 'lodash'
export default {
  name: 'audioPage',
  data () {
    return {
      forceRGB: '0',
      maxBitRate: 50,
      marks: {
        0: '0',
        100: '100'
      },
      avSignal: {
        audio_connection_guard_time_sec: 90,
        color_depth: 'bypass',
        force_rgb: 'off',
        frame_rate_percentage: 100,
        input_maximum_resolution: 'pass_through',
        maximum_bit_rate: 'best_effort'
      },
      delay: {
        shutdown_delay_on_signal_loss_sec: 10,
        sleep_delay_on_signal_loss_sec: 5,
        wake_up_delay_on_signal_detection_sec: 10
      },
      hdcp: ['0', '0', '0'],
      bitRateParam: [
        { value: 'best_effort', label: 'Best Effort' },
        { value: '200', label: '200 Mbps' },
        { value: '150', label: '150 Mbps' },
        { value: '100', label: '100 Mbps' },
        { value: '50', label: '50 Mbps' },
        { value: '10', label: '10 Mbps' }
      ],
      imgError: false,
      imgName: '',
      uploadComplete: false
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#HDCP-MOD? 1')
    if (this.$global.deviceType) {
      this.$socket.sendMsg('#HDCP-MOD? 2')
      if (this.$global.deviceType === 1) {
        this.$socket.sendMsg('#HDCP-MOD? 3')
      }
    }
    this.getAVSignal()
    // this.getDisplayDelay()
  },
  methods: {
    handleMsg (msg) {
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
      this.hdcp.splice(parseInt(data[0]) - 1, 1, data[1])
    },
    setAllHDCP (val) {
      this.$socket.sendMsg(`#HDCP-MOD 1,${val}`)
      this.$socket.sendMsg(`#HDCP-MOD 2,${val}`)
      if (this.$global.deviceType === 1) {
        this.$socket.sendMsg(`#HDCP-MOD 3,${val}`)
      }
    },
    setHDCP (val, index) {
      this.$socket.sendMsg(`#HDCP-MOD ${index},${val}`)
    },
    clickUpload () {
      this.$refs.upload.click()
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
    setAVSingle () {
      this.$http.post('/device/json', {
        path: '/av_signal/av_signal.json',
        info: {
          av_signal: this.avSignal
        }
      }).then(() => {
        this.$msg.successAlert()
      })
    },
    getDisplayDelay () {
      this.$http
        .get(
          '/device/json?path=/display/display_sleep.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.display_delays) {
            this.delay = msg.data.display_delays
          }
        })
    },
    setDisplayDelay () {
      this.$http.post('/device/json', {
        path: '/display/display_sleep.json',
        info: {
          display_delays: this.delay
        }
      })
    },
    handleForceRGB (msg) {
      this.forceRGB = msg.split(',')[1]
    },
    save: debounce(function () {
      // this.setDisplayDelay()
      this.setAVSingle()
    }, 2000, {
      leading: true,
      trailing: true
    }),
    browseImg (event) {
      this.imgName = event.target.files[0]?.name || ''
      if (this.imgName) {
        const file = event.target.files[0]
        if (file.type !== 'image/jpeg') {
          this.imgError = true
          return
        }
        const reader = new FileReader()
        reader.onload = e => {
          const data = e.target.result
          // 加载图片获取图片真实宽度和高度
          const image = new Image()
          image.onload = () => {
            const width = image.width
            const height = image.height
            if (!(width === 1280 && height === 720)) {
              this.imgError = true
            } else {
              this.imgError = false
              const xhr = new XMLHttpRequest()
              const formData = new FormData()
              formData.append('file', file)
              xhr.open('POST', '/upload/sleepimage')
              xhr.onload = () => {
                if (xhr.status === 200) {
                  this.imgName = ''
                  this.uploadComplete = true
                  setTimeout(() => {
                    this.uploadComplete = false
                  }, 2000)
                }
              }
              xhr.send(formData)
            }
          }
          image.src = data
        }
        reader.readAsDataURL(file)
      } else {
        this.imgError = false
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
.setting-content {
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
    flex: 1;
  }
  .overlay-title {
    width: 176px;
    font-family: "open sans semiblold", -apple-system, BlinkMacSystemFont,
      "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
  }

  .overlay-img {
    position: relative;

    .file-name {
      width: 180px;
      height: 30px;
      line-height: 30px;
      padding-right: 25px;
      box-sizing: border-box;
      border-bottom: 1px solid #4d4d4f;
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
