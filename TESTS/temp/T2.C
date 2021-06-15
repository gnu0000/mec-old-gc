/*
 *
 * t2.c
 * Thursday, 3/5/1998.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char *argv[])
   {
   unsigned char  c, d, e;

   c = 200;
   d = 201;

   e = (c + d) / 2;

   printf ("e = %d\n", e);

   return 0;
   }


