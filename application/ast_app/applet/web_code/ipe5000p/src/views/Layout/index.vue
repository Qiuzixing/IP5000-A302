<template>
  <div class="view-container">
    <header-bar :device-model="this.$global.deviceModel"></header-bar>
    <section class="main-container">
      <slider-model class="slider-nav"></slider-model>
      <div class="main-content">
        <setting-header-bar></setting-header-bar>
        <setting-nav></setting-nav>
        <router-view v-show="show" />
      </div>
    </section>
  </div>
</template>

<script>
// @ is an alias to /src
import headerBar from '@/components/header.vue'
import sliderModel from '@/components/sliderNav.vue'
import settingHeaderBar from './component/settingHeaderPath.vue'
import settingNav from './component/settingNav.vue'
export default {
  name: 'layout',
  components: {
    headerBar,
    sliderModel,
    settingHeaderBar,
    settingNav
  },
  data () {
    return {
      show: false
    }
  },
  created () {
    const loading = this.$loading({
      lock: true,
      text: 'Loading',
      spinner: 'el-icon-loading',
      background: 'rgba(0, 0, 0, 0.1)'
    })
    setTimeout(() => {
      loading.close()
      this.show = true
    }, 1000)
  },
  watch: {
    $route: {
      handler: function (route) {
        this.show = false
        const loading = this.$loading({
          lock: true,
          text: 'Loading',
          spinner: 'el-icon-loading',
          background: 'rgba(0, 0, 0, 0.1)'
        })
        setTimeout(() => {
          loading.close()
          this.show = true
        }, 1000)
      }
    }
  }
}
</script>
<style lang="less" scoped>
.view-container {
  height: 100vh;
  display: flex;
  flex-direction: column;
  .main-container {
    flex: 1;
    display: flex;
  }
}
.main-content {
  height: calc(100vh - 70px);
  flex: 1;
  overflow: hidden;
  .main-setting {
    height: calc(100vh - 190px);
    overflow: hidden;
    overflow-y: auto;
    box-sizing: border-box;
  }
}
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.3s;
}
.fade-enter,
.fade-leave-to {
  opacity: 0;
}
</style>
