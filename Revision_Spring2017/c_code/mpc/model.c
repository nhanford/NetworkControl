
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, s32 psi, s32 xi, s32 gamma, s32 alpha,
		size_t p, size_t q)
{
	size_t i;

	md->psi = psi*MPC_ONE/100;
	md->xi = xi*MPC_ONE/100;
	md->gamma = gamma*MPC_ONE/100;

	md->avg_rtt = 0;
	md->avg_rtt_var = 0;
	md->avg_pacing_rate = 0;

	md->predicted_rtt = 0;

	md->p = p;
	md->q = q;

	md->alpha = alpha*MPC_ONE/100;

	md->a = kmalloc(p*sizeof(s64), GFP_KERNEL);
	if (md->a == NULL)
		goto exit_failure_a;

	md->b = kmalloc(q*sizeof(s64), GFP_KERNEL);
	if (md->b == NULL)
		goto exit_failure_b;

	for (i = 0; i < p; i++)
		md->a[i] = 0;
	for (i = 0; i < q; i++)
		md->b[i] = 0;

	if (lookback_init(&md->lb_rtt, p, 0))
		goto exit_failure_c;

	if (lookback_init(&md->lb_pacing_rate, q, 0))
		goto exit_failure_d;

	mpc_dfs_init(&md->dstats);

	return 0;

exit_failure_d:
	lookback_release(&md->lb_rtt);
exit_failure_c:
	kfree(md->b);
exit_failure_b:
	kfree(md->a);
exit_failure_a:
	return 1;
}

void model_release(struct model *md)
{
	kfree(md->a);
	kfree(md->b);

	lookback_release(&md->lb_rtt);
	lookback_release(&md->lb_pacing_rate);

	mpc_dfs_release(&md->dstats);
}
