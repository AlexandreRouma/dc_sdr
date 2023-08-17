#!/bin/sh
# sudo mount /dev/sdb1 /mnt && sudo cp build/Computer.uf2 /mnt && sudo umount /mnt
sudo picotool reboot -f -u
sleep 1
sudo picotool load -f -x firmware.uf2
