import Vue from 'vue'
import VueRouter from 'vue-router'
import Layout from '../views/Layout/index.vue'
import Login from '../views/Login'
Vue.use(VueRouter)

const routes = [
  {
    path: '/',
    name: 'login',
    component: Login
  },
  {
    path: '/main',
    component: Layout,
    redirect: '/main/av_routing',
    children: [
      {
        path: 'av_routing',
        component: () => import('../views/Main/AV_Routing.vue'),
        name: 'av_routing',
        meta: { title: 'AV Routing', desc: 'Main > AV Routing', icon: 'setting' }
      }
      // {
      //   path: 'overlay',
      //   component: () => import('../views/Main/Overlay.vue'),
      //   name: 'overlay',
      //   meta: { title: 'Overlay', desc: 'Main > Overlay', icon: 'overlay' }
      // }
      // {
      //   path: 'osd',
      //   component: () => import('../views/Main/Overlay.vue'),
      //   name: 'osd',
      //   meta: { title: 'OSD', desc: 'Main > OSD', icon: 'list' }
      // },
      // {
      //   path: 'kvm-usb',
      //   component: () => import('../views/Main/AutoSwitch.vue'),
      //   name: 'kvm-usb',
      //   meta: { title: 'KVM/USB', desc: 'Main > KVM/USB', icon: 'usb' }
      // },
      // {
      //   path: 'video-wall',
      //   component: () => import('../views/Main/AutoSwitch.vue'),
      //   name: 'video-wall',
      //   meta: { title: 'Video Wall', desc: 'Main > Video Wall', icon: 'grid' }
      // },
    ]
  },
  {
    path: '/av_setting',
    component: Layout,
    children: [
      {
        path: 'auto_switch',
        component: () => import('../views/AvSetting/AutoSwitch.vue'),
        name: 'auto_switch',
        meta: { title: 'Auto Switch', desc: 'AV Setting > Auto Switch', icon: 'auto_switch', hidden: true }
      },
      {
        path: 'videoPage',
        component: () => import('../views/AvSetting/Video.vue'),
        name: 'videoPage',
        meta: { title: 'Video', desc: 'AV Setting > Video', icon: 'video' }
      },
      {
        path: 'audioPage',
        component: () => import('../views/AvSetting/Audio.vue'),
        name: 'audioPage',
        meta: { title: 'Audio', desc: 'AV Setting > Audio', icon: 'audio' }
      },
      {
        path: 'edid',
        component: () => import('../views/AvSetting/Edid.vue'),
        name: 'edid',
        meta: { title: 'EDID', desc: 'AV Setting > EDID Management', icon: 'edid' }
      }
      // {
      //   path: 'loopThough',
      //   component: () => import('../views/AvSetting/LoopThrough.vue'),
      //   name: 'loopThough',
      //   meta: { title: 'Loop Through', desc: 'AV Setting > Loop Through', icon: 'setting' }
      // }
    ]
  },
  {
    path: '/dev_setting',
    component: Layout,
    redirect: '/dev_setting/general',
    children: [
      {
        path: 'general',
        component: () => import('../views/Device/General.vue'),
        name: 'general',
        meta: { title: 'General', desc: 'Devices Setting > General', icon: 'general' }
      },
      {
        path: 'network',
        component: () => import('../views/Device/Network.vue'),
        name: 'network',
        meta: { title: 'Network', desc: 'Devices Setting > Network', icon: 'network' }
      },
      // {
      //   path: 'firmware',
      //   component: () => import('../views/Device/Firmware.vue'),
      //   name: 'firmware',
      //   meta: { title: 'Firmware', desc: 'Devices Setting > Firmware', icon: 'firmware' }
      // },
      {
        path: 'time_date',
        component: () => import('../views/Device/Time_date.vue'),
        name: 'time_date',
        meta: { title: 'Time and Date', desc: 'Devices Setting > Time and Date', icon: 'time_date' }
      },
      {
        path: 'Security',
        component: () => import('../views/Device/Security.vue'),
        name: 'Security',
        meta: { title: 'Security', desc: 'Devices Setting > Security', icon: 'security' }
      },
      {
        path: 'Users',
        component: () => import('../views/Device/Users.vue'),
        name: 'Users',
        meta: { title: 'Users', desc: 'Devices Setting > Users', icon: 'user' }
      }
    ]
  },
  {
    path: '/control',
    component: Layout,
    redirect: '/control/setting',
    children: [
      {
        path: 'setting',
        component: () => import('../views/Control/Settings.vue'),
        name: 'setting',
        meta: { title: 'Settings', desc: 'Control > Settings', icon: 'setting' }
      }
    ]
  },
  {
    path: '/diagnostics',
    component: Layout,
    redirect: '/diagnostics/status',
    children: [
      {
        path: 'status',
        component: () => import('../views/Diagnostics/Status.vue'),
        name: 'status',
        meta: { title: 'Status', desc: 'Diagnostics > Status', icon: 'status' }
      },
      {
        path: 'connections',
        component: () => import('../views/Diagnostics/Connections.vue'),
        name: 'connections',
        meta: { title: 'Connections', desc: 'Diagnostics > Connections', icon: 'connection' }
      },
      {
        path: 'advanced',
        component: () => import('../views/Diagnostics/Advanced.vue'),
        name: 'advanced',
        meta: { title: 'Advanced', desc: 'Diagnostics > Advanced', icon: 'setting' }
      }
    ]
  },
  {
    path: '/about',
    component: Layout,
    redirect: '/about/info',
    children: [
      {
        path: 'info',
        component: () => import('../views/About/Info.vue'),
        name: 'info',
        meta: { title: 'General Info', desc: 'About', icon: 'about' }
      }
    ]
  },
  { path: '*', redirect: '/' }
]

const router = new VueRouter({
  routes
})
router.beforeEach((to, from, next) => {
  if (to.name && to.name === 'login') {
    next()
  } else {
    // 拦截未登录 => main.js Vue.prototype.$global = {}
    if (!Vue.prototype.$global.isLogin) {
      next({
        path: '/'
      })
    } else {
      next()
    }
  }
})
export const constantRoutes = routes
export default router
