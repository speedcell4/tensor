// ----------------------------------------------------------------------------------------------------------
/// @file   Header file for mapper related classes and variables for mapping linear indices to multiple 
///         dimensions and from mapping multiple dimensions to a linear index etc ...
// ----------------------------------------------------------------------------------------------------------

/*
 * ----------------------------------------------------------------------------------------------------------
 *  Header file for mapper class
 *  Copyright (C) 2015 Rob Clucas robclu1818@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * ----------------------------------------------------------------------------------------------------------
 */

#ifndef FTL_MAPPER_HPP
#define FTL_MAPPER_HPP

#include "tensor_exceptions.hpp"

#include <nano/nano.hpp>
#include <array>
#include <numeric>

namespace ftl {
    
// Helper meta functions for the mapper namespace
namespace detail {
    
    // ------------------------------------------------------------------------------------------------------
    /// @struct     index_calculator
    /// @brief      Takes an index of an element in contiguous memory, and using the mapping of that memory
    ///             space to dimensions, determines the index in each of the dimensions of the given index. 
    ///             For example, if a 3x3 matrix is stored column-major in memory, then index 4 in contiguous 
    ///             memory (0 indexing) maps to row 1 column 1. This metafunction, however, does the 
    ///             calculation for any dimensional structure at compile time.
    /// @tparam     Index           The index of the element in contiguous memory.
    /// @tparam     AlDimSizes      The sizes of all dimensions of the structure.
    /// @tparam     Iteration       The iteration of the calculation.
    /// @tparam     PrevDimSizes    The sizes of the dimensions used in previous iterations.
    /// @tparam     MappedIndices   The determined mapped indices.
    // ------------------------------------------------------------------------------------------------------
    template <typename  Index           				,
  			  typename  AllDImSizes						,
  			  int 		Iteration = 0   				,
              typename  PrevDimSizes = nano::list<>    	, 
              typename  MappedIndices = nano::list<>  	>
    struct index_calculator;
    
    // Recursive case
    template <typename      Index           , 
  			  typename 	    Head			,
  			  typename... 	Tail			,
  			  int 			Iteration		,
              typename...   PrevSizes       ,
              typename...   MappedIndices   >
    struct index_calculator<Index                       ,   
                		    nano::list<Head, Tail...>   ,
  						    Iteration 					,
                            nano::list<PrevSizes...>    ,   
                            nano::list<MappedIndices...>>
    {
        // Get the product of the dim sizes of the previous iterations
        // which is the offset in contiguous memory
        static constexpr int prev_product =
            nano::multiplies<nano::list<PrevSizes...>>::result;
        
        // The case for the first iteration is different, so check for that
        using new_index = typename std::conditional<
                                Iteration == 0                                                              , 
                                nano::int_t<Index::value % Head::value>                                     ,
                                nano::int_t<(Index::value % (prev_product * Head::value)) / prev_product>
                                    >::type;
        
        // Add the new index to the list of mapped indices and recurse, 
        // eliminating the dimension size that was just used
        using result = typename index_calculator<  
                                            Index                            		, 
                              				nano::list<Tail...>						,   
                              				Iteration + 1 							,
                                            nano::list<Head, PrevSizes...>   		,
                                            nano::list<MappedIndices..., new_index>	
                                                    >::result;
        
    };
    
    // Terminating case, just return the list of mapped indices
    template <typename Index, int Iteration, typename... PrevSizes, typename... MappedIndices>
    struct index_calculator< 
                            Index                       ,
      						nano::list<>				,
  							Iteration					,
                            nano::list<PrevSizes...>    , 
                            nano::list<MappedIndices...>>
    {
        using result = nano::list<MappedIndices...>;
    };

    // ------------------------------------------------------------------------------------------------------
    /// @brief      Checks the if an index for a given dimension is valid
    /// @param[in]  idx         The index of the element in the dimension
    /// @param[in]  dim         The dimension in which the index is being searched
    /// @tparam     Container   The type of containeer used to store the dimension data
    /// @return     True if there was no error, otherwise false
    /// @throw      tensor_out_of_range  If the index is out of range for the dimension an error is thrown
    // ------------------------------------------------------------------------------------------------------
    template <typename Container>
    bool valid_index(const size_t idx, const size_t dim, const Container& dim_sizes)
    {
        try {                                                                       
            if (idx >= dim_sizes[dim]) {           
                throw tensor_out_of_range(dim + 1, dim_sizes[dim], idx); 
            } else return true;
        } catch (tensor_out_of_range& e ) {
            std::cout << e.what() << std::endl;
            return false; 
        }     
    }
   
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Recursive case helper function for the mapping of indices to and index offset, adds the 
    ///             offset due to the number of elements in one of the dimensions of the tensor
    /// @param[in]  dim_sizes   The sizes of all the dimensions in the tensor
    /// @param[in]  offset      The offset of the element at the given index positions
    /// @param[in]  idx         The index of the element in the current dimension to add to offset
    /// @param[in]  indices     The rest of the indices for which the offset has not been determined
    /// @tparam     IF          The type of the idx variable
    /// @tparam     IR          The types of the rest of the index variables 
    /// @return     The offset of the index at the postition specified by the indices
    /// @throw      tensor_out_of_range     If the index is out of range for the dimension an error is thrown
    // ------------------------------------------------------------------------------------------------------
    template <size_t Iteration, typename Container, typename IF, typename... IR>
    size_t to_index_helper(const Container& dim_sizes, size_t offset, IF idx, IR... indices)
    {  
        if (!valid_index(idx, Iteration, dim_sizes)) return 0;
        size_t num_indices = sizeof...(IR); 
        offset += std::accumulate(dim_sizes.begin()                  ,
                                  dim_sizes.end() - num_indices - 1  ,
                                  1                                  , 
                                  std::multiplies<size_t>()          ) * idx;
        // Keep iterating
        return to_index_helper<Iteration + 1>(dim_sizes, offset, indices...); 
    }
 
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Terminating case helper function for the mapping indices to an index offset, adds the 
    ///             offset due to the number of elements in one of the dimensions of the tensor
    /// @param[in]  dim_sizes   The sizes of all the dimensions in the tensor
    /// @param[in]  offset      The offset of the element at the given index positions
    /// @param[in]  idx         The index of the element in the current dimension to add to offset
    /// @tparam     IF          The type of the idx variable
    /// @return     The offset of the index at the postition specified by the indices
    /// @throw      tensor_out_of_range     If the index is out of range for the dimension an error is thrown
    // ------------------------------------------------------------------------------------------------------
    template <size_t Iteration, typename Container, typename IF>
    size_t to_index_helper(const Container&     dim_sizes   , 
                           size_t               offset      ,
                           IF                   idx         ) 
    {
        if (!valid_index(idx, Iteration, dim_sizes)) return 0;
        
        return offset + std::accumulate(dim_sizes.begin()           ,
                                        dim_sizes.end() - 1         ,
                                        1                           ,
                                        std::multiplies<size_t>()   ) * idx;
    }

}       // End namespace detail

// ----------------------------------------------------------------------------------------------------------
/// @struct     mapper
/// @brief      Mapping functions for converting an index in contiguous memory to its index values in the
///             multi-dimensional space which the contiguous memory represents (a matrix for example), and 
///             also for the reverse operations.
// ----------------------------------------------------------------------------------------------------------
struct mapper {

    // Compile time converter
    template <typename Index, typename DimSizeList>
    using index_to_dim_positions = typename detail::index_calculator<Index, DimSizeList>::result;
   

    // ------------------------------ RUNTIME FUNCTIONS -----------------------------------------------------
   
    // ------------------------------------------------------------------------------------------------------
    /// @brief  Takes an index of an element in contiguous memory and converts it to a list of indices which
    ///         represent the index but in a multidimensional space which the contiguous memory represents.
    /// @param[in]  index   The index to convert to an index list
    /// @param[in]  dim_sizes   The sizes of the dimensions of the space which the index is being mapped to
    /// @param[out] index_list  The list in indices after the mapping
    // ------------------------------------------------------------------------------------------------------
    static std::vector<size_t> index_to_index_list(size_t index                         , 
                                                   const std::vector<size_t>& dim_sizes )
    {
        std::vector<size_t> index_list(dim_sizes.size());
        
        index_list[0] = index % dim_sizes[0];
        size_t mem_offset = dim_sizes[0];
        for (int i = 1; i < dim_sizes.size(); ++i) {
            index_list[i] = (index % (mem_offset * dim_sizes[i])) / mem_offset;
            mem_offset *= dim_sizes[i];
        }
        return index_list;
    }
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Takes a list of elements which are the positions in each dimension of an element, and 
    ///             converts the list into a single index for access in contiguous memory.
    /// @param[in]  index_list  The list of indices in the tensor's dimensions
    /// @param[in]  dim_sizes   The sizes of the dimensions of the tensor
    /// @param[out] index       The index of the element in contiguous memory
    /// @tparam     Container   The type of container used for the index list
    // ------------------------------------------------------------------------------------------------------
    template <typename Container>
    static size_t index_list_to_index(const Container&            index_list   , 
                                      const std::vector<size_t>&  dim_sizes    )
    {
        size_t index     	= index_list[0];
		size_t prev_prod 	= dim_sizes[0];
        for (int i = 1; i < index_list.size(); ++i) {
      	    index       += prev_prod * index_list[i];
            prev_prod   *= dim_sizes[i];
        } 
        return index;
    };
   
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Takes a list of of variadic elements which are the positions in each dimension of an 
    ///             element, and converts the list into a single index which is a mapping of the numerous
    ///             dimensions to contiguous memory.
    /// @param[in]  dim_sizes       The sizes of the dimensions of the tensor
    /// @param[in]  first_index     The index of the element in the first dimension
    /// @param[in]  other_indices   The indices of the element in the other dimensions
    /// @tparam     Container       The type of container used for the dimension sizes
    /// @tparam     IF              The type of first_index
    /// @tparam     IR              The types of the other_indices
    // ------------------------------------------------------------------------------------------------------
    template <typename Container, typename IF, typename... IR>
    static size_t index_list_to_index(const Container&      dim_sizes       ,
                                      IF                    first_index     ,
                                      IR...                 other_indices   )
    {
        if (!detail::valid_index(first_index, 0, dim_sizes)) return 0;
        
        // Call the map helper function to determine the rest of the offset
        return detail::to_index_helper<1>(dim_sizes, first_index, other_indices...);  
    }
};

}           // End namespace ftl

#endif      // FTL_MAPPER_HPP
