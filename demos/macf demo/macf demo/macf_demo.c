#include <sys/systm.h>
#include <mach/mach_types.h>
#include <Kernel/security/mac_policy.h>
#include "macf_demo.h"

static int openCallaback(kauth_cred_t cred, struct vnode *vp, struct label *label, int acc_mode)
{

}

static struct mac_policy_ops mmPolicyOps = {
    .mpo_vnode_check_open_t = openCallback,
}

static struct mac_policy_conf policyConf = {
    .mpc_name            = m_bundleId,
    .mpc_fullname        = m_bundleId,
    .mpc_labelnames      = NULL,
    .mpc_labelname_count = 0,
    .mpc_ops             = &mmPolicyOps,
    .mpc_loadtime_flags  = MPC_LOADTIME_FLAG_UNLOADOK,
    .mpc_field_off       = NULL,
    .mpc_runtime_flags   = 0,
    .mpc_list            = NULL,
    .mpc_data            = NULL,
};

static mac_policy_handle_t handlep;

kern_return_t generic_kext_demo_start(kmod_info_t * ki, void *d);
kern_return_t generic_kext_demo_stop(kmod_info_t *ki, void *d);
static const char* g_demoName = "macf";
static const char* g_demoPath = "/tmp/macf-demo";

// The extension has been loaded. Register your callbacks..
kern_return_t generic_kext_demo_start(kmod_info_t * ki, void *d)
{
    printf("(%s) Hello, World!\n", g_demoName);
    printf("%s: Point of interest: %s\n", g_demoName, g_demoPath);

    void *ctx;
    int status = mac_policy_register(&policyConf, &handlep, ctx);
    if (status != KERN_SUCCESS) {
        IOLog("%s: failed to register MAC policy (%#x)\n", g_demoName, status);
        return status;
    }

    return KERN_SUCCESS;
}

// Clean up allocated resources.
kern_return_t generic_kext_demo_stop(kmod_info_t *ki, void *d)
{
    printf("(%s) Goodbye, World!\n", g_demoName);
    return mac_policy_unregister(handlep); // Deregistering the policy
    return KERN_SUCCESS;
}
