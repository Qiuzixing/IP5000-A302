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
Mock.mock('/log/log', (option) => {
  return 'log12323131222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222213\n123\n123\n34'
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
