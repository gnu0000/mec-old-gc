#include <stdio.h>
#include <stdlib.h>

/*
 * The first i2 = 0 line is not necessary to cause the error
 * it is simply to demonstrate that the compiler recognizes
 * the i2 identifier before the _asm line
 *
 * By the way, Microsoft gives me free compiler upgrades when 
 * I find bugs in their compilers - do you offer the same?
 *                              Craig.Fitzgerald@Cloverleaf.Net
 */
void TestFunction (void)
   {
   int i1, i2;

   i2 = 0;

   _asm push i1;

   i2 = 0;
   }


/*
 * Same deal here
 */
void TestFunction2 (void)
   {
   int i1, i2;

   i2;

   j;

   _asm push i1;

   i2;
   }
