# ZOS Environment Setup for Cygwin (Windows)

*This has only been tried on Cygwin 32 on Windows 7 Ultimate 64-Bit, other versions of the system may need some tweaking or may not work.*

The toolchain versions used were: 
- **binutils-2.27**
- **gcc-6.2.0**

### Recomended build arguments:
1. `i386-elf-g++ -c test.cpp -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti`
2. `i386-elf-ld -o test -Ttext 0x0 -e main test.o`   
3. `i386-elf-objcopy -R .note -R .comment -R .eh_frame -S -O binary test test.bin`

### Required tools to build ZOS:
- **i386-elf toolchain**
- **NASM**

### Required tools to debug ZOS:
- **BOCHS**(recommended)/QEMU

### Required tools to build i386-elf toolchain:
- **gcc-5.4.0** *installed*
- **gcc-g++-5.4.0** *installed*
- **binutils-2.27** *source*
- **gcc-6.2.0** *source*
- **gmp-5.0.2** *source*
- **mpc-0.9** *source*
- **mpfr-3.1.0** *source*

### Simply execute this script to build the toolchain:
    export PREFIX=/usr/local/cross
    export TARGET=i386-elf
    mkdir -p ~/src/build-{binutils,gcc}/
    curl ftp://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.bz2 | tar -jx -f - -C ~/src/
    curl ftp://ftp.gnu.org/gnu/gcc/gcc-6.2.0/gcc-6.2.0.tar.bz2 | tar -jx -f - -C ~/src/
	curl ftp://ftp.gnu.org/gnu/gmp/gmp-5.0.2.tar.bz2 | tar -jx -f - -C ~/src/gcc-6.2.0/
	curl ftp://ftp.gnu.org/gnu/mpfr/mpfr-3.1.0.tar.bz2 | tar -jx -f - -C ~/src/gcc-6.2.0/
	curl http://www.multiprecision.org/mpc/download/mpc-0.9.tar.gz | tar -zx -f - -C ~/src/gcc-6.2.0/
    cd ~/src/build-binutils/
    ../binutils-2.27/configure --prefix=$PREFIX --target=$TARGET --disable-multilib --disable-nls
    make all
    make install
    export PATH=$PATH:$PREFIX/bin
    echo 'export PATH=$PATH:'"$PREFIX"'/bin' >> ~/.bash_profile
    cd ~/src/build-gcc/
	mv ../gcc-6.2.0/gmp-5.0.2/ ../gcc-6.2.0/gmp/
	mv ../gcc-6.2.0/mpfr-3.1.0/ ../gcc-6.2.0/mpfr/
	mv ../gcc-6.2.0/mpc-0.9/ ../gcc-6.2.0/mpc/
    ../gcc-6.2.0/configure --prefix=$PREFIX --target=$TARGET --enable-languages=c,c++ --without-headers --disable-multilib --disable-nls
    make all-gcc
    make install-gcc
