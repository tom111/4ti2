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

#include "groebner/markov_main.h"
#include "groebner/Vector.h"
#include "groebner/VectorStream.h"
#include "groebner/VectorArray.h"
#include "groebner/VectorArrayStream.h"
#include "groebner/BitSet.h"
#include "groebner/BitSetStream.h"
#include "groebner/Feasible.h"
#include "groebner/FeasibleStream.h"
#include "groebner/GeneratingSet.h"
#include "groebner/GroebnerBasis.h"
#include "groebner/Options.h"
#include "groebner/Globals.h"

#include <iostream>
#include <fstream>

#include <string>

using namespace _4ti2_;

int
_4ti2_::markov_main(int argc, char **argv)
{
    Options::instance()->process_options(argc, argv);

    print_banner();

    Feasible* feasible = input_Feasible(Options::instance()->filename.c_str());

    // Construct the generating set.
    GeneratingSet gs(*feasible, 0);
    gs.standardise();

    // Output the Groebner basis.
    std::string markov_filename(Options::instance()->filename + ".mar");
    std::ofstream markov_file(markov_filename.c_str());
    output(markov_file, gs.get_generating_set());

    delete feasible;
    return 0;
}
