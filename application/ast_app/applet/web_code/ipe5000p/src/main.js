import 'normalize.css/normalize.css'
import vSwitch from './components/switch/'
import Vue from 'vue'
import App from './App.vue'
import router from './router'
import webSocket from './util/webSocket'
import './icons'
import './assets/css/global.less'
import './theme-chalk/color-picker.css'
import Multiselect from './components/vue-multiselect/src'
import Axios from 'axios'
import {
  Dialog,
  InputNumber,
  Upload,
  // DatePicker,
  // TimeSelect,
  // TimePicker,
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
// Vue.use(Pagination);
Vue.use(Dialog)
Vue.use(InputNumber)
Vue.use(Upload)
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
  deviceModel: '',
  deviceType: '' // 1 => DKS-SW3-EN-6X  0 =>KDS-EN-6X
}
Vue.config.productionTip = false

new Vue({
  router,
  render: h => h(App)
}).$mount('#app')
