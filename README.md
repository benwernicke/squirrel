# squirrel
a simple package manager like application for C projects

## General Information
Squirrel is a really simple application. It does not want to be a full-blown
package manager but rather a little helper to easily set up your projects and
include headers you often use in them. Therefore, it does not come with a
backend repo of available packages but rather you have to add those manually.

Squirrel does not aim at making large project development and scaling easier
but rather be a helper to quickly setup 'script-like' projects in C.

## Installation

**NOTE:** *currently just available for Unix like operating systems*

First you have to clone this repo.

```
git clone https://github.com/benwernicke/squirrel
cd squirrel
```

Now compile the `cbuild.c` file which will later build the rest of the project.
```
gcc cbuild.c -o cbuild
```

It will recognize the compiler used to compile it and use it to compile the rest of the repo by simply calling:

```
./cbuild
```

or

```
./cbuild fast
```

Depending on the flags you want to have enabled.

Now you can install the fully compiled program with:

```
./cbuild install
```

## Usage
The usage of squirrel is quite simple. To initialize a project simply run:

```
squirrel init
```

This will create a `lib` and `build` directory in the current working directory
create a `main.c` and a `cbuild.c` specified in `config.c` and compile the
`cbuild.c` file directly.

---

If you now want to add a package to your current project run:

```
squirrel get <identifier>
```

Here the `<identifier>` could be any identifier specified in `config.c`. This
will download the associated sources, configure their `#include "xxx"`
statements to match the current directory and add compile instructions to the
`cbuild.c` file if needed and `AUTO_COMPILE_INSTRUCTIONS` is defined in
`config.h`.

**NOTE** *depending on the complexity and usage of the `cbuild.c` file it may
be recommended to alter / move the added compile instructions.*

---

To update a package simply run:

```
squirrel update <identifier>
```

This will only download the sources and configure their `#include "xxx"` statements.

---

To get information about a specific package run:

```
squirrel info <identifier>
```

This will list everything squirrel knows about the associated package.

---

To list all known packages run:

```
squirrel packages
```

## Configuration

The generated `cbuild.c` and main.c` files are specified in `config.c`. You may
alter them based on your needs. Furthermore, you can add or remove dependencies
by altering the `src` array in `config.c`.

To add a new package, specify each source of that package with its type, either:
- SRC_FILE, if it needs compilation, or
- SRC_HEADER if it doesn't need compilation, or
- SRC_OTHER if it isn't header, or FILE

With its associated `identifier`. This will be how you get/identify this
package. 

**NOTE**: *multiple sources can have the same identifier if they are
within the same package.*

With its name, this is how the file is going to be called `this should of course be unique`.
And with its upstream link. This must be a link to the raw, file.
