#! /bin/sh

# Update the kernel in an existing Kairos disk image.

rm -rf "sysroot"

../jinx rebuild nomos || exit 1 # Rebuild the nomos kernel package. Exit on failure.
../jinx update base
../jinx install "sysroot" base

mkdir -p mountdir/boot

LOOP_DEV=$(sudo losetup --show -fP kairos.img)
sudo mount "${LOOP_DEV}p1" mountdir/boot || exit 1
sudo cp sysroot/usr/share/nomos/nomos mountdir/boot/nomos || exit 1
sudo cp ../build-support/limine.conf mountdir/boot/limine.conf || exit 1

sudo sync || exit 1
sudo umount mountdir/boot || exit 1
sudo rm -rf mountdir || exit 1
sudo losetup -d "${LOOP_DEV}" || exit 1

sync
echo "Updated kernel in kairos.img successfully."