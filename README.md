# PWB: Pager With Benefits

`PWB` is a tool that facilitates building Bash-based applications for
displaying and interacting with collections of data.

## INTRODUCTION

`PWB` is implemented as a Bash builtin command.  As a builtin, it can
be invoked in a script without a subprocess penalty, and it can also
call back to other functions in the script.

Within its narrow mission of managing the display of and interaction
with collections, `PWB` is very flexible because it displays or executes
items by calling script functions provided by the calling script.

## INITIAL RELEASE

This *README* file is a limited first draft, primarily concerned with
demonstrating the utility of the tool.  There is some documentation in
the form of a `man` page, but a more complete tutorial will be
authored at a later date.

I'm still making a collection of sample apps based on this tool, and
I'm finding little features that I want to add or change.

## INSTALLATION

Clone the project, the build and install to make `PWB` available:

~~~sh
git clone https://github.com/cjungmann/pwb.git
cd pwb
configure
make
sudo make install
~~~

### Configure Script

The **configure** script manages the availablity of a few libraries
used by `PWB` and its usual companion command, `ATE`.  In the initial
release, the dependent libraries will be downloaded to a directories
under *pwb*, built, with instructions for linking to the libraries
copied to the *Makefile*.

### make install

The install *Makefile* target will install the following:

1. Copy the `PWB` shared-object library *pwb.so* to */usr/local/lib*
2. Copy a collection of mini tools to */usr/local/lib/pwb_sources.d*
3. Copy scripts *enable_pwb* and *pwb_sources* to */usr/local/bin*
4. Compile and copy man pages *pwb.1* and *pwb.7* to appropriate
   directories under */usr/local/share/man*.

### make uninstall

There is a *Makefile* **uninstall** target that removes all of the
installed files listed above.

## EXAMPLES

The initial release depends on learning from example

There are several examples in [pwb_samples][pwb_samples].






[pwb_samples]: https://github.com/cjungmann/pwb_samples.git