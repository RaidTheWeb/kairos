#! /bin/sh

# Build a full disk image of Kairos.


# Remove old sysroot.
rm -rf "sysroot"

../jinx update base || exit 1 # Create jinx packages.
../jinx install "sysroot" base $INITPKGS || exit 1 # Install jinx packages to the sysroot folder.

../jinx host-build limine || exit 1 # Build limine for host, to get the limine binaries needed for the image.

rm -rf mountdir

# Build initramfs tar.
rm -rf initramfs.tar
cd "sysroot" && tar -cf initramfs.tar * && mv initramfs.tar .. && cd ..
lz4 -9 --content-size initramfs.tar initramfs.tar.lz4

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
sudo cp -a sysroot/* mountdir/ || exit 1

sudo rm -rf mountdir/boot || exit 1
sudo mkdir -p mountdir/boot || exit 1
sudo mount "${LOOP_DEV}p1" mountdir/boot || exit 1

sudo cp sysroot/usr/share/nomos/nomos mountdir/boot/nomos || exit 1
sudo cp initramfs.tar.lz4 mountdir/boot/initramfs.tar.lz4 || exit 1
rm initramfs.tar.lz4

# Limine structure
sudo mkdir -p mountdir/boot/EFI/BOOT || exit 1
sudo cp host-pkgs/limine/usr/local/share/limine/limine-bios.sys mountdir/boot/limine-bios.sys || exit 1
sudo cp host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin mountdir/boot/limine-bios-cd.bin || exit 1
sudo cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mountdir/boot/limine-uefi-cd.bin || exit 1
sudo cp host-pkgs/limine/usr/local/share/limine/BOOT*.EFI mountdir/boot/EFI/BOOT || exit 1
sudo cp ../build-support/limine.conf mountdir/boot/limine.conf || exit 1

sudo sync || exit 1
sudo umount mountdir/boot || exit 1
sudo umount mountdir || exit 1
sudo rm -rf mountdir || exit 1
sudo losetup -d "${LOOP_DEV}" || exit 1

host-pkgs/limine/usr/local/bin/limine bios-install kairos.img

sync
echo "Built kairos.img successfully."
