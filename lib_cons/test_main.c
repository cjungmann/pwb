#include <stdio.h>
#include <string.h>
#include <term.h>  // for strnames, strcodes, and strfnames

#include "cons.h"

TCENTRY keys[] = {
   { "kcud1" },    // key down
   { "kcuu1" },    // key up
   { "knp"   },    // key next page (page down)
   { "kpp"   },    // key previous page (page up)
   { NULL }        // terminator entry
};

void show_key_values(void)
{
   fill_termcap_array(keys, sizeof(TCENTRY));
   TCENTRY *ptr = keys;

   char buff[20];

   while (ptr->name)
   {
      printf("%s %s\n", ptr->name,
             transform_keystroke(buff, sizeof(buff), ptr->value, "\\x1b"));
      ++ptr;
   }
}

void print_caps_key(void)
{
   const char * const *names = NULL;
   const char * const *codes = NULL;
   const char * const *fnames = NULL;

   names = strnames;
   codes = strcodes;
   fnames = strfnames;

   int maxname=0, maxcode=0, maxfname=0;
   int curname=0, curcode=0, curfname=0;

   while (*names)
   {
      curname = strlen(*names);
      curcode = strlen(*codes);
      curfname = strlen(*fnames);

      if (curname > maxname)
         maxname = curname;
      if (curcode > maxcode)
         maxcode = curcode;
      if (curfname > maxfname)
         maxfname = curfname;

      ++names;
      ++codes;
      ++fnames;
   }

   const char *capval;
   char valbuff[128];
   const char escstr[] = "\x1b[31;1m^[\x1b[m";

   char fmt[64];
   snprintf(fmt, sizeof(fmt), "%%-%ds %%-%ds %%-%ds: %%s\n", maxname, maxcode, maxfname);

   names = strnames;
   codes = strcodes;
   fnames = strfnames;

   while (*names)
   {
      if ((capval = tigetstr(*names)))
         capval = transform_keystroke(valbuff, sizeof(valbuff), capval, escstr);
      else
         capval="n/a";

      printf(fmt, *names, *codes, *fnames, capval);

      ++names;
      ++codes;
      ++fnames;
   }
}



int main(int argc, const char **argv)
{
   print_caps_key();
   show_key_values();
}
