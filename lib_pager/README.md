# PAGER LIBRARY/SUBMODULE

This library started as an experiment in breaking a project down
into smaller parts to isolate functionality.

It's not as isolated as I'd hoped, and some efforts at isolation
are a little contrived (passing a function pointer to the pager
interface may be overdoing it).

## DESIGN CONSIDERATIONS

I had several ideas about how to handle page-up and page-down,
especially around how to keep a user from getting lost or
feeling uncertain about whether a line had been skipped.  After
a few attempts at researching a widely-accepted standard
behavior, I realized that a drop-down menu is the closest
thing to the behavior I am seeking.

The `dialog` command, with the `menu` element, was easy to
set up, as seen in the included Bash script `menu`.  I referred
to the menu behavior many times during development to keep my
design faithful to the established example.
