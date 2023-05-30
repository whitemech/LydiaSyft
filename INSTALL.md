###Compilation Instructions using CMake

==== Install CUDD ====

0.1 Make sure CUDD is installed. CUDD can be found at: 

    https://github.com/KavrakiLab/cudd.git

0.2 Install CUDD:

    ./configure --enable-silent-rules --enable-obj --enable-dddmp --prefix=[install location]
    sudo make install

    If you get an error about aclocal, this might be due to either
    a. Not having automake:
        sudo apt-get install automake
    b. Needing to reconfigure, do this before configuring:
        autoreconf -i


==== Install FLEX, BISON ====

0.3 Install flex and bison:

    sudo apt-get install flex bison


==== Install LYDIA ====

    cd submodules
    cd lydia

Follow the instructions to complete the installation of lydia.


==== Install Z3 ====

By default, the CMake configuration will fetch z3 automatically from the GitHub repository.
In order to disable this behaviour, you can configure the project by setting -DZ3_FETCH=OFF.

In that case, you have to have the library installed on your system.

To install the shared library on Ubuntu:

    sudo apt install libz3-dev

If you want to link the static library of z3, you have to install z3 it manually:

```
wget https://github.com/Z3Prover/z3/releases/download/z3-4.8.12/z3-4.8.12-x64-glibc-2.31.zip
unzip z3-4.8.12-x64-glibc-2.31.zip
cd z3-4.8.12-x64-glibc-2.31
cp bin/libz3.a /usr/local/lib
cp include/*.h /usr/local/include
```


==== Build LYDIASYFT ====

1. Make build folder so your directory is not flooded with build files:

    ```mkdir build && cd build```

3. Run CMake to generate the makefile:

    ```cmake -DCMAKE_BUILD_TYPE=Release ..```

4. Compile using the generated makefile:

    ```make -j$(nproc --ignore=1) LydiaSyft```

5. Compile and Run tests:

    ```
    make -j$(nproc --ignore=1) tests
    ./bin/tests
   ```


==== Run LYDIASYFT ====

1. Reach executable file LydiaSyft

   ```cd bin```

2. Run example:
   Agn first: ```./Lydiasyft -s ../../syfco-bin/syfco -f ../../example/test.tlsf```

