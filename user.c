/*
  libscheme
  Copyright (c) 1994 Brent Benson
  All rights reserved.

  Permission is hereby granted, without written agreement and without
  license or royalty fees, to use, copy, modify, and distribute this
  software and its documentation for any purpose, provided that the
  above copyright notice and the following two paragraphs appear in
  all copies of this software.
 
  IN NO EVENT SHALL BRENT BENSON BE LIABLE TO ANY PARTY FOR DIRECT,
  INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BRENT
  BENSON HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  BRENT BENSON SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
  NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER
  IS ON AN "AS IS" BASIS, AND BRENT BENSON HAS NO OBLIGATION TO
  PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.
*/

#include "scheme.h"
#include <math.h>

static Scheme_Object *minroot (int argc, Scheme_Object *argv[]);
static Scheme_Object *sq (int argc, Scheme_Object *argv[]);
static Scheme_Object *mag (int argc, Scheme_Object *argv[]);

void
init_user(Scheme_Env *env)
{
  scheme_add_global ("$minroot", scheme_make_prim (minroot), env);
  scheme_add_global ("sq", scheme_make_prim (sq), env);
  scheme_add_global ("$mag", scheme_make_prim (mag), env);
}

static Scheme_Object *
minroot (int argc, Scheme_Object *argv[])
{
  double a, b, c;
  double disc, discrt;
  double r1, r2, r;

  SCHEME_ASSERT ((argc == 3), "minroot: wrong number of args");
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[0]), "minroot: first arg must be a number");
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[1]), "minroot: second arg must be a number");
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[2]), "minroot: third arg must be a number");
  a = SCHEME_INTP(argv[0]) ? (double) SCHEME_INT_VAL(argv[0]) : SCHEME_DBL_VAL(argv[0]);
  b = SCHEME_INTP(argv[1]) ? (double) SCHEME_INT_VAL(argv[1]) : SCHEME_DBL_VAL(argv[1]);
  c = SCHEME_INTP(argv[2]) ? (double) SCHEME_INT_VAL(argv[2]) : SCHEME_DBL_VAL(argv[2]);
  disc = b * b - 4.0 * a * c;
  if (disc < 0.0)
    {
      return (scheme_false);
    }
  discrt = sqrt(disc);
  r1 = (-b - discrt);
  r2 = (-b + discrt);
  r  = (r1 < r2 ? r1 : r2) / (2.0 * a);
  return (scheme_make_double (r));
}

static Scheme_Object *
sq (int argc, Scheme_Object *argv[])
{
  double r;

  SCHEME_ASSERT ((argc == 1), "sq: wrong number of args");
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[0]), "minroot: arg must be a number");
  r = SCHEME_INTP(argv[0]) ? (double) SCHEME_INT_VAL(argv[0]) : SCHEME_DBL_VAL(argv[0]);
  return (scheme_make_double (r * r));
}

static Scheme_Object *
mag (int argc, Scheme_Object *argv[])
{
  double a, b, c;

  SCHEME_ASSERT ((argc == 3), "mag: wrong number of args");
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[0]), "mag: first arg must be a number");
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[1]), "mag: second arg must be a number");
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[2]), "mag: third arg must be a number");
  a = SCHEME_INTP(argv[0]) ? (double) SCHEME_INT_VAL(argv[0]) : SCHEME_DBL_VAL(argv[0]);
  b = SCHEME_INTP(argv[1]) ? (double) SCHEME_INT_VAL(argv[1]) : SCHEME_DBL_VAL(argv[1]);
  c = SCHEME_INTP(argv[2]) ? (double) SCHEME_INT_VAL(argv[2]) : SCHEME_DBL_VAL(argv[2]);
  return (scheme_make_double (sqrt(a * a + b * b + c * c)));
}

