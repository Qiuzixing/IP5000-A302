function checkLanSettings (ip, mask, gateway) {
  if (!isValidSubnetMask(mask)) {
    // alert('局域网子网掩码无效')
    return 1
  }
  if (!isValidIpAddress(ip) || isBroadcastIp(ip, mask)) {
    // alert('局域网IP无效')
    return 2
  }

  if (!isIp4addr(gateway)) {
    return 3
  }
  if (ip === gateway) {
    return 4
  }
  if (!isValidIpAddressNetwork(ip, mask, gateway)) {
    // alert('局域网IP无效1111')
    return 5
  }

  return 0
}

function isValidSubnetMask (mask) {
  let i = 0
  let num = 0
  let zeroBitPos = 0
  let oneBitPos = 0
  let zeroBitExisted = false

  if (mask === '0.0.0.0') {
    return false
  }

  const maskParts = mask.split('.')
  if (maskParts.length !== 4) return false

  for (i = 0; i < 4; i++) {
    if (isNaN(maskParts[i]) === true) {
      return false
    }
    num = parseInt(maskParts[i])
    if (num < 0 || num > 255) {
      return false
    }
    if (zeroBitExisted === true && num !== 0) {
      return false
    }
    zeroBitPos = getLeftMostZeroBitPos(num)
    oneBitPos = getRightMostOneBitPos(num)
    if (zeroBitPos < oneBitPos) {
      return false
    }
    if (zeroBitPos < 8) {
      zeroBitExisted = true
    }
  }
  return true
}

function getLeftMostZeroBitPos (num) {
  let i = 0
  const numArr = [128, 64, 32, 16, 8, 4, 2, 1]

  for (i = 0; i < numArr.length; i++) {
    if ((num & numArr[i]) === 0) {
      return i
    }
  }

  return numArr.length
}

function getRightMostOneBitPos (num) {
  let i = 0
  const numArr = [1, 2, 4, 8, 16, 32, 64, 128]

  for (i = 0; i < numArr.length; i++) {
    if (((num & numArr[i]) >> i) === 1) {
      return (numArr.length - i - 1)
    }
  }

  return -1
}

function isValidIpAddress (address) {
  const ipParts = address.split('/')
  if (ipParts.length > 2) return false
  if (ipParts.length === 2) {
    const num = parseInt(ipParts[1])
    if (num <= 0 || num > 32) {
      return false
    }
  }
  if (ipParts[0] === '0.0.0.0' ||
    ipParts[0] === '255.255.255.255') {
    return false
  }

  const addrParts = ipParts[0].split('.')
  if (addrParts.length !== 4) return false

  for (let i = 0; i < 4; i++) {
    if (isNaN(addrParts[i]) || addrParts[i] === '') {
      return false
    }
    const num = parseInt(addrParts[i])
    if (num < 0 || num > 255) {
      return false
    }
  }
  return true
}

function isBroadcastIp (ipAddress, subnetMask) {
  var maskLenNum = 0
  const tmpMask = subnetMask.split('.')
  const tmpIp = ipAddress.split('.')

  if ((parseInt(tmpIp[0]) > 223) || (parseInt(tmpIp[0]) === 127)) {
    return true
  }

  for (maskLenNum = 0; maskLenNum < 4; maskLenNum++) {
    if (parseInt(tmpMask[maskLenNum]) < 255) {
      break
    }
  }

  const tmpNum0 = parseInt(tmpIp[maskLenNum])
  const tmpNum1 = 255 - parseInt(tmpMask[maskLenNum])
  const tmpNum2 = tmpNum0 & tmpNum1
  if ((tmpNum2 !== 0) && (tmpNum2 !== tmpNum1)) {
    return false
  }

  if (maskLenNum === 3) {
    return true
  } else if (maskLenNum === 2) {
    if (((tmpIp[3] === 0) && (tmpNum2 === 0)) ||
      ((tmpIp[3] === 255) && (tmpNum2 === tmpNum1))) {
      return true
    }
  } else if (maskLenNum === 1) {
    if (((tmpNum2 === 0) && (tmpIp[3] === 0) && (tmpIp[2] === 0)) ||
      ((tmpNum2 === tmpNum1) && (tmpIp[3] === 255) && (tmpIp[2] === 255))) {
      return true
    }
  } else if (maskLenNum === 0) {
    if (((tmpNum2 === 0) && (tmpIp[3] === 0) && (tmpIp[2] === 0) && (tmpIp[1] === 0)) ||
      ((tmpNum2 === tmpNum1) && (tmpIp[3] === 255) && (tmpIp[2] === 255) && (tmpIp[1] === 255))) {
      return true
    }
  }
  return false
}

function isValidIpAddressNetwork (ipAddr, netmask, gateway) {
  if (isIp4addr(ipAddr) && isIp4addr(netmask) && isIp4addr(gateway)) {
    // 判断ip参数是否有效
    const ipCheck = ipAddr.split('.')
    const nmCheck = netmask.split('.')
    const gwCheck = gateway.split('.')

    const ipArr = []
    const maskArr = []
    const gatewayArr = []

    ipArr[0] = 0xff & parseInt(ipCheck[0])
    ipArr[1] = 0xff & parseInt(ipCheck[1])
    ipArr[2] = 0xff & parseInt(ipCheck[2])
    ipArr[3] = 0xff & parseInt(ipCheck[3])

    gatewayArr[0] = 0xff & parseInt(gwCheck[0])
    gatewayArr[1] = 0xff & parseInt(gwCheck[1])
    gatewayArr[2] = 0xff & parseInt(gwCheck[2])
    gatewayArr[3] = 0xff & parseInt(gwCheck[3])

    maskArr[0] = 0xff & parseInt(nmCheck[0])
    maskArr[1] = 0xff & parseInt(nmCheck[1])
    maskArr[2] = 0xff & parseInt(nmCheck[2])
    maskArr[3] = 0xff & parseInt(nmCheck[3])

    // // 主机号第一个自己不能全为0 或者 1
    if (ipArr[3] & ~maskArr[3] === 0 || ipArr[3] & ~maskArr[3] === 255) {
      return false
    }
    if (!((((ipArr[0]) & (maskArr[0])) === ((gatewayArr[0]) & (maskArr[0]))) &&
      (((ipArr[1]) & (maskArr[1])) === ((gatewayArr[1]) & (maskArr[1]))) &&
      (((ipArr[2]) & (maskArr[2])) === ((gatewayArr[2]) & (maskArr[2]))) &&
      (((ipArr[3]) & (maskArr[3])) === ((gatewayArr[3]) & (maskArr[3]))))) {
      if (gateway === '0.0.0.0') {
        return true
      }
      // 设置的ip和网关不合法
      // console.error('Default gateway is not at the same network(subnet), which is defined on basis of IP address and subnet mask.')
      return false
    }
    return true
  } else {
    // ip地址设置错误
    // console.error('Please input correctly!')
    return false
  }

}

function isIp4addr (ip) {
  const reg = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
  return reg.test(ip)
}

export default checkLanSettings
