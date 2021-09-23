
#ifndef __LDAP_AUTH_METHODS_H__
#define __LDAP_AUTH_METHODS_H__

#include "ldap_funcs.h"

#define AUTH_SUCCESS 		0
#define CANT_CONNECT_SERVER -1
#define INVALID_PORT      	-2
#define INVALID_IDENTITY 	0x31

typedef struct _cfg_info
{
	char *uri;
	char *dn;
	char *passwd;
	char *base_dn;
	char *uid;
	char *attr;
	char *ca_cert;
	char *client_cert;
	char *private_key;
} cfg_info;


int do_auth_init(char *URI);
int do_dn_auth_sync(char *URI, char* i_pDn, char *i_pPasswd);
int do_uid_auth_sync(char *URI, char *Base_dn, char *Uid,  char *Passwd);
int do_attr_search_sync(char *URI, char *dn, char *passwd, char *attr);
int do_tls_dn_auth_sync(char *URI, char *dn, char *passwd, cert_files *certs);
int do_tls_uid_auth_sync(char *URI, char *base_dn, char *uid,  char *passwd, cert_files *certs);
int do_tls_attr_search_sync(char *URI, char *dn, char *passwd, char *attr, cert_files *certs);
int do_tls_dn_auth_by_client_check_sync(char *URI, char *dn, char *passwd, cert_files *certs);
int do_tls_uid_auth_by_client_check_sync(char *URI, char *base_dn, char *uid,  char *passwd, cert_files *certs);
int do_tls_attr_search_by_client_check_sync(char *URI, char *dn, char *passwd, char *attr, cert_files *certs);
int do_auth_deinit();



#endif
