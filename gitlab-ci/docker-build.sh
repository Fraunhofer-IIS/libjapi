#!/bin/sh

set -e

# create a temporary directory for the build
BUILDDIR=$(mktemp -d)

cp Dockerfile $BUILDDIR/

echo "Setting up docker build context in $BUILDDIR ..."

cd $BUILDDIR

# clone build infrastructure repositories
git clone --recurse-submodules -b testing --single-branch git@git01.iis.fhg.de:ks-ip-lib/software/libjapi.git

docker build -t libjapi:latest .

# clean up
cd /

echo "Build completed successfully. Removing $BUILDDIR ..."
rm -rf "$BUILDDIR"

