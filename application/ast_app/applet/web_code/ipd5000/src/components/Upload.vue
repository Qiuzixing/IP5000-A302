<template>
  <div class="upload-model">
    <span class="upload-title" v-if="show">File:</span>
    <input type="text" readonly v-model="filename" v-if="show" />
    <input type="file" ref="file" @change="fileChange" :accept="accept" />
    <label
      class="btn"
      :class="[show ? 'btn-primary' : 'btn-plain-primary']"
      @click="browse"
    >
      <slot></slot>
      <template v-if="!$slots.default">BROWSE</template>
    </label>
  </div>
</template>

<script>
export default {
  name: 'Upload',
  props: {
    accept: {
      type: String,
      default: ''
    },
    show: {
      type: Boolean,
      default: true
    }
  },
  data () {
    return {
      filename: ''
    }
  },
  methods: {
    browse () {
      this.$refs.file.click()
    },
    fileChange () {
      if (this.$refs.file.files.length === 0) {
        this.filename = ''
        return
      }
      this.filename = this.$refs.file.files[0].name
      this.$emit('change', this.$refs.file.files[0])
    },
    submit () {
      console.log(123231)
    },
    clearFile () {
      this.$refs.file.value = ''
      this.filename = ''
    }
  }
}
</script>

<style lang="less">
.upload-model {
  display: flex;
  .upload-title {
    padding-right: 5px;
    line-height: 36px;
    font-family: "open sans semiblold";
  }
  input[type="file"] {
    display: none;
  }
  input[type="text"] {
    flex: 1;
    margin-right: 5px;
    &:focus {
      outline: none;
    }
  }
  .btn {
    box-sizing: border-box;
    height: 36px;
    min-width: 90px;
    padding: 8px 18px;
    border-radius: 3px;
    font-size: 14px;
    font-family: "open sans semiblold";
    overflow: visible;
    border: 0;
    text-transform: none;
    cursor: pointer;
    &:focus {
      outline: none;
    }
    &[disabled] {
      cursor: not-allowed;
      opacity: 0.5;
    }
  }

  .btn-plain-primary {
    border: 2px solid #35acf8;
    background-color: #ffffff;
    color: #35acf8;
    &:hover {
      background-color: #f9f9f9;
    }
    &:active {
      background: #fafafa;
      box-shadow: inset 1px 1px 1px rgb(0 0 0 / 20%), 0 0 1px transparent;
    }
  }
  .btn-primary {
    background-color: #35acf8;
    color: #ffffff;
    &:hover {
      background-color: #0098e3;
    }
    &:active {
      background: #0087d2;
    }
  }
  .btn-default {
    color: #4d4d4f;
    background-color: #ebebeb;
    &:hover {
      background-color: #e2e2e2;
    }
    &:active {
      background-color: #ebebeb;
    }
  }
}
</style>
