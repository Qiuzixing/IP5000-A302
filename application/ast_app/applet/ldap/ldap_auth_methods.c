
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "ldap_auth_methods.h"

int do_auth_init(char *URI)
{
	int ret = 0;	
	do 
	{
		if (URI == NULL)
		{
			printf("uri is NULL\n");
			ret = -1;
			break;
		}
		if (strstr(URI, "ldaps") != NULL && strstr(URI, "636") == NULL)
		{
			printf("port should is 636\n");
			ret = INVALID_PORT;
			break;
		}
		if (strstr(URI, "ldaps") == NULL && strstr(URI, "389") == NULL)
		{
			printf("port should is 389\n");
			ret = INVALID_PORT;
			break;			
		}
		
		ret = init_internal(URI);
	}while(0);
	
	return ret;
}

int do_dn_auth_sync(char *URI, char* i_pDn, char *i_pPasswd)
{
	int ret;	
	ret = ldap_dn_auth_s(URI, i_pDn, i_pPasswd);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_uid_auth_sync(char *URI, char *Base_dn, char *Uid,  char *Passwd)
{
	int ret;
	ret = ldap_uid_auth_s(URI, Base_dn, Uid, Passwd);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);	
	}
	return ret;
}

int do_attr_search_sync(char *URI, char *dn, char *passwd, char *attr)
{
	int ret;
	
	ret = ldap_attr_search_s(URI, dn, passwd, attr, NULL);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_tls_dn_auth_sync(char *URI, char *dn, char *passwd, cert_files *certs)
{
	int ret;
		
	ret = ldap_tls_dn_auth_s(URI, dn, passwd, certs);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_tls_uid_auth_sync(char *URI, char *base_dn, char *uid,  char *passwd, cert_files *certs)
{
	int ret;

	ret = ldap_tls_uid_auth_s(URI, base_dn, uid, passwd, certs);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_tls_attr_search_sync(char *URI, char *dn, char *passwd, char *attr, cert_files *certs)
{
	int ret;
	ret = ldap_attr_search_s(URI, dn, passwd, attr, certs);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_tls_dn_auth_by_client_check_sync(char *URI, char *dn, char *passwd, cert_files *certs)
{
	int ret;
	ret = ldap_tls_dn_auth_s(URI, dn, passwd, certs);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_tls_uid_auth_by_client_check_sync(char *URI, char *base_dn, char *uid,  char *passwd, cert_files *certs)
{
	int ret;
	ret = ldap_tls_uid_auth_s(URI, base_dn, uid, passwd, certs);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_tls_attr_search_by_client_check_sync(char *URI, char *dn, char *passwd, char *attr, cert_files *certs)
{
	int ret;	
	ret = ldap_attr_search_s(URI, dn, passwd, attr, certs);
	if (ret == AUTH_SUCCESS) {
		printf("(%s):auth success!\n", __func__);
	} else {
		printf("(%s):auth fail!\n", __func__);
	}
	return ret;
}

int do_auth_deinit()
{
	int ret = 0;
	ret = ldap_auth_deinit();
	return ret;
}


