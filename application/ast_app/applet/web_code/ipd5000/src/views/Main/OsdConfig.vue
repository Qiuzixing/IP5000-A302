<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Menu Timeout (sec)</span>
        <el-input-number v-model="osdConfig.timeout_sec"
                         controls-position="right"
                         :max="3600"
                         :precision="0"
                         @blur="checkBlur"
                         :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Channels per Page</span>
        <el-input-number v-model="osdConfig.max_channels_per_page"
                         controls-position="right"
                         @blur="checkBlur"
                         :max="10"
                         :min="5"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Maximum Channels</span>
        <el-input-number v-model="osdConfig.max_channels"
                         controls-position="right"
                         :max="999"
                         @blur="checkBlur"
                         :min="0"></el-input-number>
      </div>
      <div>
        <span class="setting-title">Channel List</span>
        <ul class="edid-list">
          <li>
            <button class="btn btn-primary"
                    @click="dialogType = 1"
                    :disabled="this.channelList.length >= osdConfig.max_channels">ADD</button>
            <button class="btn btn-plain-primary"
                    style="margin-left: 24px"
                    type="button">IMPORT</button>
            <button class="btn btn-plain-primary"
                    type="button"
                    style="margin-left: 24px">
              EXPORT
            </button>
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
                          icon-class="edit" />
              </span>
              <el-popconfirm :title="'Are you sure you want to delete '+ item.name + '?'"
                             @confirm="deleteChannel(index)">
                <icon-svg slot="reference"
                          icon-class="rubbish" />
              </el-popconfirm>
            </span>
          </li>
        </ul>
        <div class="channel-list-pagination"
             style="margin-bottom: 24px">
          <span>Page {{ currentPage }} of
            {{countPages()}}</span>
          <div>
            <span class="channel-list-icon"
                  @click="next(currentPage - 1)"><img src="../../assets/img/arrow.svg" /></span>
            <span class="channel-list-icon"
                  @click="next(currentPage + 1)"><img src="../../assets/img/arrow.svg" /></span>
          </div>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Menu Font Size</span>
        <multiselect v-model="osdConfig.font_size"
                     :options="osdSize.param"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Menu Position</span>
        <multiselect v-model="osdConfig.position"
                     :options="osdPosition.param"></multiselect>
      </div>
      <!--      <div class="setting">-->
      <!--        <span class="setting-title">Channel Lineup</span>-->
      <!--        -->
      <!--      </div>-->
      <div class="setting">
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
                style="margin-left: 24px">DISPLAY NOW</button>
      </div>
    </div>
    <footer><button class="btn btn-primary"
              @click="save">SAVE</button></footer>
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
                 class="setting-text" />
          <span class="alert-error">Numbers only</span>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title"
              style="width: 80px">Name</span>
        <div style="position: relative;flex: 1"
             :class="{'error-input': idError}">
          <input type="text"
                 maxlength="24"
                 v-model="addChannel.name"
                 class="setting-text" />
          <span class="alert-error">Name must be 1 to 24 characters in length, <br>alphanumeric only and space</span>
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
          {{editObj.id}}
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
                 class="setting-text" />
          <span class="alert-error">Name must be 1 to 24 characters in length, <br>alphanumeric only and space.</span>
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
      }
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
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#KDS-OSD-DISPLAY? ')
    this.getOsdJson()
    this.getAvChannelMap()
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@KDS-OSD-DISPLAY /i) !== -1) {
        this.handleOsdInfo(msg)
      }
    },
    getOsdJson () {
      this.$http.post('/device/json?path=/osd/osd.json&t=' + Math.random()).then(msg => {
        if (msg.data.channel_menu) {
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
        if (this.channelList[i].id === this.addChannel.id) {
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
      return Math.ceil(this.channelList.length / this.osdConfig.max_channels_per_page)
    },
    isID (id) {
      return id.match(/^[1-9]?[1-9]?[1-9]$/)
    },
    isName (name) {
      return name.match(/^[A-Za-z0-9 ]{1,15}$/)
    },
    setDisplayInfo (val) {
      this.$socket.sendMsg('#KDS-OSD-DISPLAY ' + val)
    },
    handleOsdInfo (msg) {
      this.osdInfo = msg.split(' ')[1] !== '0' ? '1' : '0'
    },
    save () {
      this.$http.post('/device/json', {
        path: '/osd/osd.json',
        info: {
          channel_menu: this.osdConfig
        }
      })
      this.$http.post('/device/json', {
        path: '/channel/channel_map.json',
        info: {
          channels_list: this.channelList
        }
      })
    },
    checkBlur () {
      this.osdConfig.timeout_sec = this.osdConfig.timeout_sec || 0
      this.osdConfig.max_channels_per_page = this.osdConfig.max_channels_per_page || 0
      this.osdConfig.max_channels = this.osdConfig.max_channels || 0
    }
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
  width: 220px;
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
  height: 400px;
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
