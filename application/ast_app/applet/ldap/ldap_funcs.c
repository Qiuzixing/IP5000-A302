#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "ldap.h"
#include "ldap_funcs.h"

LDAP_F( int )
ldap_result2error LDAP_P((
	LDAP *ld,
	LDAPMessage *r,
	int freeit ));

static LDAP *ld = NULL;
int init_internal(char *URI)
{
	int rc = LDAP_SUCCESS;
	int ldap_version = LDAP_VERSION3;
	
	struct timeval network_timeout = {
		.tv_usec = 0,
		.tv_sec  = 6,
	};
	rc = ldap_initialize(&ld, URI);
	if (rc != LDAP_SUCCESS)
		return -1;
	ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, (const void *)&ldap_version);
	ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, (const void *)&network_timeout);

	return rc;
}

static void set_tls_channel(LDAP *ld, cert_files *certs)
{
	int tls_ctx	= 0;
	int require_cert = LDAP_OPT_X_TLS_DEMAND;
	
	if (certs != NULL) {
		
		if (certs->ca_cert != NULL ){
		ldap_set_option(ld, LDAP_OPT_X_TLS_CACERTFILE, 		(const void*)certs->ca_cert);	
		}
		if (certs->client_cert != NULL ){
			ldap_set_option(ld, LDAP_OPT_X_TLS_CERTFILE, 		(const void*)certs->client_cert);
		}
		if (certs->private_key != NULL ){
			ldap_set_option(ld, LDAP_OPT_X_TLS_KEYFILE, 		(const void*)certs->private_key);
		}
		if (certs->ca_cert != NULL)
		{
			ldap_set_option(ld, LDAP_OPT_X_TLS_REQUIRE_CERT, 	(const void*)&require_cert);
		}
		
		ldap_set_option(ld, LDAP_OPT_X_TLS_NEWCTX, 			(const void*)&tls_ctx);
	}
}

static int reauth_internal(LDAP *ld, char *base_dn, char *uid, char *passwd)
{
	int rc = LDAP_SUCCESS;
	int entries = 0;
	LDAPMessage *result = NULL;
	LDAPMessage *e = NULL;
	struct berval bval = {
		.bv_len = strlen(passwd),
		.bv_val = passwd
	};

	rc = ldap_search_ext_s(ld, base_dn, LDAP_SCOPE_SUBTREE,
			uid, NULL, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &result);
	if(rc != LDAP_SUCCESS){
		printf("error:%s----\n", ldap_err2string(rc));
		rc = -1;
	} else {
		entries = ldap_count_entries(ld, result);
		if (entries == 1) {
			e = ldap_first_entry(ld, result);
			char *dn = ldap_get_dn(ld, e);
			bval.bv_len = strlen(passwd);
			bval.bv_val = passwd;
			rc = ldap_sasl_bind_s(ld, dn, LDAP_SASL_SIMPLE, &bval, NULL, NULL, NULL);
			if (rc != LDAP_SUCCESS) {
				printf("error:%s\n", ldap_err2string(rc));
				rc = -1;
			}
			ber_memfree(dn);
		} else {
			printf("find current dn index error!\n");
			rc = -1;
		}
	}
	ldap_msgfree(result);

	return rc;
}

static int search_internal(LDAP *ld, char *base, char *attr)
{
	int rc = LDAP_SUCCESS;
	int entries = 0;
	LDAPMessage *result = NULL;

	rc = ldap_search_ext_s(ld, base, LDAP_SCOPE_SUBTREE,
							attr, NULL, 0, NULL, NULL, NULL,
							LDAP_NO_LIMIT, &result);
	if (rc != LDAP_SUCCESS) {
		printf("error:%s\n", ldap_err2string(rc));
		rc = -1;
	} else {
		entries = ldap_count_entries(ld, result);
		printf("find count = %d\n", entries);
		if (entries < 1) {
			printf("find current dn index error!\n");
			rc = -1;
		}
	}
	ldap_msgfree(result);

	return rc; 
}

int ldap_dn_auth_s(char *URI, char *dn, char *passwd)
{
	int rc = LDAP_SUCCESS;
	struct berval bval = {
		.bv_len = strlen(passwd),
		.bv_val = passwd
	};
		
	do {
		
		if (ld == NULL) {
			rc = init_internal(URI);
			if (rc != 0)
				break;
		}
		
		rc = ldap_sasl_bind_s(ld, dn, LDAP_SASL_SIMPLE, &bval, NULL, NULL, NULL);
		if (rc != LDAP_SUCCESS) {
			printf("sasl bind error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
		}
		
	} while(0);
		
	return rc;
}

int ldap_uid_auth_s(char *URI, char *base_dn, char *uid, char *passwd)
{
	int rc = LDAP_SUCCESS;
	struct berval bval = {
		.bv_len = 0,
		.bv_val = "",
	};
	do {
		if (ld == NULL) {
			rc = init_internal(URI);
			if (rc != 0)
				break;
		}
		
		rc = ldap_sasl_bind_s(ld, NULL, LDAP_SASL_SIMPLE, &bval, NULL, NULL, NULL);
		if (rc != LDAP_SUCCESS) {
			printf("sasl bind error[%d]: %d %s\n", __LINE__, rc, ldap_err2string(rc));
			break;
		}

		rc = reauth_internal(ld, base_dn, uid, passwd);
		if (rc != LDAP_SUCCESS) {
			printf("reauth error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
			break;
		}
		
	}while(0);
	return rc;
}

int ldap_tls_dn_auth_s(char *URI, char *dn, char *passwd, cert_files *certs)
{
	int rc = LDAP_SUCCESS;
	struct berval bval = {
		.bv_len = strlen(passwd),
		.bv_val = passwd
	};
		
	do {
		if (ld == NULL) {
			rc = init_internal(URI);
			if (rc != 0)
				break;	
		}
		
		set_tls_channel(ld, certs);
		
		rc = ldap_sasl_bind_s(ld, dn, LDAP_SASL_SIMPLE, &bval, NULL, NULL, NULL);
		if(rc != LDAP_SUCCESS){
			printf("sasl bind error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
			break;
		}
			
	} while(0);
		
	return rc;	
}

int ldap_tls_uid_auth_s(char *URI, char *base_dn, char *uid,  char *passwd, cert_files *certs)
{
	int rc = LDAP_SUCCESS;
	struct berval bval = {
		.bv_len = 0,		
		.bv_val = ""
	};
		
	do {
		if (ld == NULL) {
			rc = init_internal(URI);
			if (rc != 0)
				break;
		}
		set_tls_channel(ld, certs);

		rc = ldap_sasl_bind_s(ld, NULL, LDAP_SASL_SIMPLE, &bval, NULL, NULL, NULL);
		if(rc != LDAP_SUCCESS){
			printf("sasl bind error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
			break;
		}

		rc = reauth_internal(ld, base_dn, uid, passwd);
		if(rc != LDAP_SUCCESS){
			printf("reauth error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
			break;
		}
		
	} while(0);
		
	return rc;
}

int ldap_attr_search_s(char *URI, char *dn, char *passwd, char *attr, cert_files *certs)
{
	int rc = LDAP_SUCCESS;
	char *pdn = NULL;
	struct berval bval = {
		.bv_len = 0,
		.bv_val = "",
	};
	
	if(passwd != NULL){
		bval.bv_len = strlen(passwd);
		bval.bv_val = passwd;
		pdn = dn;
	}
	
	do {
		if (ld == NULL) {
			rc = init_internal(URI);
			if (rc != 0)
				break;
		}

		set_tls_channel(ld, certs);	
		
		rc = ldap_sasl_bind_s(ld, pdn, LDAP_SASL_SIMPLE, &bval, NULL, NULL, NULL);
		if(rc != LDAP_SUCCESS){
			printf("sasl bind error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
			break;
		}
		
		rc = search_internal(ld, dn, attr);
		if(rc != LDAP_SUCCESS){
			printf("search error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
			break;
		}

	}while(0);
		
	return rc;
}

int ldap_auth_deinit()
{
	int rc = LDAP_SUCCESS;
	
	if (ld != NULL)
	{
		rc = ldap_unbind_ext_s(ld, NULL, NULL);
		if(rc != LDAP_SUCCESS){
			printf("unbind error[%d]:%d %s\n", __LINE__, rc, ldap_err2string(rc));
		}
		
	} 
	return rc;
}


