# Pager with Benefits

Pagers like **more**, **less**, etc, are very useful utilities for
displaying and searching long text documents.  However, they are not
designed for scripted access, with collecting and reporting a user's
actions or responses.

**PWB**, pager with benefits, is an effort to address this shortcoming
in the standard pagers.  It is not a standalone pager, but rather a
library exposing a simple API that can be embedded in other projects.

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