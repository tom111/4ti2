/*
4ti2 -- A software package for algebraic, geometric and combinatorial
problems on linear spaces.

Copyright (C) 2006 4ti2 team.
Main author(s): Matthias Walter.

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

#ifndef _4ti2_zsolve__ExtendedPottier_
#define _4ti2_zsolve__ExtendedPottier_

#include <map>
#include "zsolve/Algorithm.hpp"
#include "zsolve/BitSet.h"
#include "zsolve/LinearSystem.hpp"
#include "zsolve/Controller.hpp"
#include "zsolve/Norms.hpp"
#include "zsolve/Heuristics.hpp"
#include "zsolve/Timer.h"
#include "zsolve/ValueTree.hpp"

namespace _4ti2_zsolve_
{

template <typename T> class Controller;

template <typename T> 
class ExtendedPottier : public Algorithm <T>
{
protected:
    typedef std::map <T, ValueTree <T> *> RootMap;
    typedef std::map <NormPair <T>, bool> NormMap;

    Controller <T> * m_controller; ///< Controller for computation
    Lattice <T> * m_lattice; ///< Integer lattice that contains the solutions

    T m_maxnorm; ///< current maximum norm
    size_t m_current_variable; ///< current component
    size_t m_variables; ///< components
    T m_sum_norm; ///< ||u+v||_1
    T m_first_norm; ///< ||u||_1
    T m_second_norm; ///< ||v||_1

    NormMap m_norms;
    RootMap m_roots; ///< roots of valuetrees
    T* m_first_vector; ///< u
    T* m_second_vector; ///< v
    T* m_sum_vector; ///< u+v

    bool m_symmetric; ///< are 0 .. current symmetric components?

    Timer m_backup_timer; ///< timer
    int m_backup_frequency; ///< frequency for backup

protected:

    /**
     * \brief ???
     *
     * 
     */
    void insert_tree (ValueTree <T> *& tree, size_t vid, bool split_recursive)
    {
        if (tree->level < 0)
        {
//            std::cout << "insert_tree::insert " << vid << std::endl;
            tree->vector_indices.push_back (vid);
            if (split_recursive)
                split_tree (tree);
        }
        else
        {
            T value = (*m_lattice)[vid][tree->level];
//            std::cout << "insert_tree::search on level " << tree->level << " with value " << value << std::endl;
            if (value > 0)
            {
		auto iter = tree->pos.begin();
                for (; iter != tree->pos.end (); iter++)
                    if (value <= (*iter)->value)
                        break;
                if (iter == tree->pos.end() || value != (*iter)->value)
                    tree->pos.insert (iter, new ValueTreeNode <T> (value, vid));
                else
                    insert_tree ((*iter)->sub_tree, vid, split_recursive);
            }
            else if (value < 0)
            {
		auto iter = tree->neg.begin();
                for (; iter != tree->neg.end (); iter++)
                    if (value >= (*iter)->value)
                        break;
                if (iter == tree->neg.end() || value != (*iter)->value)
                    tree->neg.insert (iter, new ValueTreeNode <T> (value, vid));
                else
                    insert_tree ((*iter)->sub_tree, vid, split_recursive);
                
            }
            else
            {
                if (tree->zero == NULL)
                    tree->zero = new ValueTree <T> ();
                insert_tree (tree->zero, vid, split_recursive);
            }
        }
    }

    void split_tree (ValueTree <T> * tree, int start = -1)
    {
        int compo = start < 0 ? m_current_variable : start;
        bool has_pos, has_neg, has_zero;

        if (tree->level >= 0)
            return;

        for (; start < (int) m_current_variable; start++)
        {
            compo = start < 0 ? m_current_variable : start;
            has_pos = has_neg = has_zero = false;
            for (size_t i = 0; i < tree->vector_indices.size (); i++)
            {
                T value = (*m_lattice)[tree->vector_indices[i]][compo];
                if (value > 0)
                    has_pos = true;
                else if (value < 0)
                    has_neg = true;
                else
                    has_zero = true;
                if (has_pos && has_neg)
                    break;
            }
            if (has_pos && has_neg)
                break;
        }

        if ((start < (int) m_current_variable) && (tree->vector_indices.size () >= 1))
        {
//            std::cout << "Splitting on " << compo << std::endl;
            tree->level = compo;
            for (size_t i = 0; i < tree->vector_indices.size (); i++)
                insert_tree (tree, tree->vector_indices[i], false);
            start++;
            if (tree->zero != NULL)
                split_tree (tree->zero, start);
            for (size_t i = 0; i < tree->pos.size (); i++)
                split_tree (tree->pos[i]->sub_tree, start);
            for (size_t i = 0; i < tree->neg.size (); i++)
                split_tree (tree->neg[i]->sub_tree, start);
        }
    }

    void insert_trees (T* vector, T norm)
    {
        int vid = m_lattice->append_vector (copy_vector (vector, m_variables));

        if (m_roots.find (norm) == m_roots.end ())
        {
            m_roots[norm] = new ValueTree <T> ();
            
            for (auto i = m_roots.begin (); i != m_roots.end (); i++)
            {
                NormPair <T> pair (i->first, norm);
                m_norms[pair] = true;
            }
        }

        insert_tree (m_roots[norm], vid, true);
    }

    void enum_first (ValueTree <T> * tree)
    {
        if (tree->level < 0)
        {
            for (size_t i = 0; i < tree->vector_indices.size(); i++)
            {
                m_first_vector = (*m_lattice)[tree->vector_indices[i]];
                //std::cout << "enum_first enumerated [";
                //print_vector (std::cout, m_first_vector, m_variables);
                //std::cout << "]" << std::endl;
                if ((!m_symmetric && m_first_vector[m_current_variable] < 0) || (m_first_vector[m_current_variable] > 0))
                    enum_second (m_roots[m_second_norm]);
            }
            if (tree->level >= 0)
                enum_first (tree);
        }
        else
        {
            if (tree->zero != NULL)
                enum_first (tree->zero);
            for (size_t i = 0; i < tree->pos.size (); i++)
                enum_first (tree->pos[i]->sub_tree);
            for (size_t i = 0; i < tree->neg.size (); i++)
                enum_first (tree->neg[i]->sub_tree);
        }
    }

    void enum_second (ValueTree <T> * tree)
    {
        if (tree->level < 0)
        {
            for (size_t i = 0; i < tree->vector_indices.size(); i++)
            {
                m_second_vector = (*m_lattice)[tree->vector_indices[i]];
                //std::cout << "enum_second enumerated [";
                //print_vector (std::cout, m_second_vector, m_variables);
                //std::cout << "]" << std::endl;
                build_sum ();
            }
            if (tree->level >= 0)
                enum_second (tree);
        }
        else if (tree->level == (int) m_current_variable)
        {
            T value = m_first_vector[tree->level];
            if (value <= 0)
                for (size_t i = 0; i < tree->pos.size(); i++)
                    enum_second (tree->pos[i]->sub_tree);
            if (value >= 0)
                for (size_t i = 0; i < tree->neg.size(); i++)
                    enum_second (tree->neg[i]->sub_tree);
        }
        else
        {
            T value = m_first_vector[tree->level];
            if (tree->zero != NULL)
                enum_second (tree->zero);
            if (value >= 0)
                for (size_t i = 0; i < tree->pos.size(); i++)
                    enum_second (tree->pos[i]->sub_tree);
            if (value <= 0)
                for (size_t i = 0; i < tree->neg.size(); i++)
                    enum_second (tree->neg[i]->sub_tree);
        }
    }

    bool enum_reducer (ValueTree <T> * tree)
    {
        if (tree->level < 0)
        {
            for (int i = tree->vector_indices.size ()-1; i >= 0; i--)
            {
                T* reducer = (*m_lattice)[tree->vector_indices[i]];
                bool flag = true;
                for (size_t j = 0; j <= m_current_variable; j++)
                {
                    if (reducer[j] < 0)
                    {
                        if (m_sum_vector[j] >= 0 || abs (reducer[j]) > abs (m_sum_vector[j]))
                        {
                            flag = false;
                            break;
                        }
                    }
                    else if (reducer[j] > 0)
                    {
                        if (m_sum_vector[j] <= 0 || abs (reducer[j]) > abs (m_sum_vector[j]))
                        {
                            flag = false;
                            break;
                        }
                    }
                }
                if (flag)
                {
                    //std::cout << "Reduced (";
                    //print_vector (std::cout, m_sum_vector, m_current_variable+1);
                    //std::cout << ") by (";
                    //print_vector (std::cout, reducer, m_current_variable+1);
                    //std::cout << ")" << std::endl;
                    //count_reduces++;
                    return true;
                }
            }
        }
        else
        {
            T value = m_sum_vector[tree->level];
            if (value > 0)
            {
                for (auto iter = tree->pos.begin (); iter != tree->pos.end(); iter++)
                {
                    if ((*iter)->value > value)
                        break;
                    if (enum_reducer ((*iter)->sub_tree))
                        return true;
                }
            }
            else if (value < 0)
            {
                for (auto iter = tree->neg.begin (); iter != tree->neg.end(); iter++)
                {
                    if ((*iter)->value < value)
                        break;
                    if (enum_reducer ((*iter)->sub_tree))
                        return true;
                }
                
            }
            if (tree->zero != NULL)
                if (enum_reducer (tree->zero))
                    return true;
        }
        return false;
    }

    void build_sum ()
    {
        //std::cout << "buildSum (";
        //print_vector (std::cout, m_first_vector, m_variables);
        //std::cout << ") + (";
        //print_vector (std::cout, m_second_vector, m_variables);
        //std::cout << ")" << std::endl;

        if (m_first_vector == m_second_vector)
            return;

        //count_builds++;

        if (m_first_vector[m_current_variable] <= 0 && m_second_vector[m_current_variable] <= 0)
            return;
        if (m_first_vector[m_current_variable] >= 0 && m_second_vector[m_current_variable] >= 0)
            return;

        for (size_t i = 0; i < m_current_variable; i++)
        {
            if (m_first_vector[i] < 0 && m_second_vector[i] > 0)
                return;
            if (m_first_vector[i] > 0 && m_second_vector[i] < 0)
                return;
        }

        //bool flag = false;
        for (size_t i = 0; i < m_variables; i++)
        {
            m_sum_vector[i] = m_first_vector[i] + m_second_vector[i];
            //if (m_second_vector[i] != 0)
            //    flag = true;

	    int current_precision = calcPrecision (m_sum_vector[i]);
	    int max_precision = maxPrecision (m_sum_vector[i]);
	    if (max_precision > 0 && current_precision + 1 >= max_precision)
	    {
		throw PrecisionException (max_precision);
	    }
        }
        //if (!flag)
        //    std::cerr << "\n\nGENERATED ZERO VECTOR!!!!\n\n" << std::endl;


        T norm = norm_vector (m_sum_vector, m_current_variable);

        if (norm == 0)
            return;
	if (m_controller != NULL)
	    m_controller->log_status (m_current_variable+1, m_sum_norm, m_maxnorm, m_first_norm, m_lattice->vectors (), m_backup_frequency, m_backup_timer);

        // TODO: norm / 2 ??
        for (auto iter = m_roots.begin (); iter != m_roots.end() && iter->first <= norm/2; iter++)
        {
            if (enum_reducer (iter->second))
                return;
        }
/*        for (T i = 0; i <= norm/2; i++)
        {
            if (i <= m_maxnorm && m_roots.find (i) != m_roots.end())
            {
                //std::cout << "enum_reducer (roots[" << i << "])" << std::endl;
                if (enum_reducer (m_roots[i]))
                    return;
            }
        } */
        if (m_roots.find (norm) != m_roots.end ())
        {
            //std::cout << "enum_reducer (roots[" << norm << "])" << std::endl;
            if (enum_reducer (m_roots[norm]))
                return;
        }

        for (size_t i = 0; i < m_current_variable; i++)
        {
            if (!m_lattice->get_variable (i).check_bounds (m_sum_vector[i]))
                return;
        }

        if (norm > m_maxnorm)
            m_maxnorm = norm;
        
        //std::cout << "Tree before insertion:\n";
        //dump_trees ();

        //std::cout << "Inserting [";
        //print_vector (std::cout, m_sum_vector, m_variables);
        //std::cout << "]" << std::endl;
        
        //count_insertions++;
        insert_trees (m_sum_vector, norm);
        if (m_symmetric)
        {
            for (size_t i = 0; i < m_variables; i++)
                m_sum_vector[i] = -m_sum_vector[i];
            //std::cout << "Inserting [";
            //print_vector (std::cout, m_sum_vector, m_variables);
            //std::cout << "]" << std::endl;
            insert_trees (m_sum_vector, norm);
        }

        //std::cout << "Tree after insertion:\n";
        //dump_trees ();
    }
/*
    void dump_tree (ValueTree <T> * tree)
    {
        if (tree->level < 0)
        {
            std::cout << "dump_tree::leaf:\n";
            for (size_t i = 0; i < tree->vector_indices.size(); i++)
            {
                std::cout << "  [" << tree->vector_indices[i] << "] = ";
                print_vector (std::cout, (*m_lattice)[tree->vector_indices[i]], m_variables);
                std::cout << std::endl;
            }
        }
        else
        {
            std::cout << "dump_tree::node at level " << tree->level << "\n";
            if (tree->zero != NULL)
            {
                std::cout << "dump_tree::zero\n";
                dump_tree (tree->zero);
            }
            for (size_t i = 0; i < tree->pos.size (); i++)
            {
                std::cout << "dump_tree::pos (" << tree->pos[i]->value << ")" << std::endl;
                dump_tree (tree->pos[i]->sub_tree);
            }
            for (size_t i = 0; i < tree->neg.size (); i++)
            {
                std::cout << "dump_tree::neg (" << tree->neg[i]->value << ")" << std::endl;
                dump_tree (tree->neg[i]->sub_tree);
            }
        }
    }

    void dump_trees ()
    {
        std::cout << "============================ DUMP ==========================" << std::endl;
        for (auto iter = m_roots.begin (); iter != m_roots.end (); iter++)
        {
            std::cout << "dump_trees::root[" << iter->first << "]" << std::endl;
            dump_tree (iter->second);
            std::cout << std::endl;
        }
        std::cout << "============================ /DUMP ==========================" << std::endl;
    }
*/
    
    
    /** 
     * \brief Initialize reduction trees
     * 
     */
    void create_trees ()
    {
        m_maxnorm = -1;
        for (size_t i = 0; i < m_lattice->vectors (); i++)
        {
            T* vector = (*m_lattice)[i];
            T norm = norm_vector (vector, m_current_variable);
            if (norm == 0 && vector[m_current_variable] == 0)
                continue;
            m_maxnorm = max (m_maxnorm, norm);
            if (m_roots.find (norm) == m_roots.end ())
            {
                m_roots[norm] = new ValueTree <T> ();
            }
            //std::cout << "Inserting vector " << i << " into roots[" << norm << "]" << std::endl;
            insert_tree (m_roots[norm], i, false);
        }

//        std::cout << "Before SPLIT:\n\n";
//        dump_trees ();

        for (auto iter = m_roots.begin (); iter != m_roots.end (); iter++)
        {
            split_tree (iter->second);
        }

//        std::cout << "After SPLIT:\n\n";
//        dump_trees ();
    }

    void delete_trees ()
    {
        for (auto iter = m_roots.begin (); iter != m_roots.end (); iter++)
        {
            delete iter->second;
        }
        m_roots.clear ();
        m_maxnorm = -1;
    }

    int chooseNextVariable ()
    {
        //std::cout << "chooseNextVariable " << std::endl;
        BitSet allowed (m_variables, true);
        BitSet tempset (m_variables, false);

        // handled, free, range
        T best_range = 0;
        int best_infinity_count = 3;
        for (size_t i = 0; i < m_variables; i++)
        {
            VariableProperty <T> & var = m_lattice->get_variable (i);
            if ( i < m_current_variable || var.free ())
            {
                allowed.unset (i);
                continue;
            }

            int infinity_count = var.count_infinity ();
            T range = var.get_range ();

            if (infinity_count < best_infinity_count || (infinity_count == best_infinity_count && range < best_range))
            {
                best_range = range;
                best_infinity_count = infinity_count;
                tempset.zero ();
                tempset.set (i);
            }
            else if (infinity_count == best_infinity_count && range == best_range)
                tempset.set (i);
        }
        allowed.set_intersection (tempset);

        // best gcd
        T best_gcd = -1;
        tempset.zero ();


        for (size_t i = 0; i < m_variables; i++)
        {
            if (!allowed[i])
                continue;
            T gcd = m_lattice->gcd_column (i, 0, m_lattice->vectors ());
            if (best_gcd < 0 || gcd < best_gcd)
            {
                best_gcd = gcd;
                tempset.zero ();
                tempset.set (i);
            }
            else if (gcd == best_gcd)
                tempset.set (i);
        }
        allowed.set_intersection (tempset);

        return Heuristics <T> :: chooseNextVariable (*m_lattice, allowed);
    }
    
    /**
     * \brief Do some mysterious preprocessing
     *
     * This seems to be a row reduction.
     */
    void preprocess ()
    {
        T* last_reducer = NULL;
        bool repeat;
        do
        {
            repeat = false;
            for (size_t reducer_row = 0; reducer_row < m_lattice->vectors (); reducer_row++)
            {
                T* reducer = (*m_lattice)[reducer_row];
                T norm = norm_vector (reducer, m_current_variable);
                if (norm != 0 || reducer[m_current_variable] == 0)
                    continue;

                last_reducer = reducer;

                for (size_t row = 0; row < m_lattice->vectors (); row++)
                {
                    if (row == reducer_row)
                        continue;
                    T* vec = (*m_lattice)[row];
                    if (abs(vec[m_current_variable]) >= abs(reducer[m_current_variable]))
                    {
                        T factor = abs(vec[m_current_variable]) / abs(reducer[m_current_variable]);
                        if (factor != 0)
                        {
                            repeat = true;
                            if (vec[m_current_variable] * reducer[m_current_variable] > 0)
                                factor = -factor;
                            m_lattice->combine_rows (row, factor, reducer_row);
                        }
                    }
                }
            }
        }
        while (repeat);
        if (last_reducer != NULL)
        {
            last_reducer = copy_vector (last_reducer, m_lattice->variables ());
            negate_vector (last_reducer, m_lattice->variables ());
            m_lattice->append_vector (last_reducer);
        }
    }

    void complete ()
    {
	if (m_controller != NULL)
	    m_controller->log_status (m_current_variable+1, m_sum_norm, m_maxnorm, m_first_norm, m_lattice->vectors (), m_backup_frequency, m_backup_timer);
        //std::cout << "Variable: " << m_current_variable+1 << ", Sum: " << m_first_norm << " + " << m_second_norm << " = " << m_sum_norm << ", Solutions = " << m_lattice->vectors () << std::endl;

        //dump_trees ();

        if ((m_roots.find (m_first_norm) != m_roots.end ()) && (m_roots.find (m_second_norm) != m_roots.end ()))
        {
            //std::cout << "enum_first (roots[" << m_first_norm << "])" << std::endl;
            enum_first (m_roots[m_first_norm]);
            //std::cout << "enum_first finished." << std::endl;
        }
    }

public:
    ExtendedPottier () { };

    void init (LinearSystem <T> * system, Controller <T>* controller)
    {
        m_controller = controller;

        // system
	if (m_controller != NULL)
	    m_controller->log_system (system);

	//std::cout << *system << std::endl;

        // homogenized system
        LinearSystem <T> * homo = homogenize_linear_system (system);
	if (m_controller != NULL)
	    m_controller->log_homogenized_system (homo);

        // lattice
        m_lattice = generate_lattice (homo);
        delete homo;
	if (m_controller != NULL)
	{
	    //m_controller->save_lattice (m_lattice);
	    m_controller->log_lattice (m_lattice);
	}

        m_maxnorm = -1;
        m_current_variable = 0;
        m_variables = m_lattice->variables ();
        m_sum_norm = m_first_norm = m_second_norm = 0;
        m_sum_vector = m_first_vector = m_second_vector = NULL;
        m_symmetric = true;
    }

    void init (Lattice <T> * lattice, Controller <T> * controller)
    {
        m_controller = controller;
        m_lattice = new Lattice <T> (*lattice);

        m_controller->log_lattice (m_lattice);

        m_maxnorm = -1;
        m_current_variable = 0;
        m_variables = m_lattice->variables ();
        m_sum_norm = m_first_norm = m_second_norm = 0;
        m_sum_vector = m_first_vector = m_second_vector = NULL;
        m_symmetric = true;
    }

    void init (std::ifstream& stream, Controller <T> * controller)
    {
        m_controller = controller;
        controller->read_backup (stream);
        stream >> m_current_variable >> m_sum_norm >> m_first_norm >> m_symmetric;
        int vectors;
        stream >> vectors >> m_variables;

        m_maxnorm = -1;
        m_second_norm = m_sum_norm - m_first_norm;

        VariableProperties <T> * properties = new VariableProperties <T> (m_variables, false, 0, 0);
        for (size_t i = 0; i < m_variables; i++)
        {
            int column;
            bool free;
            T lower, upper;
            stream >>  column >> free >> lower >> upper;
            VariableProperty <T> & var = properties->get_variable (i);
            var.set (column, free, lower, upper);
        }

        m_lattice = new Lattice <T> (properties);

        delete properties;

        for (int i = 0; i < vectors; i++)
        {
            T* vector = read_vector <T> (stream, m_variables);
            m_lattice->append_vector (vector);
        }

        m_controller->log_resume (m_variables, m_current_variable+1, m_sum_norm, m_first_norm, vectors);
    }

    ~ExtendedPottier ()
    {
        delete m_lattice;
    }


    /**
     * What does compute do?
     *
     * At time of start, some things have been initialized:
     *
     * After a normal run of init, the following have been set:
     * 
     *    m_maxnorm = -1;
     *    m_current_variable = 0;
     *    m_variables = m_lattice->variables ();
     *    m_sum_norm = m_first_norm = m_second_norm = 0;
     *    m_sum_vector = m_first_vector = m_second_vector = NULL;
     *    m_symmetric = true;
     *     
     * The linear system and a controller have been set up.
     * 
     */
    void compute (int backup_frequency = 0)
    {
        m_norms.clear ();

        m_backup_frequency = backup_frequency;
        m_backup_timer.reset ();

        m_sum_vector = create_vector <T> (m_variables);

	//std::cout << "\n\nSTARTED ON " << m_current_variable << ", " << m_sum_norm << ", " << m_first_norm << std::endl;

	// This for loop iterates of the variables of the system.  It
	// assumes that the system is already solved on the variables
	// that are done and solves it for the next variable.
        for (; m_current_variable < m_variables; m_current_variable++)
        {
            // DEBUG
            //if (m_current_variable == 3)
            //    break;

            // first run in this variable
            if (m_sum_norm == 0)
            {
                //std::cout << "\n\n\nLattice before choose:\n" << (*m_lattice) << "\nvectors = " << m_lattice->vectors () << std::endl;
                int next = chooseNextVariable ();
                if (next < 0)
                    break;

                //std::cout << "\n-------------------------------------------" << std::endl;
                //std::cout << "\nChoice for next component is: " << next << std::endl;

		if (m_controller != NULL)
		    m_controller->log_variable_start (m_current_variable+1, m_lattice->vectors ());

                m_lattice->swap_columns (m_current_variable, next);
                
                //std::cout << "Lattice after choosing new component:\n" << (*m_lattice) << "\nMaxnorm = " << m_maxnorm << std::endl;
                preprocess ();

                //std::cout << "Lattice after preprocessing it:\n" << (*m_lattice) << "\nMaxnorm = " << m_maxnorm << std::endl;
            }

	    create_trees ();

            // create norms
            m_norms.clear ();
            for (auto i = m_roots.begin (); i != m_roots.end (); i++)
            {
                for (auto j = m_roots.begin (); j != m_roots.end (); j++)
                {
                    NormPair <T> pair (i->first, j->first);
                    m_norms[pair] = true;
                }
            }

            // norm pairs
            T old_sum = -1;
	    auto i = m_norms.begin ();
	    // scroll until old pair is reached
	    if (m_sum_norm != 0)
	    {
		while (i->first.sum != m_sum_norm || i->first.first != m_first_norm)
		    i++;
		i++;
		//std::cout << "Scrolled to " << i->first.first << " + " << (i->first.sum - i->first.first) << std::endl;
	    }

            for (; i != m_norms.end (); i++)
            {
                NormPair <T> pair = i->first;

                m_first_norm = pair.first;
                m_second_norm = pair.second;
                m_sum_norm = pair.sum;

                if (old_sum != pair.sum)
                {
                    //std::cout << "completes: " << complete_calls << ", builds: " << count_builds << ", reductions: " << count_reduces << ", insertions: " << count_insertions << std::endl;
		    if (m_controller != NULL)
			m_controller->log_sum_start (m_current_variable+1, m_sum_norm, m_lattice->vectors ());
                }

		if (m_controller != NULL)
		    m_controller->log_norm_start (m_current_variable+1, m_sum_norm, m_first_norm, m_lattice->vectors ());

                //complete_calls++;
                complete ();
                //if (m_backup_timer.get_elapsed_time () > 300)
                //    exit (2);

		if (m_controller != NULL)
		    m_controller->log_norm_end (m_current_variable+1, m_sum_norm, m_first_norm, m_lattice->vectors ());
                
                if (old_sum != pair.sum)
                {
		    if (m_controller != NULL)
			m_controller->log_sum_end (m_current_variable+1, m_sum_norm, m_lattice->vectors ());
                    old_sum = pair.sum;
                }
            
                if (backup_frequency != 0 && m_backup_timer.get_elapsed_time () > backup_frequency)
                {
                    // Backup
                    m_backup_timer.reset ();
		    if (m_controller != NULL)
			m_controller->backup_data (*m_lattice, m_current_variable, m_sum_norm, m_first_norm, m_symmetric);
                }

            }

            //std::cout << "Lattice after generating minimals:\n" << (*m_lattice) << "\nMaxnorm = " << m_maxnorm << std::endl;

            delete_trees ();
            if (!m_lattice->get_variable (m_current_variable).is_symmetric ())
                m_symmetric = false;
            m_lattice->filter_bounds (m_current_variable);
            
            //std::cout << "Lattice after filtering bounds:\n" << (*m_lattice) << "\nMaxnorm = " << m_maxnorm << std::endl;

	    if (m_controller != NULL)
		m_controller->log_variable_end (m_current_variable+1, m_lattice->vectors ());

	    m_sum_norm = 0;
        }

        //std::cout << "Finished with " << m_lattice->vectors () << " solutions." << std::endl;

        delete_vector <T> (m_sum_vector);

        m_lattice->sort_columns ();
    }

    Lattice <T>& lattice () const
    {
        return *m_lattice;
    }

    void extract_zsolve_results (VectorArray <T>& inhoms, VectorArray <T>& homs, VectorArray <T>& free)
    {
        int split = m_lattice->get_splitter ();
        int result_variables = m_lattice->get_result_num_variables ();

        inhoms.clear ();
        homs.clear ();
        free.clear ();
        if (split < 0)
            inhoms.append_vector (create_zero_vector <T> (result_variables));

        for (size_t i = 0; i < m_lattice->vectors (); i++)
        {
            T* vector = (*m_lattice)[i];
            T* result = copy_vector <T> (vector, result_variables);

            //std::cout << "looking at: ";
            //print_vector (std::cout, result, result_variables);
            //std::cout << std::endl;

            bool is_hom = split < 0 || vector[split] == 0;

            bool is_free = true;
            for (size_t j = 0; j < m_variables; j++)
                if (vector[j] != 0 && !m_lattice->get_variable (j).free ())
                    is_free = false;

            // bool has_symmetric = true;
            // for (size_t j = 0; j < m_variables; j++)
            //     if (!m_lattice->get_variable (j).check_bounds (-vector[j]))
            //         has_symmetric = false;
	    // 
            // //std::cout << is_free << std::endl;
            // 
            // assert (!is_free || has_symmetric);

            //int lex_cmp = lex_compare_vector_with_negative (vector, result_variables);

            if (is_free)
            {
                free.append_vector (result);
            }
            else
            {
                if (is_hom)
                    homs.append_vector (result);
                else
                    inhoms.append_vector (result);
            }
        }

	    if (m_controller != NULL)
	        m_controller->log_result (inhoms.vectors (), homs.vectors (), free.vectors ());
    }

    void extract_graver_results (VectorArray <T>& graver)
    {
        assert (m_lattice->get_splitter () == -1);
        assert (m_lattice->get_result_num_variables () == m_variables);

        graver.clear ();

        for (size_t i = 0; i < m_lattice->vectors (); i++)
        {
            T* vector = (*m_lattice)[i];
            T* result = copy_vector <T> (vector, m_variables);

            bool has_symmetric = true;
            for (size_t j = 0; j < m_variables; j++)
                if (!m_lattice->get_variable (j).check_bounds (-vector[j]))
                    has_symmetric = false;
            
            int lex_cmp = lex_compare_vector_with_negative (vector, m_variables);

            if (!has_symmetric || lex_cmp > 0)
                graver.append_vector (result);
        }

	    if (m_controller != NULL)
	        m_controller->log_result (1, m_lattice->vectors (), 0);
    }
    
    void extract_hilbert_results (VectorArray <T>& hilbert)
    {
        assert (m_lattice->get_splitter () == -1);
        assert (m_lattice->get_result_num_variables () == m_variables);

        hilbert.clear ();

        for (size_t i = 0; i < m_lattice->vectors (); i++)
        {
            T* vector = (*m_lattice)[i];
            T* result = copy_vector <T> (vector, m_variables);
            hilbert.append_vector (result);
        }

	    if (m_controller != NULL)
	        m_controller->log_result (1, m_lattice->vectors (), 0);
    }

    T extract_maxnorm_results (VectorArray <T> & maxnorm)
    {
        int result_variables = m_lattice->get_result_num_variables ();

        maxnorm.clear ();

        m_maxnorm = -1;
        for (size_t i = 0; i < m_lattice->vectors (); i++)
        {
            T* vector = (*m_lattice)[i];
            T norm = norm_vector (vector, result_variables);
            if (norm > m_maxnorm)
            {
                m_maxnorm = norm;
                maxnorm.clear ();
            }
            if (norm == m_maxnorm)
                maxnorm.append_vector (copy_vector <T> (vector, result_variables));
        }
        return m_maxnorm;
    }

    size_t get_result_num_variables () const
    {
        return m_lattice->get_result_num_variables ();
    }

    void log_maxnorm ()
    {
	if (m_controller != NULL)
	    m_controller->log_maxnorm (this, true);
    }

    //int count_builds;
    //int count_reduces;
    //int count_insertions;
    //int complete_calls;
};

} // namespace _4ti2_zsolve_
  
#endif
