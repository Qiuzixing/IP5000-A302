<template>
  <div class="setting-nav">
    <ul>
      <template v-for="item in navList">
        <template v-if="item.meta.hidden">
          <li v-if="$global.deviceType"  :key="item.meta.title" :class="pathName ===item.path ? 'active' : ''">
            <router-link  :to="item.path">
              <icon-svg :icon-class="item.meta.icon" />
              <span>{{item.meta.title}}</span>
            </router-link>
          </li>
        </template>
        <li v-else  :key="item.meta.title" :class="pathName ===item.path ? 'active' : ''">
          <router-link  :to="item.path">
            <icon-svg :icon-class="item.meta.icon" />
            <span>{{item.meta.title}}</span>
          </router-link>
        </li>
      </template>

    </ul>
    <div class="border-line"></div>
  </div>
</template>

<script>
import { constantRoutes } from '@/router/index.js'
export default {
  name: 'settingNav',
  data () {
    return {
      navList: [],
      routerNav: constantRoutes,
      pathName: '',
      routerMap: {

      }
    }
  },
  created () {
    this.initRouterNav()
    this.checkRoute()
  },
  methods: {
    initRouterNav () {
      const fullPath = this.$route.fullPath
      for (let i = 0, len = this.routerNav.length; i < len; i++) {
        const firstPath = this.routerNav[i].path
        if (fullPath.startsWith(firstPath) && this.routerNav[i].children) {
          this.navList = this.routerNav[i].children.map(item => {
            return {
              path: firstPath + '/' + item.path,
              meta: item.meta
            }
          })
          break
        }
      }
    },
    checkRoute () {
      this.pathName = this.$route.fullPath
    }
  },
  watch: {
    $route: {
      handler: function (route) {
        this.pathName = route.fullPath
        this.initRouterNav()
      }
    }
  }
}
</script>
<style lang="less" scoped>
.setting-nav {
  padding: 0 20px;
  ul{
    margin: 0;
    padding: 0;
    display: flex;
    height: 60px;
    font-size: 18px;
    box-sizing: border-box;
    align-items: center;
    border-bottom: 3px solid #35acf8;
    li{
      list-style: none;
      width: 200px;
      a{
        color: #4D4D4F;
        text-decoration: none;
        a:link,
        &:visited,
        &:hover,
        &:active
        {
          color: #4D4D4F;
        }
        img{
          width: 24px;
          height: 24px;
          vertical-align: middle;
          margin-right: 10px;
        }
        span{
          vertical-align: middle;
          font-size: 15px;
        }
      }
      .svg-icon{
        margin-right: 10px;
      }
      &.active{
        .svg-icon{
          color: #35ACF8;
          //fill: #35ACF8;
          transition: color .5s;
        }
        span {
          font-family: 'open sans bold', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
          color: #35ACF8;
        }
      }
    }
  }
  .border-line{

  }
}
</style>
