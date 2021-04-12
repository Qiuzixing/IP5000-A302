/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
int parse_cmd_unknown(char *cmd_buffer, struct nr_cfg_s *c);
int parse_del_chg_notify(char *cmd_buffer, struct nr_cfg_s *c);
int parse_add_chg_notify(char *cmd_buffer, struct nr_cfg_s *c);
int parse_list_chg_notify(char *cmd_buffer, struct nr_cfg_s *c);

struct ctrl_cmd {
	char *string;
	int (*parse)(char *cmd_buf, struct nr_cfg_s *c);
};

struct ctrl_cmd ctrl_cmd_table[] = {
	{"add_chg_notify", parse_add_chg_notify},
	{"del_chg_notify", parse_del_chg_notify},
	{"list_chg_notify", parse_list_chg_notify},
	{NULL, parse_cmd_unknown},
};

/*
** Return the index of found MAC or empty slot.
** Return LISTEN_NODES_MAX if no more empty slot available
*/
static unsigned int find_listen_nodes(struct nr_cfg_s *c, char *mac)
{
	node_info_s *node;
	int i, empty_idx = -1;

	node = c->listen_nodes;
	for (i = 0; i < LISTEN_NODES_MAX; i++, node++) {
		/* check empty string. */
		if (node->mac[0] == 0) {
			if (empty_idx == -1)
				empty_idx = i;
			continue;
		}
		if (!strncmp(mac, node->mac, 12))
			break;
	}
	/* No match found and no empty slot. */
	if (i != LISTEN_NODES_MAX)
		return i;
	else if (empty_idx != -1)
		return empty_idx;
	else
		return LISTEN_NODES_MAX;
}

static void add_chg_notify(struct nr_cfg_s *c, char *mac)
{
	node_info_s *node;
	int i;

	i = find_listen_nodes(c, mac);
	if (i == LISTEN_NODES_MAX) {
		err("No empty slot!?\n");
		return;
	}
	node = &c->listen_nodes[i];

	strncpy(node->mac, mac, 12);
	node->session = 0;
}

static void del_chg_notify(struct nr_cfg_s *c, char *mac)
{
	node_info_s *node;
	int i;

	node = c->listen_nodes;
	for (i = 0; i < LISTEN_NODES_MAX; i++, node++) {
		if (!strncmp(mac, node->mac, 12))
			break;
	}
	if (i == LISTEN_NODES_MAX) {
		err("Can't find %s\n", mac);
		return;
	}
	memset(node->mac, 0, 13);
	node->session = 0;
}

int parse_cmd_unknown(char *cmd_buffer, struct nr_cfg_s *c)
{
	return -1;
}

int parse_list_chg_notify(char *cmd_buffer, struct nr_cfg_s *c)
{
	node_info_s *node;
	int i;

	/* TODO. Use ipc query instead. */
	msg("|||||||||||||||||||||\n");
	node = c->listen_nodes;
	for (i = 0; i < LISTEN_NODES_MAX; i++, node++) {
		msg("%d:%s,%d\n", i, node->mac, node->session);
	}
	msg("|||||||||||||||||||||\n");
	return 0;
}

int parse_del_chg_notify(char *cmd_buffer, struct nr_cfg_s *c)
{
	char *save_ptr;
	char *token;
	/*
	 * cmd format:
	 * del_chg_notify:mac
	 *   mac: A devices's MAC address. Previously added.
	 * Ex:
	 *	del_chg_notify:0030F1112233
	 */

	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err_out;

	/* MAC addr */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token)
		goto err_out;

	del_chg_notify(c, token);

	return 0;
err_out:
	return -1;
}

int parse_add_chg_notify(char *cmd_buffer, struct nr_cfg_s *c)
{
	char *save_ptr;
	char *token;
	/*
	 * cmd format:
	 * add_chg_notify:mac
	 *   mac: to monitor notification node_query of a devices's MAC address
	 *   chg_info: which 'reply_type' to monitor
	 * Ex:
	 *	add_chg_notify:0030F1112233
	 */

	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err_out;

	/* MAC addr */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token)
		goto err_out;

	add_chg_notify(c, token);

	return 0;
err_out:
	return -1;
}

void handle_ctrl_cmd(char *cmd_buffer, struct nr_cfg_s *c)
{
	//struct video_ctrl ctrl, *pctrl = &ctrl;
	struct ctrl_cmd *pccmd;
	char *token;
	int i;

	if (!strlen(cmd_buffer))
		return;

	token = strsep(&cmd_buffer, ":");
	for (pccmd = ctrl_cmd_table; pccmd->string != NULL; pccmd++) {
		if (!strncmp(token, pccmd->string, strlen(pccmd->string))) {
			/* Found! */
			break;
		}
	}

	if (pccmd->parse(cmd_buffer, c)) {
		err("parse_cmd() error! %s\n", cmd_buffer);
		return;
	}
}

