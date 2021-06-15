/*
 *
 * t1.c
 * Thursday, 3/5/1998.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char *argv[])
   {
   char sz[16];
   int  i1;
   int  i2;

   strcpy (sz, "Hello");
   i1 = (int)*sz;
   i2 = *(int *)sz;

   printf ("i1 = %d\n", i1);
   printf ("i2 = %d\n", i2);

   return 0;
   }

