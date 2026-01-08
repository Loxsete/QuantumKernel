# Network Setup Guide for QuantumKernel

This guide explains how to bring up networking (basic internet / LAN connectivity) in **QuantumKernel** using a TAP interface and QEMU.

---

## 1. Create and Configure TAP Interface (Host Side)

First, create a TAP interface on the host system. This interface will be used by QEMU to connect the virtual machine to the host network.

```sh
sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip addr add 10.0.3.1/24 dev tap0
sudo ip link set tap0 up
```

## 2. Launch QuantumKernel in QEMU with Networking Enabled

Start QEMU with the RTL8139 network device attached to the TAP interface.
```sh
qemu-system-x86_64 \
    -kernel build/kernel.bin \
    -hda build/disk.img \
    -m 128M \
    -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
   -device rtl8139,netdev=net0
```


## 3. In shell os
``` 
arp add 167772673 82 84 0 18 52 86
arp show
ping 167772673
``` 
Now ping is not working :)
