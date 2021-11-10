<template>
  <div class="main-setting">
    <div class="overlay">
      <div>
        <v-collapse title="Overlay settings">
          <div class="overlay-setting">
            <span class="overlay-title">Display Overlay</span>
            <button class="btn btn-plain-primary">START</button>
            <button class="btn btn-plain-primary"
                    style="margin-left:15px;">STOP</button>
          </div>
        </v-collapse>
        <v-collapse title="Image settings">
          <div class="overlay-setting">
            <span class="overlay-title">Image</span>
            <div class="overlay-setting-item overlay-img">
              <span class="file-name"
                    style="display: inline-block;">logo.png</span>
              <span class="upload-icon"
                    @click="clickUpload">
                <icon-svg icon-class="upload_img" />
              </span>
              <input type="file"
                     ref="upload"
                     style="display: none;">
            </div>
          </div>
          <div class="overlay-setting">
            <span class="overlay-title">Align</span>
            <div class="overlay-setting-item">
              <multiselect v-model="imgPosition"
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
                         v-model="imgTransparency"
                         :marks="marks">
              </el-slider>
              <span style="margin-left: 15px;">{{ imgTransparency }}</span>
            </div>
          </div>
          <div class="overlay-setting"
               style="margin-top: 24px;">
            <span class="overlay-title">Show</span>
            <v-checkbox v-model="showImg"></v-checkbox>
          </div>
        </v-collapse>
        <v-collapse title="Text settings">
          <div class="overlay-setting">
            <span class="overlay-title">Text</span>
            <div class="overlay-setting-item">
              <input type="text"
                     style="width: 100%;"
                     class="setting-text"
                     value="Next presentation will start at 14:00">
            </div>
          </div>
          <div class="overlay-setting">
            <span class="overlay-title">Size</span>
            <div class="fontSize.val">
              <multiselect v-model="fontSize.val"
                           :options="fontSize.param"></multiselect>
            </div>
          </div>
          <div class="overlay-setting">
            <span class="overlay-title">Align</span>
            <div class="overlay-setting-item">
              <multiselect v-model="textAlign"
                           :options="horizontalPosition.param"></multiselect>
            </div>
          </div>
          <div class="overlay-setting">
            <span class="overlay-title">Color</span>
            <div class="overlay-setting-item"
                 style="position: relative">
              <div class="color-desc">
                <p style="margin: 0"><span class="color-box"
                        :style="{'background': boxColor}"></span>
                  {{colorHex}}</p>
                <color-picker v-model="color"
                              color-format="rgb"
                              :show-alpha="false"
                              @change="closeColor"></color-picker>
              </div>
              <!--              <v-color v-model="color" v-show="showColor" ref="colorPicker" class="color-picker"></v-color>-->

            </div>

          </div>
          <div class="overlay-setting">
            <span class="overlay-title">Transparency (%)</span>
            <div class="overlay-setting-item">
              <el-slider style="width: 200px;"
                         :min="0"
                         :max="100"
                         :show-tooltip="false"
                         v-model="textTransparency"
                         :marks="marks">
              </el-slider>
              <span style="margin-left: 15px">{{ textTransparency }}</span>
            </div>
          </div>
          <div class="overlay-setting"
               style="margin-top: 24px;">
            <span class="overlay-title">Show</span>
            <v-checkbox v-model="showText"></v-checkbox>
          </div>
        </v-collapse>
      </div>
      <!-- <div class="overlay-preview">
        <h3 class="preview-title">Preview</h3>
        <img src="../../assets/img/img.png"
             alt="">
      </div> -->
    </div>
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
      num: 61,
      marks: {
        0: '0',
        100: '100'
      },
      showText: true,
      showImg: true,
      show: false,
      imgPosition: 'left_top',
      textAlign: 'left_top',
      horizontalPosition: {
        param: [
          {
            value: 'left_top',
            label: 'Left top'
          },
          {
            value: 'left_centered',
            label: 'Left center'
          },
          {
            value: 'left_bottom',
            label: 'Left bottom'
          },
          {
            value: 'centered_top',
            label: 'Center top'
          },
          {
            value: 'centered_centered',
            label: 'Center center'
          },
          {
            value: 'centered_bottom',
            label: 'Center bottom'
          },
          {
            value: 'right_top',
            label: 'Right top'
          },
          {
            value: 'right_centered',
            label: 'Right center'
          },
          {
            value: 'right_bottom',
            label: 'Right bottom'
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
      displayOverlay: 'off',
      overLayTransparency: 50,
      imgTransparency: 50,
      textTransparency: 50,
      color: '#fff',
      boxColor: '#fff',
      colorHex: '255,255,255',
      showColor: false
    }
  },
  mounted () {
  },
  methods: {
    checkBitmapFile (file) {
      console.log(file)
    },
    clickUpload () {
      this.$refs.upload.click()
    },
    closeColor (color) {
      this.boxColor = color
      this.colorHex = color.match(/[^(\\)]+(?=\))/g)[0] || ''
    }
  }
}
</script>
<style lang="less" scoped>
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
