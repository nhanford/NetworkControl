/*
 * q_mpc.c		MPC.
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

#include "sch_mpc.h"

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
    fprintf(stderr, "Usage: ... mpc [ limit <limit> ] [ maxrate <maxrate> ]\n");
}

static int mpc_parse_opt(struct qdisc_util *qu, int argc, char **argv,
        struct nlmsghdr *n, const char *dev)
{
    unsigned int limit;
    unsigned int max_rate;
    bool set_limit = false;
    bool set_max_rate = false;

    struct rtattr *tail;

    while (argc > 0) {
        if (strcmp(*argv, "limit") == 0) {
            NEXT_ARG();

            if (get_unsigned(&limit, *argv, 0)) {
                fprintf(stderr, "Illegal \"limit\"\n");
                return -1;
            }

            set_limit = true;
        } else if (strcmp(*argv, "maxrate") == 0) {
            NEXT_ARG();

            if (get_rate(&max_rate, *argv)) {
                fprintf(stderr, "Illegal \"maxrate\"\n");
                return -1;
            }

            set_max_rate = true;
        } else {
            explain();
            return -1;
        }

        argc--; argv++;
    }


    tail = addattr_nest(n, 1024, TCA_OPTIONS);

    if (set_limit)
        addattr_l(n, 1024, TCA_MPC_LIMIT, &limit, sizeof(limit));
    if (set_max_rate)
        addattr_l(n, 1024, TCA_MPC_MAXRATE, &max_rate, sizeof(max_rate));

    addattr_nest_end(n, tail);

    return 0;
}

static int mpc_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
    struct rtattr *tb[TCA_FQ_MAX + 1];
    unsigned int limit;
    unsigned int max_rate;

    SPRINT_BUF(b1);

    if (opt == NULL)
        return 0;

    parse_rtattr_nested(tb, TCA_FQ_MAX, opt);

    if (tb[TCA_MPC_LIMIT] &&
            RTA_PAYLOAD(tb[TCA_MPC_LIMIT]) >= sizeof(__u32)) {
        limit = rta_getattr_u32(tb[TCA_MPC_LIMIT]);
        fprintf(f, "limit %up ", limit);
    }

    if (tb[TCA_MPC_MAXRATE] &&
            RTA_PAYLOAD(tb[TCA_MPC_MAXRATE]) >= sizeof(__u32)) {
        max_rate = rta_getattr_u32(tb[TCA_MPC_MAXRATE]);

        if(max_rate != ~0U)
          fprintf(f, "maxrate %s ", sprint_rate(max_rate, b1));
    }

    return 0;
}


struct qdisc_util mpc_qdisc_util = {
    .id = "mpc",
    .parse_qopt = mpc_parse_opt,
    .print_qopt = mpc_print_opt,
};
