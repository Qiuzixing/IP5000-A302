import Mock from 'mockjs'

Mock.mock('/switch/auto_switch_delays', (option) => {
  return {
    'Auto Switch Delays': {
      'signal loss switching': 5,
      'signal detection': 5,
      'cable unplug': 10,
      'power off upon signal loss': 15,
      'manual-override inactive-signal': 15
    }
  }
})

Mock.mock('/av_signal', (option) => {
  return {
    'AV Signal': {
      'input maximum resolution': 'Pass Through',
      'maximum bit rate': 'Best Effort',
      'frame rate percentage': 100,
      'color depth': '8-bit',
      'audio connection guard time sec': 12,
      'dante vlan tag': 'DNT'
    }
  }
})

Mock.mock('/display/display_sleep', (option) => {
  return {
    'Display Delays': {
      'sleep delay on signal loss sec': 5,
      'shutdown delay on signal loss sec': 10,
      'wake-up delay on signal detection sec': 10
    }
  }
})
Mock.mock('/log/log', (option) => {
  return 'log 1232313213'
})
Mock.mock('/channel/channel_map', (option) => {
  return {
    channels_list: [
      { id: '1', name: 'ch1' },
      { id: '2', name: 'ch2' },
      { id: '3', name: 'ch3' },
      { id: '4', name: 'ch4' },
      { id: '5', name: 'ch5' },
      { id: '6', name: 'ch6' },
      { id: '7', name: 'ch7' },
      { id: '8', name: 'ch8' },
      { id: '9', name: 'ch9' },
      { id: '10', name: 'ch10' }
    ]
  }
})

Mock.mock('/osd/overlay', (option) => {
  return {
    'Channel menu': {
      timeout_sec: 30,
      position: 'center',
      'font size': 'Small',
      'max channels_per page': 10,
      'max channels': 999
    },
    'Device Info': {
      Enabled: 'ON',
      Timeout: 2
    }
  }
})

Mock.mock('/usb/km_usb', (option) => {
  return {
    'USB-KVM_config': {
      kvm_usb_mode: '1',
      kvm_timeout_sec: 10,
      km_roaming: [
        { mac: '008414A0B026', h: 2, v: 1 },
        { mac: '08414A0B023', h: 0, v: 1 },
        { mac: '08414A0B023', h: 1, v: 1 },
        { mac: '08414A0B023', h: -1, v: 1 },
        { mac: '08414A0B023', h: -1, v: 0 },
        { mac: '08414A0B023', h: 0, v: 0 },
        { mac: '08414A0B023', h: 1, v: 0 },
        { mac: '08414A0B023', h: 2, v: 0 },
        { mac: '08414A0B023', h: -1, v: -1 },
        { mac: '08414A0B023', h: 0, v: -1 },
        { mac: '08414A0B023', h: 1, v: -1 },
        { mac: '08414A0B023', h: 2, v: -1 }
      ]
    }
  }
})
