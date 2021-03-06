/*
4ti2 -- A software package for algebraic, geometric and combinatorial
problems on linear spaces.

Copyright (C) 2006 4ti2 team.
Main author(s): Peter Malkin.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 
*/

#include "Euclidean.h"
#include <iostream>

void
_4ti2_::euclidean(
                IntegerType a,
                IntegerType b,
                IntegerType& g0)
{
#ifdef _4ti2_GMP_
    mpz_gcd(g0.get_mpz_t(), a.get_mpz_t(), b.get_mpz_t());
#else
    IntegerType tmp;
    while (b != 0)
    {
        tmp = a%b;
        a = b;
        b = tmp;
    }
    g0 = a;
    if (g0 < 0) { g0 *= -1; }
#endif
}

void
_4ti2_::euclidean(
                IntegerType a,
                IntegerType b,
                IntegerType& g0,
                IntegerType& p0,
                IntegerType& q0)
{
    IntegerType p1, q1;
    _4ti2_::euclidean(a,b,g0,p0,q0,p1,q1);
}

// g0 is the gcd of a and b (g0 > 0).
// p0*a + q0*b = g0.
// p1*a + q1*b = 0 (p1 > 0).
void
_4ti2_::euclidean(
                IntegerType a,
                IntegerType b,
                IntegerType& g0,
                IntegerType& p0,
                IntegerType& q0,
                IntegerType& p1,
                IntegerType& q1)
{
    g0 = a;
    IntegerType g1 = b;
    p0 = 1; p1 = 0; q0 = 0; q1 = 1;
    IntegerType d;
    IntegerType t = 1;
    IntegerType temp;
 
    while (g1 != 0)
    {
        d = g0/g1;

        temp = g0 - d*g1;
        g0 = g1;
        g1 = temp;

        temp = p0 + d*p1;
        p0 = p1;
        p1 = temp;

        temp = q0 + d*q1; 
        q0 = q1;
        q1 = temp;

        t *= -1;
    }
    p0 *= t; p1 *= t; q0 *= -t; q1 *= -t;
    if (g0 < 0)
    {
        g0 *= -1;
        p0 *= -1;
        q0 *= -1;
    }
    if (p1 < 0)
    {
        p1 *= -1;
        q1 *= -1;
    }
}

void
_4ti2_::lcm(
                IntegerType a,
                IntegerType b,
                IntegerType& lcm)
{
    IntegerType g0, p0, q0, p1, q1;
    _4ti2_::euclidean(a,b,g0,p0,q0,p1,q1);
    lcm = a*p1;
    if (lcm < 0) { lcm *= -1; }
}

