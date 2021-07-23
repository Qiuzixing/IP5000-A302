<template>
  <div class="main-setting">
    <div class="setting-model">
      <div>
        <span class="setting-title" style="margin-right: 15px;">Horizontal</span>
        <el-input-number style="width: 50px;margin-right: 30px;" v-model="col" :max="16" :min="1" controls-position="right"  class="video-wall-child"></el-input-number>
        <span class="setting-title" style="margin-right: 15px;">Vertical</span>
        <el-input-number v-model="row" :max="16" :min="1" style="width: 50px;" @change="checkSelectId" class="video-wall-child" controls-position="right"></el-input-number>
      </div>
      <p class="setting-title" style="margin: 15px 0 0;">Layout</p>
      <div class="video-wall">
          <div class="video-wall-row" v-for="rowItem in row" :key="rowItem">
            <div class="video-wall-col" @click="selectedId = (rowItem - 1) * col + colItem" :class="{'active': (rowItem - 1) * col + colItem === selectedId}" v-for="colItem in col"  :key="colItem">
              <span>Screen {{ (rowItem - 1) * col + colItem}}</span>
            </div>
          </div>
      </div>
      <div class="setting">
        <span class="setting-title">Bezel Compensation</span>
        <el-input-number v-model="bezel" :max="100" :min="0" controls-position="right" @blur="checkBlur"></el-input-number>
      </div>
      <div class="radio-setting">
        <span class="setting-title">Stretch Type</span>
        <div>
          <radio-component v-model="stretchType" label="0">Fit In</radio-component>
          <radio-component v-model="stretchType" label="1">Fit Out</radio-component>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Video Rotation</span>
        <multiselect v-model="videoRotation" :options="rotationParam"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Video Wall Test</span>
        <v-switch open-text="Start" close-text="Stop"></v-switch>
      </div>
    </div>
    <footer><button class="btn btn-primary" @click="save">SAVE</button></footer>
  </div>
</template>

<script>
import radioComponent from '@/components/radio.vue'
export default {
  name: 'videoWall',
  components: {
    radioComponent
  },
  data () {
    return {
      stretchType: '0',
      roaming: '0',
      bezel: '0',
      roamingMode: false,
      videoRotation: '0',
      col: 2,
      row: 2,
      selectedId: 1,
      rotationParam: [
        { value: '0', label: '0' },
        { value: '1', label: '90' },
        { value: '2', label: '180' },
        { value: '3', label: '270' }
      ]
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#VIEW-MOD? ')
    this.$socket.sendMsg('#VIDEO-WALL-SETUP? ')
    this.$socket.sendMsg('#WND-BEZEL? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@VIEW-MOD /i) !== -1) {
        this.handleViewMode(msg)
        return
      }
      if (msg.search(/@VIDEO-WALL-SETUP /i) !== -1) {
        this.handleSelectedLayoutRotation(msg)
        return
      }
      if (msg.search(/@WND-STRETCH /i) !== -1) {
        this.handleStretch(msg)
        return
      }
      if (msg.search(/@WND-BEZEL /i) !== -1) {
        this.handleBEZEL(msg)
      }
    },
    handleViewMode (data) {
      const arr = data.split(',')
      this.col = +arr[1]
      this.row = +arr[2]
    },
    handleSelectedLayoutRotation (data) {
      const arr = data.split(' ')[1].split(',')
      this.selectedId = +arr[0]
      this.videoRotation = arr[1]
      this.$socket.sendMsg('#WND-STRETCH? ' + this.selectedId)
    },
    handleStretch (data) {
      this.stretchType = data.split(' ')[1].split(',')[1]
    },
    handleBEZEL (data) {
      this.bezel = +data.split(',')[2]
    },
    checkBlur () {
      this.bezel = this.bezel || 0
    },
    checkSelectId () {
      if (this.selectedId > this.col * this.row) {
        this.selectedId = 1
      }
    },
    save () {
      this.$socket.sendMsg(`#VIEW-MOD 15,${this.col},${this.row}`)
      this.$socket.sendMsg(`#VIDEO-WALL-SETUP ${this.selectedId},${this.videoRotation}`)
      this.$socket.sendMsg(`#WND-STRETCH ${this.selectedId},${this.stretchType}`)
      this.$socket.sendMsg(`#WND-BEZEL 0,1,${this.bezel},${this.bezel},${this.bezel},${this.bezel}`)
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
.setting-title{
  width: 180px;
}
table{
  border: 0;
  padding: 0;
  margin: 0;
  th,td{
    border: 0;
    text-align: left;
    padding: 0;
    margin: 0;
  }
  .title{
    font-family: 'open sans bold';
    color: #4D4D4F;
    font-size: 15px;
  }
}
.video-wall-setting{
  display: flex;
  flex-direction: column;
  .video-wall-child{
    height: 33px;
    margin-bottom: 24px;
    //vertical-align: middle;
  }
  .setting-title{
    line-height: 33px;
  }
}
.video-wall{
  margin: 15px 0;
  display: inline-block;
  max-width: 968px;
  max-height: 480px;
  overflow: auto;
  border: 1px solid #4D4D4F;

}
.video-wall-row{
  display: flex;
  .video-wall-col{
    font-family: 'open sans semiblold', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
    width: 160px;
    height: 90px;
    padding-top: 5px;
    padding-left: 5px;
    flex-shrink: 0;
    border: 1px solid #4D4D4F;
    box-sizing: border-box;
    background: #fff;
    cursor: pointer;
    user-select: none;
    &.active{
      background: rgba(53, 172, 248, 0.7);
      //color: #fff;
    }
  }
}
</style>
