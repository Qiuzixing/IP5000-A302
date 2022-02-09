/*
* File     : example_bk2_common.h
* Created  : October 2014
* Updated  : 2014/10/02
* Author   : Chamira Perera, Michael Ung
* Synopsis : Functionality used across the Ultimo HostCPU API.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#include "example_uhip_common.h"
#include "example_rx_uhip.h"
#include "uhip_hostcpu_tx_timer.h"
#include "uhip_hostcpu_rx_timer.h"

/**
* Contextual data used across the HostCPU UHIP
*/
static hostcpu_uhip_t g_hostcpu_uhip;

void hostcpu_uhip_common_init()
{
	memset(&g_hostcpu_uhip, 0, sizeof(g_hostcpu_uhip));
}

/**
* Checks the rx and tx UHIP timers
*/
void check_uhip_timers(void)
{
	//If the RX timer has fired / expired
	if (uhip_hostcpu_rx_timer_has_fired())
	{
		//increment the counters
		g_hostcpu_uhip.stats.num_rx_timeouts++;

		//we have probably lost synchronisation on the RX path. We need to reset the RX buffer.
		reset_uhip_rx_buffers();

		//stop the rx timer
		uhip_hostcpu_rx_timer_stop();
	}

	//If the TX timer has fired / expired
	if (uhip_hostcpu_tx_timer_has_fired())
	{
		switch (g_hostcpu_uhip.state)
		{
		case HOSTCPU_UHIP_STATE_NO_TX_WAIT_ACK:
		{
			//stop the timer
			uhip_hostcpu_tx_timer_stop();

			//increment the counters
			g_hostcpu_uhip.stats.num_tx_timeouts++;

			//change the state, so we don't send until the timer expires again
			g_hostcpu_uhip.state = HOSTCPU_UHIP_STATE_NO_TX_RESYNC;

			//start the tx timer again
			uhip_hostcpu_tx_timer_start(TX_RESYNC_STOP_TIMEOUT_MILLISECONDS);
		}
			break;

		case HOSTCPU_UHIP_STATE_NO_TX_RESYNC:
		{
			//stop the tx timer
			uhip_hostcpu_tx_timer_stop();

			//change the state - it is OK to send now
			g_hostcpu_uhip.state = HOSTCPU_UHIP_STATE_NORMAL;
		}
			break;

		default:
		{
			AUD_PRINTF("ERROR tx timer fired in unknown state\n");
		}
			break;
		}
	}
}

ddp_request_id_t hostcpu_uhip_get_ddp_tx_request_id()
{
	return g_hostcpu_uhip.ddp_tx_request_id;
}

void hostcpu_uhip_set_ddp_tx_request_id(ddp_request_id_t request_id)
{
	g_hostcpu_uhip.ddp_tx_request_id = request_id;
}

aud_bool_t hostcpu_uhip_is_tx_flag_set(hostcpu_uhip_tx_packet_flags_t tx_flag)
{
	return ((g_hostcpu_uhip.tx_flags & tx_flag) != 0);
}

void hostcpu_uhip_set_tx_flag(hostcpu_uhip_tx_packet_flags_t tx_flag)
{
	g_hostcpu_uhip.tx_flags |= tx_flag;
}

void hostcpu_uhip_clear_tx_flag(hostcpu_uhip_tx_packet_flags_t tx_flag)
{
	g_hostcpu_uhip.tx_flags &= ~tx_flag;
}

void hostcpu_uhip_increment_num_tx_prot_ctrl_pkts()
{
	g_hostcpu_uhip.stats.num_tx_prot_ctrl_pkts++;
}

void hostcpu_uhip_increment_num_tx_cmc_pb_pkts()
{
	g_hostcpu_uhip.stats.num_tx_cmc_pb_pkts++;
}

void hostcpu_uhip_increment_num_tx_udp_pb_pkts()
{
	g_hostcpu_uhip.stats.num_tx_udp_pb_pkts++;
}

void hostcpu_uhip_increment_num_rx_overflows()
{
	g_hostcpu_uhip.stats.num_rx_overflows++;
}

void hostcpu_uhip_increment_num_rx_ddp_pkts()
{
	g_hostcpu_uhip.stats.num_rx_ddp_pkts++;
}

void hostcpu_uhip_increment_num_rx_dante_evnt_pkts()
{
	g_hostcpu_uhip.stats.num_rx_dante_evnt_pkts++;
}

void hostcpu_uhip_increment_num_rx_udp_pb_pkts()
{
	g_hostcpu_uhip.stats.num_rx_udp_pb_pkts++;
}

void hostcpu_uhip_increment_num_rx_cmc_pb_pkts()
{
	g_hostcpu_uhip.stats.num_rx_cmc_pb_pkts++;
}

void hostcpu_uhip_increment_num_rx_malformed_errs()
{
	g_hostcpu_uhip.stats.num_rx_malformed_errs++;
}

void hostcpu_uhip_increment_num_rx_cobs_decode_errs()
{
	g_hostcpu_uhip.stats.num_rx_cobs_decode_errs++;
}

void hostcpu_uhip_increment_num_rx_prot_ctrl_pkts()
{
	g_hostcpu_uhip.stats.num_rx_prot_ctrl_pkts++;
}

hostcpu_uhip_state_t hostcpu_uhip_get_state()
{
	return g_hostcpu_uhip.state;
}

void hostcpu_uhip_set_state(hostcpu_uhip_state_t state)
{
	g_hostcpu_uhip.state = state;
}

/*******************************************
 ********* User specific functions *********
 *******************************************/
void hostcpu_ddp_route_ready(uint8_t route_ready_flag)
{
	g_hostcpu_uhip.route_ready_received = route_ready_flag;
}

void hostcpu_ddp_srate_update(int new_srate)
{
	g_hostcpu_uhip.new_sample_rate = new_srate;
}
