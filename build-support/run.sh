#! /bin/sh

if [ -z "$ARCH" ]; then
    ARCH=x86_64
fi

INT_FLAGS="-m 2G"
INT_FLAGS=${INT_FLAGS} ${QEMU_FLAGS}
qemu_flags=${INT_FLAGS}

if ! [ -z "$KVM" ]; then
    echo "Running with KVM..."
    qemu_flags="${qemu_flags} ${INT_FLAGS} -enable-kvm -cpu host -smp 4"
fi

if ! [ -z "$DEBUG" ]; then
    echo "Debugging..."
    qemu_flags="${qemu_flags} -s -S"

    kitty -e gdb sysroot/usr/share/nomos/nomos -ex 'target remote :1234' -x 'build-support/gdbinit' &
fi


qemu-system-${ARCH} -M q35 -cpu max ${qemu_flags} -drive file=kairos.iso -drive file=nvm.img,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm -no-reboot -debugcon stdio
