<template>
  <div class="main-setting">
    <div class="setting">
      <span class="setting-title">HDMI Output</span>
      <v-switch></v-switch>
    </div>
    <div class="radio-setting">
      <span class="setting-title">HDMI Output Setting</span>
      <div>
        <radio-component v-model="test" label="1">Separate A/V Settings</radio-component>
        <radio-component v-model="test" label="0">LAN AV Settings</radio-component>
      </div>
    </div>
  </div>
</template>

<script>
import radioComponent from '@/components/radio.vue'
export default {
  name: 'loopThrough',
  components: {
    radioComponent
  },
  data () {
    return {
      test: '1'
    }
  },
  mounted () {
  }
}
</script>
<style lang="less" scoped>
.setting-title{
  width: 200px;
}
</style>
