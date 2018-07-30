
#ifndef SCH_MPC_H
#define SCH_MPC_H

#define mpc_qd_log(args, ...) printk(KERN_INFO "mpc qdisc: " args, ##__VA_ARGS__)

enum {
	TCA_MPC_UNSPEC,
	TCA_MPC_LIMIT,
	__TCA_MPC_MAX
};

#define TCA_MPC_MAX (__TCA_MPC_MAX - 1)

#endif /* end of include guard: SCH_MPC_H */

