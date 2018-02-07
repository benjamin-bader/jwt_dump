jwt_dump
--------

A fast and easy tool to print the contents of encoded JWTs to your terminal.  No node.js runtime needed!

To build:

```
cmake .
make
```

To run:

```
./jwt_dump 'encoded-jwt-here'

# or, on a mac:
pbpaste | ./jwt_dump
```