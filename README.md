mkeykernel
=======

This is a kernel that can read the characters a-z and 0-9 from the keyboard and print them on screen.

See the repo [mkernel](http://github.com/arjun024/mkernel) which is a minimal kernel that prints a string on the screen. mkeykernel just extends this to include keyboard support. 


####Blog post####

Kernel 201 - Let’s write a Kernel with keyboard and screen support

(http://arjunsreedharan.org/post/99370248137/kernel-201-lets-write-a-kernel-with-keyboard-and)

####Build commands####
```
make
```

####Test on emulator####
```
qemu-system-i386 -kernel kernel
```

####Get to boot####
GRUB requires your kernel executable to be of the pattern `kernel-<version>`.

So, rename the kernel:

```
mv kernel kernel-701
```

Copy, it to your boot partition (assuming you are superuser):

```
cp kernel-701 /boot/kernel-701
```

Configure your grub/grub2 similar to what is given in `_grub_grub2_config` folder of [mkernel repo](http://github.com/arjun024/mkernel).

Reboot.

Voila !!

![kernel screenshot](http://31.media.tumblr.com/1afd75b433b13df613fa0c2301977893/tumblr_inline_ncy1p0kSGj1rivrqc.png "Screenshot")
