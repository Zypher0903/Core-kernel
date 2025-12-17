#!/bin/bash
# Core Kernel Cross-Compiler Build Script

set -e

export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

BINUTILS_VERSION=2.41
GCC_VERSION=13.2.0

echo "Core Kernel Cross-Compiler Build Script"
echo "========================================"
echo ""
echo "Target: $TARGET"
echo "Install prefix: $PREFIX"
echo ""

mkdir -p $PREFIX
mkdir -p build-binutils
mkdir -p build-gcc

echo "Downloading sources..."

if [ ! -f "binutils-$BINUTILS_VERSION.tar.xz" ]; then
    wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz
fi

if [ ! -f "gcc-$GCC_VERSION.tar.xz" ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz
fi

echo "Extracting sources..."
tar -xf binutils-$BINUTILS_VERSION.tar.xz
tar -xf gcc-$GCC_VERSION.tar.xz

echo "Building Binutils..."
cd build-binutils
../binutils-$BINUTILS_VERSION/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror

make -j$(nproc)
make install
cd ..

echo "Building GCC..."
cd build-gcc
../gcc-$GCC_VERSION/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers

make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..

echo ""
echo "Cross-compiler build complete!"
echo ""
echo "Add to your PATH:"
echo "  export PATH=\"$PREFIX/bin:\$PATH\""