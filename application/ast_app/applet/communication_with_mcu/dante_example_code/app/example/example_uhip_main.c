/*
 * File     : example_uhip_main.c
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Chamira Perera, Michael Ung
 * Synopsis : HostCPU UHIP API main
 *
 * Copyright 2005-2014, Audinate Pty Ltd and/or its licensors
 * Confidential
 * Audinate Copyright Header Version 1
 */

#include "example_uhip_common.h"
#include "example_tx_uhip.h"
#include "example_rx_uhip.h"

#include "uhip_hostcpu_rx_timer.h"
#include "uhip_hostcpu_tx_timer.h"
#include "hostcpu_transport.h"
#include "../dante_cmd/dante_cmd_packet.h"
enum
{
	SET_HOSTNAME = 0,
	CMD_CNT
};

int run_flag = 1;
int __example_main(int dante_cmd_index)
{
	hostcpu_uhip_common_init();

	// Initialize transport & timers
	if (hostcpu_transport_init() != AUD_TRUE) {
		AUD_PRINTF("FATAL ERROR - unable to initialise hostcpu transport\n");
		return -1;
	}
	uhip_hostcpu_rx_timer_init();
	uhip_hostcpu_tx_timer_init();

	//reset the rx buffers
	reset_uhip_rx_buffers();
	// NOTE: This example application sends a SINGLE DDP Tx request and prints out the response when it is received
	// It will also print the contents of any DDP events received while running
	// only enable ONE FLAG at a time as the host application is required to wait for a response before sending the next Tx request
	// In a complete application the sending of DDP requests would need to be triggered by an external event such as a push button press or by a periodic timer
	switch(dante_cmd_index)
	{
		case SET_HOSTNAME:
			hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_IDENTITY_FLAG);		//send a DDP device manufacture with manf override - see
			break;
		case SET_DANTE_DEFAULT_NAME_GET_IT:
			hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_IDENTITY_FLAG);	//send a DDP device manufacture with manf override - see
			break;
		default:
			printf("unknow dante_cmd quit\n");
			return -1;
	}
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_CMC_PB_FLAG);			//send a ConMon packet bridge message - see send_conmon_packetbridge_packet()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_UDP_PB_FLAG);			//send a UDP packet bridge message - see send_udp_packetbridge_packet()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_NET_IF_QUERY_FLAG);		//send a DDP network interface message - see ddp_write_send_network_interface_query()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_CONFIG_STATIC_IP_FLAG);		//send a DDP network config message - see ddp_write_configure_static_ip()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_SUB_RX_CHANS_FLAG);		//send a DDP routing rx subscribe message - see ddp_write_subscribe_channels()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_ADD_MCAST_TX_FLAG);		//send a DDP routing tx multicast flow message - see ddp_write_create_tx_multicast_flow()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DELETE_FLOW_FLAG);		//send a DDP routing delete flow message - see ddp_write_delete_flow()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_UPGRADE_XMODEM_FLAG);		//send a DDP device upgrade message - see ddp_write_upgrade_via_xmodem()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_CONFIG_SRATE_FLAG);		//send a DDP device audio sample rate config message - see ddp_write_configure_sample_rate()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_CONFIG_CLOCK_PULLUP_FLAG);	//send a DDP clock pullup message - see ddp_write_configure_clock_pullup()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_GPIO_FLAG);	//send a DDP device GPIO message - see ddp_write_set_gpio_output()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_SWITCH_LED_FLAG);	//send a DDP device Switch LED message - ddp_write_see turn_off_switch_leds()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_LOCK_UNLOCK_FLAG);	//send a DDP device lock unlock query message - see ddp_write_device_lock_unlock()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_AUDIO_SIGNAL_PRESENCE_FLAG);	//send a DDP audio signal presence config query message - see ddp_write_enable_audio_signal_presence()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_DANTE_DOMAIN_FLAG);		//send a DDP device Dante Domain query - see ddp_write_device_dante_domain()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_SWITCH_STATUS_FLAG);		//send a DDP device switch status query - see ddp_write_device_switch_status_query()
	//hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_MANF_OVERRIDE_FLAG);		//send a DDP device manufacture with manf override - see 
	
	//set normal state
	hostcpu_uhip_set_state(HOSTCPU_UHIP_STATE_NORMAL);
	handle_uhip_tx();
/* 	while (0 < run_flag && run_flag < 500)
	{
		run_flag ++;

		//check the uhip rx and tx timers
		check_uhip_timers();

		//send any uhip packet
		handle_uhip_tx();

		//process any received uhip packets
		//handle_uhip_rx();

		usleep(1000);

	} */

	return 0;
}

