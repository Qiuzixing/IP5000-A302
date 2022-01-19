import Message from '../components/message'
// notify 提示

function warnAlert (message, delay) {
  Message({
    message: message || '',
    type: 'warning',
    duration: delay || 2000
  })
}
function successAlert (message, delay) {
  Message({
    message: message || 'Data saved',
    type: 'success',
    duration: delay || 2000
  })
}
function errorAlert (message, delay) {
  Message({
    message: message || '',
    type: 'error',
    duration: delay || 2000
  })
}

export default {
  errorAlert,
  successAlert,
  warnAlert
}
