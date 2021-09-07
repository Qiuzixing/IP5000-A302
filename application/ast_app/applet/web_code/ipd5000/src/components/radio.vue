<template>
  <label class="radio-model"
         :class="{'is-disabled': disabled}">
    <span class="radio_input"
          :class="{'is-checked': model === label}">
      <span class="radio__inner"></span>
      <input type="radio"
             v-model="model"
             :value="label"
             :disabled="disabled"
             @change="handleChange"
             :name="name">
    </span>
    <span class="radio__label"
          @keydown.stop>
      <slot></slot>
      <template v-if="!$slots.default">{{isEmpty ? '' : label}}</template>
    </span>
  </label>
</template>

<script>
export default {
  name: 'radioComponent',
  props: {
    label: {
      type: String,
      default: ''
    },
    disabled: {
      type: Boolean,
      default: false
    },
    name: String,
    value: {},
    isEmpty: {
      type: Boolean,
      default: false
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
    }
  },
  methods: {
    handleChange () {
      this.$nextTick(() => {
        this.$emit('change', this.model)
      })
    }
  }
}
</script>

<style lang="less">
.radio-model.is-disabled {
  opacity: 0.5;
  cursor: not-allowed !important;
  .radio_input {
    cursor: not-allowed !important;
  }
}
.radio-model {
  font-family: "open sans semiblold";
  font-size: 15px;
  margin-bottom: 24px;
  color: #4d4d4f;
  position: relative;
  cursor: pointer;
  display: block;
  white-space: nowrap;
  .radio_input {
    margin-right: 14px;
    white-space: nowrap;
    cursor: pointer;
    outline: none;
    display: inline-block;
    line-height: 1;
    position: relative;
    vertical-align: middle;
    &.is-checked {
      .radio__inner {
        border: 2px solid #35acf8;
        transition: border 0.3s;
        &:after {
          background-color: #35acf8;
          transition: background-color 0.3s;
        }
      }
    }
  }
  .radio__inner {
    border: 2px solid #4d4d4f;
    border-radius: 100%;
    width: 16px;
    height: 16px;
    background-color: #fff;
    position: relative;
    cursor: pointer;
    display: inline-block;
    box-sizing: border-box;
    &:after {
      width: 7px;
      height: 7px;
      border-radius: 100%;
      background-color: #fff;
      content: "";
      position: absolute;
      left: 50%;
      top: 50%;
      transform: translate(-50%, -50%);
    }
  }
  input[type="radio"] {
    opacity: 0;
    outline: none;
    position: absolute;
    z-index: -1;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    margin: 0;
  }
}
</style>
