jwt_dump
--------

A fast and easy tool to print the contents of encoded JWTs to your terminal.  No node.js runtime needed!

Build requirements:
cmake >= 3.10.0
A c++ compiler supporting c++14, such as clang 4.0+ or a recent Visual Studio/MSVC release.

To build, on a mac:
```
mkdir build
cd build
cmake ..
make && make tst && make install
```

To run:
```
./jwt_dump 'encoded-jwt-here'

# or, on a mac:
pbpaste | ./jwt_dump
```
