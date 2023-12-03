#include <stdlib.h>   // for env()
#include <string.h>
#include <curses.h>
#include <term.h>
// #include <termios.h>
#include <unistd.h>     // STDIN_IN, etckkkkkkk
#include <stdbool.h>
#include <assert.h>

#include "terminal.h"


bool term_is_setup = false;

bool launch_terminal(void)
{
   if (!term_is_setup)
   {
      int error_return;
      int result = setupterm((char*)NULL, STDIN_FILENO, &error_return);
      if (result)
      {
         switch(error_return)
         {
            case 1: printf("curses unavailable in hardcopy mode.\n"); break;
            case 0: printf("curses unavailable in generic terminal.\n"); break;
            case -1: printf("curses unavailable, terminfo database missing.\n"); break;
         }
         return false;
      }
      else
         term_is_setup = true;
   }

   return term_is_setup;
}


void fill_termcap_array(TCENTRY* array, int entry_len)
{
   char *ptr = (char*)array;
   TCENTRY *tcp;
   while ((tcp=(TCENTRY*)ptr)->name)
   {
      char *val = tigetstr(tcp->name);
      if (val)
         tcp->value = val;

      ptr += entry_len;
   }
}


void print_full_list(void)
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

   char fmt[48];
   snprintf(fmt, sizeof(fmt), "%%-%ds %%-%ds %%-%ds\n", maxname, maxcode, maxfname);

   names = strnames;
   codes = strcodes;
   fnames = strfnames;

   while (*names)
   {
      printf(fmt, *names, *codes, *fnames);

      ++names;
      ++codes;
      ++fnames;
   }
}

const char *lookup_capname(const char *capcode)
{
   const char * const *names = strnames;
   const char * const *codes = strcodes;
   const char * const *ptr = codes;
   while (*ptr)
   {
      if (strcmp(*ptr, capcode)==0)
      {
         int index = ptr - strcodes;
         return names[index];
      }
      ++ptr;
   }

   return NULL;
}

/**
 * @brief Get console string from termcap code
 *
 * Use this function to get console strings, from how to set bold,
 * italic or other modes, to getting or setting console values like
 * screen size, cursor position, etc.
 *
 * First checked is an override value from an environment variable
 * with the "LESS_TERMCAP_" prefix, then the value is sought from the
 * terminfo database.
 *
 * The @p value pointer-to-pointer argument will be set with the
 * system/environment string if found.
 *
 * @param "value"    pointer to string pointer to which the value will be referenced
 * @param "capcode"  2-character termcap code for which to search
 * @return true if the termcap value was found, false if not.
 */
bool get_termcap_string(const char **value, const char *capcode)
{
   launch_terminal();

   char buff[16];
   snprintf(buff, sizeof(buff), "LESS_TERMCAP_%-s2", capcode);

   const char *capstr = getenv(buff);
   if (capstr == NULL)
   {
      const char *capname = lookup_capname(capcode);
      if (capname)
         capstr = tigetstr(capname);
   }

   if (capstr)
   {
      *value = capstr;
      return true;
   }

   return false;
}

/**
 * @brief Change settings to raw terminal io mode
 *
 * Use `tcgetattr` for current settings, the `cf_make_row` to modify,
 * then `tcsetattr` to use the raw settings.

 * Code borrowd from not-guaranteed C library function.
 * cfmakeraw().
 *
 * @param "tos"  pointer to struct with valid termios values
 *
 * @code(c)
 * struct termios original, raw;
 * tcgetattr(STDOUT_FILENO, &original);
 * // Preserve original settings with copy to modify: 
 * raw = original;
 * cf_make_raw(&raw);
 * tcsetattr(STDOUT_FILENO, &raw);
 * work_with_row();
 * tcsetattr(&original);
 * @endcode
 */
void cf_make_raw(struct termios* tos)
{
   tos->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                           |INLCR|IGNCR|ICRNL|IXON);

   tos->c_oflag &= ~OPOST;

   tos->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

   tos->c_cflag &= ~(CSIZE|PARENB);

   tos->c_cflag |= CS8;
}

/**
 * @brief Set file handle to raw mode, original settings to 'original'.
 */
void set_tios_raw_mode(struct termios* original, int fh)
{
   assert(original);
   tcgetattr(fh, original);
   struct termios raw = *original;
   cf_make_raw(&raw);
   tcsetattr(fh, TCSANOW, &raw);
}

/**
 * @brief Set read mode to existing, assuming original already saved
 */
void set_tios_read_mode(int fh, int min_chars, int timeout)
{
   struct termios orig;
   tcgetattr(fh, &orig);
   orig.c_cc[VMIN] = min_chars;
   orig.c_cc[VTIME] = timeout;
   tcsetattr(fh, TCSAFLUSH, &orig);
}

/**
 * @brief Restore settings from a saved termios, presumably a saved original
 */
void restore_tios_mode(struct termios* original, int fh)
{
   tcsetattr(fh, TCSANOW, original);
}

