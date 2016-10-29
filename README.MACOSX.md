# ZOS Environment Setup for Mac OS X

*This has only been tried on Mac OS X Yosemite 10.10.4 , other versions of the system may need some tweaking or may not work.*

The toolchain versions used were: 

 - **binutils-2.27**
 - **gcc-6.2.0**

### Recomended build arguments:

 1. `i386-elf-g++ -c test.cpp -ffreestanding -O2 -Wall -Wextra
   -fno-exceptions -fno-rtti i386-elf-ld -o test -Ttext 0x0 -e main test.o`   
 2. `i386-elf-objcopy -R .note -R .comment -R .eh_frame -S -O binary
    test test.bin`

### Required tools to build ZapperOS:
- **i386-elf toolchain**
- **NASM**

### Required tools to debug ZapperOS:
- **BOCHS**(recommended)/QEMU

###Required tools to build i386-elf toolchain:
- **Command Line Tools OS X 10.10 for Xcode 7.2** *installed*
- **binutils-2.27** *source*
- **gcc-6.2.0** *source*

### Simply execute this script to build the toolchain:

    export PREFIX=/usr/local/cross
    export TARGET=i386-elf
    mkdir -p ~/src/build-{binutils,gcc}/
    curl ftp://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.bz2 | tar -jx -f - -C ~/src/
    curl ftp://ftp.gnu.org/gnu/gcc/gcc-6.2.0/gcc-6.2.0.tar.bz2 | tar -jx -f - -C ~/src/
    cd ~/src/build-binutils/
    ../binutils-2.22/configure --prefix=$PREFIX --target=$TARGET --disable-multilib --disable-nls
    make all
    sudo make install
    export PATH=$PATH:$PREFIX/bin
    echo 'export PATH=$PATH:'"$PREFIX"'/bin' >> ~/.bash_profile
    cd ~/src/build-gcc/
    ../gcc-6.2.0/configure --prefix=$PREFIX --target=$TARGET --enable-languages=c,c++ --without-headers --disable-multilib --disable-nls
    make all-gcc
    sudo make install-gcc
