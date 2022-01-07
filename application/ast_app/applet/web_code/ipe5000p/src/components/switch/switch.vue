<template>
  <div class="switch" :class="{'disabled': disabled}" >
    <span class="switch-on switch-btn" :class="{'active':checked}" @click="changeVal(true)">{{openText}}</span>
    <span class="switch-off switch-btn" :class="{'active':!checked}" @click="changeVal(false)">{{closeText}}</span>
  </div>
</template>

<script>
export default {
  name: 'vSwitch',
  props: {
    openText: {
      type: String,
      default: 'On'
    },
    closeText: {
      type: String,
      default: 'Off'
    },
    activeValue: {
      type: [Boolean, String, Number],
      default: true
    },
    inactiveValue: {
      type: [Boolean, String, Number],
      default: false
    },
    activeFunc: {
      type: Function,
      default: null
    },
    disabled: {
      type: Boolean,
      default: false
    },
    inactiveFunc: {
      type: Function,
      default: null
    },
    value: {}
  },
  computed: {
    model: {
      get () {
        return this.value
      },
      set (val) {
        this.$emit('input', val)
      }
    },
    checked () {
      return this.value === this.activeValue
    }
  },
  methods: {
    changeVal (isActive) {
      if (this.disabled) return
      if (isActive) {
        if (this.checked) return
        this.$emit('input', this.activeValue)
        this.$emit('change', this.activeValue)
      } else {
        if (!this.checked) return
        this.$emit('input', this.inactiveValue)
        this.$emit('change', this.inactiveValue)
      }
    }
  }
}
</script>

<style lang="less">
.switch{
  display: flex;
  background: #A6A6A6;
  border-radius: 5px;
  &.disabled{
    background: #F3F3F3;
    .switch-btn{
      color: #D0D0D0;
      cursor: not-allowed;
      &.switch-off.active,
      &.active{
        box-shadow: 0px 0px 3px #D0D0D0;
        border: 1px solid #D0D0D0;
        color: #D0D0D0;
      }
    }
  }
  .switch-btn{
    font-family: 'open sans semiblold';
    min-width: 60px;
    padding: 0 10px;
    height: 28px;
    text-align: center;
    line-height: 25px;
    border-radius: 5px;
    border: 1px solid transparent;
    color: #fff;
    cursor: pointer;
    font-size: 15px;
    user-select: none;
    box-sizing: border-box;
    &.switch-off.active{
      box-shadow: 0px 0px 3px #6E6E6E;
      border: 1px solid #6E6E6E;
      color: #6E6E6E;
    }
    &.active{
      color: #35ACF8;
      background: #fff;
      border: 1px solid #35ACF8;
      box-shadow:0 0 3px #35ACF8;
      transition: all 0.3s;
      //font-family: "open sans bold";
    }
  }
  // 文字较长
}
.switch.large .switch-btn{
  width: 78px;
}
</style>
