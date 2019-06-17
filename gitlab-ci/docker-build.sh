#!/bin/sh

set -e

# create a temporary directory for the build
BUILDDIR=$(mktemp -d)

cp Dockerfile $BUILDDIR/

echo "Setting up docker build context in $BUILDDIR ..."

cd $BUILDDIR

docker build -t libjapi:latest .

# clean up
cd /

echo "Build completed successfully. Removing $BUILDDIR ..."
rm -rf "$BUILDDIR"

