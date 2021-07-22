<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">HTTPS</h3>
      <div class="setting">
        <span class="setting-title">Server</span>
        <v-switch></v-switch>
      </div>
      <div>
        <radio-component v-model="https" label="1">Internal Certificate</radio-component>
        <radio-component v-model="https" label="0">Server Certificate</radio-component>
        <div v-if="https === '0'" style="margin-left: 30px">
          <div class="setting">
            <span class="setting-title" style="width: 180px;">Upload Certificate</span>
            <input type="text" class="setting-text" readonly>
            <img class="upload-icon" src="../../assets/img/Upload.svg" alt="">
          </div>
          <div class="setting">
            <span class="setting-title" style="width: 180px;">Private Key Password</span>
            <input type="text" class="setting-text">
          </div>
        </div>
        <button class="btn btn-primary" style="margin-left: 30px">APPLY & REBOOT</button>
      </div>
    </div>
    <div class="setting-model">
      <h3 class="setting-model-title">802.1x</h3>
      <div class="setting">
        <span class="setting-title">IEE 802.1x Authentication</span>
        <v-switch></v-switch>
      </div>
      <div>
        <radio-component v-model="security801" label="1">EAP-MSCHAP V2
        </radio-component>
        <div v-if="security801 === '1'" style="margin-left: 30px">
          <div class="setting">
            <span class="setting-title" style="width: 180px;">Username</span>
            <input type="text" class="setting-text">
          </div>
          <div class="setting" >
            <span class="setting-title" style="width: 180px;">Password</span>
            <input type="password" class="setting-text">
          </div>
        </div>
        <radio-component v-model="security801" label="0">EAP-TLS</radio-component>
        <div v-if="security801 === '0'" style="margin-left: 30px">
          <div class="setting">
            <span class="setting-title" style="width: 180px;">Username</span>
            <input type="text" class="setting-text">
          </div>
          <div class="setting">
            <span class="setting-title" style="width: 180px;">CA Certificate</span>
            <input type="text" class="setting-text" readonly>
            <img class="upload-icon" src="../../assets/img/Upload.svg" alt="">
          </div>
          <div class="setting">
            <span class="setting-title" style="width: 180px;">Client Certificate</span>
            <input type="text" class="setting-text" readonly>
            <img class="upload-icon" src="../../assets/img/Upload.svg" alt="">
          </div>
          <div class="setting">
            <span class="setting-title" style="width: 180px;">Private Key</span>
            <input type="text" class="setting-text" readonly>
            <img class="upload-icon" src="../../assets/img/Upload.svg" alt="">
          </div>
          <div class="setting">
            <span class="setting-title" style="width: 180px;">Private Key Password</span>
            <input type="text" class="setting-text">
          </div>
        </div>
        <button class="btn btn-primary" style="margin-left: 30px">APPLY</button>
      </div>
    </div>
  </div>
</template>

<script>
import radioComponent from '@/components/radio.vue'
export default {
  name: 'autoSwitch',
  components: {
    radioComponent
  },
  data () {
    return {
      security801: '0',
      https: '0'
    }
  },
  mounted () {
  },
  methods: {
    checkBitmapFile (file) {
      console.log(file)
    }
  }
}
</script>
<style lang="less" scoped>
.setting-title{
  width: 220px;
}
.upload-icon{
  cursor: pointer;
  width: 26px;
}
</style>
