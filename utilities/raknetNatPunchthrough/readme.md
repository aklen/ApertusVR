# Debugging "Protocol Not Available (errno: 92)" in RakNet NAT Punchthrough on Raspberry Pi

When setting up a NAT Punchthrough server using RakNet on a Raspberry Pi, you may encounter the error:

```
Command failed with error: Protocol not available (errno: 92)
```

This guide walks you through all necessary checks and configurations to resolve this issue.

## 1️⃣ Checking and Configuring Firewall & Networking Rules

First, ensure that your system allows the required network traffic.

### 📌 **Flush iptables Rules (For Debugging)**
If you are experiencing connectivity issues, temporarily flushing iptables rules can help:

```sh
sudo iptables -F
sudo iptables -X
sudo iptables -t nat -F
sudo iptables -t nat -X
```

To save and restore iptables rules:
```sh
sudo iptables-save > ~/iptables_backup
```

To restore later:
```sh
sudo iptables-restore < ~/iptables_backup
```

### 📌 **Enable Port Forwarding (if required)**
Ensure that your Raspberry Pi is allowed to forward packets.

```sh
sudo sysctl -w net.ipv4.ip_forward=1
```

To make it permanent, add this line to `/etc/sysctl.conf`:
```
net.ipv4.ip_forward=1
```

### 📌 **Check ufw (Uncomplicated Firewall) Status**
If you're using `ufw`, make sure the required ports are open:

```sh
sudo ufw status
```

To allow UDP traffic on the NAT Punchthrough port (e.g., 61666):

```sh
sudo ufw allow 61666/udp
```

Restart the firewall:
```sh
sudo ufw reload
```

---

## 2️⃣ Checking Kernel Support & Modules

If `errno: 92` persists, the issue is likely a missing kernel feature.

### 📌 **Check Available Protocols**
Run the following to see which networking protocols are available:

```sh
cat /proc/net/protocols
```

If `PACKET` is missing, the kernel lacks support for raw sockets.

### 📌 **Check Kernel Configuration for AF_PACKET Support**
Try checking if `CONFIG_PACKET` is enabled:

```sh
grep CONFIG_PACKET /boot/config-$(uname -r)
```

If the above fails, try:

```sh
zgrep CONFIG_PACKET /proc/config.gz
```

If no results appear, it means the kernel lacks **AF_PACKET support**, which is required for low-level networking.

### 📌 **Check Kernel Modules**
Ensure that required modules are loaded:

```sh
lsmod | grep packet
```

If empty, manually load the module:

```sh
sudo modprobe af_packet
lsmod | grep packet
```

If the module is missing entirely, your kernel might not have compiled support for it.

### 📌 **Check if Raw Sockets Work**
Test raw socket creation in Python:

```python
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_UDP)
print("RAW socket created successfully")
```

Run with:
```sh
sudo python3 test.py
```

If successful, but `IP_HDRINCL` fails:

```python
s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
```

then the kernel lacks **IP_HDRINCL support**.

### 📌 **Enable Raw Sockets and Packet Filtering**
Try enabling raw sockets manually:

```sh
sudo sysctl -w net.core.busy_poll=50
sudo sysctl -w net.core.dev_weight=64
```

Make it permanent by adding to `/etc/sysctl.conf`:
```
net.core.busy_poll=50
net.core.dev_weight=64
```

---

## 3️⃣ Recompiling the Kernel (If Necessary)

If none of the above steps work, recompiling the kernel with the necessary features is the last resort.

1. Install necessary tools:
```sh
sudo apt install raspberrypi-kernel-headers build-essential bc bison flex libssl-dev
```
2. Clone the Raspberry Pi kernel source:
```sh
git clone --depth=1 https://github.com/raspberrypi/linux.git ~/rpi-linux
cd ~/rpi-linux
```
3. Configure the kernel:
```sh
make menuconfig
```
Enable:
- `CONFIG_PACKET=y`
- `CONFIG_NET_PKTGEN=y`

4. Compile and install:
```sh
make -j$(nproc) Image modules dtbs
sudo make modules_install
sudo make install
```

5. Reboot and test again.

```sh
sudo reboot
```

---

## 🏁 **Final Steps & Testing**

1️⃣ **Restart the NAT Punchthrough Server**
```sh
./apeRaknetNatPunchthroughServer
```

2️⃣ **Check for error logs**
If `errno: 92` is gone, the issue was kernel-related!

3️⃣ **Re-enable firewalls if disabled earlier**
```sh
sudo ufw enable
```

---

## 📌 **Conclusion**
If you still see `Protocol not available (errno: 92)`, the likely causes are:
- Missing `CONFIG_PACKET` support in the kernel.
- Misconfigured firewall rules.
- Unavailable `af_packet` kernel module.
- Raw socket restrictions in the OS.

This guide covers all known workarounds and debugging steps. If the issue persists, consider upgrading your OS or using a different Linux distribution with better networking support.

🚀 **Good luck debugging!**
