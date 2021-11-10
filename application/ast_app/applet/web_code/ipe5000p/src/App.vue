<template>
  <div id="app">
    <router-view />
  </div>
</template>
<script>
export default {
  name: 'App',
  created () {
    this.$socket.initWebsocket()
  },
  beforeDestroy () {
    sessionStorage.setItem('username', '123')
  }
}
</script>
