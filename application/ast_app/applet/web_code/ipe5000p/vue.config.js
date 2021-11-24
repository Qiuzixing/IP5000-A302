const version = 'V1.0.22'

const path = require('path')
function resolve (dir) {
  return path.join(__dirname, dir)
}
module.exports = {
  publicPath: './', // 默认'/'，部署应用包时的基本 URL
  outputDir: 'www', // 'dist', 生产环境构建文件的目录
  assetsDir: 'static', // 相对于outputDir的静态资源(js、css、img、fonts)目录
  lintOnSave: false,
  productionSourceMap: false, // 生产环境的 source map,
  filenameHashing: false,
  configureWebpack: {
    output: {
      // 输出重构  打包编译后的 文件名称  【模块名称.版本号】
      filename: `static/js/[name].js?${version}`,
      chunkFilename: `static/js/[name].js?${version}`
    }
  },
  chainWebpack (config) {
    config.module
      .rule('svg')
      .exclude.add(resolve('src/icons'))
      .end()
    config.module
      .rule('icons')
      .test(/\.svg$/)
      .include.add(resolve('src/icons'))
      .end()
      .use('svg-sprite-loader')
      .loader('svg-sprite-loader')
      .options({
        symbolId: 'icon-[name]'
      })
      .end()
  },
  devServer: {
    before (app) {
      app.post('/upload/upgradesoftware', function (req, res) {
        res.json({})
      })
    }
  }
}

// 测试命令使用
// 创建websocket服务器
if (process.env.NODE_ENV !== 'production') {
  var WebSocketServer = require('ws').Server
  var wss = new WebSocketServer({ port: 18888 })
  wss.on('connection', function connection (ws) {
    ws.on('message', function incoming (message) {
      if (message.startsWith('#X-ROUTE?')) {
        ws.send(`~nn@X-ROUTE out.hdmi.1.video.1,in.stream.${parseInt(Math.random() * 3 + 1)}.video.1`)
      } else if (message.startsWith('#KDS-DEFINE-CHANNEL?')) {
        ws.send('~nn@KDS-DEFINE-CHANNEL 1')
      } else if (message.startsWith('#KDS-DEFINE-CHANNEL-NAME?')) {
        ws.send('~nn@KDS-DEFINE-CHANNEL-NAME bbc')
      } else if (message.startsWith('#X-AUD-LVL?')) {
        ws.send('~nn@X-AUD-LVL out.analog_audio.1.audio.1,10')
      } else if (message.startsWith('#KDS-ACTION? ')) {
        ws.send(`~nn@KDS-ACTION ${Math.round(Math.random())}`)
      } else if (message.startsWith('#X-MUTE? ')) {
        ws.send('~nn@#X-MUTE audio.1,on')
      } else if (message.startsWith('#HDCP-STAT? ')) {
        ws.send(`~nn@HDCP-STAT 1,1,${Math.round(Math.random())}`)
      } else if (message.startsWith('#KDS-RESOL? ')) {
        ws.send(`~nn@KDS-RESOL 1,1,${Math.round(Math.random() * 30 + 1)}`)
      } else if (message.startsWith('#KDS-RATIO?')) {
        ws.send('~nn@KDS-RATIO 16:9')
      } else if (message.startsWith('#X-AUD-DESC?')) {
        ws.send('~nn@X-AUD-DESC out.hdmi.1.audio.1,2,44,1K,PCM')
      } else if (message.startsWith('#X-AV-SW-MODE?')) {
        ws.send(`~nn@X-AV-SW-MODE out.hdmi.1.video.1,${parseInt(Math.random() * 3)}`)
        ws.send(`~nn@X-AV-SW-MODE out.hdmi.1.audio.1,${parseInt(Math.random() * 3)}`)
      } else if (message.startsWith('#X-PRIORITY?')) {
        // ws.send('~nn@X-PRIORITY out.stream.1.video, [in.usb_c.3.video,in.hdmi.1.video,in.hdmi.2.video]')
        // ws.send('~nn@X-PRIORITY out.stream.1.audio, [in.dante.1.audio,in.hdmi.1.audio,in.analog_audio.1.audio]')
        ws.send('~nn@X-PRIORITY out.stream.1.audio, [in.hdmi.1.audio]')
      } else if (message.startsWith('#HDCP-MOD? ')) {
        ws.send(`~nn@HDCP-MOD 1,${Math.round(Math.random())}`)
        ws.send(`~nn@HDCP-MOD 2,${Math.round(Math.random())}`)
        ws.send(`~nn@HDCP-MOD 3,${Math.round(Math.random() * 4)}`)
      } else if (message.startsWith('#CS-CONVERT? ')) {
        ws.send(`~nn@CS-CONVERT 1,${Math.round(Math.random())}`)
      } else if (message.startsWith('#PORT-DIRECTION? ')) {
        ws.send('~nn@PORT-DIRECTION both.ir,1.ir,IN')
        ws.send('~nn@PORT-DIRECTION both.analog.1.audio,OUT')
      } else if (message.startsWith('NET-STAT ')) {
        ws.send('~01@NET-STAT [(TCP:80,0.0.0.0:0),LISTEN],[(TCP:5000,0.0.0.0:0),LISTEN],[(TCP:80,192.168.114.3:52400),ESTABLISHED],[(TCP:5000,192.168.1.100:51647),ESTABLISHED]')
      } else if (message.startsWith('#KDS-AUD-OUTPUT? ')) {
        ws.send('~nn@KDS-AUD-OUTPUT [1,2,3]')
      } else if (message.startsWith('#LOCK-EDID? ')) {
        ws.send(`~nn@LOCK-EDID 1,${Math.round(Math.random())}`)
      } else if (message.startsWith('#EDID-MODE? ')) {
        ws.send(`~nn@EDID-MODE 1,CUSTOM,${Math.round(Math.random()) * 5}`)
      } else if (message.startsWith('#EDID-NET-SRC? ')) {
        ws.send('~nn@EDID-NET-SRC  1,00-14-22-01-23-45')
      } else if (message.startsWith('#EDID-LIST? ')) {
        ws.send('~nn@EDID-LIST [0,"DEFAULT"],[1,"1-1"],[2,"1-2"],[3,"1-3"],[4,"SONY"],[5,"PANASONIC"]')
      } else if (message.startsWith('#NAME? ')) {
        ws.send('~nn@NAME room-442')
      } else if (message.startsWith('#MODEL? ')) {
        ws.send(`~nn@MODEL ${['KDS-EN7', 'KDS-SW3-EN7', 'WP-SW2-EN7'][Math.floor(Math.random() * 3)]}`)
      } else if (message.startsWith('#HW-VERSION? ')) {
        ws.send('~nn@HW-VERSION 1.12.123')
      } else if (message.startsWith('#NET-MAC? ')) {
        ws.send('~nn@NET-MAC 00-14-22-01-23-45')
      } else if (message.startsWith('#SN? ')) {
        ws.send('~nn@SN 19763840581123')
      } else if (message.startsWith('#LOCK-FP? ')) {
        ws.send(`~nn@LOCK-FP ${Math.round(Math.random())}`)
      } else if (message.startsWith('#STANDBY? ')) {
        ws.send(`~nn@STANDBY ${Math.round(Math.random())}`)
      } else if (message.startsWith('#VERSION? ')) {
        ws.send('~nn@VERSION 1.12.123')
      } else if (message.startsWith('#TIME? ')) {
        ws.send('~nn@TIME wen,05-12-2018,14:30:00')
      } else if (message.startsWith('#NET-DHCP? ')) {
        ws.send('~nn@NET-DHCP 0,0')
        ws.send('~nn@NET-DHCP 1,1')
      } else if (message.startsWith('#NET-CONFIG? ')) {
        ws.send('~nn@NET-CONFIG 0,192.168.113.10,255.255.0.0,192.168.0.1')
        ws.send('~nn@NET-CONFIG 1,192.168.113.10,255.255.0.0,192.168.0.1,8.8.8.8,0.0.0.0')
      } else if (message.startsWith('#KDS-DANTE-VLANTAG? ')) {
        ws.send('~nn@KDS-DANTE-VLANTAG vlanTag')
      } else if (message.startsWith('#KDS-GW-ETH? ')) {
        ws.send('~nn@KDS-GW-ETH 0,0')
        ws.send('~nn@KDS-GW-ETH 1,1')
        ws.send('~nn@KDS-GW-ETH 2,0')
      } else if (message.startsWith('#CEC-GW-PORT-ACTIVE? ')) {
        ws.send('~nn@CEC-GW-PORT-ACTIVE 0')
      } else if (message.startsWith('#LOGIN ')) {
        ws.send('~nn@login ,ok')
      } else {
        ws.send(message.replace(/#/i, '@'))
      }
    })
    ws.on('error', e => {
      console.log(e)
    })
  })
}
