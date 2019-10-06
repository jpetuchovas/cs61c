# CS61C: Great Ideas in Computer Architecture (Machine Structures)

Solutions to various problems from [CS61C](http://inst.eecs.berkeley.edu/~cs61c/sp15/). Lecture videos are available on [the Internet Archive](https://archive.org/details/ucberkeley-webcast-PL-XXv-cvA_iCl2-D-FS5mk0jFF6cYSJs_).

## Docker Usage

The following steps require [Docker](https://docs.docker.com/install/) and [Docker Compose](https://docs.docker.com/compose/install/) to be installed.

### Preparing the Image

To prepare the `cs61c` image, run the command (this step is necessary only once):

```bash
docker-compose build
```

To check that the image was built, run the command `docker images`.

### Using the Container

Run the commands:

```bash
docker-compose run --rm cs61c
cd /cs61c
```

### Using a Cross-Compiler for MIPS

A cross-compiler for MIPS called `mips-gcc` can be used by running the commands:

```bash
docker-compose run --rm mips bash
cd /cs61c
```

Then use `$CC` to compile C code.
