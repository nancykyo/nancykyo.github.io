#!/bin/bash
group=$1
disk_dest=$2
disk_src=$3

#mapping, before/after, disk=sda;group=cinder-volumes-ssd

parted /dev/$disk_dest mklabel gpt
parted -a optimal /dev/$disk_dest mkpart primary 0% 100%

# not supported for two or more outage volume
uuid=`(vgs 1> /dev/null) 2>&1|awk '{print $5}'`
source=`grep $uuid /etc/lvm/archive|tail -n1|awk '{print $1}'|awk -F":" '{print $1}'`
pvcreate --uuid  $uuid --restorefile /etc/lvm/archive/$source /dev/$disk_dest'1'

vgcfgrestore $group
vgchange -ay

# the sample command is
: '
fdisk /dev/sdx
parted -a optimal /dev/sdf mkpart primary 0% 100%

pvcreate --uuid  YSSSiM-qXb1-lNq5-IGEn-T749-4Njt-c4AL58 --restorefile cinder-volumes-ssd_00004-1767345423.vg /dev/sdd1
vgcfgrestore cinder-volumes-ssd 
vgchange -ay
'