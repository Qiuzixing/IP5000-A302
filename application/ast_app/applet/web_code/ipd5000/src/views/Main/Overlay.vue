<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="overlay">
        <div>
          <v-collapse title="Image settings">
            <div class="overlay-setting">
              <span class="overlay-title">Image</span>
              <div class="overlay-setting-item overlay-img">
                <span class="file-name"
                      style="display: inline-block;overflow: hidden;">{{ imgName }}</span>
                <span class="
                    upload-icon"
                      @click="clickUpload">
                  <icon-svg icon-class="upload_img"/>
                </span>
                <input type="file"
                       ref="upload"
                       @change="browseImg"
                       accept="image/png"
                       style="display: none;">
                <span class="range-alert"
                      v-if="imgError"
                      style="white-space: nowrap;">{{ imgErrorMsg }}</span>
                <span v-if="uploadComplete"
                      style="font-size:20px;margin-left:15px;color:#67c23a;"><i class="el-icon-circle-check"></i></span>
              </div>
            </div>
            <div class="overlay-setting">
              <span class="overlay-title">Align</span>
              <div class="overlay-setting-item">
                <multiselect v-model="imageInfo.objects[0].position"
                             :options="horizontalPosition.param"></multiselect>
              </div>
            </div>
            <div class="overlay-setting">
              <span class="overlay-title">Transparency (%)</span>
              <div class="overlay-setting-item">
                <el-slider style="width: 200px;display: inline-block;"
                           :min="0"
                           :max="100"
                           :show-tooltip="false"
                           v-model="imageInfo.genral.transparency"
                           :marks="marks">
                </el-slider>
                <span style="margin-left: 15px;">{{ imageInfo.genral.transparency }}</span>
              </div>
            </div>
            <div class="overlay-setting"
                 style="margin-top: 24px;">
              <span class="overlay-title">Show</span>
              <v-checkbox v-model="imageInfo.genral.enable"
                          active-value="on"
                          @click="setShow($event , 1)"
                          inactive-value="off"></v-checkbox>
            </div>
          </v-collapse>
          <v-collapse title="Text settings">
            <div class="overlay-setting">
              <span class="overlay-title">Text</span>
              <div class="overlay-setting-item">
                <input type="text"
                       style="width: 100%;"
                       class="setting-text"
                       v-model="textInfo.objects[0].caption">
              </div>
            </div>
            <div class="overlay-setting">
              <span class="overlay-title">Size</span>
              <div class="fontSize.val">
                <multiselect v-model="textInfo.objects[0].size"
                             :options="fontSize.param"></multiselect>
              </div>
            </div>
            <div class="overlay-setting">
              <span class="overlay-title">Align</span>
              <div class="overlay-setting-item">
                <multiselect v-model="textInfo.objects[0].position"
                             :options="horizontalPosition.param"></multiselect>
              </div>
            </div>
            <div class="overlay-setting">
              <span class="overlay-title">Color</span>
              <div class="overlay-setting-item"
                   style="position: relative">
                <div class="color-desc">
                  <p style="margin: 0"><span class="color-box"
                                             :style="{'background': color}"></span>
                    {{ color }}</p>
                  <color-picker v-model="color"
                                color-format="hex"
                                :show-alpha="false"></color-picker>
                </div>
              </div>

            </div>
            <div class="overlay-setting">
              <span class="overlay-title">Transparency (%)</span>
              <div class="overlay-setting-item">
                <el-slider style="width: 200px;"
                           :min="0"
                           :max="100"
                           :show-tooltip="false"
                           v-model="textInfo.genral.transparency"
                           :marks="marks">
                </el-slider>
                <span style="margin-left: 15px">{{ textInfo.genral.transparency }}</span>
              </div>
            </div>
            <div class="overlay-setting"
                 style="margin-top: 24px;">
              <span class="overlay-title">Show</span>
              <v-checkbox v-model="textInfo.genral.enable"
                          @click="setShow($event , 2)"
                          active-value="on"
                          inactive-value="off"></v-checkbox>
            </div>
          </v-collapse>
        </div>

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
import vCollapse from '@/components/v-collapse'
import vCheckbox from '@/components/checkbox.vue'

export default {
  name: 'autoSwitch',
  components: {
    vCollapse,
    vCheckbox
  },
  data () {
    return {
      imgError: false,
      imgErrorMsg: '',
      uploadComplete: false,
      imgName: '',
      imageInfo: {
        genral:
          {
            enable: 'on',
            timeout: 2,
            transparency: 0
          },
        objects: [
          {
            type: 'image',
            position: 'left_top'
          }
        ]
      },
      textInfo: {
        genral:
          {
            enable: 'off',
            timeout: 2,
            transparency: 0
          },
        objects: [
          {
            type: 'text',
            position: 'left_top',
            caption: '',
            size: 'small',
            color: '0xffffff'
          }
        ]
      },
      num: 61,
      marks: {
        0: '0',
        100: '100'
      },
      show: false,
      horizontalPosition: {
        param: [
          {
            value: 'top_left',
            label: 'Top left'
          },
          {
            value: 'top_right',
            label: 'Top right'
          },
          {
            value: 'top_center',
            label: 'Top center'
          },
          {
            value: 'bottom_left',
            label: 'Bottom left'
          },
          {
            value: 'bottom_right',
            label: 'Bottom right'
          },
          {
            value: 'bottom_center',
            label: 'Bottom center'
          },
          {
            value: 'left',
            label: 'Left'
          },
          {
            value: 'right',
            label: 'Right'
          },
          {
            value: 'center',
            label: 'Center'
          }
        ]
      },
      fontSize: {
        val: 'small',
        param: [
          {
            value: 'small',
            label: 'Small'
          },
          {
            value: 'medium',
            label: 'Medium'
          },
          {
            value: 'large',
            label: 'Large'
          }
        ]
      },
      color: '#FFFFFF'
    }
  },
  created () {
    this.getTextInfo()
    this.getImgInfo()
  },
  methods: {
    showOverlay (start) {
      if (start) {
        const msg = this.imageInfo.genral.enable === 'on' ? 'overlay2_setting' : 'overlay1_setting'
        this.$socket.sendMsg(`#KDS-START-OVERLAY ${msg},0`)
      } else {
        this.$socket.sendMsg('#KDS-STOP-OVERLAY ')
      }
    },
    clickUpload () {
      this.$refs.upload.click()
    },
    browseImg (event) {
      this.imgName = event.target.files[0]?.name || ''
      if (this.imgName) {
        const file = event.target.files[0]
        if (file.size > 1024 * 512) {
          this.imgErrorMsg = 'The file size is less than 512KB '
          this.imgError = true
          return
        }
        if (file.type !== 'image/png') {
          this.imgErrorMsg = 'The image format must be PNG'
          this.imgError = true
          return
        }
        this.imgError = false
      } else {
        this.imgError = false
      }
    },
    setShow (e, num) {
      if (num === 1) {
        if (e === 'on' && this.textInfo.genral.enable === 'on') {
          this.textInfo.genral.enable = 'off'
        }
      } else {
        if (e === 'on' && this.imageInfo.genral.enable === 'on') {
          this.imageInfo.genral.enable = 'off'
        }
      }
    },
    getTextInfo () {
      this.$http
        .get(
          '/device/json?path=/overlay/overlay1_setting.json&t=' + Math.random()
        )
        .then(msg => {
          this.textInfo = msg.data
          this.color = msg.data.objects[0].color.replace('0x', '#')
        })
    },
    getImgInfo () {
      this.$http
        .get(
          '/device/json?path=/overlay/overlay2_setting.json&t=' + Math.random()
        )
        .then(msg => {
          this.imageInfo = msg.data
        })
    },
    save () {
      this.saveTextInfo()
      this.saveImgInfo()
      this.saveImg()
    },
    saveTextInfo () {
      this.textInfo.objects[0].color = this.color.replace('#', '0x')
      this.$http.post('/device/json', {
        path: '/overlay/overlay1_setting.json',
        info: this.textInfo
      })
    },
    saveImgInfo () {
      this.$http.post('/device/json', {
        path: '/overlay/overlay2_setting.json',
        info: this.imageInfo
      })
    },
    saveImg () {
      if (!this.imgError && this.imgName) {
        const file = this.$refs.upload.files[0]
        const xhr = new XMLHttpRequest()
        const formData = new FormData()
        formData.append('file', file)
        xhr.open('POST', '/upload/overlayimage')
        xhr.onload = () => {
          if (xhr.status === 200) {
            this.imgName = ''
            this.$refs.upload.value = ''
            this.uploadComplete = true
            setTimeout(() => {
              this.getImgInfo()
              this.uploadComplete = false
            }, 1000)
          }
        }
        xhr.send(formData)
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

.overlay {
  display: flex;
}

.overlay-preview {
  padding: 5px;
  margin-left: 61px;
}

.preview-title {
  margin: 0 0 20px 0;
  padding: 0;
  font-family: "open sans bold";
  font-size: 15px;
}

.overlay-setting {
  display: flex;
  margin-bottom: 15px;
  align-items: center;

  .overlay-setting-item {
    flex: 1;
    display: flex;
    align-items: center;
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

.color-picker {
  position: absolute;
  left: 0;
  /* top: -100%; */
  bottom: 33px;
  z-index: 10;
}

.color-desc {
  display: flex;
  justify-content: space-between;
  width: 180px;
  //height: 33px;
  //line-height: 33px;
  border-bottom: 1px solid #4d4d4f;
  margin: 0;
}

.color-box {
  display: inline-block;
  width: 16px;
  height: 16px;
  border: 1px solid #6e6e6e;
  margin-right: 5px;
  vertical-align: middle;
}
</style>
