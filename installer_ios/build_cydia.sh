#/bin/bash
set -e
mkdir -p Play/Applications/Play.app
rsync -a ../build/Source/ui_ios/Release-iphoneos/Play.app/* Play/Applications/Play.app
chmod -R 775 Play
sed "s/Version: .*/Version: $(git describe)/" Play/DEBIAN/control_template > Play/DEBIAN/control
dpkg-deb -Zgzip -b Play
dpkg-scanpackages -m . /dev/null > Packages
bzip2 Packages
