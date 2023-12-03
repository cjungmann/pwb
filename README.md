# Pager with Benefits

Pagers like **more**, **less**, etc, are very useful utilities for
displaying and searching long text documents.  However, they are not
designed for scripted access, with collecting and reporting a user's
actions or responses.

**PWB**, pager with benefits, is an effort to address this shortcoming
in the standard pagers.  While it is (will be) a standalone pager, it
lacks many features of `less` and the like.  What it adds is the
ability to call back to the script for printing each line and as a
response to script-defined keys.  This permits a more customizable
pager suitable for implementing interactive displays for simple things
like selecting options to more sophisticated, like a browseable
list of `git` commits that can drill down to details.

# Work in Progress

This README page will include build and installation instructions
as well as usage examples when I feel like it's stable enough to
invite people to use it.

## Development Resources

I think I may be able to get most of the information from

- **curs_terminfo(3x)**  
  Functions that can read and write to a console, especially
  for retrieving machine-specific console codes.

- **terminfo(5)**  
  For descriptions of the terminfo values available through the
  `tigetstr` and other functions described in **curs_terminfo(3x).

- **infocmp(1m)**  
  This command will list all of the terminfo codes for the current
  terminal.

- **termintel**  
  [This repository][termintel] contains code and utilities,
  especially `ti_comp.sh` for viewing values with termcap and terminfo
  names and a description of the value.



[termintel]:  [https://github.com/cjungmann/termintel]