<template>
  <div class="v-collapse" :class="{'open': open}">
    <div class="v-collapse-header" @click="openCollapse">
      <div><img src="../../assets/img/arrow.svg" alt="">{{ title }}</div>
    </div>
    <transition name="slide">
      <div v-if="open" class="v-collapse-body">
        <slot></slot>
      </div>
    </transition>
  </div>
</template>

<script>
export default {
  name: 'Collapse',
  props: {
    title: {
      required: true
    }
    // bgc: {
    //   required: true
    // }
  },
  data () {
    return {
      open: true
    }
  },
  methods: {
    openCollapse () {
      this.open = !this.open
    }
  }
}
</script>

<style lang="less" scoped>
.v-collapse {
  padding: 5px;
  font-size: 15px;
  margin-bottom: 3px;
  border-bottom: 1px solid #0000004D;
  width: 475px;
  &.open{
    img{
      transform: rotate(180deg);
    }
  }
  img{
    width: 14px;
    //box-sizing: border-box;
    padding: 6px 6px;
    margin-right: 8px;
    vertical-align: middle;
    transform: rotate(90deg);
    transition: transform .3s;
  }
}
.v-collapse-header {
  //padding: 20px 25px;
  color: #4D4D4F;
  font-size: 15px;
  font-family: 'open sans bold';
  cursor: pointer;
}

.v-collapse-body{
  margin-top: 15px;
  padding-left: 34px;
  padding-bottom: 16px;
}
.slide-enter-active {
  transition-duration: 0.3s;
  transition-timing-function: ease-in;
}
.slide-leave-active {
  transition-duration: 0.2s;
  transition-timing-function: cubic-bezier(0, 1, 0.5, 1);
}
.slide-enter-to,
.slide-leave {
  max-height: 600px;
  overflow-y: hidden;
}
.slide-enter,
.slide-leave-to {
  overflow: hidden;
  max-height: 0;
}
</style>
