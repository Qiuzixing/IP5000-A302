/**************************************************************************
    Command                          CmdData

    // update command
    CMD_UPDATE_GET_MCU_STATUS,      N/A
    CMD_UPDATE_MCU_STATUS,          struct CmdDataMCUStatus  
    CMD_UPDATE_SEND_FILE,           N/A
    CMD_UPDATE_READY_REC_FILE,      N/A
    CMD_UPDATE_GET_CRC,             N/A
    CMD_UPDATE_CRC,                 struct CmdDataCRC
    CMD_UPDATE_JUMP_TO_APP,         N/A
    CMD_UPDATE_GET_MCU_VERSION,     N/A
    CMD_UPDATE_MCU_VERSION,         struct CmdDataMCUVersion
    CMD_UPDATE_JUMP_TO_BOOT,        N/A
    CMD_UPDATE_READY_TO_BOOT,       N/A
    
    // video command
    EVENT_HDMI_LINK_STATUS,         struct CmdDataLinkStatus  
    CMD_HDMI_GET_LINK_STATUS,       uint32_t port
    EVENT_HDMI_EDID,                struct CmdDataEDID
    CMD_HDMI_GET_EDID,              uint32_t port
    -CMD_HDMI_SET_EDID,             struct CmdDataEDID
    EVENT_HDMI_VIDEO_STATUS,        struct CmdDataVideoStatus
    CMD_HDMI_GET_VIDEO_STATUS,      uint32_t port
    -CMD_HDMI_VIDEO_CONTROL,        struct CmdDataVideoControl
    -CMD_HDMI_SET_VIDEO_MODE,       struct CmdDataVideoMode
    CMD_HDMI_SET_INPUT_SOURCE,      struct CmdDataInputSorce
    CMD_HDMI_GET_INPUT_SOURCE,      struct CmdDataInputSorce
    CMD_HDMI_INPUT_SOURCE,          struct CmdDataInputSorce
    
    // HDCP command
    EVENT_HDCP_CAP,                 struct CmdDataHDCPCap
    CMD_HDCP_GET_CAP,               uint32_t port
    CMD_HDCP_SET_CAP,               struct CmdDataHDCPCap
    EVENT_HDCP_STATUS,              struct CmdDataHDCPStatus
    CMD_HDCP_GET_STATUS,            uint32_t port
    CMD_HDCP_SET_MODE,              struct CmdDataHDCPMode
    CMD_HDCP_GET_MODE,              uint32_t port
    CMD_HDCP_MODE,                  struct CmdDataHDCPMode
    
    // audio command
    EVENT_HDMI_AUDIO_STATUS,        struct CmdDataAudioStatus
    CMD_HDMI_GET_AUDIO_STATUS,      uint32_t port
    CMD_HDMI_AUDIO_CONTROL,         struct CmdDataAudioControl
    CMD_HDMI_SET_AUDIO_INSERT_EXTRACT, struct CmdDataAudioInsertAndExtract

    // LCD/LED/KEY command
    -EVENT_KEY_STATUS,              struct CmdDataKey
    -CMD_LED_CONTROL,               struct CmdDataLEDControl
    -CMD_LCD_GET_TYPE,              N/A
    -CMD_LCD_TYPE,                  struct CmdDataLCDType
    -CMD_LCD_CONTROL,               struct CmdDataLCDControl
    -CMD_LCD_SET_CONTENT,           struct CmdDataLCDContect

    // other command
    CMD_UART_PASSTHROUGH,           struct CmdDataUartPassthrough
    CMD_GPIO_CONFIG                 struct CmdDataGpioCfg
    EVENT_GPIO_VAL                  struct CmdDataGpioVal
    CMD_GPIO_GET_VAL                struct CmdDataGpioList
    CMD_GPIO_SET_VAL                struct CmdDataGpioVal

    Note: prefix '-' indicates that the command is not implemented.
 **************************************************************************/

#ifndef __IPX5000_A30_CMD_H__
#define __IPX5000_A30_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "command_protocol_type.h"

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
//typedef unsigned long   uint32_t;
typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t; 
typedef signed char     bool_t;

#pragma pack(1)
enum IPX5000_A30_CMD {
    // update command
    CMD_UPDATE_GET_MCU_STATUS = STM32_CMD_START_RANGE + 0x00,
    CMD_UPDATE_MCU_STATUS,
    CMD_UPDATE_SEND_FILE,
    CMD_UPDATE_READY_REC_FILE,
    CMD_UPDATE_GET_CRC,
    CMD_UPDATE_CRC,
    CMD_UPDATE_JUMP_TO_APP,
    CMD_UPDATE_GET_MCU_VERSION,
    CMD_UPDATE_MCU_VERSION,
    CMD_UPDATE_JUMP_TO_BOOT,
    CMD_UPDATE_READY_TO_BOOT,
    
    // video command
    EVENT_HDMI_LINK_STATUS = STM32_CMD_START_RANGE + 0x10,
    CMD_HDMI_GET_LINK_STATUS,
    EVENT_HDMI_EDID,
    CMD_HDMI_GET_EDID,
    CMD_HDMI_SET_EDID,
    EVENT_HDMI_VIDEO_STATUS,
    CMD_HDMI_GET_VIDEO_STATUS,
    CMD_HDMI_VIDEO_CONTROL,
    CMD_HDMI_SET_VIDEO_MODE,
    CMD_HDMI_SET_INPUT_SOURCE,
    CMD_HDMI_GET_INPUT_SOURCE,
    CMD_HDMI_INPUT_SOURCE,
    
    // HDCP command
    EVENT_HDCP_CAP = STM32_CMD_START_RANGE + 0x20,
    CMD_HDCP_GET_CAP,
    CMD_HDCP_SET_CAP,
    EVENT_HDCP_STATUS,
    CMD_HDCP_GET_STATUS,
    CMD_HDCP_SET_MODE,
    CMD_HDCP_GET_MODE,
    CMD_HDCP_MODE,
    
    // audio command
    EVENT_HDMI_AUDIO_STATUS = STM32_CMD_START_RANGE + 0x30,
    CMD_HDMI_GET_AUDIO_STATUS,
    CMD_HDMI_AUDIO_CONTROL,
    CMD_HDMI_SET_AUDIO_INSERT_EXTRACT,
    
    // LCD/LED/KEY command
    EVENT_KEY_STATUS = STM32_CMD_START_RANGE + 0x40,
    CMD_LED_CONTROL,
    CMD_LCD_GET_TYPE,
    CMD_LCD_TYPE,
    CMD_LCD_CONTROL,
    CMD_LCD_SET_CONTENT,

    // other command
    CMD_UART_PASSTHROUGH = STM32_CMD_START_RANGE + 0x50,
    CMD_GPIO_CONFIG,
    EVENT_GPIO_VAL,
    CMD_GPIO_GET_VAL,
    CMD_GPIO_SET_VAL,
};

enum MCUStatus {
    MCU_STATUS_BOOT,
    MCU_STATUS_APP,
};

struct CmdDataMCUStatus {
    uint32_t status; // 0-boot, 1-app
};

struct CmdDataCRC {
    uint32_t crc; // when using CRC16, the high 16bit is not use, and aways is zero. 
};

struct CmdDataMCUVersion {
    uint8_t version[20]; // string array, end of '\0'. 
};

enum HDMIPort {
    HDMIRX1 = 0x00,
    HDMIRX2,
    HDMIRX3,

    HDMITX1 = 0x10,
    HDMITX2,
};

struct CmdDataLinkStatus {
    uint8_t port; // enum HDMIPort
    bool_t  isConnect; // hdmirx is 5V status, hdmitx is RxSense status.
    bool_t  isHpd; 
    uint8_t rsvd;
};

struct CmdDataEDID {
    uint8_t port; // enum HDMIPort
    uint8_t rsvd[3];
    uint8_t rawEdid[256];
};

enum ColorSpace {
    CS_RGB,
    CS_YCbcR422,
    CS_YCbCr444,
    CS_YCbCr420,
    CS_AUTO,
};

enum DeepColor {
    DC_8BIT,
    DC_10BIT,
    DC_12BIT,
    DC_16BIT,
    DC_AUTO,
};

struct CmdDataVideoStatus {
    uint8_t port; // enum HDMIPort
    bool_t  isStable;
    bool_t  isMute;
    uint8_t rsvd;

    /* hacitve, vactive and frameRate only valid when isStable is true */
    uint16_t hactive; 
    uint16_t vactive;
    uint8_t frameRate;
    uint8_t colorspace;
    uint8_t deepcolor;
    uint8_t rsvd1;
};

struct CmdDataVideoControl {
    uint8_t port; // enum HDMIPort
    bool_t  enable;
    bool_t  mute;    
    uint8_t rsvd;
};

struct CmdDataVideoMode {
    uint8_t port; // enum HDMIPort
    uint8_t colorspace;
    uint8_t deepcolor;
    uint8_t rsvd;
};

struct CmdDataInputSorce {
    uint8_t txPort; // enum HDMIPort
    uint8_t rxPort; // enum HDMIPort
    uint8_t rsvd[2];
};

enum HDCPCap {
    HDCP_CAP_NONE,
    HDCP_CAP_HDCP14,
    HDCP_CAP_HDCP22,
    HDCP_CAP_HDCP14_HDCP22,
    HDCP_CAP_FLOW_SINK,
};

enum HDCPStatus {
    HDCP_STATUS_NONE,
    HDCP_STATUS_HDCP14,
    HDCP_STATUS_HDCP22,
};

enum HDCPMode {
    HDCP_MODE_NONE,
    HDCP_MODE_HDCP14,
    HDCP_MODE_HDCP22,
    HDCP_MODE_AUTO,
    HDCP_MODE_FLOW_SOURCE,
};

struct CmdDataHDCPCap {
    uint8_t port; // enum HDMIPort
    uint8_t cap;  // enum HDCPCap
    uint8_t rsvd[2];
};

struct CmdDataHDCPStatus {
    uint8_t port; // enum HDMIPort
    bool_t  isEncrypted;
    uint8_t status;  // enum HDCPStatus
    uint8_t rsvd;
};

struct CmdDataHDCPMode {
    uint8_t port; // enum HDMIPort
    uint8_t mode; // enum HDCPMode
    uint8_t rsvd[2];
};

enum AudioPort {
    AUDIO_PORT_HDMIRX1 = 0x00,
    AUDIO_PORT_HDMIRX2,
    AUDIO_PORT_HDMIRX3,

    AUDIO_PORT_HDMITX1 = 0x10,
    AUDIO_PORT_HDMITX2,

    AUDIO_PORT_TTL1 = 0x20,
    AUDIO_PORT_TTL2,

    AUDIO_PORT_ARC1 = 0x30,
    AUDIO_PORT_ARC2,
    AUDIO_PORT_ARC3,
    AUDIO_PORT_ARC4,
};

enum TTLMode {
    TTLMODE_I2S,
    TTLMODE_SPDIF,
};

enum MclkRatio {
    MCLK_RATIO_128FS,
    MCLK_RATIO_256FS,
    MCLK_RATIO_384FS,
    MCLK_RATIO_512FS,
};

enum I2sFormat {
    I2S_FORMAT_NORMAL,
    I2S_FORMAT_RIGHT_JUST,
    I2S_FORMAT_LEFT_JUST,
    MCLK_RATIO_AES3,
};

enum AudioCodingType {
    AUDIO_CODING_LPCM,
    AUDIO_CODING_AC3,
    AUDIO_CODING_MPEG1,
    AUDIO_CODING_MP3,
    AUDIO_CODING_MPEG2,
    AUDIO_CODING_AAC_LC,
    AUDIO_CODING_DTS,
    AUDIO_CODING_ATRAC,
    AUDIO_CODING_ONE_BIT_AUDIO,
    AUDIO_CODING_EAC3,
    AUDIO_CODING_DTSHD,
    AUDIO_CODING_MAT,
    AUDIO_CODING_DST,
    AUDIO_CODING_WMA_PRO,
};

enum AudioSampleFreq {
    AUDIO_SF_32KHZ,
    AUDIO_SF_44KHZ,
    AUDIO_SF_48KHZ,
    AUDIO_SF_88KHZ,
    AUDIO_SF_96KHZ,
    AUDIO_SF_176KHZ,
    AUDIO_SF_192KHZ,
    AUDIO_SF_768KHZ,
};

enum AudioSampleDepth {
    AUDIO_SD_16BIT,
    AUDIO_SD_20BIT,
    AUDIO_SD_24BIT,
};

enum AudioChannels {
    AUDIO_CH_2 = 2,
    AUDIO_CH_3,
    AUDIO_CH_4,
    AUDIO_CH_5,
    AUDIO_CH_6,
    AUDIO_CH_7,
    AUDIO_CH_8,
};

struct CmdDataAudioInsertAndExtract {
    uint8_t fromPort; // enum AudioPort
    uint8_t toPort;   // enum AudioPort
    uint8_t ttlMode;  // enum TTLMode

    // only use for TTLMODE_I2S and TTLMODE_HBR
    bool_t  useExtMclk;
    uint8_t mclkRatio; // enum MclkRatio
    uint8_t i2sFormat; // enum I2sFormat

    uint8_t audioCoding; // enum AudioCodingType
    uint8_t sampleFreq;  // enum AudioSampleFreq
    uint8_t sampleDepth; // enum AudioSampeDepth
    uint8_t channels;    // enum AudioChannels
    uint8_t rsvd[2];
};

struct CmdDataAudioControl {
    uint8_t port; // enum HDMIPort
    bool_t  enable; 
    bool_t  mute;
};

struct CmdDataAudioStatus {
    uint8_t port; // enum AudioPort
    uint8_t fromPort; // enum AudioPort
    bool_t  isStable;
    bool_t  isMute;

    uint8_t audioCoding; // enum AudioCodingType
    uint8_t sampleFreq;  // enum AudioSampleFreq
    uint8_t sampleDepth; // enum AudioSampeDepth
    uint8_t channels;    // enum AudioChannels
};

enum KeyVal {
    KEY_PRESS,
    KEY_RELEASE,
};

enum Key {
    KEY1,
    KEY2,
    KEY3,
    KEY4,
    KEY5,
};

struct CmdDataKey {
    uint8_t key; // enum Key
    uint8_t value;
    uint8_t rsvd[2];
};

enum Led {
    LED1,
    LED2,
    LED3,
    LED4,
    LED5,
};

enum LEDMode {
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_FLASH,
};

struct CmdDataLEDControl {
    uint8_t led;  // enum Led
    uint8_t mode; // enum LEDMode
    uint8_t onTime;  // ms
    uint8_t offTime; // ms
};

enum LCDType {
    LCD_TYPE_LED,
    LCD_TYPE_LCD,
};

struct CmdDataLCDType {
    uint8_t type; // enum LCDType
    uint8_t rsvd[3];
};

struct CmdDataLCDControl {
    uint8_t type; // enum LCDType
    bool_t enable; 

    // only use for LCD
    bool_t backLightOn; 
    bool_t clearScreen; 
    bool_t refresh;
};

struct CmdDataLCDContect {
    uint8_t type; // enum LCDType
    
    // only use for LED
    uint8_t number; 

    // only use for LCD
    uint8_t x;
    uint8_t y;
    uint16_t xSize;
    uint16_t ySize; 
    uint16_t dataLength;
    /**
     * variable length array, allocate memory as needed. Due to protocol restrictions,
     * the maximum length can only be 502bytes. If you need to display full-screen
     * content (128*64/8)bytes, it needs to be split into several times for transmission.
     */
    uint8_t data[0]; 
};

enum UartPort {
    UART_PORT_1,
    UART_PORT_2,
    UART_PORT_3,
    UART_PORT_4,
    UART_PORT_5,
};

struct CmdDataUartPassthrough {
    uint8_t fromPort; // enum UartPort
    uint8_t toPort;   // enum UartPort

    uint16_t dataLength;
    /**
     * variable length array, allocate memory as needed. Due to protocol restrictions,
     * the maximum length can only be 508bytes. 
     */
    uint8_t data[0]; 
};

enum GpioPin {
    GPIOA_PIN0 = 0x00,
    GPIOA_PIN1,
    GPIOA_PIN2,
    GPIOA_PIN3,
    GPIOA_PIN4,
    GPIOA_PIN5,
    GPIOA_PIN6,
    GPIOA_PIN7,
    GPIOA_PIN8,
    GPIOA_PIN9,
    GPIOA_PIN10,
    GPIOA_PIN11,
    GPIOA_PIN12,
    GPIOA_PIN13,
    GPIOA_PIN14,
    GPIOA_PIN15,

    GPIOB_PIN0 = 0x20,
    GPIOB_PIN1,
    GPIOB_PIN2,
    GPIOB_PIN3,
    GPIOB_PIN4,
    GPIOB_PIN5,
    GPIOB_PIN6,
    GPIOB_PIN7,
    GPIOB_PIN8,
    GPIOB_PIN9,
    GPIOB_PIN10,
    GPIOB_PIN11,
    GPIOB_PIN12,
    GPIOB_PIN13,
    GPIOB_PIN14,
    GPIOB_PIN15,

    GPIOC_PIN0 = 0x40,
    GPIOC_PIN1,
    GPIOC_PIN2,
    GPIOC_PIN3,
    GPIOC_PIN4,
    GPIOC_PIN5,
    GPIOC_PIN6,
    GPIOC_PIN7,
    GPIOC_PIN8,
    GPIOC_PIN9,
    GPIOC_PIN10,
    GPIOC_PIN11,
    GPIOC_PIN12,
    GPIOC_PIN13,
    GPIOC_PIN14,
    GPIOC_PIN15,

    GPIOD_PIN0 = 0x60,
    GPIOD_PIN1,
    GPIOD_PIN2,
    GPIOD_PIN3,
    GPIOD_PIN4,
    GPIOD_PIN5,
    GPIOD_PIN6,
    GPIOD_PIN7,
    GPIOD_PIN8,
    GPIOD_PIN9,
    GPIOD_PIN10,
    GPIOD_PIN11,
    GPIOD_PIN12,
    GPIOD_PIN13,
    GPIOD_PIN14,
    GPIOD_PIN15,
};

enum GpioDirection {
    GPIO_INPUT,
    GPIO_OUTPUT,
};

enum GpioVal {
    GPIO_OFF,
    GPIO_ON,
};

struct CmdDataGpioCfg {
    uint16_t numOfGpio;
    bool_t   active;
    uint8_t  rsvd;
    /**
     * variable length array, allocate memory as needed.
     * gpio[][0] is enum GpioPin, gpio[][1] is enum GpioDirection
     */
    uint8_t gpio[0][2];
};

struct CmdDataGpioVal {
    uint16_t numOfGpio;
    /**
     * variable length array, allocate memory as needed.
     * 
     * for input pin, when command is CMD_GPIO_GET_VAL, 
     * the gpio[][0] is enum GpioPin, the gpio[][1] is enum GpioVal.
     * for input pin, CMD_GPIO_SET_VAL is not valid.
     *  
     * for output pin, when command is CMD_GPIO_GET_VAL, 
     * the gpio[][0] is enum GpioPin, the gpio[][1] is enum GpioDirection.
     * for output pin, when command is CMD_GPIO_SET_VAL, 
     * the gpio[][0] is enum GpioPin, the gpio[][1] is enum GpioVal.
     */
    uint8_t gpio[0][2];
};

struct CmdDataGpioList {
    uint16_t numOfGpio;
    /**
     * variable length array, allocate memory as needed.
     */
    uint8_t gpioPin[0];
};

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif 
