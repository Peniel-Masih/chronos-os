# chronos-os
Header: 🛡️ Chronos OS: Ring 0 Ransomware-Immune Architecture  The Hook: A custom, bare-metal operating system built in C that physically neutralizes ransomware payloads at the CPU level using a custom Linux Kernel Module (LKM).
# 🛡️ Chronos OS: Ring 0 Ransomware-Immune Architecture

Chronos OS is a custom, bare-metal appliance operating system built in C. It is designed to physically neutralize ransomware payloads at the CPU level and force all data into an append-only, time-travel capable timeline. 

## ⚙️ Core Architecture
* **User Space (PID 1 Timeline):** Replaces standard `systemd` and standard shells. Acts as an immutable tracking engine that allows sub-second rewinds to previous system states, surviving hard reboots.
* **Kernel Space (Ring 0 Active Defense):** A custom Linux Kernel Module (LKM) utilizing `kprobes` to wiretap the `vfs_write` hardware command. It intercepts malicious write payloads and manipulates CPU registers (`regs->dx = 0`) to force a zero-byte write, paralyzing the ransomware.

## 🚀 How to Run the Prototype

**1. Boot the OS Environment (QEMU)**
\`\`\`bash
gcc -static -O2 init.c -o rootfs/init
cd rootfs && find . | cpio -o --format=newc | gzip > ../initramfs.cpio.gz
qemu-system-x86_64 -kernel ./vmlinuz -initrd ./initramfs.cpio.gz -hda chronos_disk.img -nographic -append "console=ttyS0 rdinit=/init quiet"
\`\`\`

**2. Inject the Active Defense Shield**
\`\`\`bash
cd kernel_module
make
sudo insmod chronos_guard.ko
\`\`\`
*(Note: Requires Secure Boot bypass or MOK signature enrollment).*

**3. Test the Hardware Interception**
Execute the provided `./virus_test` binary. The terminal will freeze as the Kernel Module traps the payload in an infinite hardware loop. Verify the interception via `sudo dmesg`.
