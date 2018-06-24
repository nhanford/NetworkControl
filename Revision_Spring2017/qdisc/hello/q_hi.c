/*
 * q_hi.c		HI.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Taran Lynn <taranlynn0@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <libnetlink.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
    fprintf(stderr, "Usage: ... hi\n");
}

static int hi_parse_opt(struct qdisc_util *qu, int argc, char **argv,
        struct nlmsghdr *n, const char *dev)
{
    printf("Parsing options\n");

    if (argc > 0) {
        explain();

        return -1;
    }

    return 0;
}

static int hi_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
    return 0;
}


struct qdisc_util hi_qdisc_util = {
    .id = "pfifo",
    .parse_qopt = hi_parse_opt,
    .print_qopt = hi_print_opt,
};
