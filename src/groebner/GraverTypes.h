/*
4ti2 -- A software package for algebraic, geometric and combinatorial
problems on linear spaces.

Copyright (C) 2013 4ti2 team.
Main author(s): Thomas Kahle.

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

#ifndef _4ti2_groebner__GraverTypes_
#define _4ti2_groebner__GraverTypes_

#include <vector>
#include <map>
#include <unordered_map>
#include <functional>

#include "groebner/GraverVector.h"

namespace _4ti2_
{

typedef std::vector<Vector>::iterator GraverVectorsIterator;
typedef std::vector < GraverVector > VecVecP;
typedef std::map < IntegerType, VecVecP > NormOL;   // Type for Norm overlay

typedef std::map < std::pair <BitSet, BitSet>, VecVecP > GraverFilter;
typedef std::map <IntegerType, GraverFilter > NormToFilter;
// map from norms to GraverFilters

}; // namespace 4ti2

#endif
