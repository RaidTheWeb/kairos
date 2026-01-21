#! /bin/sh

# Build a bootable ISO image of Kairos.


# Remove old sysroot
rm -rf "sysroot"

../jinx update base # Create jinx packages.
../jinx install "sysroot" base # Install jinx packages to the sysroot folder (to be later put into ISO)

../jinx host-build limine

rm -rf isodir # Remove old ISO building directory.
mkdir isodir
mkdir isodir/boot # Boot directory (for bootloader section of ISO)

cp sysroot/usr/share/nomos/nomos isodir/

# Limine structure
mkdir isodir/boot/limine
mkdir -p isodir/EFI/BOOT

# Copy all second stage limine binaries.
cp host-pkgs/limine/usr/local/share/limine/limine-bios.sys isodir/boot/limine
cp host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin isodir/boot/limine

# UEFI
cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin isodir/boot/limine
cp host-pkgs/limine/usr/local/share/limine/BOOT*.EFI isodir/EFI/BOOT

cp ../build-support/limine.conf isodir/boot/limine/limine.conf

# Create initramfs tar, and copy it to ISO.
rm -rf initramfs.tar
cd "sysroot" && tar -cf initramfs.tar * && mv initramfs.tar .. && cd ..
cp initramfs.tar isodir/initramfs.tar

xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 \
    -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
    -efi-boot-part --efi-boot-image --protective-msdos-label "isodir" -o kairos.iso

host-pkgs/limine/usr/local/bin/limine bios-install kairos.iso

rm -rf isodir
echo "Built kairos.iso successfully."