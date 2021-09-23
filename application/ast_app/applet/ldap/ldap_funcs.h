
#ifndef __LDAP_FUNCS_H__
#define __LDAP_FUNCS_H__


typedef struct {
	char *ca_cert;
	char *client_cert;
	char *private_key;
}cert_files;

int init_internal(char *URI);
int ldap_dn_auth_s(char *URI, char *dn, char *passwd);
int ldap_uid_auth_s(char *URI, char *base_dn, char *uid, char *passwd);
int ldap_tls_dn_auth_s(char *URI, char *dn, char *passwd, cert_files *certs);
int ldap_tls_uid_auth_s(char *URI, char *base_dn, char *uid,  char *passwd, cert_files *certs);
int ldap_attr_search_s(char *URI, char *dn, char *passwd, char *attr, cert_files *certs);
int ldap_auth_deinit();

#endif


