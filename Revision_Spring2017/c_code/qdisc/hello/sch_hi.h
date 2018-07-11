
#ifndef SCH_HI_H
#define SCH_HI_H

#define hi_log(args, ...) printk(KERN_INFO "qdisk hi: " args, ##__VA_ARGS__)

enum {
    TCA_HI_UNSPEC,
    TCA_HI_LIMIT,
    TCA_HI_MAXRATE,  // Rate in Hertz.
    __TCA_HI_MAX
};

#define TCA_HI_MAX (__TCA_HI_MAX - 1)

#endif /* end of include guard: SCH_HI_H */

