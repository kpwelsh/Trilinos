// @HEADER
// ***********************************************************************
// 
//                           Stokhos Package
//                 Copyright (2009) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Eric T. Phipps (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef STOKHOS_LANCZOSPROJPCEBASIS_HPP
#define STOKHOS_LANCZOSPROJPCEBASIS_HPP

#include "Teuchos_RCP.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_SerialDenseMatrix.hpp"
#include "Teuchos_BLAS.hpp"

#include "Stokhos_RecurrenceBasis.hpp"
#include "Stokhos_OrthogPolyApprox.hpp"
#include "Stokhos_Sparse3Tensor.hpp"
#include "Stokhos_Lanczos.hpp"

namespace Stokhos {


  template <typename ord_type, typename val_type>
  class DenseOperator {
  public:
    typedef ord_type ordinal_type;
    typedef val_type value_type;
    typedef Teuchos::SerialDenseMatrix<ordinal_type, value_type> matrix_type;
    typedef Teuchos::Array<value_type> vector_type;

    DenseOperator(const matrix_type& A_): A(A_), blas() {}
    
    void 
    apply(const vector_type& u, vector_type& v) const {
      blas.GEMV(Teuchos::NO_TRANS, A.numRows(), A.numCols(), value_type(1), 
                &A(0,0), A.stride(), &u[0], ordinal_type(1), value_type(0), 
                &v[0], ordinal_type(1));
    }

  protected:

    const matrix_type& A;
    Teuchos::BLAS<ordinal_type, value_type> blas;

  };

  /*! 
   * \brief Generates three-term recurrence using the Lanczos 
   * procedure applied to a polynomial chaos expansion in another basis.
   */
  template <typename ordinal_type, typename value_type>
  class LanczosProjPCEBasis : 
    public RecurrenceBasis<ordinal_type, value_type> {
  public:

    //! Constructor
    /*!
     * \param p order of the basis
     * \param pce polynomial chaos expansion defining new density function
     * \param quad quadrature data for basis of PC expansion
     */
    LanczosProjPCEBasis(
      ordinal_type p,
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& pce,
      const Stokhos::Sparse3Tensor<ordinal_type, value_type>& Cijk,
      bool normalize);

    //! Destructor
    ~LanczosProjPCEBasis();

    //! \name Implementation of Stokhos::OneDOrthogPolyBasis methods
    //@{

    /*! 
     * \brief Clone this object with the option of building a higher order
     * basis.
     */
    /*!
     * This method is following the Prototype pattern (see Design Pattern's textbook).
     * The slight variation is that it allows the order of the polynomial to be modified,
     * otherwise an exact copy is formed. The use case for this is creating basis functions
     * for column indices in a spatially varying adaptive refinement context.
     */
    virtual Teuchos::RCP<OneDOrthogPolyBasis<ordinal_type,value_type> > cloneWithOrder(ordinal_type p) const;

    //@}

  protected:

    //! \name Implementation of Stokhos::RecurrenceBasis methods
    //@{ 

    //! Compute recurrence coefficients
    virtual bool
    computeRecurrenceCoefficients(ordinal_type n,
				  Teuchos::Array<value_type>& alpha,
				  Teuchos::Array<value_type>& beta,
				  Teuchos::Array<value_type>& delta,
				  Teuchos::Array<value_type>& gamma) const;

    //@}

  private:

    // Prohibit copying
    LanczosProjPCEBasis(const LanczosProjPCEBasis&);

    // Prohibit Assignment
    LanczosProjPCEBasis& operator=(const LanczosProjPCEBasis& b);

    //! Copy constructor with specified order
    LanczosProjPCEBasis(ordinal_type p, const LanczosProjPCEBasis& basis);

  protected:

    typedef Stokhos::Lanczos< WeightedVectorSpace<ordinal_type,value_type>, 
                              DenseOperator<ordinal_type,value_type> > lanczos_type;
    typedef typename DenseOperator<ordinal_type,value_type>::matrix_type matrix_type;
    typedef typename lanczos_type::vector_type vector_type;

    //! Triple-product matrix used in generating lanczos vectors
    matrix_type Cijk_matrix;

    //! Weighting vector used in inner-products
    vector_type weights;

    //! Initial Lanczos vector
    vector_type u0;

    //! Lanczos vectors
    mutable Teuchos::Array<vector_type> lanczos_vecs;

  }; // class LanczosProjPCEBasis

} // Namespace Stokhos

// Include template definitions
#include "Stokhos_LanczosProjPCEBasisImp.hpp"

#endif