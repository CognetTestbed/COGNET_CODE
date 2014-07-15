#!/usr/sh
make ARCH=i386 -j3
cp ./arch/i386/boot/bzImage $HOME/EXPORT_ALIX/vmlinuz-3.8.11-voyage-Matteo3
make ARCH=i386 modules_install INSTALL_MOD_PATH=$HOME/voyage-0.9.1-COGNET/
cd $HOME/voyage-0.9.1-COGNET/
tar -cf $HOME/EXPORT_ALIX/3.8.11.tar ./lib/modules/3.8.11
sudo chroot ./ /bin/bash -c "mkinitramfs -v -o initrd.img-3.8.11-voyage-Matteo3 3.8.11"
cp initrd.img-3.8.11-voyage-Matteo $HOME/EXPORT_ALIX/
