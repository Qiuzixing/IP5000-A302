<template>
  <div class="main-setting">

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
