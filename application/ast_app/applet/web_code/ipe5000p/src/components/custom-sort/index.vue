<template>
  <div class="sort-model">
    <div class="list-group-item" :class="[{'disabled': disabled}]" v-for="(item, index) in model"
         :key="item">
      <span class="sort-title" v-if="index===0">High</span>
      <span class="sort-title" v-if="index===model.length - 1">Low</span>
      <div class="sort-source" v-if="!disabled" @dragend="dragEnterIndex=-1,dragIndex=-1"
           :class="[{'active': dragIndex === index}, {'drag-enter': dragEnterIndex === index}]" draggable="true"
           @dragenter="dragEnter(index)" @dragleave="dragEnterIndex = -1" @dragstart="dragStart($event, index)"
           @dragover="allowDrop" @drop="drop($event, index)">
        {{ listMap[item] }}
      </div>
      <div v-else class="sort-source" draggable="false">
        {{ listMap[item] }}
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'customSort',
  props: {
    disabled: {
      type: Boolean,
      default: false
    },
    listMap: {
      type: Object,
      default: {}
    },
    value: {}
  },
  data () {
    return {
      dragIndex: -1,
      dragEnterIndex: -1
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
    dragEnter (index) {
      if (index === this.dragIndex) return
      this.dragEnterIndex = index
    },
    allowDrop (e) {
      e.preventDefault()
    },
    drop (e, index) {
      this.allowDrop(e)
      this.dragIndex = -1
      this.dragEnterIndex = -1
      if (index === this.dragIndex) return
      // 使用一个新数组重新排序后赋给原变量
      const arr = this.value.concat([])
      const temp = arr.splice(this.dragIndex, 1)
      arr.splice(index, 0, temp[0])
      this.$emit('input', arr)
      // this.lists = arr
    },
    dragStart (e, index) {
      this.dragIndex = index
      // let tar = e.target
      // e.dataTransfer.setData('Text', index)
      // if (tar.tagName.toLowerCase() == 'li') {
      //   // console.log('drag start')
      //   // console.log('drag Index: ' + index)
      // }
    }
  }
}
</script>

<style lang="less" scoped>
.sort-model {
  &.disabled {
    .list-group-item {
      border-top: 2px solid #D0D0D0;

      &:after {
        background: #D0D0D0;
      }

      .sort-source {
        border: 2px solid #D0D0D0;
        color: #D0D0D0;
      }

      .sort-title {
        color: #D0D0D0;
      }
    }
  }

  .list-group-item {
    border-top: 2px solid #35acf8;
    display: inline-block;
    width: 110px;
    padding-top: 25px;
    position: relative;

    &.disabled {
      border-top: 2px solid #D0D0D0;

      &:after {
        background: #D0D0D0;
      }

      .sort-source {
        border: 2px solid #D0D0D0;
        color: #D0D0D0;
        cursor: not-allowed;
      }

      .sort-title {
        color: #D0D0D0;
      }
    }

    &:after {
      position: absolute;
      content: '';
      top: 0;
      left: 50%;
      transform: translate(-50%, 0);
      height: 25px;
      width: 2px;
      background: #35acf8;
    }

    .sort-title {
      font-family: "open sans semiblold";
      position: absolute;
      top: -22px;
      left: 50%;
      transform: translate(-50%, 0);
    }

    .sort-source {
      &.active {
        background: #35acf8;
        opacity: .3;
      }

      &.drag-enter {
        background: #D0D0D0;
      }

      font-family: "open sans semiblold";
      user-select: none;
      cursor: pointer;
      margin: 0 auto;
      width: 90px;
      text-align: center;
      border: 2px solid #35acf8;
      //padding: 15px;
      padding: 5px 0;
      border-radius: 5px;
    }
  }
}
</style>
