#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

static struct kprobe kp = {
    .symbol_name = "vfs_write",
};

// This fires the exact microsecond before the hard drive writes the data
static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
    if (strcmp(current->comm, "virus_test") == 0) {
        printk(KERN_ALERT "[CHRONOS GUARD] ACTIVE DEFENSE: Neutralizing payload from %s!\n", current->comm);
        
        // SECURITY OVERRIDE: Modify the CPU register holding the data length.
        // By forcing it to 0, the hard drive writes absolutely nothing.
        regs->dx = 0;
    }
    return 0;
}

static int __init chronos_guard_init(void) {
    kp.pre_handler = handler_pre;
    if (register_kprobe(&kp) < 0) {
        printk(KERN_ERR "[CHRONOS] Failed to arm Kprobe.\n");
        return -1;
    }
    printk(KERN_INFO "[CHRONOS] Guard Active. Hardware Write Prevention Online.\n");
    return 0;
}

static void __exit chronos_guard_exit(void) {
    unregister_kprobe(&kp);
    printk(KERN_INFO "[CHRONOS] Guard Offline.\n");
}

module_init(chronos_guard_init);
module_exit(chronos_guard_exit);
