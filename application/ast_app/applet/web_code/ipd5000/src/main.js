import 'normalize.css/normalize.css'
import vSwitch from './components/switch/'
import Multiselect from './components/vue-multiselect/src'
import Vue from 'vue'
import App from './App.vue'
import router from './router'
import webSocket from './util/webSocket'
import './assets/css/global.less'
import './theme-chalk/color-picker.css'
import './icons'
import Axios from 'axios'
import {
  Dialog,
  InputNumber,
  // DatePicker,
  // TimeSelect,
  // TimePicker,
  Popover,
  Popconfirm,
  Slider
} from 'element-ui'
import lang from 'element-ui/lib/locale/lang/en'
import locale from 'element-ui/lib/locale'
import colorPicker from './components/color-picker'
if (process.env.NODE_ENV !== 'production') {
  require('./mock/mock.js')
}
locale.use(lang)
Vue.component('colorPicker', colorPicker)
Vue.use(Dialog)
Vue.use(InputNumber)
Vue.use(Popover)
Vue.use(Popconfirm)
// Vue.use(DatePicker)
// Vue.use(TimeSelect)
// Vue.use(TimePicker)
Vue.use(Slider)
Vue.component('multiselect', Multiselect)
Vue.use(vSwitch)
Vue.prototype.$socket = webSocket
Vue.prototype.$http = Axios
Vue.prototype.$global = {
  isLogin: false,
  deviceModel: ''
}
Vue.config.productionTip = false

new Vue({
  router,
  render: h => h(App)
}).$mount('#app')
