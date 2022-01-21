#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

static  unsigned int crc_table[256] =
{
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
};

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
static unsigned int crc32(unsigned int crc, char *buf, unsigned int len)
{
    crc = crc ^ 0xffffffffL;

    while (len >= 8)
    {
        DO8(buf);
        len -= 8;
    }

    if (len) do
        {
            DO1(buf);
        }
        while (--len);

    return crc ^ 0xffffffffL;
}

struct T_PKG_HDR
{
    int magic_number;   // 0xF0E1D2C3
    int hdr_len;
    int customer_id;
    int product_id;
    uint8_t ver_major;
    uint8_t ver_minor;
    uint8_t ver_revision;
    uint8_t ver_build;
    int build_time;
    int data_len;
    int data_crc;
    int hdr_crc;
} pkg_hdr;

static const struct option longopts[] =
{
    {"build",       no_argument,        NULL, 'b'},
    {"extract",     no_argument,        NULL, 'x'},
    {"cid",         required_argument,  NULL, 'c'},
    {"pid",         required_argument,  NULL, 'p'},
    {"version",     required_argument,  NULL, 'v'},
    {"input",       required_argument,  NULL, 'i'},
    {"output",      required_argument,  NULL, 'o'},
    {"help",        no_argument,  NULL, 'h'},
    {NULL,      0,      NULL,  0}
};

void help()
{
    printf(" Guess \n");
    exit(__LINE__);
}

void dump_hdr_info()
{
    fprintf(stdout, "\npackage informations as below:\n"
            "===================="
            "magic_number = 0x%08x\n"
            "hdr_len = 0x%d\n"
            "customer_id = 0x%08x\n"
            "product_id = 0x%08x\n"
            "ver_major = %u\n"
            "ver_minor = %u\n"
            "ver_revision = %u\n"
            "ver_build = %u\n"
            "build_time = 0x%08x\n"
            "data_len = %u\n"
            "data_crc = 0x%08x\n"
            "hdr_crc = 0x%08x\n"
            "====================\n",
            pkg_hdr.magic_number,
            pkg_hdr.hdr_len,
            pkg_hdr.customer_id,
            pkg_hdr.product_id,
            pkg_hdr.ver_major,
            pkg_hdr.ver_minor,
            pkg_hdr.ver_revision,
            pkg_hdr.ver_build,
            pkg_hdr.build_time,
            pkg_hdr.data_len,
            pkg_hdr.data_crc,
            pkg_hdr.hdr_crc
           );
}

#define MAX_READ_BUF_LEN    4096
#define CMD_NONE            0
#define CMD_BUILD           1
#define CMD_EXTRACT         2

int build_bin(const char *in_file_name, const char *out_file_name)
{
    FILE *fp_s = NULL;
    FILE *fp_t = NULL;
    char buf[MAX_READ_BUF_LEN];
    pkg_hdr.magic_number = 0xF0E1D2C3;
    pkg_hdr.hdr_len = sizeof(pkg_hdr);
    fp_s = fopen(in_file_name, "rb");
    struct timeval time;
    gettimeofday(&time, NULL);
    pkg_hdr.build_time = time.tv_sec;

    if (fp_s == NULL)
    {
        printf("can not open input file\n");
        return -1;
    }

    fp_t = fopen(out_file_name, "wb");

    if (fp_t == NULL)
    {
        printf("can not open output file\n");
        return -1;
    }

    fseek(fp_s, 0, SEEK_END);
    pkg_hdr.data_len = ftell(fp_s) + 1;
    fseek(fp_t, sizeof(pkg_hdr), SEEK_SET);
    rewind(fp_s);
    int bytes = 0;
    int crc = 0;

    do
    {
        memset(buf, 0, sizeof(buf));
        bytes = fread(buf, 1, sizeof(buf), fp_s);

        if (bytes > 0)
        {
            crc = crc32(crc, buf, bytes);
            fwrite(buf, 1, bytes, fp_t);
        }
        else
        {
            break;
        }
    }
    while (1);

    pkg_hdr.data_crc = crc;
    pkg_hdr.hdr_crc = crc32(0, (void*)(&pkg_hdr), (sizeof(pkg_hdr) - sizeof(pkg_hdr.hdr_crc)));
    fseek(fp_t, 0, SEEK_SET);
    fwrite(&pkg_hdr, 1, sizeof(pkg_hdr), fp_t);
    fclose(fp_s);
    fclose(fp_t);
    dump_hdr_info();
    return 0;
}

int extract_bin(const char *in_file_name, const char *out_file_name)
{
    FILE *fp_s = NULL;
    FILE *fp_t = NULL;
    char buf[MAX_READ_BUF_LEN];
    fp_s = fopen(in_file_name, "rb");

    if (fp_s == NULL)
    {
        printf("can not open input file\n");
        return -1;
    }

    fp_t = fopen(out_file_name, "wb");

    if (fp_t == NULL)
    {
        printf("can not open output file\n");
        return -1;
    }

    int bytes = 0;
    int crc = 0;
    int data_len = 0;
    fseek(fp_s, 0, SEEK_SET);
    bytes = fread(&pkg_hdr, 1, sizeof(pkg_hdr), fp_s);
    dump_hdr_info();

    if (bytes != sizeof(pkg_hdr))
    {
        fprintf(stderr, "file croppted\n");
        exit(__LINE__);
    }

    if (sizeof(pkg_hdr) != pkg_hdr.hdr_len)
    {
        fprintf(stderr, "file croppted\n");
        exit(__LINE__);
    }

    crc = crc32(0, (void *)(&pkg_hdr), (sizeof(pkg_hdr) - sizeof(pkg_hdr.hdr_crc)));

    if (crc != pkg_hdr.hdr_crc)
    {
        fprintf(stderr, "ERROR: file CRC wrong\n");
        exit(__LINE__);
    }

    fseek(fp_s, 0, SEEK_END);
    data_len = ftell(fp_s) + 1;
    data_len -= pkg_hdr.hdr_len;

    if (data_len != pkg_hdr.data_len)
    {
        fprintf(stderr, "file croppted, data_len:%d\n", data_len);
        exit(__LINE__);
    }

    fseek(fp_s, sizeof(pkg_hdr), SEEK_SET);
    rewind(fp_t);
    crc = 0;

    do
    {
        memset(buf, 0, sizeof(buf));
        bytes = fread(buf, 1, sizeof(buf), fp_s);

        if (bytes > 0)
        {
            crc = crc32(crc, buf, bytes);
            fwrite(buf, 1, bytes, fp_t);
        }
        else
        {
            break;
        }
    }
    while (1);

    if (pkg_hdr.data_crc != crc)
    {
        fprintf(stderr, "file croppted\n");
        exit(__LINE__);
    }

    fclose(fp_s);
    fclose(fp_t);
    fprintf(stdout, "extract file done\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int cmd = CMD_NONE;
    const char *in_file_name = NULL;
    const char *out_file_name = NULL;
    char *version = NULL;
    char *dotv = NULL;
    const char *sep = ".";

    for (;;)
    {
        int c;
        int index = 0;
        c = getopt_long(argc, argv, "bxc:p:v:i:o:h", longopts, &index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'b':
                if (cmd != CMD_NONE)
                {
                    fprintf(stderr, "support only -b or -x\n");
                }

                cmd = CMD_BUILD;
                break;

            case 'x':
                if (cmd != CMD_NONE)
                {
                    fprintf(stderr, "support only -b or -x\n");
                }

                cmd = CMD_EXTRACT;
                break;

            case 'c':
                pkg_hdr.customer_id = strtoul(optarg, NULL, 0);
                break;

            case 'p':
                pkg_hdr.product_id = strtoul(optarg, NULL, 0);
                break;

            case 'v':
                version = optarg;
                uint8_t *pver = &pkg_hdr.ver_major;
                uint32_t idx = 0;

                while ((dotv = strsep((char **)&version, sep)) != NULL)
                {
                    *pver = atoi(dotv);
                    pver++;

                    if (pver > &pkg_hdr.ver_build)
                    {
                        break;
                    }
                }

                // sscanf(optarg, "%u.%u.%u.%u", pkg_hdr.ver_major, pkg_hdr.ver_minor, pkg_hdr.ver_revision, pkg_hdr.ver_build);
                break;

            case 'i':
                in_file_name = optarg;
                break;

            case 'o':
                out_file_name = optarg;
                break;

            case 'h':
                help();
                break;

            default:
                // fprintf(stderr, "getopt\n");
                exit(__LINE__);
        }
    }

    switch (cmd)
    {
        case CMD_BUILD:
            build_bin(in_file_name, out_file_name);
            break;

        case CMD_EXTRACT:
            extract_bin(in_file_name, out_file_name);
            break;

        default:
            fprintf(stderr, "must specific a -b or -x\n");
            help();
            break;
    }

    return 0;
}

