#!/bin/bash

###---------------------------------------------------
# 0. change to target directory and cleanup environment
# 1. create directory structure for rpm build process
# 2. archive software repo & start build process
###---------------------------------------------------

basedir=$(dirname "$0")
topdir=rpmbuild
sourcepath="${topdir}/SOURCES"
tarball=${sourcepath}/libjapi.tar.gz

cd ${basedir}
rm -rf ${topdir}
mkdir -p ${topdir}/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

tar -czf ${tarball} -C ../../ --exclude=build --exclude=package/rpmbuild libjapi
rpmbuild --define "_topdir $(pwd)/${topdir}" -ba libjapi.spec
