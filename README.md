# BadAppleFreeBSDKernelModule

This project is to put BadApple into FreeBSDKernel.

The project ascii-art sources are from this [repo](https://github.com/trung-kieen/bad-apple-ascii).

## How to Use

```bash
sudo ./install.sh
kldload badapple_lkm.so
```

After the above operations, there will be a device ==/dev/badapple==, everyone can use read system call to read from the device with the parameter in param.h, or you can just use the userspace.c I provide as the following steps:

```bash
cc userspace.c -o main
sudo ./main
```

Then you can play BadApple

## How to Remove

```bash
sudo ./clear.sh
```
