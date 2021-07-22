<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Menu Timeout (sec)</span>
        <el-input-number v-model="osdConfig['Channel menu'].timeout_sec" controls-position="right" :max="3600" :precision="0" @blur="checkBlur"
                         :min="0"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Channels per Page</span>
        <el-input-number v-model="osdConfig['Channel menu']['max channels_per page']" controls-position="right" @blur="checkBlur"
                         :max="10" :min="5"></el-input-number>
      </div>
      <div class="setting">
        <span class="setting-title">Maximum Channels</span>
        <el-input-number v-model="osdConfig['Channel menu']['max channels']" controls-position="right" :max="999" @blur="checkBlur"
                         :min="0"></el-input-number>
      </div>
      <div>
        <span class="setting-title">Channel List</span>
        <ul class="edid-list">
          <li>
            <button class="btn btn-primary" @click="dialogType = 1" :disabled="this.channelList['channels list'].length >= osdConfig['Channel menu']['max channels']">ADD</button>
            <button class="btn btn-plain-primary" style="margin-left: 24px" type="button">IMPORT</button>
            <button
              class="btn btn-plain-primary"
              type="button"
              style="margin-left: 24px"
            >
              EXPORT
            </button>
          </li>
          <li>
            <span class="channel-title">#ID</span>
            <span>Name</span>
          </li>
          <li
            :class="{ active: edidListIndex === index }"
            v-for="(item, index) in currentData"
            :key="item.id"
          >
            <span @click="edidListIndex = index" class="channel-title">{{ item.id }}</span>
            <span @click="edidListIndex = index" class="channel-name">{{ item.name }}</span>
            <span class="channel-icon">
              <span @click="editChannel(index)">
                <icon-svg style="margin-right: 5px" icon-class="edit" />
              </span>
              <el-popconfirm :title="'Are you sure you want to delete '+ item.name + '?'" @confirm="deleteChannel(index)">
                <icon-svg slot="reference" icon-class="rubbish"/>
              </el-popconfirm>
            </span>
          </li>
        </ul>
        <div class="channel-list-pagination" style="margin-bottom: 24px">
          <span>Page {{ currentPage }} of
            {{countPages()}}</span>
          <div>
            <span class="channel-list-icon" @click="next(currentPage - 1)"
            ><img src="../../assets/img/arrow.svg"
            /></span>
            <span class="channel-list-icon" @click="next(currentPage + 1)"
            ><img src="../../assets/img/arrow.svg"
            /></span>
          </div>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title">Menu Font Size</span>
        <multiselect v-model="osdConfig['Channel menu']['font size']" :options="osdSize.param"></multiselect>
      </div>
      <div class="setting">
        <span class="setting-title">Menu Position</span>
        <multiselect v-model="osdConfig['Channel menu'].position" :options="osdPosition.param"></multiselect>
      </div>
      <!--      <div class="setting">-->
      <!--        <span class="setting-title">Channel Lineup</span>-->
      <!--        -->
      <!--      </div>-->
      <div class="setting">
        <span class="setting-title">Display Device Information</span>
        <v-switch v-model="osdConfig['Device Info'].Enabled" active-value="ON" inactive-value="OFF" @change="setDisplayInfo"></v-switch>
        <button
          @click="setDisplayInfo('2')"
          class="btn"
          :disabled="osdConfig['Device Info'].Enabled === 'on'"
          :class="[osdConfig['Device Info'].Enabled ==='off' ? 'btn-plain-primary' : 'btn-default']"
          type="button"
          style="margin-left: 24px">DISPLAY NOW</button>
      </div>
    </div>
    <footer><button class="btn btn-primary" @click="save">SAVE</button></footer>
    <el-dialog
      title="Channel"
      :visible="dialogType == 1"
      width="400px"
      :before-close="closeDialog"
    >
      <div class="setting">
        <span class="setting-title" style="width: 80px">ID</span>
        <div style="position: relative;flex: 1" :class="{'error-input': idError}">
          <input
            type="text"
            maxlength="3"
            v-model="addChannel.id"
            class="setting-text"
          />
          <span class="alert-error">Numbers only</span>
        </div>
      </div>
      <div class="setting">
        <span class="setting-title" style="width: 80px">Name</span>
        <div style="position: relative;flex: 1" :class="{'error-input': idError}">
          <input
            type="text"
            maxlength="24"
            v-model="addChannel.name"
            class="setting-text"
          />
          <span class="alert-error">Name must be 1 to 24 characters in length, <br>alphanumeric only and space</span>
        </div>
      </div>
      <span v-if="idRulerError" style="color: #d50000;font-size: 14px;font-family: open sans bold;">This id already exists</span>
      <span slot="footer" class="dialog-footer" style="padding: 0">
         <button class="btn btn-primary" @click="handleAddChannel">APPLY</button>
        </span>
    </el-dialog>
    <el-dialog
      title="Channel Edit"
      :visible="dialogType == 2"
      width="400px"
      :before-close="closeDialog"
    >
      <div class="setting">
        <span class="setting-title" style="width: 80px">ID</span>
        <div style="position: relative;flex: 1">
          {{editObj.id}}
        </div>
      </div>
      <div class="setting">
        <span class="setting-title" style="width: 80px">Name</span>
        <div style="position: relative;flex: 1" :class="{'error-input': editError}">
          <input
            type="text"
            maxlength="24"
            v-model="editObj.name"
            class="setting-text"
          />
          <span class="alert-error">Name must be 1 to 24 characters in length, <br>alphanumeric only and space.</span>
        </div>
      </div>
      <span slot="footer" class="dialog-footer" style="padding: 0">
         <button class="btn btn-primary" @click="handleEditChannel">APPLY</button>
        </span>
    </el-dialog>
  </div>
</template>

<script>
export default {
  name: 'osd',
  data () {
    return {
      osdConfig: {
        'Channel menu': {
          timeout_sec: 0,
          position: 'TopCenter',
          'font size': 'Small',
          'max channels_per page': 5,
          'max channels': 999
        },
        'Device Info': {
          Enabled: 'OFF',
          Timeout: 2
        }
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
        val: 'topleft',
        param: [
          {
            value: 'topleft',
            label: 'Top Left'
          },
          {
            value: 'top',
            label: 'Top'
          },
          {
            value: 'topright',
            label: 'Top Right'
          },
          {
            value: 'left',
            label: 'Left'
          },
          {
            value: 'center',
            label: 'Center'
          },
          {
            value: 'right',
            label: 'Right'
          },
          {
            value: 'bottomleft',
            label: 'Bottom Left'
          },
          {
            value: 'bottom',
            label: 'Bottom'
          },
          {
            value: 'bottomright',
            label: 'Bottom Right'
          }
        ]
      },
      displayDevice: false,
      channelList: {
        'channels list': []
      },
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
      if (this.channelList['channels list']) {
        const start = (this.currentPage - 1) * this.osdConfig['Channel menu']['max channels_per page']
        const end = this.currentPage * this.osdConfig['Channel menu']['max channels_per page']
        return this.channelList['channels list'].slice(start, end)
      }
      return []
    }
  },
  created () {
    this.getOsdJson()
    this.getAvChannelMap()
  },
  methods: {
    getOsdJson () {
      this.$http.post('/osd/overlay').then(msg => {
        if (msg.data) {
          this.osdConfig =msg.data
        }
      })
    },
    getAvChannelMap () {
      this.$http.post('/channel/channel_map').then(msg => {
        if (msg.data) {
          this.channelList = msg.data
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
      for (const i in this.channelList['channels list']) {
        if (this.channelList['channels list'][i].id === this.addChannel.id) {
          isExists = true
        }
      }
      this.idRulerError = isExists
      if (isExists) return
      this.channelList['channels list'].push({
        id: parseInt(this.addChannel.id),
        name: this.addChannel.name
      })
      this.channelList['channels list'] = this.sortChannel(this.channelList['channels list'])
      this.dialogType = 0
    },
    sortChannel (channelList) {
      return channelList.sort((a, b) => a.id - b.id)
    },
    deleteChannel (index) {
      const num = this.osdConfig['Channel menu']['max channels_per page'] * (this.currentPage - 1) + index
      this.channelList['channels list'].splice(num, 1)
      if (this.currentPage > this.countPages()) {
        this.currentPage = this.countPages()
        this.edidListIndex = 0
      }
    },
    editChannel (index) {
      const currentIndex = this.osdConfig['Channel menu']['max channels_per page'] * (this.currentPage - 1) + index
      const obj = this.channelList['channels list'].slice(currentIndex, currentIndex + 1)[0]
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
      this.channelList['channels list'][this.editObj.index].name = this.editObj.name
      this.dialogType = 0
    },
    // 统计显示的总页数
    countPages () {
      return Math.ceil((this.channelList['channels list'] ? this.channelList['channels list'].length : 0) / this.osdConfig['Channel menu']['max channels_per page'])
    },
    isID (id) {
      return id.match(/^[1-9]?[1-9]?[1-9]$/)
    },
    isName (name) {
      return name.match(/^[A-Za-z0-9 ]{1,15}$/)
    },
    setDisplayInfo (val) {
      this.$socket.sendMsg('#KDS_OSD_DISPLAY ' + (val === '2' ? val : (val === 'on' ? 1 : 0)))
    },
    save () {
      this.$http.post('/osd/set_overlay', this.osdConfig)
      this.$http.post('/channel/set_channel_map', this.channelList)
    },
    checkBlur () {
      this.osdConfig['Channel menu'].timeout_sec = this.osdConfig['Channel menu'].timeout_sec || 0
      this.osdConfig['Channel menu']['max channels_per page'] = this.osdConfig['Channel menu']['max channels_per page'] || 0
      this.osdConfig['Channel menu']['max channels'] = this.osdConfig['Channel menu']['max channels'] || 0
    }
  }
}
</script>
<style lang="less" scoped>
.main-setting{
  display: flex;
  flex-direction: column;
}
.setting-model{
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
