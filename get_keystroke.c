#include <stdio.h>
// #include <termios.h>
// #include <unistd.h>
#include <string.h>   // for memset
#include <assert.h>
#include <alloca.h>
#include <errno.h>

#include "terminal.h"

char *transform_keystroke(char *buff, int bufflen, const char *keystroke, const char *esc_str)
{
   const char *source = keystroke;
   char *target = buff;
   char *tend = target + bufflen - 1;  // leave room for terminating '\0'
   int esc_str_len = esc_str ? strlen(esc_str) : 0;

   while (*source)
   {
      if (*source==27 && esc_str_len)
      {
         if (target + esc_str_len < tend)
         {
            strcpy(target, esc_str);
            target += esc_str_len;
         }
      }
      else if (*source < 32)
      {
         if (target + 2 < tend)
         {
            *target++ = '^';
            *target++ = *source + 64;
         }
      }
      else
         *target++ = *source;

      ++source;
   }

   *target = '\0';

   return buff;
}


char* get_keystroke(char *buff, int bufflen)
{
   int filehandle = STDIN_FILENO;
   struct termios original;
   set_tios_raw_mode(&original, filehandle);
   set_tios_read_mode(filehandle, 1, 1);

   memset(buff, 0, bufflen);
   int bytes_read = read(filehandle, buff, bufflen);
   if (bytes_read < bufflen-1)
      buff[bytes_read] = '\0';
   else
   {
      buff = NULL;
      errno = ENOMEM;
   }

   restore_tios_mode(&original, filehandle);

   return buff;
}

char* get_transformed_keystroke(char *buff, int bufflen, const char *esc_str)
{
   memset(buff, 0, bufflen);

   char *raw_buff = (char*)alloca(bufflen);
   char *raw_string = get_keystroke(raw_buff, bufflen);
   if (raw_string)
      transform_keystroke(buff, bufflen, raw_string, esc_str);
   else
      buff = NULL;

   return buff;
}

char* old_get_keystroke(char *buff, int bufflen)
{
   int fh = STDIN_FILENO;
   struct termios original;
   set_tios_raw_mode(&original, fh);
   set_tios_read_mode(fh, 1, 1);

   char *keybuff = (char*)alloca(bufflen);
   int bytes_read = read(fh, keybuff, bufflen);
   if (bytes_read && bytes_read < bufflen)
   {
      keybuff[bytes_read] = '\0';
      char *kptr = keybuff;
      char *kend = keybuff + bufflen;
      char *tptr = buff;
      char *tend = tptr + bufflen;

      while (kptr < kend && *kptr)
      {
         assert(tptr < tend);

         if ( *kptr < 32 )
         {
            *tptr = '^';
            ++tptr;
            assert(tptr < tend);
            *tptr = (*kptr + 64);
            ++tptr;
         }
         else
         {
            *tptr = *kptr;
            ++tptr;
         }

         ++kptr;
      }

      if (tptr < tend)
         *tptr = '\0';
   }

   restore_tios_mode(&original, fh);

   return buff;
}

