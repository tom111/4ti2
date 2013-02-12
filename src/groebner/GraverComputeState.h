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

#ifndef _4ti2_groebner__GraverComputeState_
#define _4ti2_groebner__GraverComputeState_

#include <vector>
#include <map>

#include "groebner/VectorArray.h"

namespace _4ti2_
{

typedef std::map <IntegerType, VectorArray* > NormBST;

class GraverComputeState
{
public:
    // GraverComputeState(const VectorArray& vs); ///< copy constructor
    // GraverComputeState(VectorArray&& vs); ///< move constructor
    GraverComputeState(VectorArray *vs); ///< self-made move constructor
    
    ~GraverComputeState();

    VectorArray& get_vectors () { return *m_array; }; ///< Current state of computation
    VectorArray& get_latticeBasis () { return *m_latticeBasis; };
    NormBST& get_tree () { return *m_normTree; };

    void createNormBST (Index stop);
    IntegerType maximum_norm () const;
private:

private:
    VectorArray *m_latticeBasis;
    VectorArray *m_array;
    NormBST *m_normTree;
    Index m_stopindex;
};

} // namespace _4ti2_

#endif
