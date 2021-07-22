<template>
  <div class="main-setting">
    <div class="setting">
      <span class="setting-title">Default EDID</span>
      <v-model-select :options="edid.param"  v-model="edid.val"></v-model-select>
    </div>
    <div class="setting">
      <span class="setting-title">Custom EDID</span>
      <upload-components :show="false">Browse</upload-components>
    </div>
    <div class="setting">
      <span class="setting-title">Use EDID Received From Specific Decoder</span>
      <v-model-select :options="['???']"></v-model-select>
    </div>
    <div class="radio-setting">
      <span class="setting-title">Copy EDID from Input</span>
      <div>
        <radio-component v-model="test" label="1">1</radio-component>
        <radio-component v-model="test" label="0">2</radio-component>
      </div>
    </div>
  </div>
</template>

<script>
import uploadComponents from '@/components/Upload.vue'
import radioComponent from '@/components/radio.vue'
export default {
  name: 'edidManage',
  components: {
    uploadComponents,
    radioComponent
  },
  data () {
    return {
      edid: {
        val: '4K. Multi-channel',
        param: [
          { value: '4K. Multi-channel', label: '4K. Multi-channel' },
          { value: '2.0ch080P', label: '2.0ch080P' },
          { value: 'multi-ch', label: 'multi-ch' },
          { value: ' DVI', label: ' DVI' },
          { value: 'VGA', label: 'VGA' }
        ]
      },
      test: '1'
    }
  },
  mounted () {
  }
}
</script>
<style lang="less" scoped>
.main-setting .setting-title {
  width: 350px;
}
.vs__dropdown-toggle{
  width: 180px;
}
</style>
