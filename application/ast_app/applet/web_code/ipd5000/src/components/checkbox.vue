<template>
  <label class="checkbox"  :class="{'is-checked':checked, 'isDisabled': disabled}" @click="changeVal">
    <span class="checkbox-input" :class="{'is-checked':checked}">
      <span class="checkbox-inner"></span>
      <span class="checkbox-inner-label">{{label}}</span>
    </span>
  </label>
</template>
<script>
export default {
  name: 'checkBox',
  props: {
    label: {
      type: String,
      default: ''
    },
    disabled: {
      type: Boolean,
      default: false
    },
    activeValue: {
      type: [Boolean, String, Number],
      default: true
    },
    inactiveValue: {
      type: [Boolean, String, Number],
      default: false
    },
    value: {}
  },
  data () {
    return {
    }
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
    changeVal () {
      if (!this.disabled) {
        const val = this.checked ? this.inactiveValue : this.activeValue
        this.$emit('input', val)
        this.$emit('click', val)
      }
    }
  },
  watch: {
    checked (newVal) {
      this.isChecked = newVal
    }
  }

}
</script>
<style rel="stylesheet/less" lang="less">
.checkbox{

  margin: 0;
  cursor: pointer;
  outline: none;
  display: inline-block;
  line-height: 1;
  position: relative;
  vertical-align: middle;
  &.isDisabled .checkbox-inner{
    //background-color: #808080!important;
    cursor: not-allowed;
  }
  .checkbox-inner {
    vertical-align: middle;
    display: inline-block;
    position: relative;
    border: 2px solid #6e6e6e;
    border-radius: 4px;
    box-sizing: border-box;
    width: 18px;
    height: 18px;
    //margin-bottom: 5px;
    background-color: #fff;
    box-sizing: border-box;
  }
  &.is-checked .checkbox-inner {
    background: url("../assets/img/check_box.svg") no-repeat center;
  }
  .checkbox-inner-label{
    font-weight: normal;
    padding-left: 10px;
  }
}
</style>
