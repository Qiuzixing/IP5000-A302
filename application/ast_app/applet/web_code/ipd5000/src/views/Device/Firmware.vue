<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">Version</h3>
      <div class="setting" style="margin-bottom: 24px">
        <span class="setting-title">Firmware Version</span>
        <span>01.01.5556</span>
        <upload-components :show="false">Upgrade</upload-components>
      </div>
      <div class="setting">
        <span class="setting-title">Last Upgrade Date/Time</span>
        <span>2021.04.22 12:00:00</span>
      </div>
      <div class="setting">
        <span class="setting-title">Firmware Standby Version  </span>
        <span>01.01.5555</span>
      </div>
    </div>
    <vodal :show="show" animation="fade" @hide="show = false" :width="480">
      <div class="vodal-model">
        <div class="vodal-header">Firmware Upgrade</div>
        <div class="vodal-body">
          <upload-components ref="test" @change="checkBitmapFile"></upload-components>
        </div>
        <div class="vodal-footer">
          <button class="btn btn-primary" @click="show = false">Upgrade</button>
          <button class="btn btn-primary" @click="show = false">Cancel</button>
        </div>
      </div>
    </vodal>
  </div>
</template>

<script>
import uploadComponents from '@/components/Upload.vue'
export default {
  name: 'autoSwitch',
  components: {
    uploadComponents
  },
  data () {
    return {
      show: false
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

</style>
