#! /bin/sh

# Build a full disk image of Kairos.


# Remove old sysroot.
rm -rf "sysroot"

../jinx update base # Create jinx packages.
../jinx install "sysroot" base $INITPKGS # Install jinx packages to the sysroot folder.

../jinx host-build limine

rm -rf mountdir

# Build initramfs tar.
rm -rf initramfs.tar
cd "sysroot" && tar -cf initramfs.tar * && mv initramfs.tar .. && cd ..

if [ -z "$IMAGE_SIZE" ]; then
    IMAGE_SIZE=2048M
fi

rm -rf kairos.img
fallocate -l ${IMAGE_SIZE} kairos.img

parted -s kairos.img mklabel gpt
parted -s kairos.img mkpart ESP fat32 2048s 256MiB
parted -s kairos.img set 1 esp on
parted -s kairos.img mkpart bios_boot 256MiB 257MiB
parted -s kairos.img set 2 bios_grub on
parted -s kairos.img mkpart kairos ext4 257MiB 100%

LOOP_DEV=$(sudo losetup --show -fP kairos.img)
sudo mkfs.fat -F32 "${LOOP_DEV}p1"
sudo mkfs.ext4 "${LOOP_DEV}p3"

mkdir -p mountdir
sudo mount "${LOOP_DEV}p3" mountdir


rm -rf "sysroot"
../jinx update base $DISTPKGS # Create jinx packages.
../jinx install "sysroot" base $DISTPKGS # Install jinx packages to the sysroot folder

# Copy new sysroot contents to the image.
sudo cp -a sysroot/* mountdir/

sudo rm -rf mountdir/boot
sudo mkdir -p mountdir/boot
sudo mount "${LOOP_DEV}p1" mountdir/boot

sudo cp sysroot/usr/share/nomos/nomos mountdir/boot/nomos
sudo cp initramfs.tar mountdir/boot/initramfs.tar

# Limine structure
sudo mkdir -p mountdir/boot/EFI/BOOT
sudo cp host-pkgs/limine/usr/local/share/limine/limine-bios.sys mountdir/boot/limine-bios.sys
sudo cp host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin mountdir/boot/limine-bios-cd.bin
sudo cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mountdir/boot/limine-uefi-cd.bin
sudo cp host-pkgs/limine/usr/local/share/limine/BOOT*.EFI mountdir/boot/EFI/BOOT
sudo cp ../build-support/limine.conf mountdir/boot/limine.conf

sync
sudo umount mountdir/boot
sudo umount mountdir
sudo rm -rf mountdir
sudo losetup -d "${LOOP_DEV}"

host-pkgs/limine/usr/local/bin/limine bios-install kairos.img

sync
echo "Built kairos.img successfully."