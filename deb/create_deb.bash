#!/bin/bash
SCRIPT_DIR=$(cd $(dirname $0); pwd)
COPY_TARGET_BIN=FT_SCServo_Debug_Qt
# amd64 or arm64
ARCH=${1:-amd64}
MOUNT_TARGET=./build
DEB_ROOT=./deb_root
VERSION=${2:-0.0.1}
DEB_NAME=ft-scservo-debug-qt_${VERSION}_${ARCH}

if [ ${ARCH} == "arm64" ]; then
    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
fi

# select arch
docker build -t deb_build -f ${SCRIPT_DIR}/dockerfile.${ARCH} ${SCRIPT_DIR}
docker run -it --rm -v ${MOUNT_TARGET}:/build deb_build /build/build.bash

mkdir -p ${DEB_ROOT}/usr/bin ${DEB_ROOT}/DEBIAN/
cp ${MOUNT_TARGET}/FT_SCServo_Debug_Qt/FT_SCServo_Debug_Qt ${DEB_ROOT}/usr/bin/

echo "Package: ft-scservo-debug-qt" > ${DEB_ROOT}/DEBIAN/control
echo "Version: $VERSION" >> ${DEB_ROOT}/DEBIAN/control
echo "Section: base" >> ${DEB_ROOT}/DEBIAN/control
echo "Priority: optional" >> ${DEB_ROOT}/DEBIAN/control
echo "Architecture: $ARCH" >> ${DEB_ROOT}/DEBIAN/control
echo "Depends: libqt5serialport5-dev, qtbase5-dev" >> ${DEB_ROOT}/DEBIAN/control
echo "Maintainer: Kotakku <Kotakkucu@gmail.com>" >> ${DEB_ROOT}/DEBIAN/control
echo "Description: FeeTech Servo Debug Qt" >> ${DEB_ROOT}/DEBIAN/control

dpkg-deb --build -Z xz --root-owner-group ${DEB_ROOT} ${SCRIPT_DIR}/${DEB_NAME}.deb

sudo rm -rf build/FT_SCServo_Debug_Qt deb_root/
echo "Create deb package done!"
