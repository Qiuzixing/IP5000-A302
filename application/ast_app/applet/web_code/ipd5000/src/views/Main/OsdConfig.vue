<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">OSD Menu</h3>
      <div class="setting" v-if="this.$global.deviceType">
        <span class="setting-title">Menu Timeout (sec)</span>
        <el-input-number v-model="osdConfig.timeout_sec"
                         controls-position="right"
                         :max="3600"
                         :precision="0"
                         :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Channels per Page</span>
        <el-input-number v-model="osdConfig.max_channels_per_page"
                         controls-position="right"
                         :max="10"
                         :min="5"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Maximum Channels</span>
        <el-input-number v-model="osdConfig.max_channels"
                         controls-position="right"
                         :max="999"
                         :min="0"></el-input-number>
      </div>
      <div>
        <span class="setting-title">Channel List</span>
        <ul class="edid-list">
          <li>
            <button class="btn btn-primary"
                    @click="idRulerError=false,idError = false, nameError = false,addChannel.name='',addChannel.id='',dialogType = 1"
                    :disabled="this.channelList.length >= osdConfig.max_channels">ADD
            </button>
            <button class="btn btn-plain-primary"
                    style="margin-left: 24px"
                    type="button"
                    @click="browseChannelList">IMPORT
            </button>
            <button class="btn btn-plain-primary"
                    type="button"
                    @click="exportChannel"
                    style="margin-left: 24px">
              EXPORT
            </button>
            <input type="file"
                   ref="channelList"
                   accept="application/json"
                   @change="channelFileChange"
                   style="display:none;width:0;height:0;">
          </li>
          <li v-if="channelError"
              style="color: #d50000;
  font-size: 12px;
  font-family: 'open sans bold';">{{ channelErrorMsg }}
          </li>
          <li>
            <span class="channel-title">#ID</span>
            <span>Name</span>
          </li>
          <li :class="{ active: edidListIndex === index }"
              v-for="(item, index) in currentData"
              :key="item.id">
            <span @click="edidListIndex = index"
                  class="channel-title">{{ item.id }}</span>
            <span @click="edidListIndex = index"
                  class="channel-name">{{ item.name }}</span>
            <span class="channel-icon">
              <span @click="editChannel(index)">
                <icon-svg style="margin-right: 5px"
                          icon-class="edit"/>
              </span>
              <el-popconfirm :title="'Are you sure you want to delete '+ item.name + '?'"
                             @confirm="deleteChannel(index)">
                <icon-svg slot="reference"
                          icon-class="rubbish"/>
              </el-popconfirm>
            </span>
          </li>
        </ul>
        <div class="channel-list-pagination"
             style="margin-bottom: 24px">
          <span>Page {{ currentPage }} of
            {{ countPages() }}</span>
          <div>
            <span class="channel-list-icon"
                  @click="next(currentPage - 1)"><img src="../../assets/img/arrow.svg"/></span>
            <span class="channel-list-icon"
                  @click="next(currentPage + 1)"><img src="../../assets/img/arrow.svg"/></span>
          </div>
        </div>
      </div>
      <div class="setting" v-if="this.$global.deviceType">
        <span class="setting-title">Menu Font Size</span>
        <el-select v-model="osdConfig.font_size">
          <el-option
            v-for="item in osdSize.param"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <div class="setting" v-if="this.$global.deviceType">
        <span class="setting-title">Menu Position</span>
        <el-select v-model="osdConfig.position">
          <el-option
            v-for="item in osdPosition.param"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <h3 class="setting-model-title"  v-if="this.$global.deviceType">Device Information</h3>
      <div class="setting" v-if="this.$global.deviceType">
        <span class="setting-title">Display Device Information</span>
        <v-switch v-model="osdInfo"
                  active-value="1"
                  inactive-value="0"
                  @change="setDisplayInfo"></v-switch>
        <button @click="setDisplayInfo('2')"
                class="btn"
                :disabled="osdInfo === '0'"
                :class="[osdInfo ==='1' ? 'btn-plain-primary' : 'btn-default']"
                type="button"
                style="margin-left: 24px">DISPLAY NOW
        </button>
      </div>
      <div class="setting" v-if="this.$global.deviceType">
        <span class="setting-title">Device Information Timeout(min)</span>
        <el-input-number v-model="osdJson.device_info.timeout" style="margin-right: 15px;"
                         controls-position="right"
                         :max="60"
                         :min="0"></el-input-number>(0 for never)
      </div>
    </div>
    <footer>
      <button class="btn btn-primary"
              @click="save">SAVE
      </button>
    </footer>
    <el-dialog title="Channel"
               :visible="dialogType == 1"
               width="400px"
               :before-close="closeDialog">
      <div class="setting">
        <span class="setting-title"
              style="width: 80px">ID</span>
        <div style="position: relative;flex: 1"
             :class="{'error-input': idError}">
          <input type="text"
                 maxlength="3"
                 v-model="addChannel.id"
                 class="setting-text"/>
          <span class="alert-error">Numbers only</span>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title"
              style="width: 80px">Name</span>
        <div style="position: relative;flex: 1"
             :class="{'error-input': nameError}">
          <input type="text"
                 maxlength="24"
                 v-model="addChannel.name"
                 class="setting-text"/>
          <span
            class="alert-error">Alphanumeric, hyphen and underscore within 24 characters, hyphen and underscore can not at beginning or end</span>
        </div>
      </div>
      <span v-if="idRulerError"
            style="color: #d50000;font-size: 14px;font-family: open sans bold;">This id already exists</span>
      <span slot="footer"
            class="dialog-footer"
            style="padding: 0">
        <button class="btn btn-primary"
                @click="handleAddChannel">APPLY</button>
      </span>
    </el-dialog>
    <el-dialog title="Channel Edit"
               :visible="dialogType == 2"
               width="400px"
               :before-close="closeDialog">
      <div class="setting">
        <span class="setting-title"
              style="width: 80px">ID</span>
        <div style="position: relative;flex: 1">
          {{ editObj.id }}
        </div>
      </div>
      <div class="setting">
        <span class="setting-title"
              style="width: 80px">Name</span>
        <div style="position: relative;flex: 1"
             :class="{'error-input': editError}">
          <input type="text"
                 maxlength="24"
                 v-model="editObj.name"
                 class="setting-text"/>
          <span
            class="alert-error">Alphanumeric, hyphen and underscore within 24 characters, hyphen and underscore can not at beginning or end</span>
        </div>
      </div>
      <span slot="footer"
            class="dialog-footer"
            style="padding: 0">
        <button class="btn btn-primary"
                @click="handleEditChannel">APPLY</button>
      </span>
    </el-dialog>
  </div>
</template>

<script>
import { saveAs } from 'file-saver'
import { debounce } from 'lodash'

export default {
  name: 'osd',
  data () {
    return {
      osdInfo: '0',
      osdConfig: {
        timeout_sec: 100,
        position: 'top_center',
        font_size: 'small',
        max_channels_per_page: 10,
        max_channels: 999
      },
      dialogType: 0,
      edidListIndex: 0,
      prePage: 5,
      maxChannels: 100,
      currentPage: 1,
      show: false,
      osdSize: {
        val: 'large',
        param: [
          {
            value: 'large',
            label: 'Large'
          },
          {
            value: 'medium',
            label: 'Medium'
          },
          {
            value: 'small',
            label: 'Small'
          }
        ]
      },
      osdPosition: {
        val: 'top_left',
        param: [
          {
            value: 'top_left',
            label: 'Top Left'
          },
          {
            value: 'top_right',
            label: 'Top Right'
          },
          {
            value: 'top_center',
            label: 'Top Center'
          },
          {
            value: 'bottom_left',
            label: 'Bottom Left'
          },
          {
            value: 'bottom_right',
            label: 'Bottom Right'
          },
          {
            value: 'bottom_center',
            label: 'Bottom Center'
          },
          {
            value: 'left',
            label: 'Left'
          },
          {
            value: 'right',
            label: 'Right'
          },
          {
            value: 'center',
            label: 'Center'
          }
        ]
      },
      displayDevice: false,
      channelList: [],
      addChannel: {
        id: '',
        name: ''
      },
      idError: false,
      nameError: false,
      idRulerError: false,
      editError: false,
      editObj: {
        index: 0,
        name: '',
        id: ''
      },
      osdJson: {
        device_info: {
          timeout: 2
        }
      },
      channelError: false,
      channelErrorMsg: ''
    }
  },
  computed: {
    currentData () {
      if (this.channelList.length > 0) {
        const start = (this.currentPage - 1) * this.osdConfig.max_channels_per_page
        const end = this.currentPage * this.osdConfig.max_channels_per_page
        return this.channelList.slice(start, end)
      }
      return []
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#KDS-OSD-DISPLAY? ')
    this.getOsdJson()
    this.getAvChannelMap()
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@KDS-OSD-DISPLAY /i) !== -1) {
        this.handleOsdInfo(msg)
      }
    },
    getOsdJson () {
      this.$http.get('/device/json?path=/osd/osd.json&t=' + Math.random()).then(msg => {
        if (msg.data.channel_menu) {
          this.osdJson = msg.data
          this.osdConfig = msg.data.channel_menu
        }
      })
    },
    getAvChannelMap () {
      this.$http
        .get(
          '/device/json?path=/channel/channel_map.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.channels_list) {
            this.channelList = msg.data.channels_list
          }
        })
    },
    exportChannel () {
      this.$http
        .get(
          '/device/json?path=/channel/channel_map.json&t=' + Math.random()
        )
        .then(msg => {
          if (msg.data.channels_list) {
            const blob = new Blob([JSON.stringify(msg.data)], { type: 'text/plain;charset=utf-8' })
            saveAs(blob, 'channel_map.json')
          }
        })
    },
    browseChannelList () {
      this.$refs.channelList.click()
    },
    channelFileChange (e) {
      const file = e.target.files[0]
      if (file) {
        if (file.type !== 'application/json') {
          this.channelErrorMsg = 'File format error'
          this.channelError = true
          return
        }
        if (file.size > 1024 * 128) {
          this.channelErrorMsg = 'The file size is less than 128KB'
          this.channelError = true
          return
        }
        const reader = new FileReader()
        reader.readAsText(file, 'UTF-8')
        reader.onload = (e) => {
          try {
            const text = JSON.parse(e.target.result)
            if (Array.isArray(text.channels_list)) {
              this.channelError = false
              this.channelList = text.channels_list
            } else {
              this.channelErrorMsg = 'File format error'
              this.channelError = true
            }
          } catch (e) {
            this.channelErrorMsg = 'File format error'
            this.channelError = true
          }
        }
      } else {
        this.channelError = false
      }
    },
    next (num) {
      const pageCount = this.countPages()
      if (num <= 0 || num > pageCount) return
      this.currentPage = num
      this.edidListIndex = 0
    },
    closeDialog () {
      this.dialogType = 0
    },
    handleAddChannel () {
      this.idError = !this.isID(this.addChannel.id)
      this.nameError = !this.isName(this.addChannel.name)
      if (this.idError || this.nameError) return
      // this id already exists
      let isExists = false
      for (const i in this.channelList) {
        if (+this.channelList[i].id === +this.addChannel.id) {
          isExists = true
        }
      }
      this.idRulerError = isExists
      if (isExists) return
      this.channelList.push({
        id: parseInt(this.addChannel.id),
        name: this.addChannel.name
      })
      this.channelList = this.sortChannel(this.channelList)
      this.dialogType = 0
    },
    sortChannel (channelList) {
      return channelList.sort((a, b) => a.id - b.id)
    },
    deleteChannel (index) {
      const num = this.osdConfig.max_channels_per_page * (this.currentPage - 1) + index
      this.channelList.splice(num, 1)
      if (this.currentPage > this.countPages()) {
        this.currentPage = this.countPages()
        this.edidListIndex = 0
      }
    },
    editChannel (index) {
      const currentIndex = this.osdConfig.max_channels_per_page * (this.currentPage - 1) + index
      const obj = this.channelList.slice(currentIndex, currentIndex + 1)[0]
      this.editObj = {
        index: currentIndex,
        name: obj.name,
        id: obj.id
      }
      this.dialogType = 2
    },
    handleEditChannel () {
      this.editError = !this.isName(this.editObj.name)
      if (this.editError) return
      this.channelList[this.editObj.index].name = this.editObj.name
      this.dialogType = 0
    },
    // 统计显示的总页数
    countPages () {
      const totalPage = Math.ceil(this.channelList.length / this.osdConfig.max_channels_per_page)
      if (this.currentPage > totalPage) {
        this.currentPage = 1
      }
      return totalPage
    },
    isID (id) {
      return id.match(/^[1-9][0-9]?[0-9]?$/)
    },
    isName (name) {
      return /^[a-zA-Z0-9]$|^[a-zA-Z0-9][\w-]{0,22}[a-zA-Z0-9]$/.test(name)
    },
    setDisplayInfo (val) {
      this.$socket.sendMsg('#KDS-OSD-DISPLAY ' + val)
    },
    handleOsdInfo (msg) {
      this.osdInfo = msg.split(' ')[1] !== '0' ? '1' : '0'
    },
    save: debounce(function () {
      this.osdJson.channel_menu = this.osdConfig
      if (this.osdJson?.device_info) {
        this.osdJson.device_info.enabled = this.osdInfo === '1' ? 'on' : 'off'
      }
      this.$http.post('/device/json', {
        path: '/osd/osd.json',
        info: this.osdJson
      })
      this.$http.post('/device/json', {
        path: '/channel/channel_map.json',
        info: {
          channels_list: this.channelList
        }
      }).then(() => {
        this.$msg.successAlert()
      })
    }, 2000, {
      leading: true,
      trailing: true
    })
  }
}
</script>
<style lang="less" scoped>
.main-setting {
  display: flex;
  flex-direction: column;
}

.setting-model {
  flex: 1;
}

.main-setting footer {
  flex-shrink: 0;
  margin-top: 15px;
  margin-bottom: 15px;
}

.setting-title {
  width: 260px;
}

.edid-list {
  &.disabled {
    li {
      cursor: not-allowed;
      opacity: 0.5;
    }
  }

  margin: 15px 0;
  list-style: none;
  padding: 0;
  border: 1px solid #4d4d4f;
  width: 400px;
  height: 420px;
  border-radius: 5px;

  li {
    display: flex;

    .channel-title {
      width: 100px;
    }

    .channel-name {
      flex: 1;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
      padding-right: 15px;
    }

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
    padding: 5px 15px;
  }
}

.channel-list-pagination {
  width: 400px;
  display: flex;
  justify-content: space-between;

  .channel-list-icon {
    display: inline-block;
    cursor: pointer;

    &:first-child {
      transform: rotate(-90deg);
    }

    &:last-child {
      transform: rotate(90deg);
    }
  }
}
</style>
