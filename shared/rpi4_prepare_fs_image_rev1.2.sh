#!/usr/bin/env bash

BUILD_JOBS=1

while [ -n "$1" ]
do
case "$1" in
-h | --help)
echo 'rpi4_prepare_fs_image.sh [-h | --help] [-j {number of jobs}]'
echo 'This script prepares file system image for work with Raspberry Pi.'
echo -n 'It builds u-boot bootloader and place it with minimal config in '
echo 'file system image.'
echo '-h, --help             Show this message'
echo -n '-j {number of jobs}    Specifies the number of jobs (commands) to run simultaneously '
echo "(see man make for '-j' key)"
exit 0;;
-j) BUILD_JOBS="$2"
echo "jobs ${BUILD_JOBS}"
shift ;;
*) echo "'$1' is unknown option"
exit 1;;
esac
shift
done


sudo apt update

if [[ -n "$(git --version)" ]]
then
  read -p "The script will try to install the latest version of git. Do you accept this action? (y/n) " -n 1 -r
  echo
  if [[ $REPLY =~ ^[Yy]$ ]]
  then
    sudo apt install git -y
  else
    echo "Yours $(git --version)"
  fi
else
  sudo apt install git -y
fi

sudo apt install build-essential libssl-dev bison flex unzip parted gcc-aarch64-linux-gnu -y

build_temp_dir=$(mktemp -d)
echo "New temp directory for build was created: ${build_temp_dir}"

cd ${build_temp_dir}

DOWNLOAD_MAX_TRIES=10
DOWNLOAD_DELAY_TIME_S=1

echo "Trying to download raspberry file system image:"
wget https://downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2020-02-14/2020-02-13-raspbian-buster-lite.zip \
    --tries=${DOWNLOAD_MAX_TRIES} \
    --retry-connrefused \
    --retry-on-http-error=429,500,502,503,504

if [ $? -ne 0 ]
then
    echo "Error: can not download raspberry file system image"
    rm -rf ${build_temp_dir}
    exit 1
fi

unzip 2020-02-13-raspbian-buster-lite.zip
fs_image_name=$(find ${build_temp_dir} -iname *.img)

for (( i=1; i <= DOWNLOAD_MAX_TRIES; ++i ))
do
    echo "Trying to download u-boot source code (try $i):"
    git clone --depth 1 --single-branch --branch v2020.10 https://github.com/u-boot/u-boot.git u-boot-armv8

    if [ $? -eq 0 ]
    then
        break
    fi

    if [ $i -ne ${DOWNLOAD_MAX_TRIES} ]
    then
        sleep ${DOWNLOAD_DELAY_TIME_S}
    else
        echo "Error: can not download u-boot source code"
        rm -rf ${build_temp_dir}
        exit 1
    fi
done

cd u-boot-armv8 && \
make ARCH=arm CROSS_COMPILE=aarch64-linux-gnu- rpi_4_defconfig && \
sed -ie "s|CONFIG_BOOTCOMMAND=.*|CONFIG_BOOTCOMMAND=\"fatload mmc 0 \$\{loadaddr\} kos-image; bootelf \$\{loadaddr\}\"|g" .config && \
sed -ie "s| usb start;||g" .config && \
make ARCH=arm CROSS_COMPILE=aarch64-linux-gnu- u-boot.bin -j ${BUILD_JOBS}

if [ $? -ne 0 ]
then
    echo "Error: can not build u-boot"
    rm -rf ${build_temp_dir}
    exit 1
fi

mount_temp_dir=$(sudo mktemp -d -p /mnt)
echo "New temp directory for mount was created: ${mount_temp_dir}"

loop_device=$(sudo losetup --find --show --partscan ${fs_image_name})

if [ $? -ne 0 ]
then
    echo "Error: can not attach raspbian image as loop device"
    sudo losetup -d ${loop_device}
    sudo rm -rf ${build_temp_dir} ${mount_temp_dir}
    exit 1
fi

sudo parted ${loop_device} rm 2 && \
sudo parted ${loop_device} resizepart 1 1G && \
sudo parted ${loop_device} mkpart primary ext2 1000 1256M && \
sudo parted ${loop_device} mkpart primary ext3 1256 1512M && \
sudo parted ${loop_device} mkpart primary ext4 1512 1768M && \
sudo mkfs.ext2 ${loop_device}p2 && \
sudo mkfs.ext3 ${loop_device}p3 && \
sudo mkfs.ext4 -O ^64bit,^extent ${loop_device}p4

if [ $? -ne 0 ]
then
    echo "Error: can not edit raspbian image with parted and mkfs"
    sudo losetup -d ${loop_device}
    sudo rm -rf ${build_temp_dir} ${mount_temp_dir}
    exit 1
fi

sudo mount ${loop_device}p1 ${mount_temp_dir}

if [ $? -ne 0 ]
then
    echo "Error: can not mount boot partition"
    sudo losetup -d ${loop_device}
    sudo rm -rf ${build_temp_dir} ${mount_temp_dir}
    exit 1
fi

sudo cp ${build_temp_dir}/u-boot-armv8/u-boot.bin ${mount_temp_dir}/u-boot.bin
sudo touch ${mount_temp_dir}/config.txt
sudo sh -c "echo '[all]' > ${mount_temp_dir}/config.txt"
sudo sh -c "echo 'arm_64bit=1' >> ${mount_temp_dir}/config.txt"
sudo sh -c "echo 'enable_uart=1' >> ${mount_temp_dir}/config.txt"
sudo sh -c "echo 'kernel=u-boot.bin' >> ${mount_temp_dir}/config.txt"
sudo sh -c "echo 'dtparam=i2c_arm=on' >> ${mount_temp_dir}/config.txt"
sudo sh -c "echo 'dtparam=i2c=on' >> ${mount_temp_dir}/config.txt"
sudo sh -c "echo 'dtparam=spi=on' >> ${mount_temp_dir}/config.txt"

sync
sudo umount ${mount_temp_dir}

if [ $? -eq 0 ]
then
    sudo rm -rf ${mount_temp_dir}
    echo "${mount_temp_dir} was deleted"
    sudo losetup -d ${loop_device}
    echo "Loop device ${loop_device} was detached"
fi

rm -rf ${build_temp_dir}/*[^.img]

echo ""
echo "Raspberry file system image is ready: ${fs_image_name}"
echo "You can flash it on your SD-card, for example:"
echo -n "sudo dd bs=64k if=${fs_image_name} "
echo 'of=/dev/[name_of_SD-card_device] conv=fsync'
