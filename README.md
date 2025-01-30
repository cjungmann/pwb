# Custom Bash Builtin - `pwb`

## OVERVIEW

`pwb` is a pager with benefits.  When integrated into a Bash script,
it enables the presentation and interaction with items in an arbitrarily
large collection.

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
  Bash builtins can avoid the performance penalty of starting new
  processes, and initiate script responses through callbacks to
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
  tutorial that works through a series of increasingly sophisticated
  scripts.

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

## Example

This example can be found in [PWB samples][pwb_samples], file
*pwb_example_01_readme*, which is part of a series of examples
with names prefixed with *pwb_example_*.

1. Enable the builtin  
   ```bash
   enable pwb
   ```

2. Create a line-printing function (array data source example)
   ```bash
   print_line()
   {
      local -i index="$1"
      local data_source="$2"
      local -i chars_limit="$3"

      local -n lp_array="$data_source"
      local line="${lp_array[$index]}"

      printf "%-*s" "$chars_limit" "${line:0:$chars_limit}"
   }
   ```

3. Collect content lines
   ```bash
   declare -a files=( * )
   declare -i file_count="{#files[*]}"
   ```

4. Declare the `pwb` handle
   ```bash
   declare phandle
   pwb declare phandle files "$file_count" print_line
   ```

5. Initiate the interaction
   ```bash
   pwb init
   pwb start phandle
   pwb restore
   ```

### Additional Installation

The [ate project][ate_project] was developed to be a high-performance
`pwb` data source using Bash variables.  Most of the [pwb_samples][pwb_samples]
scripts use both `pwb` and `ate`, so I recommend installing the [ate project][ate_project]
as well.









[pwb_project]: https://github.com/cjungmann/pwb
[ate_project]: https://github.com/cjungmann/ate
[pwb_samples]: https://github.com/cjungmann/pwb_samples
