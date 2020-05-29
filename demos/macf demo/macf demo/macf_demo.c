#include <IOKit/IOLib.h>

extern "C" {
#include <Kernel/security/mac_policy.h>
}

#include "macf_demo.hpp"

// This required macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires.
OSDefineMetaClassAndStructors(com_jzlka_driver_MACF_demo, IOService)

// Define the driver's superclass.
#define super IOService


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



bool com_jzlka_driver_MACF_demo::init(OSDictionary *dict)
{
    IOLog("(%s)_init Hello, World!\n", m_demoName);
    if (!super::init(dict)) {
        IOLog("%s: super::init() failed.\n", m_demoName);
        return false;
    }

    IOLog("%s: Point of interest: %s\n", m_demoName, m_demoPath);


    void *ctx;
    int status = mac_policy_register(&policyConf, &handlep, ctx);
    if (status != KERN_SUCCESS) {
        IOLog("%s: failed to register MAC policy (%#x)\n", m_demoName, status);
        return false;
    }

    return true;
}

void com_jzlka_driver_MACF_demo::free(void)
{
    IOLog("(%s)_free Goodbye, World!\n", m_demoName);
    super::free();
    return mac_policy_unregister(handlep); // Deregistering the policy
}

IOService *com_jzlka_driver_MACF_demo::probe(IOService *provider,
    SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("%s: Probing\n", m_demoName);
    return result;
}

bool com_jzlka_driver_MACF_demo::start(IOService *provider)
{
    IOLog("%s: Starting\n", m_demoName);
    if (!super::start(provider)) {
        IOLog("%s: super::start() failed.\n", m_demoName);
        return false;
    }
    return true;
}

void com_jzlka_driver_MACF_demo::stop(IOService *provider)
{
    IOLog("%s: Stopping", m_demoName);
    super::stop(provider);
}

#include <sys/systm.h>
#include <mach/mach_types.h>

kern_return_t generic_kext_demo_start(kmod_info_t * ki, void *d);
kern_return_t generic_kext_demo_stop(kmod_info_t *ki, void *d);
const char* g_demoName = "generic-kext";

// The extension has been loaded. Register your callbacks..
kern_return_t generic_kext_demo_start(kmod_info_t * ki, void *d)
{
    printf("(%s) Hello, World!\n", g_demoName);
    return KERN_SUCCESS;
}

// Clean up allocated resources.
kern_return_t generic_kext_demo_stop(kmod_info_t *ki, void *d)
{
    printf("(%s) Goodbye, World!\n", g_demoName);
    return KERN_SUCCESS;
}
