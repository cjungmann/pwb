#include <stdio.h>
// #include <termios.h>
// #include <unistd.h>
#include <string.h>   // for memset
#include <assert.h>
#include <alloca.h>
#include <ctype.h>    // for toupper()v
#include <errno.h>

#include <curses.h>
#include <term.h>

#include "export.h"
#include "terminal.h"
#include "screen.h"

/**
 * @brief Translates a keystroke-style string to a readable string.
 * @param "buff"       memory to which the transformation will be written
 * @param "bufflen"    length of transformation buffer
 * @param "keystroke"  string to transform
 * @param "esc_str"    string to use to represent an ESC character.
 *                     If NULL, it uses '^['.
 *
 * @return pointer to @p buff if successful, NULL if not.
 *
 * Use this function to replace the unprintable characters in a
 * string with 2-character representations, with special accommodation
 * for the ubiquitous ESCAPE key, whose translation can be specified.
 *
 * Use this function to show a user the keystroke string emitted by
 * the last pressed key.  This function could also be used to compare
 * a keystroke string with a typeable string to match up actions.
 */
EXPORT char *transform_keystroke(char *buff, int bufflen, const char *keystroke, const char *esc_str)
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

/**
 * @brief Translate a typeable keystring to a keystroke string.
 * @param "buff"       memory to which the translation will be written
 * @param "bufflen"    length of translation buffer
 * @param "keystring"  string to translate
 * @param "esc_str"    string to use to represent an ESC character.
 *                     If NULL, it uses '^['.
 *
 * Use this function to translate a keystring that includes control
 * characters like '^[' for ESCAPE to a keystroke string that can directly
 * match the keystroke string emitted when a user presses a key.
 *
 * Please only use this for keystroke strings.  This function makes no
 * accommodation for the possibility of an accompanied '^' character
 * that does not represent an untypeable control character.
 */
EXPORT char *transform_keystring(char *buff, int bufflen, const char *keystring, const char *esc_str)
{
   char *target = buff;
   char *tlimit = target + bufflen - 1; // leave room for terminating '\0'
   const char *ptr = keystring;

   int esc_str_len = 0;
   if (esc_str)
      esc_str_len = strlen(esc_str);

   while (*ptr && target < tlimit)
   {
      if (esc_str_len)
      {
         if (*ptr == *esc_str)
         {
            const char *esc_ptr = esc_str;
            const char *temp_ptr = ptr;
            while (*esc_ptr && *temp_ptr)
            {
               if (*esc_ptr != *temp_ptr)
                  break;
               ++esc_ptr;
               ++temp_ptr;
            }
            // If reached the end of the escape string without a
            // disqualifying mismatch, translate to an ESCAPE character
            if (! *esc_ptr)
            {
               *target++ = '\x1b';
               ptr = temp_ptr;

               // Bypass further processing of this position
               // if we matched the ESCAPE string:
               continue;
            }
         }
      }

      if (*ptr == '^' && *(ptr+1))
      {
         char val = toupper(*(ptr+1));
         *target++ = val - 64;
         ptr += 2;
      }
      else
         *target++ = *ptr++;
   }

   if (target <= tlimit)
   {
      *target = '\0';
      return buff;
   }

   return NULL;
}


/**
 * @brief Get a single keystroke from the user.
 * @param "buff"    memory to which the keystroke string will be copied.
 *                  If @p buff is NULL, a static char buffer will be used.
 * @param "bufflen" length of @p buff.  This value will be ignored if
 *                  @p buff is NULL and the static char buffer is used.
 * @return Pointer to @p buff if successful, NULL otherwise.
 *
 * This function returns the raw keystroke string from a user
 * interaction.  Non-character keys will typically return a
 * multi-character string beginning with ESCAPE, otherwise
 * recognized as '\e', '\x1b', or '\033'.
 */
EXPORT char* get_keystroke(char *buff, int bufflen)
{
   static char simple_buff[48];

   if (buff == NULL)
   {
      buff = simple_buff;
      bufflen = sizeof(simple_buff);
   }

   int filehandle = STDIN_FILENO;
   struct termios original, raw;
   tcgetattr(filehandle, &original);
   raw = original;
   set_rawread_mode(&raw);
   tcsetattr(filehandle, TCSANOW, &raw);

   enter_keyboard_transmit_mode();

   memset(buff, 0, bufflen);
   int bytes_read = read(filehandle, buff, bufflen);
   if (bytes_read == 0)
      buff = NULL;
   else if (bytes_read < bufflen-1)
      buff[bytes_read] = '\0';
   else
   {
      buff = NULL;
      errno = ENOMEM;
   }

   exit_keyboard_transmit_mode();

   tcsetattr(filehandle, TCSAFLUSH, &original);

   return buff;
}

/**
 * @brief Convenience function to get translated keystroke
 * @param "buff"    memory to which keystroke string will be copied
 * @param "bufflen" size of @p buff memory buffer
 * @param "esc_str" String to represent ESCAPE.  Set to NULL to use
 *                  the default representation, '^['.
 * @return Pointer to @p buff if successful, NULL otherwise.
 */
EXPORT char* get_transformed_keystroke(char *buff, int bufflen, const char *esc_str)
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

