#include "dongle.h"
#include "psu.h"

unsigned char encode_table[16]  =
			 {0x55, 0x56, 0x59, 0x5a, 0x65, 0x66, 0x69, 0x6a, 0x95, 0x96, 0x99, 0x9a, 0xa5, 0xa6, 0xa9, 0xaa};
unsigned char decode_table[256] =
			 {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
			  0x20, 0x21, 0x00, 0x12, 0x22, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x24,
			  0x25, 0x00, 0x16, 0x26, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x28, 0x29, 0x00, 0x1a,
			  0x2a, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x2c, 0x2d, 0x00, 0x1e, 0x2e,
			  0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  0x00
			};

int open_usb(char *device)
{

    int fd = open(device, O_RDWR | O_NONBLOCK);

    if (fd < 0) {
        printf("Serial port (%s) open error.\n", device);
        return -1;
    }
    return fd;
}

int xread(int f, void * b, int s, int timeout) {
    int r, ss=s;
    time_t cstart = time(NULL);
    unsigned char * bb = (unsigned char *) b;
    do {
        do {
	    if (timeout > 0 && (time(NULL) - cstart) >=  timeout) {
    		return ss - s;
	    }
            r = read(f, bb, s);
            if (r == -1) usleep(200);
        } while (r == -1);
    bb+=r;
    s-=r;
    } while (s != 0);
return ss;
}

int xwrite(int fd, void * buffer, unsigned int len) {
    int count = 0, ret = 0;
    while (count < len) {
	errno = 0;
	ret = write(fd, buffer + count, len - count);
	if (ret <= 0) {
	    if (errno == EAGAIN || errno == EWOULDBLOCK) {
		usleep(200);
		continue;
	    }
	    return ret;
	}
	count += ret;
    }
    return count;
}


/* taken from http://www.cs.unc.edu/~dewan/242/s00/xinu-pentium/debug/hexdump.c */
#define OPL 16 /* octets printed per line */

void dump(unsigned char *buf, int dlen) {
    char c[OPL+1];
    int i, ct;

    if (dlen < 0) {
        printf("WARNING: computed dlen %d\n", dlen);
        dlen = 0;
    }

    for (i=0; i<dlen; ++i) {
        if (i == 0)
            printf("DATA: ");
        else if ((i % OPL) == 0) {
            c[OPL] = '\0';
            printf("\t|%s|\nDATA: ", c);
        }
        ct = buf[i] & 0xff;
        c[i % OPL] = (ct >= ' ' && ct <= '~') ? ct : '.';
        printf("%02x ", ct);
    }
    c[i%OPL] = '\0';
    for (; i % OPL; ++i)
        printf("   ");
    printf("\t|%s|\n", c);
}

unsigned char * decode_answer(unsigned char *data, int size, int * nsize) {
	int i, j = 0;
	int newsize = (size/2);
	if (newsize <= 0) return NULL;

	if (nsize) *nsize = newsize;

        if (((decode_table[data[0]] & 0xf) >> 1) != 7) {
	    printf("decode_answer: wrong reply data: %d (data %x)\n", ((decode_table[data[0]] & 0xf) >> 1), data[0]);
	    return NULL;
	}

	unsigned char *ret = (unsigned char*) malloc(newsize);

	if (!ret) return NULL;


	for (i = 1; i <= size; i += 2) {
	    ret[j++] = (decode_table[data[i]] & 0xf) | ((decode_table[data[i + 1]] & 0xf) << 4);
	}

	return ret;
}

unsigned char * encode_answer(unsigned char command, unsigned char *data, int size, int * nsize) {
    int i, j = 1;
    if (size <= 0) return NULL;

    int newsize = (size * 2) + 2;

    if (nsize) *nsize = newsize;

    unsigned char *ret = (unsigned char *) malloc(newsize);
    if (!ret) return NULL;
    ret[0] = encode_table[(command << 1) & 0xf] & 0xfc;
    ret[newsize - 1] = 0;

    for (i = 1; i <= size; i++) {
	ret[j++] = encode_table[data[i - 1] & 0xf];
	ret[j++] = encode_table[data[i - 1] >> 4];
    }

    return ret;
}

// maximum 512 bytes
unsigned char * data_read_dongle(int fd, int size, int * command) {
    unsigned char buffer[1024];
    memset(buffer, 0, 1024);

    if (size < 0) size = 512;

    size *= 2;

    char r;
    if ((r = xread(fd, buffer, size - 1, 2)) == 0) return NULL;

//    printf("read=%d, exp=%d\n", r, size);
//    dump(buffer, r);

    read(fd, buffer+r, 1); // eat optional 0x00 at the end

    buffer[r] = 0x00;

    return decode_answer(buffer, size, command);
}

int data_write_dongle(int fd, unsigned char * datain, int size) {
    int s;
    unsigned char *data = encode_answer(0, datain, size, &s);

//    dump(datain, size);
//    printf("DATA TO WRITE:\n");
//    dump(data, s);

    int ret = xwrite(fd, data, s);
    free(data);

    return (ret != s) ? -1 : 0;
}

int init_dongle(int fd) {
    unsigned char buffer[1024];
    memset(buffer, 0, 1024);
    char r;
    int retry = 3;
    int answer = 0;
    int done = 0;
    int size = 2;
    if (send_init(fd) !=0) return -1;
    //unsigned char * ret = data_read_dongle(fd, 512, NULL);
    //free(data);

    //usleep(420);
    if ((r = xread(fd, buffer, size, 2)) == 0) {
        printf("Dongle communcation issue, retrying...");
        do {
            usleep(7000);
            if (send_init(fd) !=0) return -1;
            if ((r = xread(fd, buffer, size, 2)) != 0) done = 1;
            retry--;
        } while (done != 1 | retry >= 0);
    } else {
        done = 1;
//        printf("read=%d, exp=%d\n", r, size);
        return 0;
        };
    if ( done == 1){
        return 0;
    } else if (retry < 0) {
        printf("Dongle init-sequence failed.");
        };
    return -1;
}

unsigned char * read_dongle_name(int fd) {
    unsigned char d[1] = {2};

    if (data_write_dongle(fd, d, 1) != 0) return NULL;

    unsigned char * ret = data_read_dongle(fd, 512, NULL);

    return ret;
}

int read_dongle_version(int fd, float *f) {
    unsigned char d[1] = {0};

    if (data_write_dongle(fd, d, 1) != 0) return -1;

    unsigned char * ret = data_read_dongle(fd, 3, NULL);

    if (!ret) return -1;

    *f = (ret[1] >> 4) + (ret[1] & 0xf)/10.;
    free(ret);

    return 0;
}

int setup_dongle(int fd) {
    unsigned char * ret;
    unsigned char d[7] = {17, 2, 100, 0, 0, 0, 0};
    float f = 0.0;

    if (init_dongle(fd) != 0) return -1;

    if ((ret = read_dongle_name(fd)) == NULL) return -1;
    printf("Dongle name: %s\n", ret);
    free(ret);

    if (data_write_dongle(fd, d, 7) != 0) return -1;

    ret = data_read_dongle(fd, 1, NULL);

    if (!ret) return -1;

    free(ret);

    if (read_dongle_version(fd, &f) == -1) return -1;

    printf("Dongle version: %0.1f\n", f);

    if ((ret = read_data_psu(fd, 0x9a, 7)) == NULL) return -1;
//    dump(ret, 7);

    if (!memcmp(ret, "AX860", 5)) _psu_type = TYPE_AX860;
    if (!memcmp(ret, "AX1200", 6)) _psu_type = TYPE_AX1200;
    if (!memcmp(ret, "AX1500", 6)) _psu_type = TYPE_AX1500;
    // AX760 = default

    free(ret);

    printf("PSU type: %s\n", dump_psu_type(_psu_type));

    return 0;
}
