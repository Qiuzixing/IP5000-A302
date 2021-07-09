<template>
  <div class="main-setting">
    <div class="setting">
      <span class="setting-title">EDID Lock</span>
      <v-switch v-model="edidLock" active-value="1" inactive-value="0" @change="setEDIDLock"></v-switch>
    </div>
    <div class="setting">
      <span class="setting-title">EDID Mode</span>
      <multiselect :disabled="edidLock=='1'" :options="edid.param"  v-model="edid.val" @input="setEDIDMode"></multiselect>
<!--      <v-model-select :is-disabled="edidLock" :options="edid.param"  v-model="edid.val"></v-model-select>-->
    </div>
    <div class="radio-setting" v-if="edid.val === 'custom'">
      <span class="setting-title">User EDID</span>
      <ul class="edid-list" v-if="!(edidLock=='1')">
        <li :class="{'active': isSelectListIndex === item[0]}" v-for="item in edidList" @click="isSelectListIndex = item[0]" :key="item[0]">{{item[1]}}</li>
      </ul>
      <ul class="edid-list disabled" v-else>
        <li :class="{'active': isSelectListIndex === item[0]}" v-for="item in edidList" :key="item[0]">{{item[1]}}</li>
      </ul>
      <div>
        <button class="btn btn-plain-primary" :disabled="edidLock==='1'" style="margin-left: 24px;margin-bottom: 24px;">UPLOAD</button><br>
        <button class="btn btn-plain-primary" :disabled="edidLock==='1' || isSelectListIndex == '0'" style="margin-left: 24px;margin-bottom: 24px;" @click="deleteEDID">REMOVE</button><br>
        <button class="btn btn-plain-primary" :disabled="edidLock==='1'" style="margin-left: 24px;width: 97px;" @click="setEDID">APPLY</button>
      </div>

    </div>
    <div class="setting" v-if="edid.val === 'passthru'">
      <span class="setting-title">Read EDID from Specific Decoder</span>
      <input type="text" class="setting-text" v-model="mac" placeholder="MAC/IP Address" :disabled="edidLock=='1'">
      <button class="btn btn-plain-primary" :disabled="edidLock=='1'" style="margin-left: 24px" @click="readEDID">READ</button>
    </div>
  </div>
</template>

<script>
export default {
  name: 'edidManage',
  data () {
    return {
      edid: {
        val: 'passthru',
        param: [
          { value: 'passthru', label: 'Passthrough' },
          { value: 'custom', label: 'Custom' },
          { value: 'default', label: 'Default EDID' }
        ]
      },
      mac: '',
      edidLock: '0',
      edidList: [],
      edidListIndex: -1,
      isSelectListIndex: -1
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#LOCK-EDID? 1')
    this.$socket.sendMsg('#EDID-MODE? 1')
    this.$socket.sendMsg('#EDID-NET-SRC? 1')
    this.$socket.sendMsg('#EDID-LIST? ')
    this.$socket.sendMsg('#EDID-ACTIVE? 1 ')
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@LOCK-EDID /i) !== -1) {
        this.handleEDIDLock(msg)
        return
      }
      if (msg.search(/@EDID-MODE /i) !== -1) {
        this.handleEdidMode(msg)
        return
      }
      if (msg.search(/@EDID-NET-SRC /i) !== -1) {
        this.handelEDIDRead(msg)
        return
      }
      if (msg.search(/@EDID-LIST /i) !== -1) {
        this.handleEDIDList(msg)
        return
      }
      if (msg.search(/@EDID-ACTIVE /i) !== -1) {
        this.handleEDIDCustomSelect(msg)
        return
      }
      if (msg.search(/@EDID-RM /i) !== -1) {
        this.handleDeleteEDID(msg)
      }
    },
    handleEDIDLock (msg) {
      this.edidLock = msg.split(',')[1]
    },
    setEDIDLock (msg) {
      this.$socket.sendMsg(`#LOCK-EDID 1,${msg}`)
    },
    handleEdidMode (msg) {
      const data = msg.split(',')
      this.edid.val = data[1].toLowerCase()
      if (this.edid.val === 'custom') {
        this.edidListIndex = parseInt(data[2])
        this.isSelectListIndex = parseInt(data[2])
      }
    },
    setEDIDMode (msg) {
      // const index = this.edidList.length > 0 ? this.edidList[0][0] : 1
      this.$socket.sendMsg(`#EDID-MODE 1,${msg},${this.isSelectListIndex === -1 ? 1 : this.edidListIndex}`)
    },
    handelEDIDRead (msg) {
      this.mac = msg.split(',')[1]
    },
    readEDID () {
      this.$socket.sendMsg(`#EDID-NET-SRC 1,${this.mac}`)
    },
    setEDID () {
      if (this.isSelectListIndex === -1) return
      this.$socket.sendMsg(`#EDID-ACTIVE 1,${this.isSelectListIndex}`)
    },
    deleteEDID () {
      if (this.isSelectListIndex === -1 || this.isSelectListIndex === this.edidListIndex) return
      this.$socket.sendMsg(`#EDID-RM ${this.isSelectListIndex}`)
    },
    handleDeleteEDID (msg) {
      let index = msg.split(' ')[1]
      if (index) {
        index = parseInt(index)
        let start = -1
        for (let i = 0, len = this.edidList.length; i < len; i++) {
          if (this.edidList[i][0] === index) {
            start = i
            break
          }
        }
        if (start !== -1) {
          this.edidList.splice(start, 1)
          this.$socket.sendMsg('#EDID-ACTIVE? 1 ')
        }
      }
    },
    handleEDIDCustomSelect (msg) {
      if (msg.search(/error/i) !== -1) return
      const index = parseInt(msg.split(',')[1])
      this.edidListIndex = index
      this.isSelectListIndex = index
    },
    handleEDIDList (msg) {
      const startIndex = msg.indexOf('[')
      const result = msg.substr(startIndex).split(',')
      console.log(result.toString())
      this.edidList = JSON.parse('[' + result.toString() + ']')
    }
  }
}
</script>
<style lang="less" scoped>
.vs__dropdown-toggle{
  width: 180px;
}
.edid-list{
  &.disabled{
    li{
      cursor: not-allowed;
      opacity: .5;
    }
  }
  margin: 0;
  list-style: none;
  padding: 0;
  border: 1px solid #4D4D4F;
  width: 250px;
  border-radius: 5px;
  li{
    &:first-child{
      border-top-left-radius: 5px;
      border-top-right-radius: 5px;
    }
    &:last-child{
      border-bottom-left-radius: 5px;
      border-bottom-right-radius: 5px;
    }
    &.active{
      background: #F3F3F3;
      color: #404040;
      font-family: "open sans semiblold";
    }
    cursor: pointer;
    padding: 5px;
  }
}
</style>
