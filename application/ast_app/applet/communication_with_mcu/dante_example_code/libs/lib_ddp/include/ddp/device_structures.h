/*
 * File     : device_structures.h
 * Created  : May 2014
 * Updated  : Date: 2014/05/16
 * Author   : Michael Ung <michael.ung@audinate.com>
 * Synopsis : All device DDP messages structure definitions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_DEVICE_STRUCTURES_H
#define _DDP_DEVICE_STRUCTURES_H

#include "ddp/ddp.h"

/**
* @file device_structures.h
* @brief All device DDP messages structure definitions.
*/

/** \addtogroup DDP
*  @{
*/

/** \addtogroup Device
*  @{
*/

/**
* \defgroup DeviceEnums DDP Device Enums
* @{
*/

/**
* @enum device_general_capability_flags
* @brief device general capability flags
*/
enum device_general_capability_flags
{
	CAPABILITY_FLAG_SUPPORT_IDENTIFY				= 0x00000001, /*!< Supports the identify message */
	CAPABILITY_FLAG_SUPPORT_DEVICE_MANUF_REQ		= 0x00000002, /*!< Supports the device manufacturer request message */
	CAPABILITY_FLAG_SUPPORT_GPIO_CMD				= 0x00000004, /*!< Support the GPIO command message */
	CAPABILITY_FLAG_SUPPORT_DEVICE_RESET_CMD		= 0x00000008, /*!< Supports the device reset command message */
	CAPABILITY_FLAG_SUPPORT_DEVICE_SECURITY_CMD		= 0x00000010, /*!< Supports the device security command message */
	CAPABILITY_FLAG_SUPPORT_SERIAL_PORT_CMD			= 0x00000020, /*!< Supports the device serial port command message */
	CAPABILITY_FLAG_SUPPORT_AES67_CMD				= 0x00000040, /*!< Supports the device AES67 command message */
	CAPABILITY_FLAG_SUPPORT_LOCK_EVNT				= 0x00000080, /*!< Supports the device lock event message */
	CAPABILITY_FLAG_WILL_NOT_REBOOT_AFTER_UPGRADE		= 0x00000100, /*!< Indicates the device will not reboot after upgrading */
	CAPABILITY_FLAG_SUPPORT_DOMAINS				= 0x00000200, /*!< Indicates the device supports domains */
};

/**
* @enum device_general_status_flags
* @brief device general status flags
*/
enum device_general_status_flags
{
	STATUS_FLAG_CORRUPT_CAPABILITY = 0x00000001, /*!< Device has a corrupt capability */
	STATUS_FLAG_CORRUPT_USER_PARTITION = 0x00000002, /*!< Device has a corrupt user partition */
	STATUS_FLAG_CONFIG_STORE_ERROR = 0x00000004, /*!< Device has an error storing the Dante configuration */
};

/**
* @enum device_manuf_override_valid_flags
* @brief device manufacturer request override valid flags bitmask - which fields in the override_valid_flags of the ddp_device_manf_request message is valid
*/
enum device_manuf_override_valid_flags
{
	MANF_OVERRIDE_MANF_NAME_STRING_VALID	= 0x00000001, /*!< Overriding the manufacturer name is valid */
	MANF_OVERRIDE_MODEL_ID_STRING_VALID		= 0x00000002, /*!< Overriding the model ID string is valid */
	MANF_OVERRIDE_SOFTWARE_VER_VALID		= 0x00000004, /*!< Overriding the 32-bit manufacturer software version is valid */
	MANF_OVERRIDE_SOFTWARE_BUILD_VALID		= 0x00000008, /*!< Overriding the 32-bit manufacturer software build is valid */
	MANF_OVERRIDE_FIRMWARE_VER_VALID		= 0x00000010, /*!< Overriding the 32-bit manufacturer firmware version is valid */
	MANF_OVERRIDE_FIMRWARE_BUILD_VALID		= 0x00000020, /*!< Overriding the 32-bit manufacturer firmware build is valid */
	MANF_OVERRIDE_CAPABILITY_FLAGS_VALID	= 0x00000040, /*!< Overriding the 32-bit manufacturer capability flags is valid */
	MANF_OVERRIDE_MODEL_VER_VALID			= 0x00000080, /*!< Overriding the 32-bit manufacturer model version is valid */
	MANF_OVERRIDE_MODEL_VER_STRING_VALID	= 0x00000100  /*!< Overriding the manufacturer model version string is valid */
};

/**
* @enum device_upgrade_request_valid_flags
* @brief device upgrade request valid flags bitmask - which fields in the "device upgrade command/request" messages are valid
*/
enum device_upgrade_request_valid_flags
{
	UPGRADE_REQUEST_ACTIONS_VALID = 0x0001, /*!< The upgrade_actions field is valid */
	UPGRADE_REQUEST_FLAGS_VALID = 0x0002, /*!< The upgrade_flags field is valid */
	UPGRADE_REQUEST_PROTOCOL_TYPE_VALID = 0x0004, /*!< The protocol_type field is valid */
	UPGRADE_REQUEST_PROTOCOL_PARAM_OFFSET_VALID = 0x0008, /*!< The prot params offset field is valid */
	UPGRADE_REQUEST_MANF_OVERRIDE_VALID = 0x0010, /*!< The manf override field is valid */
	UPGRADE_REQUEST_MODEL_OVERRIDE_VALID = 0x0020, /*!< The model override field is valid */
};

/**
* @enum device_upgrade_action_flags
* @brief device upgrade action flags bitmask - which actions to trigger on the upgrade
*/
enum device_upgrade_action_flags
{
	UPGRADE_ACTION_DOWNLOAD = 0x0001, /*!< Download the upgrade */
	UPGRADE_ACTION_FLASH = 0x0002, /*!< Flash the upgrade */
};

/**
* @enum device_upgrade_protocol
* @brief device upgrade protocol type
*/
enum device_upgrade_protocol
{
	UPGRADE_PROTOCOL_FILE_LOCAL = 1, /*!< local file */
	UPGRADE_PROTOCOL_TFTP_GET, 		 /*!< tftp */
	UPGRADE_PROTOCOL_XMODEM, 		 /*!< x-modem */
};

/**
* @enum device_upgrade_tftp_prot_valid_flags
* @brief device upgrade tftp params valid flags bitmask - which fields in the tftp protocol parameters structure are valid
*/
enum device_upgrade_tftp_prot_valid_flags
{
	UPGRADE_TFTP_PROT_PARAM_FILENAME_VALID = 0x0001, /*!< The filename offset is valid */
	UPGRADE_TFTP_PROT_PARAM_FILE_LEN_VALID = 0x0002, /*!< The file length is valid */
	UPGRADE_TFTP_PROT_PARAM_FAMILY_VALID = 0x0004, /*!< The family is valid */
	UPGRADE_TFTP_PROT_PARAM_PORT_VALID = 0x0008, /*!< The port is valid */
	UPGRADE_TFTP_PROT_PARAM_IPADDR_VALID = 0x0010, /*!< The IPv4 address is valid */
};

/**
* @enum device_upgrade_xmodem_valid_flags
* @brief device upgrade xmodem params valid flags bitmask - which fields in the xmodem protocol parameters structure are valid
*/
enum device_upgrade_xmodem_valid_flags
{
	UPGRADE_XMODEM_PROT_PARAM_PORT_VALID = 0x0001, /*!< The port is valid */
	UPGRADE_XMODEM_PROT_PARAM_MASTER_CPOL_VALID = 0x0002, /*!< The SPI master clock polarity is valid */
	UPGRADE_XMODEM_PROT_PARAM_MASTER_CPHA_VALID = 0x0004, /*!< The SPI master clock phase is valid */
	UPGRADE_XMODEM_PROT_PARAM_SLAVE_CPOL_VALID = 0x0008, /*!< The SPI slave clock polarity is valid */
	UPGRADE_XMODEM_PROT_PARAM_SLAVE_CPHA_VALID = 0x0010, /*!< The SPI slave clock phase is valid */
	UPGRADE_XMODEM_PROT_PARAM_PARITY_VALID = 0x0020, /*!< The UART parity is valid */
	UPGRADE_XMODEM_PROT_PARAM_FLOW_CONTROL_VALID = 0x0040, /*!< The UART flow control is valid */
	UPGRADE_XMODEM_PROT_PARAM_BAUDRATE_VALID = 0x0080, /*!< The SPI/UART baudrate is valid */
	UPGRADE_XMODEM_PROT_PARAM_FILE_LEN_VALID = 0x0100, /*!< The file length is valid */
};

/**
* @enum device_upgrade_xmodem_port
* @brief device upgrade xmodem port
*/
enum device_upgrade_xmodem_port
{
	UPGRADE_XMODEM_PORT_UARTA = 1,	/*!< UART A */
	UPGRADE_XMODEM_PORT_UARTB = 2,	/*!< UART B */
	UPGRADE_XMODEM_PORT_SPI0 = 3,	/*!< SPI */
};

/**
* @enum ddp_spi_cpol
* @brief SPI master and slave polarity for ddp_device_upgrade_xmodem_protocol_params
*/
enum ddp_spi_cpol
{
	DDP_SPI_CPOL_IDLE_LOW = 1,	/*!< SPI Clock polarity idle low (active high) */
	DDP_SPI_CPOL_IDLE_HIGH = 2,	/*!< SPI Clock polarity idle high (active low) */
};

/**
* @enum ddp_spi_cpha
* @brief SPI master and slave phase for ddp_device_upgrade_xmodem_protocol_params
*/
enum ddp_spi_cpha
{
	DDP_SPI_CPHA_SAMPLE_LEADING_EDGE = 1,	/*!< SPI Clock phase - sample data leading edge, change data following edge */
	DDP_SPI_CPHA_SAMPLE_TRAILING_EDGE = 2,	/*!< SPI Clock phase - change data leading edge, sample data following edge */
};

/**
* @enum ddp_uart_parity
* @brief UART parity for ddp_device_upgrade_xmodem_protocol_params
*/
enum ddp_uart_parity
{
	DDP_UART_PARITY_NONE = 1,	/*!< UART parity - none / disabled */
	DDP_UART_PARITY_EVEN = 2,	/*!< UART parity even */
	DDP_UART_PARITY_ODD = 3,	/*!< UART parity odd */
};

/**
* @enum ddp_uart_flow_control
* @brief UART flow control for ddp_device_upgrade_xmodem_protocol_params
*/
enum ddp_uart_flow_control
{
	DDP_UART_FLOWCONTROL_NONE = 1,	/*!< UART flow control - none / disabled */
	DDP_UART_FLOWCONTROL_RTS_CTS = 2,	/*!< UART flow control RTS/CTS */
};

/**
* @enum device_upgrade_response_valid_flags
* @brief device upgrade response valid flags bitmask - which fields in the ddp_device_upgrade_response message are valid
*/
enum device_upgrade_response_valid_flags
{
	UPGRADE_RESPONSE_UPGRADE_ERROR_VALID = 0x0001, /*!< The upgrade error is valid */
	UPGRADE_RESPONSE_PROGRESS_CURRENT_VALID = 0x0002, /*!< The progress current is valid */
	UPGRADE_RESPONSE_PROGRESS_TOTAL_VALID = 0x0004, /*!< The progress total is valid */
	UPGRADE_RESPONSE_MANF_ID_VALID = 0x0008, /*!< The manufacturer id is valid */
	UPGRADE_RESPONSE_MODEL_ID_VALID = 0x0010, /*!< The model id is valid */
};

/**
* @enum device_upgrade_stage
* @brief device upgrade stage - current state of the upgrade
*/
enum device_upgrade_stage
{
	DEVICE_UPGRADE_STAGE_NONE = 0,	/*!< No ugprade actions are in progress */
	DEVICE_UPGRADE_STAGE_GET_FILE = 1,	/*!< Upgrade is is progress - downloading the file */
	DEVICE_UPGRADE_STAGE_SUCCESS_DONE = 2,	/*!< Upgrade has completed successfully */
	DEVICE_UPGRADE_STAGE_FAIL_DONE = 3,	/*!< Upgrade has completed with a failure */
	DEVICE_UPGRADE_STAGE_WRITE = 4,	/*!< Upgrade is is progress - writing the file */
};

/**
* @enum device_upgrade_error
* @brief device upgrade error - error that occurred during the upgrade
*/
enum device_upgrade_error
{
	DEVICE_UPGRADE_ERROR_NONE = 0x0000, 	/*!< No error */

	DEVICE_UPGRADE_ERROR_DEV_CODE_MISMATCH = 0x0101, 	/*!< error - device code mismatch */
	DEVICE_UPGRADE_ERROR_MANF_ID_MISMATCH = 0x0102, 	/*!< error - manufacturer id mismatch */
	DEVICE_UPGRADE_ERROR_MODEL_ID_MISMATCH = 0x0103, 	/*!< error - model id mismatch */

	DEVICE_UPGRADE_ERROR_DNT_FILE_MALFORMED = 0x0111, 	/*!< error - DNT file is malformed */
	DEVICE_UPGRADE_ERROR_DNT_BAD_CRC = 0x0112, 	/*!< error - DNT file has a bad CRC */
	DEVICE_UPGRADE_ERROR_DNT_DNT_VERSION_UNSUPPORTED = 0x0113, 	/*!< error - DNT version is not supported */

	DEVICE_UPGRADE_ERROR_UPSUPPORTED_UPGRADE_ACTION = 0x0121,	/*!< error - unsupported upgrade action(s) */

	DEVICE_UPGRADE_ERROR_FLASH_ERROR = 0x0200,	/*!< error - flash */
	DEVICE_UPGRADE_ERROR_FLASH_ACCESS_FAILED = 0x0201,	/*!< error - flash access failed */

	DEVICE_UPGRADE_ERROR_DOWNLOAD_ERROR = 0x0300,	/*!< error - download error */
	DEVICE_UPGRADE_ERROR_UNKNOWN_FILE = 0x0301,	/*!< error - unknown file */
	DEVICE_UPGRADE_ERROR_FILE_PERMISSION_DENIED = 0x0302,	/*!< error - file permission denied */
	DEVICE_UPGRADE_ERROR_NO_SUCH_SERVER = 0x0303,	/*!< error - no such server */
	DEVICE_UPGRADE_ERROR_SERVER_ACCESS_FAILED = 0x0304,	/*!< error - server access failed */
	DEVICE_UPGRADE_ERROR_SERVER_TIMEOUT = 0x0305,	/*!< error - server timeout */
	DEVICE_UPGRADE_ERROR_UNSUPPORTED_PROTOCOL = 0x0306,	/*!< error - unsupported protocol */
	DEVICE_UPGRADE_ERROR_FILENAME_TOO_LONG = 0x0307,	/*!< error - filename too long */
	DEVICE_UPGRADE_ERROR_OTHER = 0x0308,	/*!< error - not defined / other */
	DEVICE_UPGRADE_ERROR_ILLEGAL_OPERATION = 0x0309,	/*!< error - illegal operation */
	DEVICE_UPGRADE_ERROR_UNKNOWN_TRANSFER_ID = 0x030A,	/*!< error - unknown transfer id */
	DEVICE_UPGRADE_ERROR_MALLOC = 0x030B,	/*!< error - malloc error */
	DEVICE_UPGRADE_ERROR_PARTITION_VERSION_UNSUPPORTED = 0x030C,	/*!< error - the flash partition contained in the DNT file is too old / unsupported on this platform */
	DEVICE_UPGRADE_ERROR_PERMISSION_DENIED = 0x030D,	/*!< error - permission denied */
	DEVICE_UPGRADE_ERROR_IN_PROGRESS = 0x30E,	/*!< error - */
	DEVICE_UPGRADE_ERROR_REBOOT_REQUIRED = 0x030F	/*!< error - the device is currently in read only mode after a clear config and requires a reboot */
};

/**
* @enum device_erase_mode
* @brief device erase mode bitmask
*/
enum device_erase_mode
{
	ERASE_MODE_FACTORY_DEFAULTS = 0x0001, 	/*!< Erase to factory defaults */
	ERASE_MODE_KEEP_IP 			= 0x0002 	/*!< Erase to factory defaults, but keep static network configuration and DDM configuration (if enrolled) */
};

/**
* @enum device_reboot_mode
* @brief Device reboot modes
*/
enum device_reboot_mode
{
	REBOOT_MODE_SOFTWARE = 0x0001, 	/*!< Software reset */
};

/**
* @enum device_identity_request_valid_flags
* @brief Device identity request valid flags bitmask - which fields in the "device identity command/request" messages are valid
*/
enum device_identity_request_valid_flags
{
	IDENTITY_REQUEST_NAME_CHANGE_VALID = 0x0001, /*!< The name_change_string_offset field is valid */
};

/**
* @enum device_identity_status_valid_flags
* @brief Device identity status valid flags bitmask
*/
enum device_identity_status_valid_flags
{
	IDENTITY_STATUS_NAME_CONFLICT = 0x0001, /*!< The friendly name is conflicting with another device */
};

/**
* @enum device_switch_led_mode
* @brief Device switch led modes
* @note Only supported on the Ultimo platform
*/
enum device_switch_led_mode
{
	SWITCH_LED_MODE_NONE      = 0x00,	/*!< Switch LEDs mode is none / not available */
	SWITCH_LED_MODE_FORCE_OFF = 0x01, 	/*!< Switch LEDs are forced off */
	SWITCH_LED_MODE_FORCE_ON  = 0x02, 	/*!< Switch LEDs are forced on */
	SWITCH_LED_MODE_NORMAL    = 0x03, 	/*!< Switch LEDs are normal - link / activity controlled by the switch/PHY */
	SWITCH_LED_MODE_LINK_ONLY = 0x04	/*!< Switch LEDs are link on only controlled by the switch/PHY */
};

/**
* @enum device_switch_led_request_valid_flags
* @brief Device switch led request valid flags bitmask - which fields in the "device switch led command/request" messages are valid
* @note Only supported on the Ultimo platform
*/
enum device_switch_led_request_valid_flags
{
	SWITCH_LED_REQUEST_SWITCH_LED_MODE_VALID = 0x0001, /*!< The switch_led_mode field is valid */
};

/**
* @enum device_aes67_mode
* @brief Device aes67 mode
* @note Only supported on the Brooklyn-II, Broadway and UltimoX platforms
*/
enum device_aes67_mode
{
	AES67_MODE_NONE			= 0x00,		/*!< AES67 mode is none / not available */
	AES67_MODE_ENABLED		= 0x01,		/*!< AES67 mode are enabled */
	AES67_MODE_DISABLED		= 0x02		/*!< AES67 mode are disabled */
};

/**
* @enum device_aes67_supported
* @brief Device aes67 supported
* @note Only supported on the Brooklyn-II, Broadway and UltimoX platforms
*/
enum device_aes67_supported
{
	AES67_MODE_NOT_SUPPORTED	= 0x00,		/*!< Device AES67 mode is not enabled in the device capability file */
	AES67_MODE_SUPPORTED		= 0x01		/*!< Device AES67 mode is enabled in the device capability file */
};

/**
* @enum device_aes67_request_valid_flags
* @brief Device aes67 request valid flags bitmask - which fields in the "device aes67 command/request" messages are valid
* @note Only supported on the Brooklyn-II, Broadway and UltimoX platforms
*/
enum device_aes67_request_valid_flags
{
	AES67_REQUEST_AES67_ENABLED_VALID = 0x0001, /*!< AES67 enable field is valid */
};

/**
* @enum device_lock_unlock_status
* @brief Device lock unlock status
*/
enum device_lock_unlock_status
{
	LOCK_UNLOCK_STATUS_NONE			= 0x00,		/*!< lock/unlock status is none / not available /not supported */
	LOCK_UNLOCK_STATUS_LOCKED		= 0x01,		/*!< lock/unlock status is locked */
	LOCK_UNLOCK_STATUS_UNLOCKED		= 0x02		/*!< lock/unlock status is unlocked */
};

/**
* @enum device_switch_redundancy_mode
* @brief Device switch redundancy mode
* @note Only supported on the Brooklyn-II platform
*/
enum device_switch_redundancy_mode
{
	SWITCH_REDUNDANCY_MODE_NONE			= 0x00,		/*!< SWITCH/REDUNDANCY mode is none / not available */
	SWITCH_REDUNDANCY_SWITCH_MODE_ENABLED		= 0x01,		/*!< SWITCH/REDUNDANCY SWITCH mode is enabled */
	SWITCH_REDUNDANCY_REDUNDANCY_MODE_ENABLED	= 0x02		/*!< SWITCH/REDUNDANCY REDUNDANT mode are enabled */
};

/**
* @enum device_switch_redundancy_supported
* @brief Device switch redundancy supported
* @note Only supported on the Brooklyn-II platform
*/
enum device_switch_redundancy_supported
{
	SWITCH_REDUNDANCY_MODE_NOT_SUPPORTED	= 0x00,		/*!< Device switch redundancy mode is not supported / not available */
	SWITCH_REDUNDANCY_MODE_SUPPORTED	= 0x01		/*!< Device switch redundancy mode is supported */
};

/**
* @enum device_switch_redundancy_request_valid_flags
* @brief Device switch redundancy valid flags bitmask - which fields in the "device device_switch_redundancy_mode command/request" messages are valid
* @note Only supported on the Brooklyn-II platform
*/
enum device_switch_redundancy_request_valid_flags
{
	SWITCH_REDUNDANCY_REQUEST_ENABLED_VALID = 0x0001, /*!< The device_switch_redundancy_mode field is valid */
};

/**
* @enum device_uart_mode
* @brief Device uart config mode
* @note Only supported on the Brooklyn-II platform
*/
enum device_uart_mode
{
	DEVICE_UART_MODE_NONE,			/*!< UART is disabled / not available */
	DEVICE_UART_MODE_UNATTACHED,	/*!< UART exists but not attached to anything */
	DEVICE_UART_MODE_CONSOLE,		/*!< UART is used as console of device */
	DEVICE_UART_MODE_BRIDGE,		/*!< UART is used as packet bridge */
	DEVICE_UART_MODE_METER			/*!< UART is used as metering */
};

/**
* @enum device_uart_user_mode
* @brief Device uart user mode
* @note Only supported on the Brooklyn-II platform
*/
enum device_uart_user_mode
{
	DEVICE_UART_USER_NONE,			/*!< This uart port has no mode */
	DEVICE_UART_USER_LOCKED,		/*!< This uart port is locked. Can't be changed */
	DEVICE_UART_USER_CONFIGURABLE,	/*!< This uart port can be configured */
	DEVICE_UART_USER_CONFIGURED		/*!< This uart config has been overriden by user */
};

/**
* @enum device_uart_data_bits
* @brief Device uart data bits
* @note Only supported on the Brooklyn-II platform
*/
enum device_uart_data_bits
{
	DEVICE_UART_DATA_BITS_NONE,		/*!< This uart port has no data bits / not available */
	DEVICE_UART_DATA_BITS_7 = 7,	/*!< This uart port has 7bits data */
	DEVICE_UART_DATA_BITS_8 = 8		/*!< This uart port has 8bits data */
};

/**
* @enum device_uart_parity_check
* @brief Device uart parity check
* @note Only supported on the Brooklyn-II platform
*/
enum device_uart_parity_check
{
	DEVICE_UART_PARITY_CHECK_NONE,	/*!< This uart port has no parity check */
	DEVICE_UART_PARITY_CHECK_ODD,	/*!< This uart port has odd parity check */
	DEVICE_UART_PARITY_CHECK_EVEN 	/*!< This uart port has even parity check */
};

/**
* @enum device_uart_stop_bits
* @brief Device uart stop bits
* @note Only supported on the Brooklyn-II platform
*/
enum device_uart_stop_bits
{
	DEVICE_UART_stop_bits_NONE,	/*!< This uart port has stop bit 0 / not available */
	DEVICE_UART_stop_bits_0,	/*!< This uart port has stop bit 0 */
	DEVICE_UART_stop_bits_1		/*!< This uart port has stop bit 1 */
};

/**
* @enum device_uart_config_request_valid_flags
* @brief Device uart config request valid flags bitmask - which fields in the "device ddp_device_uart_config_response command/request" messages are valid
* @note Only supported on the Brooklyn-II platform
*/
enum device_uart_config_request_valid_flags
{
	DEVICE_REQUEST_UART_CONFIG_INDEX_NUMBER_VALID	= 0x0001, /*!< The device uart config index number field is valid */
	DEVICE_REQUEST_UART_CONFIG_DATA_BITS_VALID		= 0x0002, /*!< The device uart config data bits field is valid */
	DEVICE_REQUEST_UART_CONFIG_PARITY_VALID			= 0x0004, /*!< The device uart config parity field is valid */
	DEVICE_REQUEST_UART_CONFIG_STOP_BITS_VALID		= 0x0008, /*!< The device uart config stop bit field is valid */
	DEVICE_REQUEST_UART_CONFIG_SPEED_VALID			= 0x0010, /*!< The device uart speed field is valid */
};

/**
* @enum vlan_config_id
* @brief Device vlan config id
* @note Only supported on the Brooklyn-II platform
*/
enum vlan_config_id
{
	DEVICE_VLAN_CONFIG_ID_NONE, /*!< ID none - Don't use this ID when sending command to set a configuration */
	DEVICE_VLAN_CONFIG_ID_1,	/*!< First VLAN configuration */
	DEVICE_VLAN_CONFIG_ID_2,	/*!< Second VLAN configuration */
	DEVICE_VLAN_CONFIG_ID_3,	/*!< Third VLAN configuration */
	DEVICE_VLAN_CONFIG_ID_4		/*!< Fourth VLAN configuration */
};

/**
* @enum device_vlan_config_request_valid_flags
* @brief Device vlan config request valid flags bitmask - which fields in the "device ddp_device_vlan_config_response command/request" messages are valid
* @note Only supported on the Brooklyn-II platform
*/
enum device_vlan_config_request_valid_flags
{
	DEVICE_REQUEST_VLAN_CONFIG_INDEX_ID_VALID	= 0x0001, /*!< The device vlan config index number field is valid */
};

/**
* @enum meter_config_rate
* @brief Device meter config rate
* @note Only supported on the Brooklyn-II platform
*/
enum meter_config_rate
{
	DEVICE_METER_CONFIG_RATE_NONE,	/*!< Meter rate none - Don't use this rate when sending a command to set the meter rate */
	DEVICE_METER_CONFIG_RATE_10HZ,	/*!< Set meter update rate to 10Hz */
	DEVICE_METER_CONFIG_RATE_30HZ	/*!< Set meter update rate to 30Hz */
};

/**
* @enum device_meter_config_request_valid_flags
* @brief Device meter config request valid flags bitmask - which fields in the "device ddp_device_meter_config_response command/request" messages are valid
* @note Only supported on the Brooklyn-II platform
*/
enum device_meter_config_request_valid_flags
{
	DEVICE_REQUEST_METER_CONFIG_RATE_VALID	= 0x0001, /*!< The device meter config rate field is valid */
};

/**
* @enum device_dante_domain_response_valid_flags
* @brief Device dante domain response valid flags bitmask - which fields in the "device ddp_device_dante_domain_response response/event" messages are valid
*/
enum device_dante_domain_response_valid_flags
{
	DEVICE_DANTE_DOMAIN_RESPONSE_INDEX_LOCAL_ACCESS_CONTROL_VALID	= 0x00000001, /*!< The device dante domain response local access control field is valid */
	DEVICE_DANTE_DOMAIN_RESPONSE_INDEX_REMOTE_ACCESS_CONTROL_VALID	= 0x00000002, /*!< The device dante domain response remote access control field is valid */
};

/**
* @enum device_dante_domain_flags
* @brief Device dante domain flags bitmask - used to indicate domain state information
*/
enum device_dante_domain_flags
{
	DEVICE_DANTE_DOMAIN_IS_ENROLLED 		= 0x0001, /*!< device is enrolled in a domain */
	DEVICE_DANTE_DOMAIN_IS_DDM_CONNECTED 	= 0x0002, /*!< device is connected to a domain manager */
};

/**
* @enum device_dante_domain_local_access_control_policy
* @brief Device Dante domain DDM administrator local access control policy
*/
enum device_dante_domain_local_access_control_policy
{
	DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_UNKNOWN_INVALID = 0,	/*!< DDM administrator local access control policy = unknown/invalid */
	DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_NONE,					/*!< DDM administrator local access control policy = not set */
	DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_READ_ONLY,				/*!< DDM administrator local access control policy = read only */
	DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_READ_WRITE,				/*!< DDM administrator local access control policy = read + write */
};

/**
* @enum device_dante_domain_remote_access_control_policy
* @brief Device Dante domain DDM administrator remote access control policy
*/
enum device_dante_domain_remote_access_control_policy
{
	DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_UNKNOWN_INVALID = 0,	/*!< DDM administrator remote access control policy = unknown/invalid */
	DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_NONE,					/*!< DDM administrator remote access control policy = not set */
	DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_READ_ONLY,				/*!< DDM administrator remote access control policy = read only */
	DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_READ_WRITE,			/*!< DDM administrator remote access control policy = read + write */
};

/**
* @enum device_switch_port_status_valid_flags
* @brief Device switch status port status valid flags bitmask - which fields in the "device ddp_switch_port_status_t" port elements are valid
*/
enum device_switch_port_status_valid_flags
{
	DEVICE_SWITCH_PORT_STATUS_PORT_NO_VALID			= 0x0001, /*!< The port_no field is valid */
	DEVICE_SWITCH_PORT_STATUS_PORT_CMODE_VALID		= 0x0002, /*!< The port_cmode field is valid */
	DEVICE_SWITCH_PORT_STATUS_LINK_SPEED_VALID		= 0x0004, /*!< The link_speed field is valid */
	DEVICE_SWITCH_PORT_STATUS_ERR_COUNT_VALID		= 0x0008, /*!< The error_count field is valid */
};

/**
* @enum device_switch_port_status_link_flags
* @brief Device switch status port status link flags bitmask
*/
enum device_switch_port_status_link_flags
{
	DEVICE_SWITCH_PORT_STATUS_LINK_STATE		= 0x01, /*!< Link state flag - if bit is set link is up, otherwise link doan */
	DEVICE_SWITCH_PORT_STATUS_DUPLEX			= 0x02, /*!< Duplex flag - if bit is set link is full duplex, otherwise half duplex */
};

/**@}*/

/**
* \defgroup DeviceStructs DDP Device Structures
* @{
*/

/**
 * @struct ddp_device_general_request
 * @brief Structure format for the "device general request" message
 */
struct ddp_device_general_request
{
	ddp_message_header_t header;			/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */
};

/**
 * @struct ddp_device_general_response
 * @brief Structure format for the "device general response" message
 */
struct ddp_device_general_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		// base payload. Additional elements must be appended to the end of this list.
		dante_id64_t model_id;						/*!< 64-bit model id of the device */
		ddp_raw_offset_t model_id_string_offset;	/*!< offset from the start of the payload to a NULL terminated model id string */
		uint16_t pad0;								/*!< pad byte for alignment, must be 0x0000 */
		uint32_t software_version; 					/*!< software version as a 8/8/16 major/minor/bugfix version */
		uint32_t software_build;					/*!< software build */
		uint32_t firmware_version; 					/*!< firmware version as a 8/8/16 major/minor/bugfix version */
		uint32_t firmware_build;					/*!< firmware build */
		uint32_t bootloader_version;				/*!< bootloader/u-boot version as a 8/8/16 major/minor/bugfix version */
		uint32_t bootloader_build;					/*!< bootloader build */
		uint32_t api_version;						/*!< api version as a 8/8/16 major/minor/build version */
		uint32_t capability_flags;					/*!< capability flags bitmask @see device_general_capability_flags  */
		uint32_t status_flags;						/*!< status flags bitmask @see device_general_status_flags*/
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_manf_request
 * @brief Structure format for the "device manufacturer request" message
 */
struct ddp_device_manf_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint32_t override_valid_flags;					/*!< bitwise OR'd flags to indicate which fields to override in the ddp_device_general_response. @see device_manuf_override_valid_flags */
		uint8_t pad0[8];								/*!< pad bytes for alignment, must be zero */
		ddp_raw_offset_t manf_name_string_offset;		/*!< offset from the start of the payload to a NULL terminated manufacturer name string */
		ddp_raw_offset_t model_id_string_offset;		/*!< offset from the start of the payload to a NULL terminated model id string */
		uint8_t pad1[8];								/*!< pad bytes for alignment, must be zero */
		uint32_t software_version; 						/*!< manufacturer software version as a 8/8/16 major/minor/bugfix version */
		uint32_t software_build; 						/*!< manufacturer software build version */
		uint32_t firmware_version; 						/*!< manufacturer firmware version as a 8/8/16 major/minor/bugfix version */
		uint32_t firmware_build; 						/*!< manufacturer firmware build version */
		uint32_t capability_flags;						/*!< manufacturer device capability flags bitmask */
		uint32_t model_version; 						/*!< manufacturer model version as a 8/8/16 major/minor/bugfix version */
		ddp_raw_offset_t model_version_string_offset;	/*!< offset from the start of the payload to a NULL terminated model version string */
		uint16_t pad2;									/*!< pad byte for alignment, must be 0x0000 */
	} payload; /*!< fixed payload */
};

/**
 * @struct ddp_device_manf_response
 * @brief Structure format for the "device general response" message
 */
struct ddp_device_manf_response
{
	ddp_message_header_t header;				/*!< message header */
	ddp_response_subheader_t subheader;			/*!< response message subheader */

	struct
	{
		// base payload. Additional elements must be appended to the end of this list.
		dante_id64_t manf_id;							/*!< 64-bit manufacturer id of the device */
		ddp_raw_offset_t manf_name_string_offset;		/*!< offset from the start of the payload to a NULL terminated manufacturer name string */
		ddp_raw_offset_t model_id_string_offset;		/*!< offset from the start of the payload to a NULL terminated model id string */
		dante_id64_t model_id;							/*!< 64-bit model id of the device */
		uint32_t software_version; 						/*!< manufacturer software version as a 8/8/16 major/minor/bugfix version */
		uint32_t software_build; 						/*!< manufacturer software build version */
		uint32_t firmware_version; 						/*!< manufacturer firmware version as a 8/8/16 major/minor/bugfix version */
		uint32_t firmware_build; 						/*!< manufacturer firmware build version */
		uint32_t capability_flags;						/*!< manufacturer device capability flags bitmask */
		uint32_t model_version; 						/*!< manufacturer model version as a 8/8/16 major/minor/bugfix version */
		ddp_raw_offset_t model_version_string_offset;	/*!< offset from the start of the payload to a NULL terminated model version string */
		uint16_t pad0;									/*!< pad byte for alignment, must be 0x0000 */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_upgrade_request
 * @brief Structure format for the "device upgrade command/request" message
 */
struct ddp_device_upgrade_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;					/*!< valid flags bitmask indicating which fields in this messages are valid @see device_upgrade_request_valid_flags */
		uint16_t upgrade_action_flags;			/*!< which upgrade actions to trigger @see device_upgrade_action_flags */
		uint32_t upgrade_flags;					/*!< upgrade flags - not currently used */
		uint32_t upgrade_flags_mask;			/*!< mask of valid upgrade flags - not currently used */
		uint16_t protocol_type;					/*!< protocol type to use for the upgrade @see device_upgrade_protocol */
		ddp_raw_offset_t prot_params_offset;	/*!< offset from the start of the payload to the protocol parameter structure */
		dante_id64_t manf_override;				/*!< 64-bit manufacturer override id for the upgrade DNT */
		dante_id64_t model_override;			/*!< 64-bit model override id for the upgrade DNT */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_upgrade_tftp_protocol_params
 * @brief Structure format for the tftp protocol parameters in the "device upgrade command/request" message
 */
struct ddp_device_upgrade_tftp_protocol_params
{
	uint16_t valid_flags;				/*!< valid flags bitmask indicating which field in this structure is valid @see device_upgrade_tftp_prot_valid_flags*/
	ddp_raw_offset_t filename_offset;	/*!< filename - offset from the start of the payload to a NULL terminated string */
	uint32_t file_length;				/*!< length of the upgrade file in bytes */
	uint16_t family; 					/*!< IP address family for the TFTP server IP address @see ddp_address_family */
	uint16_t port;						/*!< port for the TFTP server */
	uint32_t ip_address;				/*!< IPv4 address for the TFTP server */
};

//! @cond Doxygen_Suppress
typedef struct ddp_device_upgrade_tftp_protocol_params ddp_device_upgrade_tftp_protocol_params_t;
//! @endcond

/**
 * @struct ddp_device_upgrade_xmodem_protocol_params
 * @brief Structure format for the xmodem protocol parameters in the "device upgrade command/request" message
 * @note Only supported on the Ultimo platform
 */
struct ddp_device_upgrade_xmodem_protocol_params
{
	uint16_t valid_flags;		/*!< valid flags bitmask indicating which field in this structure is valid @see device_upgrade_response_valid_flags*/
	uint16_t port;				/*!< port for the xmodem upgrade @see device_upgrade_xmodem_port */
	uint8_t master_cpol;		/*!< SPI master clock polarity @see ddp_spi_cpol*/
	uint8_t master_cpha;		/*!< SPI master clock phase @see ddp_spi_cpha*/
	uint8_t slave_cpol;			/*!< SPI slave clock polarity @see ddp_spi_cpol*/
	uint8_t slave_cpha;			/*!< SPI slave clock phase @see ddp_spi_cpha*/
	uint8_t parity;				/*!< UART parity @see ddp_uart_parity*/
	uint8_t flow_control;		/*!< UART flow control @see ddp_uart_flow_control*/
	uint16_t pad0;				/*!< pad bytes for alignment, must be 0x0000 */
	uint32_t baudrate;			/*!< SPI/UART buadrate */
	uint32_t file_length;		/*!< length of the upgrade file in bytes */
};

//! @cond Doxygen_Suppress
typedef struct ddp_device_upgrade_xmodem_protocol_params ddp_device_upgrade_xmodem_protocol_params_t;
// @endcond

/**
 * @struct ddp_device_upgrade_response
 * @brief Structure format for the "device upgrade response" message
 */
struct ddp_device_upgrade_response
{
	ddp_message_header_t header;				/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		// base payload. Additional elements must be appended to the end of this list.
		uint16_t valid_flags;		/*!< valid flags bitmask indicating which field in this message is valid @see device_upgrade_response_valid_flags */
		uint16_t upgrade_stage;		/*!< current upgrade stage @see device_upgrade_stage */
		uint32_t upgrade_error;		/*!< upgrade error @see device_upgrade_error */
		uint32_t progress_current;	/*!< current upgrade progress for this stage (progress = progress_current / progress_total) */
		uint32_t progress_total;	/*!< total upgrade progress for this stage (progress = progress_current / progress_total) */
		dante_id64_t manf_id;		/*!< 64-bit manufacturer id of the device */
		dante_id64_t model_id;		/*!< 64-bit model id of the device */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_erase_request
 * @brief Structure format for the "device erase command/request" message
 */
struct ddp_device_erase_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t mode;			/*!< Which erase mode to trigger @see device_erase_mode */
		uint16_t mode_mask;		/*!< Bitwise OR'd flags of which mode to trigger @see device_erase_mode */
	} payload;	/*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_erase_response
 * @brief Structure format for the "device erase response" message
 */
struct ddp_device_erase_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		// base payload. Additional elements must be appended to the end of this list.
		uint16_t supported_flags;		/*!< which erase modes are supported @see device_erase_mode */
		uint16_t executed_flags;		/*!< what modes have been executed @see device_erase_mode */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_reboot_request
 * @brief Structure format for the "device reboot command/request" message
 */
struct ddp_device_reboot_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t mode;					/*!< which erase mode to trigger @see device_reboot_mode */
		uint16_t mode_mask;				/*!< Bitwise OR'd flags of which mode to trigger @see device_reboot_mode */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_reboot_response
 * @brief Structure format for the "device reboot response" message
 */
struct ddp_device_reboot_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint16_t supported_flags;		/*!< which reboot modes are supported @see device_reboot_mode */
		uint16_t executed_flags;		/*!< what modes have been executed @see device_reboot_mode */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_identity_request
 * @brief Structure format for the "device identity command/request" message
 */
struct ddp_device_identity_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;				/*!< valid flags bitmask indicating which fields in this messages are valid @see device_identity_request_valid_flags */
		uint16_t name_change_string_offset;	/*!< name change string - offset from the start of the payload to the NULL terminated string */
	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
 * @struct ddp_device_identity_response_payload
 * @brief Structure format for the payload of the "device identity response" message
 */
struct ddp_device_identity_response_payload
{
	uint16_t identity_status_flags;				/*!< identity status flags bitmask @see device_identity_status_valid_flags */
	dante_process_id_t process_id;				/*!< process id - used in conjunction with the dante_device_id to uniquely identify a device */
	dante_device_id_t device_id;				/*!< EUI64 that when combined with the process_id uniquely identifies the dante device */
	ddp_raw_offset_t default_name_offset;		/*!< default name string - offset from the start of the payload to the NULL terminated string */
	ddp_raw_offset_t friendly_name_offset;		/*!< friendly name string - offset from the start of the payload to the NULL terminated string */
	ddp_raw_offset_t dante_domain_offset;		/*!< dante domain string - offset from the start of the payload to the NULL terminated string */
	ddp_raw_offset_t advertised_name_offset; 	/*!< advertised string - offset from the start of the payload to the NULL terminated string */
};

//! @cond Doxygen_Suppress
typedef struct ddp_device_identity_response_payload ddp_device_identity_response_payload_t;
//! @endcond

/**
* @struct ddp_device_identity_response
* @brief Structure format for the "device identity response" message
*/
struct ddp_device_identity_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	ddp_device_identity_response_payload_t payload;	/*!< Payload @see ddp_device_identity_response_payload */

	// Heap goes here
};

/**
* @struct ddp_device_identify_response
* @brief Structure format for the "device identify response" message
*/
struct ddp_device_identify_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */
};

/**
 * @struct ddp_device_gpio_request
 * @brief Structure format for the "device GPIO command/request" message
 * @note Only supported on the Ultimo platform
 */
struct ddp_device_gpio_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint32_t output_state_valid_flags;		/*!< bitmask of which output_state_values are valid */
		uint32_t output_state_values;			/*!< bitmask of output state values - this is the value to set each GPIO output pin */
	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
 * @struct ddp_device_gpio_response
 * @brief Structure format for the "device gpio response" message
 * @note Only supported on the Ultimo platform
 */
struct ddp_device_gpio_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint32_t interrupt_trigger_mask;		/*!< bitmask of which GPIO's triggered an interrupt  */
		uint32_t input_state_valid_mask;		/*!< bitmask of which input_state_values are valid */
		uint32_t input_state_values;			/*!< bitmask of input state values - this is the current value of each GPIO input pin */
		uint32_t output_state_valid_mask;		/*!< bitmask of which output_state_values are valid */
		uint32_t output_state_values;			/*!< bitmask of input state values - this is the current value of each GPIO output pin */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_switch_led_request
 * @brief Structure format for the "device switch led command/request" message
 * @note Only supported on the Ultimo platform
 */
struct ddp_device_switch_led_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;			/*!< valid flags bitmask indicating which fields in this messages are valid @see device_switch_led_request_valid_flags */
		uint8_t switch_led_mode;		/*!< switch led mode @see device_switch_led_mode */
		uint8_t pad0;					/*!< pad byte for alignment, must be 0x00 */
	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
 * @struct ddp_device_switch_led_response
 * @brief Structure format for the "device switch led response" message
 * @note Only supported on the Ultimo platform
 */
struct ddp_device_switch_led_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint8_t switch_led_mode;		/*!< switch led mode @see device_switch_led_mode */
		uint8_t pad0;					/*!< pad byte for alignment, must be 0x00 */
		uint16_t pad1;					/*!< pad byte for alignment, must be 0x0000 */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_aes67_request
 * @brief Structure format for the "device aes67 request" message
 * @note Only supported on the Brooklyn-II, Broadway, and UltimoX platforms
 */
struct ddp_device_aes67_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;		/*!< valid flags bitmask indicating which fields in this messages are valid @see device_aes67_request_valid_flags */
		uint8_t aes67_mode;			/*!< aes67 mode @see device_aes67_mode */
		uint8_t pad0;				/*!< pad byte for alignment, must be 0x00 */
	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
 * @struct ddp_device_aes67_response
 * @brief Structure format for the "device aes67 response" message
 * @note Only supported on the Brooklyn-II, Broadway, and UltimoX platforms
 */
struct ddp_device_aes67_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint8_t device_aes67_support;		/*!< current device aes67 support in the capability @see device_aes67_supported */
		uint8_t device_aes67_current;		/*!< current device aes67 enabled @see device_aes67_mode */
		uint8_t device_aes67_reboot;		/*!< next device boot aes67 enabled @see device_aes67_mode */
		uint8_t pad;						/*!< pad byte for alignment, must be 0x0000 */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_lockunlock_request
 * @brief Structure format for the "device lock/unlock request" message
 */
struct ddp_device_lockunlock_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */
};

/**
 * @struct ddp_device_lockunlock_response
 * @brief Structure format for the "device lock/unlock response" message
 */
struct ddp_device_lockunlock_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint8_t lock_unlock_status;	/*!< current device lock state @see device_lock_unlock_status */
		uint8_t pad0;				/*!< pad byte for alignment, must be 0x0000 */
		uint16_t pad1;				/*!< pad byte for alignment, must be 0x0000 */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_switch_redundancy_request
 * @brief Structure format for the "device switch/redundancy request" message
 * @note Only supported on the Brooklyn-II platform
 */
struct ddp_device_switch_redundancy_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;			/*!< valid flags bitmask indicating which fields in this messages are valid @see device_switch_redundancy_request_valid_flags */
		uint8_t switch_redundancy_mode;		/*!< switch led mode @see device_switch_redundancy_mode */
		uint8_t pad0;				/*!< pad byte for alignment, must be 0x00 */
	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
 * @struct ddp_device_switch_redundancy_response
 * @brief Structure format for the "device switch_redundancy response" message
 * @note Only supported on the Brooklyn-II platform
 */
struct ddp_device_switch_redundancy_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint8_t device_switch_redundancy_support;	/*!< current device support switch_redundancy @see device_switch_redundancy_supported */
		uint8_t device_switch_redundancy_current;	/*!< current device switch_redundancy enabled @see device_switch_redundancy_mode */
		uint8_t device_switch_redundancy_reboot;	/*!< next device boot switch_redundancy enabled @see device_switch_redundancy_mode */
		uint8_t pad;					/*!< pad byte for alignment, must be 0x0000 */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_uart_config_request
 * @brief Structure format for the "device uart config request" message
 * @note Only supported on the Brooklyn-II platform
 */
struct ddp_device_uart_config_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;			/*!< valid flags bitmask indicating which fields in this messages are valid @see device_uart_config_request_valid_flags */
		uint8_t uart_index_number;		/*!< current device uart index number which is about to configure (0,1,2 ...) */
		uint8_t uart_bits;			/*!< current device uart bits (7,8) @see device_uart_data_bits */
		uint8_t uart_parity;			/*!< current device uart parity check @see device_uart_parity_check */
		uint8_t uart_stop_bits;			/*!< current device uart stop bits @see device_uart_stop_bits */
		uint16_t pad0;				/*!< pad byte for alignment, must be 0x0000 */
		uint32_t uart_speed;			/*!< current device uart config speed */

	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
* @struct ddp_device_uart_config_response_payload
* @brief Structure format of the uart configuration information for "ddp device uart config response" messages
* @note Only supported on the Brooklyn-II platform
*/
struct ddp_device_uart_config_response_payload
{
	uint8_t uart_mode;			/*!< current device uart config mode @see device_uart_mode */
	uint8_t uart_user_configurable;		/*!< current device uart config user configurable @see device_uart_user_mode */
	uint8_t uart_bits;			/*!< current device uart bits (7,8) @see device_uart_data_bits */
	uint8_t uart_parity;			/*!< current device uart parity check @see device_uart_parity_check */
	uint8_t uart_stop_bits;			/*!< current device uart stop bits @see device_uart_stop_bits */
	uint8_t pad0;				/*!< pad byte for alignment, must be 0x0000 */
	uint16_t pad1;				/*!< pad byte for alignment, must be 0x0000 */
	uint32_t uart_speed;			/*!< current device uart config speed */
};

//! @cond Doxygen_Suppress
typedef struct ddp_device_uart_config_response_payload ddp_device_uart_config_response_payload_t;
//! @endcond

/**
 * @struct ddp_device_uart_config_response
 * @brief Structure format for the "device uart config response" message
 * @note Only supported on the Brooklyn-II platform
 */
struct ddp_device_uart_config_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint8_t uart_number;			/*!< current device uart number in total */
		uint8_t pad0;
		uint16_t pad1;
		uint16_t uart_config_size;		/*!< Size of each uart config structure @see ddp_device_uart_config_response_payload_t */
		ddp_raw_offset_t uart_config_offset;	/*!< Offset from the start of the TLV0 header to the first uart config structure @see ddp_device_uart_config_response_payload_t */
	} payload; /*!< fixed payload */

	//heap goes here!
};



/**
 * @struct ddp_device_vlan_config_request
 * @brief Structure format for the "device VLAN config request" message
 */
struct ddp_device_vlan_config_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;		/*!< valid flags bitmask indicating which fields in this messages are valid @see device_vlan_config_request_valid_flags */
		uint8_t vlan_config_id;		/*!< device vlan config id configure @see vlan_config_id */
		uint8_t pad0;			/*!< Pad for alignment - must be '0' */
	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
* @struct ddp_device_vlan_config_response_payload
* @brief Structure format of the vlan configuration information block for "ddp device vlan config response" messages
* @note The vlan_secondary_bitmap field is only applicable for the Brooklyn-II platform
*/
struct ddp_device_vlan_config_response_payload
{
	uint32_t vlan_primary_bitmap;			/*!< device vlan config primary ports bitmap bit0 : port 0, bit1 : port 1 .. */
	uint32_t vlan_secondary_bitmap;			/*!< device vlan config secondary ports bitmap bit0 : port 0, bit1 : port 1 .. */
	uint32_t vlan_user_2_bitmap;			/*!< device vlan config user vlan 2 ports bitmap bit0 : port 0, bit1 : port 1 .. */
	uint32_t vlan_user_3_bitmap;			/*!< device vlan config user vlan 3 ports bitmap bit0 : port 0, bit1 : port 1 .. */
};

//! @cond Doxygen_Suppress
typedef struct ddp_device_vlan_config_response_payload ddp_device_vlan_config_response_payload_t;
//! @endcond

/**
* @struct ddp_vlan_name_offsets
* @brief Structure format of an element of the array of offsets to vlan name strings for the "device vlan config response" message
*/
struct ddp_vlan_name_offsets
{
	ddp_raw_offset_t offset;	/*!< Offset from the start of the TLV0 header to the vlan name string */
};

//! @cond Doxygen_Suppress
typedef struct ddp_vlan_name_offsets ddp_vlan_name_offsets_t;
//! @endcond

/**
 * @struct ddp_device_vlan_config_response
 * @brief Structure format for the "ddp_device vlan config response" message
 */
struct ddp_device_vlan_config_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint8_t vlan_max_number;				/*!< device vlan max number */
		uint8_t vlan_config_number;				/*!< device vlan config number in total */
		uint8_t vlan_current_config_id;				/*!< device current vlan config id (1~4) @see vlan_config_id */
		uint8_t vlan_reboot_config_id;				/*!< device reboot vlan config id (1~4)  @see vlan_config_id */
		uint16_t vlan_config_port_mask;				/*!< device vlan config port mask bitmap (0x7f - 7 ports) */
		uint16_t vlan_config_structure_size;			/*!< Size of each uart config structure @see ddp_device_vlan_config_response_payload_t */
		ddp_raw_offset_t vlan_config_structure_offset;		/*!< Offset from the start of the TLV0 header to the first vlan config structure @see ddp_device_vlan_config_response_payload_t */
		ddp_raw_offset_t vlan_config_name_string_offset;	/*!< Offset from the start of the TLV0 header to the first vlan config structure @see ddp_device_vlan_config_response_payload_t */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_meter_config_request
 * @brief Structure format for the "device meter config request" message
 * @note Only supported on the Brooklyn-II platform
 */
struct ddp_device_meter_config_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t valid_flags;		/*!< valid flags bitmask indicating which fields in this messages are valid @see device_meter_config_request_valid_flags */
		uint16_t meter_config_rate;	/*!< device meter config rate configure @see meter_config_rate */
	} payload; /*!< fixed payload */

	// Heap goes here
};

/**
 * @struct ddp_device_meter_config_response
 * @brief Structure format for the "ddp_device meter config response" message
 * @note Only supported on the Brooklyn-II platform
 */
struct ddp_device_meter_config_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint16_t current_meter_config_rate;	/*!< device current meter config rate */
		uint16_t pad1;				/*!< Pad for alignment - must be '0' */
	} payload; /*!< fixed payload */

	//heap goes here!
};

/**
 * @struct ddp_device_dante_domain_request
 * @brief Structure format for the "dante domain request" message
 */
struct ddp_device_dante_domain_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */
};

/**
 * @struct ddp_device_dante_domain_response
 * @brief Structure format for the "ddp_device dante domain response" message
 */
struct ddp_device_dante_domain_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint32_t valid_flags;					/*!< valid flags bitmask indicating which fields in this messages are valid @see device_dante_domain_response_valid_flags */
		uint16_t dante_domain_flags_valid_mask;	/*!< dante domain flags bitmask indicating which bit values in dante_domain_flags are valid  @see device_dante_domain_flags */
		uint16_t dante_domain_flags;			/*!< dante domain flags bitmask indicating current dante domain state  @see device_dante_domain_flags */
		uint8_t local_access_control_policy;	/*!< local access control DDM administrator policy  @see device_dante_domain_local_access_control_policy  */
		uint8_t remote_access_control_policy;	/*!< remote access control DDM administrator policy  @see device_dante_domain_remote_access_control_policy*/
		uint16_t pad0;							/*!< Pad for alignment - must be '0' */
	} payload; /*!< fixed payload */
};

/**
 * @struct ddp_device_switch_status_request
 * @brief Structure format for the "device switch status request" message
 */
struct ddp_device_switch_status_request
{
	ddp_message_header_t header;			/*!< message header */
	ddp_request_subheader_t subheader;		/*!< request message subheader */
};

/**
* @struct ddp_switch_port_status
* @brief Structure format of the port status information for "switch status response" messages
*/
struct ddp_switch_port_status
{
	uint16_t valid_flags;		/*!< valid flags bitmask indicating which fields in this messages are valid @see enum device_switch_port_status_valid_flags */
	uint16_t link_speed;		/*!< Link speed in Mbps */
	uint8_t port_no;			/*!< Port number */
	uint8_t port_cmode;			/*!< Port mode - see the relevant Marvell datasheet for mode details on the C_MODE  */
	uint8_t link_flags_mask;	/*!< bitmask of which bits in the link_flags are valid @see device_switch_port_status_link_flags */
	uint8_t link_flags;			/*!< bitmask of link flags @see device_switch_port_status_link_flags  */
	uint32_t error_count;		/*!< Error count - RX errors, FCS errors, etc */
};

//! @cond Doxygen_Suppress
typedef struct ddp_switch_port_status ddp_switch_port_status_t;
//! @endcond

/**
 * @struct ddp_device_switch_status_response
 * @brief Structure format for the "device switch status response" message
 */
struct ddp_device_switch_status_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint16_t enabled_ports_values;			/*!< bitmask of enabled ports values - this is the whether a particular port is enabled */
		uint8_t num_ports;						/*!< Number of switch status ports in the array */
		uint8_t port_array_element_size;		/*!< Size in bytes of each element in the port array */
		ddp_raw_offset_t port_offset;			/*!< Offset to the port array, each element is a port structure @see ddp_switch_port_status */
		uint16_t pad0;							/*!< Pad for alignment - must be '0' */
	} payload; /*!< fixed payload */

	// Heap goes here
};

//! @cond Doxygen_Suppress
typedef struct ddp_device_general_request ddp_device_general_request_t;
typedef struct ddp_device_general_response ddp_device_general_response_t;
typedef struct ddp_device_manf_request ddp_device_manf_request_t;
typedef struct ddp_device_manf_response ddp_device_manf_response_t;
typedef struct ddp_device_upgrade_request ddp_device_upgrade_request_t;
typedef struct ddp_device_upgrade_response ddp_device_upgrade_response_t;
typedef struct ddp_device_erase_request ddp_device_erase_request_t;
typedef struct ddp_device_erase_response ddp_device_erase_response_t;
typedef struct ddp_device_reboot_request ddp_device_reboot_request_t;
typedef struct ddp_device_reboot_response ddp_device_reboot_response_t;
typedef struct ddp_device_identity_request ddp_device_identity_request_t;
typedef struct ddp_device_identity_response ddp_device_identity_response_t;
typedef struct ddp_device_identify_response ddp_device_identify_response_t;
typedef struct ddp_device_gpio_request ddp_device_gpio_request_t;
typedef struct ddp_device_gpio_response ddp_device_gpio_response_t;
typedef struct ddp_device_switch_led_request ddp_device_switch_led_request_t;
typedef struct ddp_device_switch_led_response ddp_device_switch_led_response_t;
typedef struct ddp_device_aes67_request ddp_device_aes67_request_t;
typedef struct ddp_device_aes67_response ddp_device_aes67_response_t;
typedef struct ddp_device_lockunlock_request ddp_device_lockunlock_request_t;
typedef struct ddp_device_lockunlock_response ddp_device_lockunlock_response_t;
typedef struct ddp_device_switch_redundancy_request ddp_device_switch_redundancy_request_t;
typedef struct ddp_device_switch_redundancy_response ddp_device_switch_redundancy_response_t;
typedef struct ddp_device_uart_config_request ddp_device_uart_config_request_t;
typedef struct ddp_device_uart_config_response ddp_device_uart_config_response_t;
typedef struct ddp_device_vlan_config_request ddp_device_vlan_config_request_t;
typedef struct ddp_device_vlan_config_response ddp_device_vlan_config_response_t;
typedef struct ddp_device_meter_config_request ddp_device_meter_config_request_t;
typedef struct ddp_device_meter_config_response ddp_device_meter_config_response_t;
typedef struct ddp_device_dante_domain_request ddp_device_dante_domain_request_t;
typedef struct ddp_device_dante_domain_response ddp_device_dante_domain_response_t;
typedef struct ddp_device_switch_status_request ddp_device_switch_status_request_t;
typedef struct ddp_device_switch_status_response ddp_device_switch_status_response_t;

//! @endcond

/**@}*/
/**@}*/
/**@}*/

#endif // _DDP_DEVICE_STRUCTURES_H
