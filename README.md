===ZapperOS=By=Omar Emad Eldin===

*Preferred build arguments:
	i386-elf-gcc -c test.c (or) i386-elf-g++ -c test.cpp -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
	i386-elf-ld -o test -Ttext 0x0 -e main test.o
	i386-elf-objcopy -R .note -R .comment -R .eh_frame -S -O binary test test.bin

*Required tools to build ZapperOS:
	- i386-elf toolchain
	- NASM

*Required tools to debug ZapperOS:
	- BOCHS(recommended)/QEMU

*Optional tools:
	- cloc (for counting code lines & comment lines)

*Required tools to build i386-elf toolchain:
	- gcc-4.8			installed
	- gcc-core-4.6.3 	source
	- gcc-g++-4.6.3		source
	- binutils-2.2 		source
	- gmp-5.0.2 		source
	- mpc-0.9 			source
	- mpfr-3.1.0 		source

*Simply execute this script to build the toolchain:

	export CC=/usr/local/bin/gcc-4.8
 	export CXX=/usr/local/bin/g++-4.8
	export CPP=/usr/local/bin/cpp-4.8
	export LD=/usr/local/bin/gcc-4.8
	export PREFIX=/usr/local/cross
	export TARGET=i386-elf
	export MAKEFLAGS="-j 16"
	mkdir -p ~/src/build-{binutils,gcc}/
	curl ftp://ftp.gnu.org/gnu/binutils/binutils-2.22.tar.bz2 | tar -x -f - -C ~/src/
	curl ftp://ftp.gnu.org/gnu/gcc/gcc-4.6.3/gcc-core-4.6.3.tar.bz2 | tar -x -f - -C ~/src/
	curl ftp://ftp.gnu.org/gnu/gcc/gcc-4.6.3/gcc-g++-4.6.3.tar.bz2 | tar -x -f - -C ~/src/
	curl ftp://ftp.gnu.org/gnu/gmp/gmp-5.0.2.tar.bz2 | tar -x -f - -C ~/src/gcc-4.6.3/
	curl ftp://ftp.gnu.org/gnu/mpfr/mpfr-3.1.0.tar.bz2 | tar -x -f - -C ~/src/gcc-4.6.3/
	curl http://www.multiprecision.org/mpc/download/mpc-0.9.tar.gz | tar -x -f - -C ~/src/gcc-4.6.3/
	cd ~/src/build-binutils/
	../binutils-2.22/configure --prefix=$PREFIX --target=$TARGET --disable-nls
	make all
	sudo make install
	export PATH=$PATH:$PREFIX/bin
	cd ~/src/build-gcc/
	mv ../gcc-4.6.3/gmp-5.0.2/ ../gcc-4.6.3/gmp/
	mv ../gcc-4.6.3/mpfr-3.1.0/ ../gcc-4.6.3/mpfr/
	mv ../gcc-4.6.3/mpc-0.9/ ../gcc-4.6.3/mpc/
	../gcc-4.6.3/configure --prefix=$PREFIX --target=$TARGET --disable-nls --without-headers --with-mpfr-include=$HOME/src/gcc-4.6.3/mpfr/src/ --with-mpfr-lib=$HOME/src/build-gcc/mpfr/src/.libs/ --with-languages=c,c++
	make all-gcc
	sudo make install-gcc
	
*Note: if you have an error in a header file called tree.h just remove the line with error.