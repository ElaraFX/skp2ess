#include <stdio.h>
#include "util.h"

bool isFileOccupied(const char *filepath)
{
   /* Check for existence */
   if( (ACCESS(filepath, 0 )) != -1 )
   {
      /* Check for write permission */
      if( (ACCESS(filepath, 2 )) == -1 )
	  {
		 char buf[1024] = "";
		 sprintf(buf, "File %s doesn't have write permission!\n", filepath);
         printf(buf);
		 return true;
	  }
   }

   return false;
}