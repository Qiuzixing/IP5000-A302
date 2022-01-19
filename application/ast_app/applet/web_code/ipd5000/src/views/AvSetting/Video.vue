<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Maximum Resolution</span>
        <el-select placeholder=" " v-model="maxResolution.val">
          <el-option
            v-for="item in maxResolution.param"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <div class="setting-model">
        <h3 class="setting-model-title">Display</h3>
        <div class="setting">
          <span class="setting-title">Sleep (5V-off) Delay On Video Signal Loss (sec)</span>
          <el-input-number v-model="delay.sleep_delay_on_signal_loss_sec"
                           controls-position="right"
                           :max="90"
                           :min="0"></el-input-number>
        </div>
        <div class="setting">
          <span class="setting-title">Shutdown (CEC) Delay On Video Signal Loss (sec)</span>
          <el-input-number v-model="delay.shutdown_delay_on_signal_loss_sec"
                           controls-position="right"
                           :max="90"
                           :min="0"></el-input-number>
        </div>
        <div class="setting">
          <span class="setting-title">Wake-up (CEC) Delay On Video Signal Detection (sec)</span>
          <el-input-number v-model="delay.wake_up_delay_on_signal_detection_sec"
                           controls-position="right"
                           :max="90"
                           :min="0"></el-input-number>
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
            <span class="file-name"
                  style="display: inline-block;overflow: hidden;">{{imgName}}</span>
            <span class="upload-icon"
                  @click="clickUpload">
              <icon-svg icon-class="upload_img" />
            </span>
            <input type="file"
                   accept="image/jpeg"
                   @change="browseImg"
                   ref="upload"
                   style="display: none;">
            <span class="range-alert"
                  v-if="imgError"
                  style="white-space: nowrap;">The image format must be JPEG (1280 x 720)</span>
            <span v-if="uploadComplete"
                  style="font-size:20px;margin-left:15px;color:#67c23a;"><i class="el-icon-circle-check"></i></span>
          </div>
        </div>
      </div>
      <div class="radio-setting"
           style="margin-bottom: 24px;">
        <span class="setting-title">Image Preview</span>
        <img src="/stream"
             alt=""
             style="max-width: 320px;">
      </div>
      <!-- <div class="setting">
        <span class="setting-title">Force 8-bit Color Depth</span>
        <v-checkbox v-model="avSignal.color_depth"
                    active-value="8bit"
                    inactive-value="bypass"></v-checkbox>
      </div> -->
      <div class="setting">
        <span class="setting-title">Force RGB</span>
        <v-checkbox v-model="forceRGB"
                    active-value="1"
                    inactive-value="0"></v-checkbox>
      </div>
    </div>
    <footer><button class="btn btn-primary"
              @click="save">SAVE</button></footer>
  </div>
</template>

<script>
import vCheckbox from '@/components/checkbox.vue'
import { debounce } from 'lodash'
export default {
  name: 'audioPage',
  components: {
    vCheckbox
  },
  data () {
    return {
      imgName: '',
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
            value: '73',
            label: 'Ultra HD 2160p25'
          },
          {
            value: '74',
            label: 'Ultra HD 2160p30'
          }
        ]
      },
      delay: {
        sleep_delay_on_signal_loss_sec: 5,
        shutdown_delay_on_signal_loss_sec: 10,
        wake_up_delay_on_signal_detection_sec: 10
      },
      imgError: false,
      uploadComplete: false,
      saveFlag: false
      // avSignal: {
      // }
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#KDS-SCALE? ')
    this.$socket.sendMsg('#CS-CONVERT? 1')
    // this.getAVSignal()
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
      if (this.saveFlag) {
        this.saveFlag = false
        this.$msg.successAlert()
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
    setAVSingle () {
      this.$http.post('/device/json', {
        path: '/av_signal/av_signal.json',
        info: {
          av_signal: this.avSignal
        }
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
    handleResolution (data) {
      this.maxResolution.val = data.split(',').pop()
    },
    setDisplayDelay () {
      this.$http.post('/device/json', {
        path: '/display/display_sleep.json',
        info: {
          display_delays: this.delay
        }
      })
    },
    save: debounce(function () {
      this.saveFlag = true
      this.$socket.sendMsg(`#KDS-SCALE ${this.maxResolution.val === '0' ? 0 : 1},${this.maxResolution.val}`)
      this.setDisplayDelay()
      // this.setAVSingle()
      this.$socket.sendMsg(`#CS-CONVERT 1,${this.forceRGB}`)
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
              xhr.onload = oevent => {
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
.setting-model {
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
