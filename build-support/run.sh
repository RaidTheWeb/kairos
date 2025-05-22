#! /bin/sh

if [ -z "$ARCH" ]; then
    ARCH=x86_64
fi

INT_FLAGS="-m 2G"
INT_FLAGS=${INT_FLAGS} ${QEMU_FLAGS}

if ! [ -z "$KVM" ]; then
    echo "Running with KVM..."
    qemu_flags="${INT_FLAGS} -enable-kvm -cpu host -smp 4"
fi

kitty -e gdb sysroot/usr/share/nomos/nomos -ex 'target remote :1234' -x 'build-support/gdbinit' &

qemu-system-${ARCH} ${qemu_flags} -drive file=kairos.iso -s -S
