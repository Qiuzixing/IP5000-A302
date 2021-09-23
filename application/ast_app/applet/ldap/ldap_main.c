
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <getopt.h>
#include <signal.h>

#include "ldap_auth_methods.h"
#include "notify.h"

int flg;
void help(const char* i_pName)
{
    printf("%s [options]\n", i_pName);
    printf("default: [%s -r ldap://192.168.60.95:389 -d cn=jeremy,ou=People,dc=my-domain,dc=com "\
           "-p 123456 -b dc=my-domain,dc=com -u uid=jeremy -a loginShell=/bin/bash - c /certs/ca.pem "
           "-l certs/client.pem -k client.key", i_pName);
    printf("	-h --help		show this help\n");
    printf("	-r --uri		LDAP Server URI\n");
    printf("	-d --dn		    dn that user will authentication\n");
    printf("	-p --password	password for dn		\n");
    printf("	-b --base_dn	base dn\n");
    printf("	-u --uid		usename or use id\n");
    printf("	-a --attr		attribute\n");
    printf("	-c --ca_cert	ca certificate\n");
    printf("	-l --client_cert client certificate\n");
    printf("	-k --private_key	     private key\n");
}

void init_cfg_info(int argc, char **argv, cfg_info *ldap_cfg_info)
{
    static struct option longOpts[] =
    {
        { "help",       no_argument, NULL, 'h' },
        { "uri",        required_argument,  NULL,   'r' },
        { "dn",         required_argument,  NULL,   'd' },
        { "password",   required_argument,  NULL,   'p' },
        { "base_dn",    required_argument,  NULL,   'b' },
        { "uid",        required_argument,  NULL,   'u' },
        { "attr",       required_argument,  NULL,   'a' },
        { "ca_cert",    required_argument,  NULL,   'c' },
        { "client_cert", required_argument,  NULL,   'l' },
        { "private_key",        required_argument,  NULL,   'k' },
        { "number",     required_argument,  NULL,   'n' },
        { NULL, 0, NULL, 0 }
    };

    while (1)
    {
        int c;
        int index = 0;
        c = getopt_long(argc, argv, "hr:d:p:b:u:a:c:l:k:n:", longOpts, &index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'h':
                help(argv[0]);
                exit(-1);

            case 'r':
                ldap_cfg_info->uri = strdup(optarg);
                break;

            case 'd':
                ldap_cfg_info->dn = strdup(optarg);
                break;

            case 'p':
                ldap_cfg_info->passwd = strdup(optarg);
                break;

            case 'b':
                ldap_cfg_info->base_dn = strdup(optarg);
                break;

            case 'u':
                ldap_cfg_info->uid = strdup(optarg);
                break;

            case 'a':
                ldap_cfg_info->attr = strdup(optarg);
                break;

            case 'c':
                ldap_cfg_info->ca_cert = strdup(optarg);
                break;

            case 'l':
                ldap_cfg_info->client_cert = strdup(optarg);
                break;

            case 'k':
                ldap_cfg_info->private_key = strdup(optarg);
                break;

            case 'n':
                flg = atoi(optarg);
                break;

            default:
                help(argv[0]);
                exit(-1);
        }
    }

    ldap_cfg_info->uri != NULL ?        printf( "-r %s\n", ldap_cfg_info->uri) : 1;
    ldap_cfg_info->dn != NULL ?         printf( "-d %s\n", ldap_cfg_info->dn) : 1;
    ldap_cfg_info->passwd != NULL ?     printf( "-w %s\n", ldap_cfg_info->passwd) : 1;
    ldap_cfg_info->base_dn != NULL ?    printf( "-b %s\n", ldap_cfg_info->base_dn) : 1;
    ldap_cfg_info->uid != NULL ?        printf( "-u %s\n", ldap_cfg_info->uid) : 1;
    ldap_cfg_info->attr != NULL ?       printf( "-a %s\n", ldap_cfg_info->attr) : 1;
    ldap_cfg_info->ca_cert != NULL ?    printf( "-c %s\n", ldap_cfg_info->ca_cert) : 1;
    ldap_cfg_info->client_cert != NULL ? printf( "-l %s\n", ldap_cfg_info->client_cert) : 1;
    ldap_cfg_info->private_key != NULL ? printf( "-k %s\n", ldap_cfg_info->private_key) : 1;
    printf( "-n %d\n", flg);
}
/*
接口有1.非加密：通过dn认证
                通过base_dn和uid认证
                查询是否有config_info.attr属性

      加密通道：
      2单向认证: 通过dn认证
                 通过base_dn和uid认证
                 查询是否有attr属性

      3双向认证.  通过dn认证
                  通过base_dn和uid认证
                  查询是否有attr属性
*/
void free_cfg_info(cfg_info *cfg)
{
    cfg->uri != NULL ? free(cfg->uri) : 1;
    cfg->dn != NULL ? free(cfg->dn) : 1;
    cfg->passwd != NULL ? free(cfg->passwd) : 1;
    cfg->base_dn != NULL ? free(cfg->base_dn) : 1;
    cfg->attr != NULL ? free(cfg->attr) : 1;
    cfg->uid != NULL ? free(cfg->uid) : 1;
    cfg->ca_cert != NULL ? free(cfg->ca_cert) : 1;
    cfg->client_cert != NULL ? free(cfg->client_cert) : 1;
    cfg->private_key != NULL ? free(cfg->private_key) : 1;
}

int main(int argc, char *argv[])
{
    int rc;
    int count = 1;
    cfg_info config_info =
    {
        .uri        = NULL,
        .dn         = NULL,
        .passwd     = NULL,
        .base_dn    = NULL,
        .uid        = NULL,
        .attr       = NULL,
        .ca_cert    = NULL,
        .client_cert = NULL,
        .private_key        = NULL
    };
    init_cfg_info(argc, argv, &config_info);
    cert_files tls_cert_files =
    {
        .ca_cert = NULL,
        .client_cert = NULL,
        .private_key = NULL
    };
    tls_cert_files.ca_cert = config_info.ca_cert;
    tls_cert_files.client_cert = config_info.client_cert;
    tls_cert_files.private_key = config_info.private_key;
    rc = do_auth_init(config_info.uri);

    if (rc != 0)
    {
        printf("init fail: %x\n", rc);
        return -1;
    }

    switch (flg)
    {
        case 1: // dn
            if (strstr(config_info.uri, "ldaps://") != NULL)
            {
                rc = do_tls_dn_auth_sync(config_info.uri, config_info.dn, config_info.passwd, &tls_cert_files);
            }
            else
            {
                rc = do_dn_auth_sync(config_info.uri, config_info.dn, config_info.passwd);
            }

            break;

        case 2: // udi
            if (strstr(config_info.uri, "ldaps://") != NULL)
            {
                rc = rc = do_tls_uid_auth_sync(config_info.uri, config_info.base_dn,
                                               config_info.uid, config_info.passwd, &tls_cert_files);
            }
            else
            {
                rc = rc = do_uid_auth_sync(config_info.uri, config_info.base_dn,
                                           config_info.uid, config_info.passwd);
            }

            break;

        default:
            break;
    }

    switch (rc)
    {
        case AUTH_SUCCESS:
            printf("auth success\n");
            send_notify_msg("226.2.0.0", 10000, LDAP_STATUS_OK);
            break;

        case INVALID_IDENTITY:
            printf("dn or password error\n");
            send_notify_msg("226.2.0.0", 10000, LDAP_STATUS_DN_PASS_ERR);
            break;

        case CANT_CONNECT_SERVER:
            printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
            send_notify_msg("226.2.0.0", 10000, LDAP_STATUS_CONN_ERR);
            break;

        default:
            break;
    }

#if 0

    switch (flg)
    {
        case 1:
            while (count--)
            {
                //通过dn认证
                rc = do_dn_auth_sync(config_info.uri, config_info.dn, config_info.passwd);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        case 2:
            while (count--)
            {
                //通过base_dn和uid认证
                rc = do_uid_auth_sync(config_info.uri, config_info.base_dn,
                                      config_info.uid, config_info.passwd);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        case 3:
            while (count--)
            {
                //查询是否有config_info.attr属性
                rc = do_attr_search_sync(config_info.uri, config_info.dn,
                                         config_info.passwd, config_info.attr);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        /*2.tls for dual channel check*/
        case 4:
            while (count--)
            {
                rc = do_tls_dn_auth_sync(config_info.uri, config_info.dn,
                                         config_info.passwd, &tls_cert_files);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        case 5:
            while (count--)
            {
                rc = do_tls_uid_auth_sync(config_info.uri, config_info.base_dn,
                                          config_info.uid, config_info.passwd, &tls_cert_files);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        case 6:
            while (count--)
            {
                rc = do_tls_attr_search_sync(config_info.uri, config_info.dn,
                                             config_info.passwd, config_info.attr, &tls_cert_files);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        /* tls for single channel check*/
        case 7:
            while (count--)
            {
                rc = do_tls_dn_auth_by_client_check_sync(config_info.uri, config_info.dn,
                        config_info.passwd, &tls_cert_files);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        case 8:
            while (count--)
            {
                rc = do_tls_uid_auth_by_client_check_sync(config_info.uri, config_info.base_dn,
                        config_info.uid, config_info.passwd, &tls_cert_files);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;

        case 9:
            while (count--)
            {
                rc = do_tls_attr_search_by_client_check_sync(config_info.uri, config_info.dn,
                        config_info.passwd, config_info.attr, &tls_cert_files);

                if (rc == AUTH_SUCCESS)
                    printf("auth success\n");

                if (rc == INVALID_IDENTITY)
                {
                    printf("dn or password error\n");
                    break;
                }

                if (rc == CANT_CONNECT_SERVER)
                {
                    printf("1.is the server uri correct, 2.whether the server is down, 3.is the certificate correct.\n");
                    break;
                }

                usleep(500000);
            }

            break;
    }

#endif
    do_auth_deinit();
    free_cfg_info(&config_info);
    return 0;
}


