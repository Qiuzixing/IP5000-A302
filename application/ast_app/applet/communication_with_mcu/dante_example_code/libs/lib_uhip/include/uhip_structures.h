/*
 * File     : uhip_structures.h
 * Created  : Jan 2013
 * Updated  : 2014/09/29
 * Author   : Jerry Kim, Michael Ung
 * Synopsis : Ultimo Host Interface Protocol (UHIP) packet structure definitions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#ifndef __UHIP_STRUCTURES_H__
#define __UHIP_STRUCTURES_H__

#include <stdint.h>

/** \addtogroup Ultimo
*  @{
*/

/**
* \defgroup UHIP UHIP Helpers
* @{
*/

/**
* \defgroup UHIPPktHelpers Packet Helpers
* @{
*/

/**
* \defgroup UHIPMacros UHIP Macros
* @{
*/

#define UHIP_MAJOR_VER	1		/*!< Current UHIP Major version  */
#define UHIP_MINOR_VER	0		/*!< Current UHIP Minor version  */

/**@}*/

/**
* \defgroup UHIPEnums UHIP Enums
* @{
*/

/**
 * @enum UhipPacketType
 * @brief UHIP Packet Types
 */
typedef enum {
	UhipPacketType_PROTOCOL_CONTROL = 1,		/*!< protocol control / status */
	UhipPacketType_CONMON_PACKET_BRIDGE = 2,	/*!< vendor conmon packet bridge */
	UhipPacketType_UDP_PACKET_BRIDGE = 3,		/*!< vendor udp packet bridge */
	UhipPacketType_DANTE_EVENTS = 4,			/*!< dante events */
	UhipPacketType_DDP = 5,						/*!< dante device protocol (DDP) */
	UhipPacketType_MAX
} UhipPacketType;

/**
* @enum UhipProtocolControlType
* @brief UHIP protocol control types
*/
typedef enum {
	UhipProtocolControlType_ACK_SUCCESS = 1,		/*!< no errors - success */
	UhipProtocolControlType_ERROR_OTHER, 			/*!< error - other */
	UhipProtocolControlType_ERROR_MALFORMED_PACKET, /*!< error - malformed packet */
	UhipProtocolControlType_ERROR_UNKNOWN_TYPE,		/*!< error - unknown or unconfigured channel */
	UhipProtocolControlType_ERROR_NETWORK,			/*!< error - network error */
	UhipProtocolControlType_ERROR_TIMEOUT,			/*!< error - timeout for waiting response */
	UhipProtocolControlType_MAX						/*!< max placeholder - use for error checking */
} UhipProtocolControlType;

/**@}*/

/**
* \defgroup UHIPStructs UHIP Structures
* @{
*/

/**
 * @struct uhip_header
 * @brief UHIP Packet Header structure
 */
struct uhip_header {
	uint8_t 		version_major;		/*!< UHIP major version */
	uint8_t 		version_minor;		/*!< UHIP minor version */
	uint16_t		size_bytes;			/*!< Size of the whole packet in bytes (including the UHP packet header) */
	uint8_t			packet_type;		/*!< Type of the packet  @see UhipPacketType */
	uint8_t			pad1;				/*!< Pad byte for byte alignment */
	uint16_t		data_offset;		/*!< Data offset */
};

//! @cond Doxygen_Suppress
typedef struct uhip_header uhip_header_t;
//! @endcond Doxygen_Suppress

/**
 * @struct uhip_protocol_control
 * @brief UHIP protocol control packet structure
 */
struct uhip_protocol_control {
	uhip_header_t	header;		/*!< UHIP header */
	uint8_t			type;		/*!< Type of the control packet @see UhipProtocolControlType */
	uint8_t 		pad;		/*!< Pad byte for byte alignment */
	uint16_t 		pad1;		/*!< Pad byte for byte alignment */
};

/**@}*/

/**@}*/
/**@}*/

/**
* \defgroup UHIPPB Packet Bridge
* @{
*/

/**
* \defgroup UHIPPBEnums Packet Bridge Enums
* @{
*/

/**
* @enum UhipConMonPacketBridgeChannel
* @brief UHIP ConMon packet bridge channels
*/
typedef enum {
	UhipConMonPacketBridgeChannel_CONMON_CONTROL_CH = 1,	/*!< Control Channel [Network to Host CPU ONLY] */
	UhipConMonPacketBridgeChannel_CONMON_STATUS_CH,			/*!< Status Channel [Host CPU to Network ONLY] */
	UhipConMonPacketBridgeChannel_CONMON_VENDOR_BCAST_CH,	/*!< Vendor Broadcast Channel [Host CPU to Network and Network to Host CPU depending on configuration] */
	UhipConMonPacketBridgeChannel_MAX						/*!< max placeholder - use for error checking */
} UhipConMonPacketBridgeChannel;

/**
* @enum UhipUDPPacketBridgeChannel
* @brief UHIP UDP packet bridge channels
*/
typedef enum {
	UhipUDPPacketBridgeChannel_UDP_BROADCAST_CH = 1,	/*!< Broadcast Channel */
	UhipUDPPacketBridgeChannel_UDP_UNICAST_CH,			/*!< Unicast Channel */
	UhipUDPPacketBridgeChannel_MAX						/*!< max placeholder - use for error checking */
} UhipUDPPacketBridgeChannel;

/**@}*/

/**
* \defgroup UHIPPBStructs Packet Bridge Structures
* @{
*/

/**
 * @struct uhip_cmc_packet_bridge
 * @brief UHIP ConMon Packet Bridge packet structure
 */
struct uhip_cmc_packet_bridge {
	uhip_header_t		header;		/*!< UHIP header */
	uint16_t			ch_id;		/*!< ConMon Packet Bridge channel @see UhipConMonPacketBridgeChannel */
	uint16_t			pad;		/*!< Pad byte for byte alignment */
	//data payload follows
};

/**
 * @struct uhip_udp_packet_bridge
 * @brief UHIP UDP Packet Bridge packet structure
 */
struct uhip_udp_packet_bridge {
	uhip_header_t		header;	/*!< UHIP header */
	uint16_t			ch_id;	/*!< UDP Packet Bridge channel @see UhipUDPPacketBridgeChannel */
	uint16_t			port;	/*!< port */
	uint32_t			ip;		/*!< IP address */
	//data payload follows
};

/**@}*/
/**@}*/


/** \addtogroup UHIP
*  @{
*/

/** \addtogroup UHIPPktHelpers
*  @{
*/

/** \addtogroup UHIPMacros
*  @{
*/

//pad & chunk size defines and macro's
#define UHIP_CHUNK_SIZE 	32			/*!< UHIP chunk size */
#define UHIP_PAD_BYTE 		0xFF		/*!< UHIP pad byte used to pad out to a 32-byte tx chunk */
#define _DMA_CHUNK_ROUND(n) ((n + UHIP_CHUNK_SIZE - 1) & ~(UHIP_CHUNK_SIZE - 1)) /*!< macro to round up to the nearest UHIP_CHUNK_SIZE */

//packet size defines
#define UHIP_PACKET_BRIDGE_PAYLOAD_MAX		500	 /*!< Maximum size of payload (500 bytes payload) */
#define UHIP_PACKET_SIZE_MAX				(MAX(sizeof(uhip_cmc_packet_bridge_t), sizeof(uhip_udp_packet_bridge_t)) + UHIP_PACKET_BRIDGE_PAYLOAD_MAX) /*!< Maximum size of UHIP packet */
#define UHIP_PACKET_SIZE_MIN				(sizeof(uhip_protocol_control_t))	/*!< Minimum size of UHIP packet */

/**@}*/

//! @cond Doxygen_Suppress
typedef struct uhip_protocol_control uhip_protocol_control_t;
typedef struct uhip_cmc_packet_bridge uhip_cmc_packet_bridge_t;
typedef struct uhip_udp_packet_bridge uhip_udp_packet_bridge_t;
//! @endcond

/**@}*/
/**@}*/
/**@}*/


#endif // __UHIP_STRUCTURES_H__
