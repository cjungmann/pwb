# Custom Bash Builtin - `pwb`

## OVERVIEW

`pwb` is a pager with benefits.  It manages the display of collections
of items, whether they are lines of text, files in a directory,
commits in a repository, or something else.  The tool provides a
pager-like ability to navigation through the items in a collection
using traditional keystrokes, but includes other features that make
it more than a simple pager.

The 'benefits' are the flexibility of item expression, the ability to
select items in a collection, and the ability to define keystrokes
that trigger actions on a selected item.

## FEATURES

- **Bash builtin**
  Bash builtins can avoid the performance penalty of starting child
  processes, and can initiate script responses through callbacks to
  script functions.

- **Flexible**
  While `pwb` manages when, where, and how much to print, it calls
  the hosting script to perform the printing.  The Bash script
  prints the line and margin content, as well as defining hot keys
  and the provided the response when they're pressed.

- **Library**
  I'm experimenting with an interface for including Bash script
  fragments of `pwb` solutions and utilities into new Bash scripts.
  The library interface provides a list of available scriptlets,
  basic usage info about each scriptlet, and a system for managing
  includes without duplicates.

- **Tutorial**
  A companion repository, [pwb_samples][pwb_samples], contains
  several useful applications developed with `pwb`, and a simple
  tutorial (`./pwb_tutorial`) that works through a series of
  increasingly sophisticated scripts.

  Please be aware that [pwb_samples][pwb_samples] is somewhat of
  a sandbox where I am trying to develop difficult ideas.  I am
  practicing use of the tool to help me discover bugs, missing
  features, and inconsistencies.

## INSTALLATION

### Prerequisites

Ensure you have the following:

- Bash version `4.0.0` or higher
- A C-compiler (gcc) for compiling the command
- _bash_builtins_ development package
- _git_ to clone, build, and statically link a
  few library dependencies

The development environment is Linux.  I should work on any Linux
distribution, and will probably work on BSD as well (I haven't tested
it there yet).

### Installation Steps

1. Clone the repository:
    ```bash
    git clone https://github.com/cjungmann/pwb.git
    ```
2. Navigate to the cloned directory:
    ```bash
    cd pwb
    ```
3. Run the `configure` script to resolve dependencies and prepare the Makefile.
   ```bash
   ./configure
   ```
3. Build and install the builtin:
    ```bash
    make
    sudo make install
    ```
4. Verify the installation by running:
    ```bash
    enable pwb
    pwb version
    ```

## USAGE

### Syntax
```bash
pwb [action_name] [handle_name] [options]
```

### Example

This example is a copy of the code section of the **introduction**
chapter of the pwb tutorial included in [PWB samples][pwb_samples].
It is an example of a minimal viable pwb application.

~~~bash
#!/usr/bin/env bash

# Step 1: establish data source
declare -a DIR_FILES=( * )

# Step 2: implement line print function
print_line()
{
    # retrieve data
    local -i index="$1"
    local -n pl_dsource="$2"
    local line="${pl_dsource[$index]}"

    # print data
    local -i chars_limit="$3"
    printf "%s" "${line:0:$chars_limit}"
}

# Step 3: Enable the builtin
enable pwb

# Step 4: Prepare the pwb handle
pwb declare PHANDLE DIR_FILES "${#DIR_FILES[*]}" print_line

# Step 5: Start the interaction
pwb start PHANDLE
~~~

The tutorial is accessed by running the `./pwb_tutorial` script
in the [pwb_samples][pwb_samples] project.  The tutorial will
require [pwb][pwb_project] to be installed for the early chapters,
the [ate][ate_project] will be required for more advanced chapters.

### Additional Installation

The [ate project][ate_project] was developed to be a high-performance
`pwb` data source using Bash variables.  Most of the [pwb_samples][pwb_samples]
scripts use both `pwb` and `ate`, so I recommend installing the [ate project][ate_project]
as well.









[pwb_project]: https://github.com/cjungmann/pwb
[ate_project]: https://github.com/cjungmann/ate
[pwb_samples]: https://github.com/cjungmann/pwb_samples
