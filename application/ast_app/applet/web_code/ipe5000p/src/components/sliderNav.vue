<template>
  <aside class="slider-nav">
    <ul>
      <li :class="{'active': checkPathActive('/main'), 'hover': hover == 1}" @click="changeActive(1)" @mouseenter="hover=1" @mouseleave="hover=0">
        <a href="#/main"><img src="../assets/img/nav/home.svg" alt="">
          <span class="open-slider-nav">Main</span>
        </a>
      </li>
      <li :class="{'active': checkPathActive('/av_setting'), 'hover': hover == 2}" @click="changeActive(2)" @mouseenter="hover=2" @mouseleave="hover=0">
        <a href="#/av_setting"><img src="../assets/img/nav/streaming_settings.svg" alt="">
          <span class="open-slider-nav">AV Settings</span></a>
      </li>
      <li :class="{'active': checkPathActive('/dev_setting'), 'hover': hover == 3}" @click="changeActive(3)" @mouseenter="hover=3" @mouseleave="hover=0">
        <a href="#/dev_setting"><img src="../assets/img/nav/devices_settings.svg" alt=""><span class="open-slider-nav">Device Settings</span></a>
      </li>
      <li :class="{'active': checkPathActive('/control'), 'hover': hover == 4}" @click="changeActive(4)" @mouseenter="hover=4" @mouseleave="hover=0">
        <a href="#/control"><img src="../assets/img/nav/control.svg" alt=""><span class="open-slider-nav">Control</span></a>
      </li>
      <li :class="{'active': checkPathActive('/diagnostics'), 'hover': hover == 5}" @click="changeActive(5)" @mouseenter="hover=5" @mouseleave="hover=0">
        <a href="#/diagnostics"><img src="../assets/img/nav/diagnostics.svg" alt=""><span class="open-slider-nav">Diagnostics</span></a>
      </li>
      <li :class="{'active': checkPathActive('/about'), 'hover': hover == 6}" @click="changeActive(6)" @mouseenter="hover=6" @mouseleave="hover=0">
        <a href="#/about"><img src="../assets/img/nav/about.svg" alt=""><span class="open-slider-nav">About</span></a>
      </li>
    </ul>
  </aside>
</template>

<script>
export default {
  name: 'sliderModel',
  data () {
    return {
      active: 1,
      hover: 0,
      timeOut: null,
      pathName: ''
    }
  },
  created () {
    this.getRouter()
  },
  methods: {
    changeActive (index) {
      this.active = index
      this.hover = 0
    },
    getRouter () {
      this.pathName = this.$route.fullPath
    },
    checkPathActive (path) {
      return this.pathName.startsWith(path)
    }
  },
  watch: {
    $route: {
      handler: function (route) {
        this.pathName = route.fullPath
      }
    }
  }
}
</script>

<style lang="less" scoped>
.slider-nav{
  height: 100%;
  width: 65px;
  background-color: #4D4D4F;
  box-sizing: border-box;
  ul{
    padding: 0;
    margin: 0;
    a{
      display: block;
      padding: 18px 20px;
    }
    li{
      list-style: none;
      height: 60px;
      box-sizing: border-box;
      position: relative;
      border-right: 5px solid transparent;
      transition: border-right .5s;
      img{
        width: 24px;
        height: 24px;
      }
      .open-slider-nav{
        cursor: pointer;
        display: block;
        position: absolute;
        top: 0;
        height: 100%;
        line-height: 60px;
        color: #fff;
        left: calc(100% + 5px);
        background: #4D4D4F;
        width: 0;
        opacity: 0;
        box-sizing: border-box;
        user-select:none;
        z-index: -1;
      }
      &.hover{
        opacity: .9;
        box-shadow: 0px 0px 6px #00000029;
        .open-slider-nav{
          width: 180px;
          padding-left: 5px;
          opacity: .9;
          z-index: 99;
          transition: opacity .9s;
        }
      }
      //&:hover{
      //  //background: #333333;
      //}
      &.active {
        border-right: 5px solid #35ACF8;
        background: #333333;
        &.hover{
          border-right: 5px solid transparent;
        }
        &:hover .open-slider-nav{
          border-right: 5px solid #35ACF8;
        }
      }
    }
  }
}
</style>
