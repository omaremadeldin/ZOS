# ZOS Environment Setup for Windows (Windows 10 Linux Subsystem)

*This has only been tried on Windows 10 Ubuntu Bash Shell, other versions may need some tweaking or may not work.*

The toolchain versions used were: 
- **binutils-2.22**
- **gcc-5.0.2**

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
- **gcc-4.8**			*installed*
- **gcc-core-4.6.3** 	*source*
- **gcc-g++-4.6.3**		*source*
- **binutils-2.2** 		*source*
- **gmp-5.0.2** 		*source*
- **mpc-0.9**			*source*
- **mpfr-3.1.0**		*source*

### Simply execute this script to build the toolchain:
	export CC=/usr/bin/gcc-4.8
	export CXX=/usr/bin/g++-4.8
	export CPP=/usr/bin/cpp-4.8
	export LD=/usr/bin/gcc-4.8
	export PREFIX=/usr/cross
	export TARGET=i386-elf
	mkdir -p ~/src/build-{binutils,gcc}/
	curl ftp://ftp.gnu.org/gnu/binutils/binutils-2.22.tar.bz2 | tar -jx -f - -C ~/src/
	curl ftp://ftp.gnu.org/gnu/gcc/gcc-4.6.3/gcc-core-4.6.3.tar.bz2 | tar -jx -f - -C ~/src/
	curl ftp://ftp.gnu.org/gnu/gcc/gcc-4.6.3/gcc-g++-4.6.3.tar.bz2 | tar -jx -f - -C ~/src/
	curl ftp://ftp.gnu.org/gnu/gmp/gmp-5.0.2.tar.bz2 | tar -jx -f - -C ~/src/gcc-4.6.3/
	curl ftp://ftp.gnu.org/gnu/mpfr/mpfr-3.1.0.tar.bz2 | tar -jx -f - -C ~/src/gcc-4.6.3/
	curl http://www.multiprecision.org/downloads/mpc-0.9.tar.gz | tar -zx -f - -C ~/src/gcc-4.6.3/
	cd ~/src/build-binutils/
	../binutils-2.22/configure --prefix=$PREFIX --target=$TARGET --disable-nls
	make all
	sudo make install
	export PATH=$PATH:$PREFIX/bin
	echo 'export PATH=$PATH:'"$PREFIX"'/bin' >> ~/.bash_profile
	cd ~/src/build-gcc/
	mv ../gcc-4.6.3/gmp-5.0.2/ ../gcc-4.6.3/gmp/
	mv ../gcc-4.6.3/mpfr-3.1.0/ ../gcc-4.6.3/mpfr/
	mv ../gcc-4.6.3/mpc-0.9/ ../gcc-4.6.3/mpc/
	../gcc-4.6.3/configure --prefix=$PREFIX --target=$TARGET --disable-nls --without-headers --with-mpfr-include=$HOME/src/gcc-4.6.3/mpfr/src/ --with-mpfr-lib=$HOME/src/build-gcc/mpfr/src/.libs/ --with-languages=c,c++
	make all-gcc
	sudo make install-gcc
