# OpenBMC

The OpenBMC project can be described as a Linux distribution for embedded
devices that have a BMC; typically, but not limited to, things like servers,
top of rack switches or RAID appliances. The OpenBMC stack uses technologies
such as [Yocto](https://www.yoctoproject.org/),
[OpenEmbedded](https://www.openembedded.org/wiki/Main_Page),
[systemd](https://www.freedesktop.org/wiki/Software/systemd/), and
[D-Bus](https://www.freedesktop.org/wiki/Software/dbus/) to allow easy
customization for your server platform.


## Setting up your OpenBMC project

### 1) Prerequisite
- Ubuntu 20.04.6 LTS

```
sudo apt-get install -y git build-essential libsdl1.2-dev texinfo gawk chrpath diffstat
```
### 2) Download the source
```
git clone https://github.com/MCT-BMC/S8040_OpenBMC.git
cd S8040_OpenBMC
```

### 3) Target your hardware
Any build requires an environment variable known as `TEMPLATECONF` to be set
to a hardware target.
You can see all of the known targets with
`find meta-* -name local.conf.sample` in `meta-mct`. Choose the hardware target and
then move to the next step.

As an example target S8040
```
. setup s8040
```

### 4) Build

```
bitbake obmc-phosphor-image
```

Additional details can be found in the [docs](https://github.com/openbmc/docs)
repository.

## OpenBMC Development

The OpenBMC community maintains a set of tutorials new users can go through
to get up to speed on OpenBMC development out
[here](https://github.com/openbmc/docs/blob/master/development/README.md)

After building OpenBMC, you will end up with a set of image files in 
`tmp/deploy/images/<platform>/`.

After the build process finishes, you can find the BMC firmware image
* `image-bmc` → `obmc-phosphor-image-<platform>-<timestamp>.static.mtd`
  The complete flash image for the BMC

* `image-kernel` → `fitImage-obmc-phosphor-initramfs-<platform>.bin`
  The OpenBMC kernel FIT image (including the kernel, device tree, and initramfs)

* `image-rofs` → `obmc-phosphor-image-<platform>.squashfs-xz`
  The read-only OpenBMC filesystem

* `image-rwfs` → `rwfs.jffs2`
  The read-write filesystem for persistent changes to the OpenBMC filesystem

* `image-u-boot` → `u-boot.bin`
  The OpenBMC bootloader

Additionally, there are two tarballs created that can be deployed and unpacked by REST:

* `<platform>-<timestamp>.all.tar`
  The complete BMC flash content: A single file (image-bmc) wrapped in a tar archive.

* `<platform>-<timestamp>.tar`
  Partitioned BMC flash content: Multiple files wrapped in a tar archive, one for each of the u-boot, kernel, ro and rw partitions.


