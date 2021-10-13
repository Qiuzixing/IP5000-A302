<template>
  <div class="view-container">
    <header-bar :device-model="deviceModel"></header-bar>
    <section class="main-container">
      <div class="login-content">
        <h2 class="text-center">Login</h2>
        <div class="form-item">
          <h4>Username</h4>
          <input type="text"
                 class="setting-text"
                 v-model="username">
        </div>
        <div class="form-item">
          <h4>Password</h4>
          <input type="password"
                 class="setting-text"
                 v-model="password"
                 @keyup.enter="login">
        </div>
        <div>
          <button class="btn btn-primary"
                  style="width: 100%"
                  @click="login">Sign In</button>
        </div>
        <p style="margin: 5px 0;color: #D50000;"
           v-if="error">Incorrect username or password.</p>
      </div>
    </section>
  </div>
</template>

<script>
// @ is an alias to /src
import headerBar from '@/components/header.vue'
export default {
  name: 'layout',
  components: {
    headerBar
  },
  data () {
    return {
      deviceModel: '',
      count: 0,
      username: '',
      password: '',
      error: false
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.getModel()
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@MODEL /i) !== -1) {
        this.handleDeviceModel(msg)
        return
      }
      if (msg.search(/@LOGIN /i) !== -1) {
        this.handleLogin(msg)
      }
    },
    handleDeviceModel (msg) {
      const model = msg.split(' ')[1]
      this.deviceModel = model
      this.$global.deviceModel = model
      this.$global.deviceType = this.modelType(model)
      document.title = model
    },
    getModel () {
      if (this.$socket.ws && this.$socket.ws.readyState === 1) {
        this.$socket.sendMsg('#MODEL? ')
      } else if (this.count < 20) {
        this.count++
        setTimeout(this.getModel, 100)
      }
    },
    modelType (model) {
      let type = 0
      switch (model) {
        case 'KDS-DEC7':
          type = 1
          break
        case 'WP-DEC7':
          type = 0
          break
        default:
          type = 0
          break
      }
      return type
    },
    login () {
      if (this.username && this.password) {
        this.$socket.sendMsg(`#LOGIN ${this.username},${this.password}`)
      }
    },
    handleLogin (msg) {
      if (msg.split(',').pop().trim().toLowerCase() === 'ok') {
        this.$global.isLogin = true
        this.$router.push('/main')
      } else {
        this.error = true
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
    display: flex;
    justify-content: center;
    align-items: center;
  }
}
.main-container {
  height: calc(100vh - 70px);
  overflow: hidden;
  background: #f8f8f8;
}
.login-content {
  width: 480px;
  height: 320px;
  padding: 15px 100px;
  box-shadow: 0 2px 30px 0 #81838533;
  background-color: #ffffff;
  box-sizing: border-box;
  input {
    width: 100%;
    border: 1px solid #4d4d4f;
    border-radius: 4px;
  }
  h2 {
    font-family: "open sans bold";
  }
  h4 {
    margin-bottom: 10px;
    font-family: "open sans semiblold";
  }
  .form-item {
    margin-bottom: 24px;
  }
}
</style>
