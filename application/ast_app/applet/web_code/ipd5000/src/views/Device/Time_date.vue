<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Date</span>
        <el-date-picker :disabled="true"
                        style="width: 180px;"
                        v-model="date"
                        type="date"
                        value-format="dd-MM-yyyy"
                        format="dd-MM-yyyy">
        </el-date-picker>
        <!--        <VueCtkDateTimePicker id="sys-date" color="#35ACF8" v-model="dateTime" format="MM/DD/YYYY" formatted="MM/DD/YYYY" :no-clear-button="true" :no-label="true" style="width: 160px;margin: 0" :only-date="true" />-->
      </div>
      <div class="setting">
        <span class="setting-title">Time</span>
        <el-time-picker :disabled="true"
                        style="width: 180px;"
                        v-model="time"
                        value-format="HH:mm:ss"
                        :picker-options="{
            selectableRange: '00:00:00 - 23:59:59'
          }">
        </el-time-picker>
      </div>
      <div class="setting">
        <span class="setting-title">Time Zone</span>
        <el-select v-model="timeVal">
          <el-option
            v-for="item in timeZone"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
      <div class="setting">
        <span class="setting-title">NTP Time Server Usage</span>
        <div style="width:180px;flex-shrink:0;">
          <v-switch v-model="ntpMode"
                    style="width:120px;"
                    open-text="Yes"
                    close-text="No"
                    active-value="1"
                    inactive-value="0"></v-switch>
        </div>
<!--        <p style="margin:0;flex-shrink:0;padding:0"><span style="font-family: 'open sans bold';">WARNING:</span> Recommended to use time server. <br>Defined time will not be saved upon reboot.</p>-->
      </div>
      <div class="setting">
        <span class="setting-title">NTP Time Server Address</span>
        <input type="text"
               class="setting-text"
               :disabled="ntpMode === '0'"
               v-model="ntpServer">
      </div>
      <div class="setting">
        <span class="setting-title">NTP Daily Sync Hour</span>
        <el-select v-model="ntpDailySync" :disabled="ntpMode === '0'">
          <el-option
            v-for="item in ntpParam"
            :key="item.value"
            :label="item.label"
            :value="item.value">
          </el-option>
        </el-select>
      </div>
    </div>
    <footer>
      <button class="btn btn-primary"
              @click="save">SAVE</button>
    </footer>
  </div>
</template>

<script>
import { debounce } from 'lodash'
export default {
  name: 'timeDate',
  components: {
  },
  data () {
    return {
      date: '',
      time: '',
      timeVal: '0',
      daylight: '0',
      timeZone: [
        {
          value: '-12',
          label: '-12:00 International Date Line West'
        },
        {
          value: '-11',
          label: '-11:00 Midway Island/Samoa'
        },
        {
          value: '-10',
          label: '-10:00 Hawaii'
        },
        {
          value: '-9',
          label: '-09:00 Alaska'
        },
        {
          value: '-8',
          label: '-08:00 Pacific Time/Tijuana'
        },
        {
          value: '-7',
          label: '-07:00 Mountain Time'
        },
        {
          value: '-6',
          label: '-06:00 Central Time'
        },
        {
          value: '-5',
          label: '-05:00 Eastern Time'
        },
        {
          value: '-4',
          label: '-04:00 Atlantic Time'
        },
        // {
        //   value: 'right',
        //   label: '-03:30 Newfoundland'
        // },
        {
          value: '-3',
          label: '-03:00 Brasilia/Georgetown/Greenland'
        },
        {
          value: '-2',
          label: '-02:00 Mid-Atlantic'
        },
        {
          value: '-1',
          label: '-01:00 Azores, Cape Verde Is.'
        },
        {
          value: '0',
          label: '00:00 Greenwich Mean Time:Dublin/Edinburgh/Lisbon/London'
        },
        {
          value: '1',
          label: '+01:00 Amsterdam/Berlin/Rome/Paris/Stockholm/Vienna'
        },
        {
          value: '2',
          label: '+02:00 Athens/Beirut/Bucharest/Istanbul/Jerusalem/Minsk'
        },
        {
          value: '3',
          label: '+03:00 Baghdad/Kuwait/Moscow/St. Petersburg/Riyadh'
        },
        {
          value: '4',
          label: '+04:00 Abu Dhabi/Muscat'
        },
        {
          value: '5',
          label: '+05:00 Ekaterinburg/Islamabad/Karachi/Tashkent'
        },
        // {
        //   value: 'right',
        //   label: '+05:30 Chennai/Kolkata/Mumbai/New Delhi'
        // },
        // {
        //   value: 'right',
        //   label: '+05:45 Kathmandu'
        // },
        {
          value: '6',
          label: '+06:00 Almaty/Astana/Dhaka/Novosibirsk'
        },
        {
          value: '7',
          label: '+07:00 Bangkok/Hanoi/Jakarta/Krasnoyarsk'
        },
        {
          value: '8',
          label: '+08:00 Beijing/Chongqing/Hong Kong/Singapore/Taipei'
        },
        {
          value: '9',
          label: '+09:00 Osaka/Sapporo/Seoul/Tokyo/Yakutsk'
        },
        {
          value: '10',
          label: '+10:00 Brisbane/Canberra/Guam/Melbourne/Sydney'
        },
        {
          value: '11',
          label: '+11:00 Magadan/Solomon Is./New Caledonia'
        },
        {
          value: '12',
          label: '+12:00 Fiji/Kamchatka/Marshall Is.'
        }
      ],
      ntpMode: '0',
      ntpServer: '',
      ntpDailySync: '0',
      saveFlag: false,
      ntpParam: Array.from({ length: 24 }).map((_, i) => { return { value: i + '', label: i + '' } })
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#TIME? ')
    this.$socket.sendMsg('#TIME-LOC? ')
    this.$socket.sendMsg('#TIME-SRV? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@TIME /i) !== -1) {
        this.handleTime(msg)
        return
      }
      if (msg.search(/@TIME-LOC /i) !== -1) {
        this.handleTimeZone(msg)
        return
      }
      if (msg.search(/@TIME-SRV /i) !== -1) {
        this.handleNTP(msg)
      }
    },
    handleTime (msg) {
      const data = msg.split(',')
      // const dateArr = data[1].split('-')
      this.date = data[1]
      this.time = data[2]
    },
    handleTimeZone (msg) {
      const data = msg.split(' ')[1].split(',')
      this.timeVal = data[0]
      this.daylight = data[1]
    },
    handleNTP (msg) {
      const data = msg.split(' ')[1].split(',')
      this.ntpMode = data[0]
      this.ntpServer = data[1]
      this.ntpDailySync = data[2]
      if (this.saveFlag) {
        this.saveFlag = false
        this.$msg.successAlert()
      }
    },
    setDateTime () {
      const weekday = ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT']
      const week = weekday[new Date(this.date).getDay()]
      this.$socket.sendMsg('#TIME ' + week + ',' + this.date + ',' + this.time)
    },
    formatTime (time) {
      return time < 9 ? '0' + time : time.toString()
    },
    setDaylight () {
      this.$socket.sendMsg(`#TIME-LOC ${this.timeVal},${this.daylight}`)
    },
    setNTP () {
      this.$socket.sendMsg(`#TIME-SRV ${this.ntpMode},${this.ntpServer || '0.0.0.0'},${this.ntpDailySync}`)
    },
    save: debounce(function () {
      this.saveFlag = true
      this.setDaylight()
      this.setNTP()
      if (this.ntpMode === '0') {
        this.setDateTime()
      }
    }, 2000, {
      leading: true,
      trailing: true
    })
  }
}
</script>

<style scoped>
.setting-title {
  width: 220px;
}
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
</style>
<style lang="less">
.el-date-editor .el-input__icon {
  line-height: 30px;
  color: #828283;
  font-weight: 600;
}

</style>
