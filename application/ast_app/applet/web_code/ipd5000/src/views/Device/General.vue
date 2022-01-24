<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">General Preferences</h3>
      <div class="setting" style="position: relative;">
        <span class="setting-title">Host Name</span>
        <input class="setting-text"
               type="text"
               maxlength="24"
               v-model="hostname">
        <button type="button"
                class="btn btn-plain-primary"
                style="margin-left: 15px;"
                @click="setHostName">APPLY
        </button>
        <span class="range-alert"
              v-if="hostNameError"
              style="top:36px;white-space: nowrap;">Alphanumeric, hyphen and underscore within 24 characters, hyphen and underscore can not at beginning or end</span>
      </div>
      <div class="setting">
        <span class="setting-title">Device Model</span>
        <span>{{ deviceModel }}</span>
        <!--        <input type="text" class="setting-text" v-model="">-->
        <!--        <button type="button" class="btn btn-plain-primary" style="margin-left: 15px;" @click="setDeviceModel">APPLY</button>-->
      </div>
      <div class="setting">
        <span class="setting-title">Device H/W Release</span>
        <span>{{ HWVer }}</span>
      </div>
      <div class="setting">
        <span class="setting-title">MAC Address</span>
        <span>{{ macAddr }}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Serial Number</span>
        <span>{{ serialNum }}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Front Panel Lock </span>
        <v-switch v-model="frontLock"
                  active-value="1"
                  inactive-value="0"
                  @change="setFrontLock"></v-switch>
      </div>
      <div class="setting">
        <span class="setting-title">Import/Export Device Settings</span>
        <el-select v-model="exportAndImport.val">
          <el-option
            v-for="item in exportAndImport.param"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
        <button type="button"
                class="btn btn-plain-primary"
                @click="importConfig"
                style="margin-left: 15px; margin-right: 15px;">IMPORT
        </button>

        <button type="button"
                class="btn btn-plain-primary"
                @click="exportConfig">EXPORT
        </button>
        <input type="file"
               ref="uploadConfig"
               style="display:none;"
               @change="browseConfig">
        <span v-if="uploadComplete"
              style="font-size:20px;margin-left:15px;color:#67c23a;"><i class="el-icon-circle-check"></i></span>
      </div>
      <div class="setting">
        <span class="setting-title">Locate Device</span>
        <button type="button"
                class="btn btn-plain-primary"
                @click="locateDev">APPLY
        </button>
      </div>
    </div>
    <div class="setting-model">
      <h3 class="setting-model-title">Version</h3>
      <div class="radio-setting"
           style="margin-bottom:24px;">
        <span class="setting-title"
              style="line-height: 36px;">Firmware Version</span>
        <span style="width: 180px;line-height: 36px;">{{ version }}</span>
        <el-upload action="/upload/upgradesoftware"
                   :before-upload="beforeUpload"
                   :on-progress="progressEvent"
                   :on-success="upgradeFile"
                   :on-error="uploadError"
                   :show-file-list="false"
                   ref="upload">
          <button class="btn btn-plain-primary"
                  style="margin-left: 15px;">UPGRADE
          </button>

        </el-upload>
        <!-- <upload-components :show="false">UPGRADE</upload-components> -->
      </div>
      <div class="setting">
        <span class="setting-title">Last Upgrade Date/Time</span>
        <span>{{ upgradeTime }}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Firmware Standby Version </span>
        <span style="width: 180px;">{{ standbyVer }}</span>
        <button class="btn btn-plain-primary"
                style="margin-left: 15px;"
                @click="rollBack">ROLLBACK
        </button>
      </div>
    </div>
    <div class="setting-model">
      <div class="setting">
        <span class="setting-model-title">Device Reset</span>
        <button type="button"
                class="btn btn-plain-primary"
                style="margin-right: 70px"
                @click="dialogVisibleReset = true">RESTART
        </button>
        <button type="button"
                class="btn btn-plain-primary"
                @click="dialogVisibleFactory = true"><img src="../../assets/img/warning.svg"
                                                          style="vertical-align: middle;margin-top: -5px"
                                                          alt="">RESET
        </button>
      </div>
    </div>
    <el-dialog title="RESTART"
               :visible.sync="dialogVisibleReset"
               width="500px">
      <p class="dialog-second-title">Do you want to restart the device? </p>
      <span slot="footer"
            class="dialog-footer">
        <button class="btn btn-primary"
                @click="dialogVisibleReset = false, restart()">PROCEED</button>
        <button class="btn btn-primary"
                @click="dialogVisibleReset = false">CANCEL</button>
      </span>
    </el-dialog>
    <el-dialog title="RESET"
               :visible.sync="dialogVisibleFactory"
               width="500px">
      <p class="dialog-second-title">Do you want to reset the device? </p>
      <span slot="footer"
            class="dialog-footer">
        <button class="btn btn-primary"
                @click="dialogVisibleFactory = false, reset()">PROCEED</button>
        <button class="btn btn-primary"
                @click="dialogVisibleFactory = false">CANCEL</button>
      </span>
    </el-dialog>
    <el-dialog title="Upgrade Info"
               :close-on-click-modal="false"
               :close-on-press-escape="false"
               :visible.sync="showProgress"
               width="500px">
      <div>
        <p class="upgrade-info"
           v-if="fileError">(3/3) Firmware upgrading failed. Error code {{ errMsg }}.</p>
        <p class="upgrade-info"
           v-if="upgradeComplete">(3/3) Firmware upgrading completed. Rebooting...</p>
        <p class="upgrade-info"
           v-if="isUpgrade">(2/3) Firmware installing {{ upgradeProgress }}%</p>
        <p class="upgrade-info">(1/3) Firmware uploading {{ uploadProgress }}%</p>
        <!-- <p class="upgrade-info"
           v-for="(item, index) in upgradeInfo"
           :key="index">{{item}}</p> -->
      </div>
    </el-dialog>
    <iframe v-if="isExportConfig"
            :src="'/settings/export?method='+ exportConfigVal"
            frameborder="0"
            width="0"
            height="0"></iframe>
  </div>
</template>

<script>
// import uploadComponents from '@/components/Upload.vue'
export default {
  name: 'autoSwitch',
  // components: {
  //   uploadComponents
  // },
  data () {
    return {
      hostNameError: false,
      show: false,
      upgrade: false,
      exportConfigVal: '',
      exportAndImport: {
        val: '3',
        param: [
          {
            value: '0',
            label: 'All Without IP'
          },
          {
            value: '1',
            label: 'Streams'
          },
          {
            value: '2',
            label: 'AV Settings only'
          },
          {
            value: '3',
            label: 'All including IP'
          }
        ]
      },
      displayOverlay: 'off',
      hostname: '',
      deviceModel: '',
      HWVer: '',
      macAddr: '',
      serialNum: '',
      delayDuration: '1',
      frontLock: '',
      powerSave: '',
      upgradeTime: '',
      standbyVer: '',
      version: '',
      dialogVisibleReset: false,
      dialogVisibleFactory: false,
      fileList: [],
      progress: 0,
      showProgress: false,
      upgradeInfo: [],
      upgradeComplete: false,
      fileError: false,
      isUpgrade: false,
      upgradeProgress: 0,
      uploadProgress: 0,
      errMsg: '',
      isExportConfig: false,
      uploadComplete: false
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#NAME? 0')
    this.$socket.sendMsg('#MODEL? ')
    this.$socket.sendMsg('#HW-VERSION? ')
    this.$socket.sendMsg('#NET-MAC? 0')
    this.$socket.sendMsg('#SN? ')
    this.$socket.sendMsg('#LOCK-FP? ')
    this.$socket.sendMsg('#UPG-TIME? ')
    this.$socket.sendMsg('#VERSION? ')
    this.$socket.sendMsg('#STANDBY-VERSION? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@NAME /i) !== -1) {
        this.handleHostname(msg)
        return
      }
      if (msg.search(/@MODEL /i) !== -1) {
        this.handleDeviceModel(msg)
        return
      }
      if (msg.search(/@HW-VERSION /i) !== -1) {
        this.handleHWVersion(msg)
        return
      }
      if (msg.search(/@NET-MAC /i) !== -1) {
        this.handleMACAddr(msg)
        return
      }
      if (msg.search(/@SN /i) !== -1) {
        this.handleSerialNum(msg)
        return
      }
      if (msg.search(/@LOCK-FP /i) !== -1) {
        this.handleFrontLock(msg)
        return
      }
      if (msg.search(/@STANDBY /i) !== -1) {
        this.handlePowerSave(msg)
        return
      }
      if (msg.search(/@UPG-TIME /i) !== -1) {
        this.handleUpgradeTime(msg)
        return
      }
      if (msg.search(/@STANDBY-VERSION /i) !== -1) {
        this.handleStandbyVer(msg)
        return
      }
      if (msg.search(/@VERSION /i) !== -1) {
        this.handleVersion(msg)
      }
      if (msg.search(/@UPGRADE-STATUS /i) !== -1) {
        this.handleUpgradeProgress(msg)
        return
      }
      if (msg.search(/@UPGRADE /i) !== -1) {
        this.isUpgrade = true
        this.$refs.upload.clearFiles()
        setTimeout(() => {
          this.$socket.sendMsg('#UPGRADE-STATUS? ')
        }, 3000)
      }
    },
    handleHostname (msg) {
      this.hostname = msg.split(',').slice(1).join(',')
    },
    handleDeviceModel (msg) {
      this.deviceModel = msg.split(' ')[1]
    },
    handleHWVersion (msg) {
      this.HWVer = msg.split(' ')[1]
    },
    handleMACAddr (msg) {
      this.macAddr = msg.split(' ')[1].split(',').pop()
    },
    handleSerialNum (msg) {
      this.serialNum = msg.split(' ')[1]
    },
    handleFrontLock (msg) {
      this.frontLock = msg.split(' ')[1]
    },
    handlePowerSave (msg) {
      this.powerSave = msg.split(' ')[1]
    },
    handleUpgradeTime (msg) {
      this.upgradeTime = msg.split(' ')[1]
    },
    handleStandbyVer (msg) {
      this.standbyVer = msg.split(' ')[1]
    },
    handleVersion (msg) {
      this.version = msg.split(' ')[1]
    },
    restart () {
      this.$socket.sendMsg('#RESET')
      sessionStorage.removeItem('login')
    },
    reset () {
      this.$socket.sendMsg('#FACTORY')
      sessionStorage.removeItem('login')
    },
    setHostName () {
      this.hostNameError = !this.isValidName(this.hostname)
      if (!this.hostNameError) {
        this.$socket.sendMsg(`#NAME 0,${this.hostname}`)
      }
    },
    setDeviceModel () {
      this.$socket.sendMsg(`#MODEL ${this.deviceModel}`)
    },
    locateDev () {
      this.$socket.sendMsg('#IDV')
    },
    rollBack () {
      this.$socket.sendMsg('#ROLLBACK')
    },
    setFrontLock (val) {
      this.$socket.sendMsg(`#LOCK-FP ${val}`)
    },
    setPowerSave (val) {
      this.$socket.sendMsg(`#STANDBY ${val}`)
    },
    upgradeFile () {
      this.uploadProgress = 100
      this.$socket.sendMsg('#UPGRADE ')
    },
    uploadError (err) {
      this.showProgress = false
      this.$refs.upload.clearFiles()
      setTimeout(() => {
        if (err?.message.search('406') !== -1) {
          alert('Invalid File')
        } else {
          alert('Upload failed, please try again')
        }
      }, 200)
    },
    beforeUpload (file) {
      if (!(file.name.endsWith('.bin') && file.size < 1024 * 1024 * 16)) {
        alert('Please choose the correct file!')
        return false
      }
      this.uploadProgress = 0
      this.upgradeProgress = 0
      this.isUpgrade = false
      this.upgradeComplete = false
      this.fileError = false
      this.showProgress = true
    },
    progressEvent (ev) {
      this.uploadProgress = parseInt(ev.percent)
    },
    handleUpgradeProgress (msg) {
      const val = msg.split(' ')[1]
      if (val.indexOf('err') !== -1) {
        this.fileError = true
        this.errMsg = val.split(',')[2]
        return
      }
      if (val.indexOf('ok') !== -1) {
        this.upgradeProgress = 100
        this.upgradeComplete = true
        return
      }
      if (val.indexOf('ongoing') !== -1) {
        const progress = parseInt(val.split(',')[1])
        this.upgradeProgress = progress
        setTimeout(() => {
          this.$socket.sendMsg('#UPGRADE-STATUS? ')
        }, progress > 96 ? 1500 : 3500)
      }
      // this.upgradeInfo.unshift('Upload completed')
    },
    exportConfig () {
      this.isExportConfig = false
      this.exportConfigVal = this.exportAndImport.val
      setTimeout(() => {
        this.isExportConfig = true
      }, 500)
    },
    importConfig () {
      this.$refs.uploadConfig.click()
    },
    browseConfig () {
      const file = this.$refs.uploadConfig.files[0]
      if (file) {
        const xhr = new XMLHttpRequest()
        const formData = new FormData()
        formData.append('file', file)
        xhr.open('POST', '/settings/import?method=' + this.exportAndImport.val)
        xhr.onload = oevent => {
          if (xhr.status === 200) {
            this.$refs.uploadConfig.value = ''
            this.uploadComplete = true
            setTimeout(() => {
              this.uploadComplete = false
            }, 2000)
          }
        }
        xhr.send(formData)
      }
    },
    isValidName (name) {
      return /^[a-zA-Z0-9]$|^[a-zA-Z0-9][\w-]{0,22}[a-zA-Z0-9]$/.test(name)
    }
  }
}
</script>
<style lang="less" scoped>
//.upload-file{
//  display: none;
//}
.upgrade-info {
  font-family: "open sans semiblold";
  color: #4d4d4f;
  font-size: 14px;
  margin: 10px 0;
}
</style>
