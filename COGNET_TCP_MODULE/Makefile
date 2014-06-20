obj-m := cognetTCP.o


CROSS_COMPILE_DIR_SAMSUNG=/locale/ANDROID/SAMSUNG/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-
CROSS_COMPILE_DIR_NEXUS=/locale/ANDROID/NEXUS7_NEWWIFI/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-
KERNEL_DIR_SAMSUNG=/locale/ANDROID/SAMSUNG/out/target/product/p6210/obj/KERNEL_OBJ
KERNEL_DIR_NEXUS=/locale/ANDROID/NEXUS7_NEWWIFI/out/target/product/grouper/obj/KERNEL_OBJ

CROSS_COMPILE_DIR_NDK=/locale/ANDROID/android-ndk-r8e/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86_64/bin/arm-linux-androideabi-

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build
KDIRALIX := /home/danielet/voyage-0.9.1-COGNET/lib/modules/3.8.11/build
KDIRALIX_NEW := /home/danielet/voyage-0.9.2/lib/modules/3.10.11/build


nexus:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE_DIR_NEXUS)  -C $(KERNEL_DIR_NEXUS) SUBDIRS=$(PWD) CFLAGS_MODULE=-fno-pic  modules

pc:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

alix:
	$(MAKE) -C $(KDIRALIX) M=$(PWD) modules

alix_new:
	$(MAKE) -C $(KDIRALIX_NEW) M=$(PWD) modules

nexus2:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE_DIR_NDK) -C $(KERNEL_DIR_NEXUS) SUBDIRS=$(PWD) modules 

clean:
	rm -f *.o
