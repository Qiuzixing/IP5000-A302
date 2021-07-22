<template>
  <div class="setting-path">
    {{navMsg}}
  </div>
</template>

<script>
export default {
  name: 'settingNav',
  data () {
    return {
      navMsg: ''
    }
  },
  methods: {
    checkRoute () {
      this.navMsg = this.$route.meta.desc
    }
  },
  created () {
    this.checkRoute()
  },
  watch: {
    $route: {
      handler: function (route) {
        this.navMsg = route.meta.desc
      }
    }
  }
}
</script>
<style lang="less" scoped>
.setting-path {
  padding: 20px;
  font-size: 18px;
}
</style>
