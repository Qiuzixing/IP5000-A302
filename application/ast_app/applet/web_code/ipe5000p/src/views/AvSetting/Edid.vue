<template>
  <div class="main-setting">
    <div class="setting">
      <span class="setting-title">EDID Lock</span>
      <v-switch v-model="edidLock"
                active-value="1"
                inactive-value="0"
                @change="setEDIDLock"></v-switch>
    </div>
    <div class="setting">
      <span class="setting-title">EDID Mode</span>
      <el-select v-model="edid.val" :disabled="edidLock=='1'" @change="setEDIDMode">
        <el-option
          v-for="item in edid.param"
          :key="item.value"
          :label="item.label"
          :value="item.value">
        </el-option>
      </el-select>
    </div>
    <div v-show="edidMode">
      <div class="radio-setting"
           v-if="edid.val === 'custom'">
        <span class="setting-title">User EDID</span>
        <ul class="edid-list"
            v-if="!(edidLock=='1')">
          <li :class="{'active': isSelectListIndex === item[0]}"
              v-for="item in edidList"
              @click="isSelectListIndex = item[0]"
              :key="item[0]">{{ item[1] }}
          </li>
        </ul>
        <ul class="edid-list disabled"
            v-else>
          <li :class="{'active': isSelectListIndex === item[0]}"
              v-for="item in edidList"
              :key="item[0]">{{ item[1] }}
          </li>
        </ul>
        <div style="margin-left: 24px;">
          <el-upload action="/upload/edid"
                     ref="upload"
                     :on-error="upgradeFail"
                     :on-success="upgradeFile"
                     :before-upload="checkEDID"
                     :show-file-list="false"
                     :auto-upload="true">
            <button class="btn btn-plain-primary"
                    :disabled="edidLock==='1' || edidList.length > 7">UPLOAD
            </button>

          </el-upload>
          <br>
          <button class="btn btn-plain-primary"
                  :disabled="edidLock==='1' || isSelectListIndex == '0'"
                  style="margin-bottom: 24px;"
                  @click="deleteEDID">REMOVE
          </button>
          <br>
          <button class="btn btn-plain-primary"
                  :disabled="edidLock==='1'"
                  style="width: 97px;"
                  @click="setEDID">APPLY
          </button>
          <input type="file"
                 style="display:none;width:0;height:0;">
        </div>

      </div>
      <div class="setting"
           v-if="edid.val === 'passthru'">
        <span class="setting-title">Read EDID from Specific Decoder</span>
        <input type="text"
               class="setting-text"
               v-model="mac"
               placeholder="0.0.0.0"
               :disabled="edidLock=='1'">
        <button class="btn btn-plain-primary"
                :disabled="edidLock=='1'"
                style="margin-left: 24px"
                @click="readEDID">READ
        </button>
      </div>
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
          {
            value: 'passthru',
            label: 'Passthrough'
          },
          {
            value: 'custom',
            label: 'Custom'
          },
          {
            value: 'default',
            label: 'Default EDID'
          }
        ]
      },
      mac: '',
      edidLock: '0',
      edidList: [],
      edidListIndex: -1,
      isSelectListIndex: -1,
      fileList: [],
      edidMode: false
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
      if (msg.search(/@EDID-ADD /i) !== -1) {
        this.handleDeleteEDID(msg)
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
      this.edidMode = true
    },
    setEDIDMode (msg) {
      this.$socket.sendMsg(`#EDID-MODE 1,${msg},${this.isSelectListIndex === -1 ? 1 : this.edidListIndex}`)
    },
    handelEDIDRead (msg) {
      this.mac = msg.split(',')[1] || '0.0.0.0'
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
      this.$socket.sendMsg('#EDID-LIST? ')
      this.$socket.sendMsg('#EDID-ACTIVE? 1 ')
      // let index = msg.split(' ')[1]
      // if (index) {
      //   index = parseInt(index)
      //   let start = -1
      //   for (let i = 0, len = this.edidList.length; i < len; i++) {
      //     if (this.edidList[i][0] === index) {
      //       start = i
      //       break
      //     }
      //   }
      //   if (start !== -1) {
      //     this.edidList.splice(start, 1)
      //     this.$socket.sendMsg('#EDID-ACTIVE? 1 ')
      //   }
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
      this.edidList = JSON.parse('[' + result.toString() + ']')
    },
    upgradeFile (e, file) {
      if (this.edidList.length < 8) {
        this.$socket.sendMsg(`#EDID-ADD ${this.edidList.length},${file.name}`)
      }
    },
    upgradeFail () {
      alert('File corrupted')
    },
    async checkEDID (file) {
      if ((file.size === 256 || file.size === 128) && file.name.toLowerCase().endsWith('.bin')) {
        // return this.edidList.every(item =>
        //   item[1] !== file.name
        // )
        const hexArray = await this.checkEDIDBlock(file)
        if (hexArray[0] === 0x00 &&
          hexArray[1] === 0xff &&
          hexArray[2] === 0xff &&
          hexArray[3] === 0xff &&
          hexArray[4] === 0xff &&
          hexArray[5] === 0xff &&
          hexArray[6] === 0xff &&
          hexArray[7] === 0x00) {
          const blockl = hexArray.slice(0, 128)
          const block2 = hexArray.slice(128)
          const block1Checksum = blockl.reduce((a, b) => a + b).toString(2)
          let block2Checksum = '00000000'
          if (block2.length === 128) {
            block2Checksum = block2.reduce((a, b) => a + b).toString(2)
          }
          if (block1Checksum.endsWith('00000000') && block2Checksum.endsWith('00000000')) {
            return true
          }
          alert('File corrupted')
          return new Promise()
        } else {
          alert('File corrupted')
          return new Promise()
        }

      } else {
        alert('The EDID file format must be BIN with 128 or 256 bytes')
        return false
      }
    },
    checkEDIDBlock (file) {
      return new Promise((resolve, reject) => {
        const reader = new FileReader()
        reader.onload = e => {
          // 读取文件内容
          const fileString = e.target.result
          const hexArray = this.ArrayBuffer2hex(fileString)
          resolve(hexArray)
        }
        reader.onerror = e => {
          return false
        }
        reader.readAsArrayBuffer(file)
      })
    },
    ArrayBuffer2hex (buffer) {
      const hexArr = Array.prototype.map.call(
        new Uint8Array(buffer),
        function (bit) {
          return bit
        }
      )
      return hexArr
    }
  }
}
</script>
<style lang="less" scoped>
.vs__dropdown-toggle {
  width: 180px;
}

.edid-list {
  &.disabled {
    li {
      cursor: not-allowed;
      opacity: 0.5;
    }
  }

  margin: 0;
  list-style: none;
  padding: 0;
  border: 1px solid #4d4d4f;
  width: 250px;
  border-radius: 5px;

  li {
    &:first-child {
      border-top-left-radius: 5px;
      border-top-right-radius: 5px;
    }

    &:last-child {
      border-bottom-left-radius: 5px;
      border-bottom-right-radius: 5px;
    }

    &.active {
      background: #f3f3f3;
      color: #404040;
      font-family: "open sans semiblold";
    }

    cursor: pointer;
    padding: 5px;
  }
}
</style>
