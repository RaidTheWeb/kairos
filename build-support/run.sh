#! /bin/sh

if [ -z "$ARCH" ]; then
    ARCH=x86_64
fi

INT_FLAGS="-m 2G"
INT_FLAGS=${INT_FLAGS} ${QEMU_FLAGS}

if ! [ -z "$KVM" ]; then
    qemu_flags="${INT_FLAGS} -enable-kvm -cpu host"
fi

qemu-system-${ARCH} ${INT_FLAGS} -drive file=kairos.iso
