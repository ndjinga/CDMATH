/*
 * SparseMatrixPetsc.cxx
 *
 *  Created on: 04/11/2017
 *      Author: mndjinga
 */

#include "SparseMatrixPetsc.hxx"
#include "CdmathException.hxx"

#include <cstring>
#include <slepceps.h>

using namespace std;

//----------------------------------------------------------------------
SparseMatrixPetsc::SparseMatrixPetsc()
//----------------------------------------------------------------------
{
	_numberOfColumns=0;
	_numberOfRows=0;
	_numberOfNonZeros=0;
	_isSparseMatrix=true;
	_mat=NULL;
	PetscInitialize(0, (char ***)"", PETSC_NULL, PETSC_NULL);
}

//----------------------------------------------------------------------
SparseMatrixPetsc::SparseMatrixPetsc( int numberOfRows, int numberOfColumns)
//----------------------------------------------------------------------
{
	_numberOfRows = numberOfRows;
	_numberOfColumns=numberOfColumns;
	_isSparseMatrix=true;
	PetscInitialize(0, (char ***)"", PETSC_NULL, PETSC_NULL);
	MatCreateSeqAIJ(MPI_COMM_SELF,_numberOfRows,_numberOfColumns,PETSC_DEFAULT,NULL,&_mat);
}

//----------------------------------------------------------------------
SparseMatrixPetsc::SparseMatrixPetsc( Mat mat )
//----------------------------------------------------------------------
{
	PetscInitialize(0, (char ***)"", PETSC_NULL, PETSC_NULL);
	_isSparseMatrix=true;
	_mat=mat;
	//extract number of row and column
	MatGetSize(mat,&_numberOfRows,&_numberOfColumns);

	//extract an upper bound for the total number of non zero coefficients
	MatInfo info;
	MatGetInfo(mat,MAT_LOCAL,&info);
	_numberOfNonZeros = info.nz_allocated;
}

//----------------------------------------------------------------------
SparseMatrixPetsc::SparseMatrixPetsc( int numberOfRows, int numberOfColumns, int nnz )
//----------------------------------------------------------------------
{
	_numberOfRows = numberOfRows;
	_numberOfColumns=numberOfColumns;
	_numberOfNonZeros=nnz;
	_isSparseMatrix=true;
	_mat=NULL;
	PetscInitialize(0, (char ***)"", PETSC_NULL, PETSC_NULL);
	MatCreateSeqAIJ(MPI_COMM_SELF,_numberOfRows,_numberOfColumns,_numberOfNonZeros,NULL,&_mat);
}

//----------------------------------------------------------------------
SparseMatrixPetsc::SparseMatrixPetsc( int blockSize, int numberOfRows, int numberOfColumns, int nnz )
//----------------------------------------------------------------------
{
	_numberOfRows = numberOfRows;
	_numberOfColumns=numberOfColumns;
	_numberOfNonZeros=nnz;
	_isSparseMatrix=true;
	_mat=NULL;
	PetscInitialize(0, (char ***)"", PETSC_NULL, PETSC_NULL);
	MatCreateSeqBAIJ(MPI_COMM_SELF,blockSize, _numberOfRows,_numberOfColumns,_numberOfNonZeros,NULL,&_mat);
}

//----------------------------------------------------------------------
SparseMatrixPetsc::SparseMatrixPetsc(const SparseMatrixPetsc& matrix)
//----------------------------------------------------------------------
{
	_isSparseMatrix=matrix.isSparseMatrix();
	MatDuplicate(matrix.getPetscMatrix(), MAT_COPY_VALUES,&_mat);
	MatGetSize(_mat,&_numberOfRows,&_numberOfColumns);	
	//extract an upper bound for the total number of non zero coefficients
	MatInfo info;
	MatGetInfo(_mat,MAT_LOCAL,&info);
	_numberOfNonZeros = info.nz_allocated;
}

SparseMatrixPetsc
SparseMatrixPetsc::transpose() const
{
	Mat mattranspose;
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);

	MatTranspose(_mat,MAT_INITIAL_MATRIX, &mattranspose);
	return SparseMatrixPetsc(mattranspose);
}

void
SparseMatrixPetsc::setValue( int i, int j, double value )
{
	MatSetValues(_mat,1, &i, 1, &j, &value, INSERT_VALUES);
}

void
SparseMatrixPetsc::addValue( int i, int j, double value )
{
	MatSetValues(_mat,1, &i, 1, &j, &value, ADD_VALUES);
}

void
SparseMatrixPetsc::setValue( int i, int j, Matrix M  )
{
    int I,J;
    for (int k=0; k<M.getNumberOfRows(); k++)
        for (int l=0; l<M.getNumberOfColumns(); l++)
        {
            I=i+k;
            J=j+l;
            MatSetValues(_mat,1, &I, 1, &J, &M(k,l), INSERT_VALUES);
        }
}

void
SparseMatrixPetsc::addValue( int i, int j, Matrix M  )
{
    int I,J;
    for (int k=0; k<M.getNumberOfRows(); k++)
        for (int l=0; l<M.getNumberOfColumns(); l++)
        {
            I=i+k;
            J=j+l;
            MatSetValues(_mat,1, &I, 1, &J, &M(k,l), ADD_VALUES);
        }
}

void
SparseMatrixPetsc::setValuesBlocked( int i, int j, Matrix M  )
{
    int blockSize;
    MatGetBlockSize(_mat,&blockSize);
    if(blockSize!=M.getNumberOfRows() || blockSize!=M.getNumberOfColumns())
        throw CdmathException("SparseMatrixPetsc::setValuesBlocked : matrix size is different from sparse matrix block structure");
    double petscValues[blockSize*blockSize];
    for (int k=0; k<M.getNumberOfRows(); k++)
        for (int l=0; l<M.getNumberOfColumns(); l++)
            petscValues[k*blockSize+l]=M(k,l);
    MatSetValuesBlocked(_mat,1, &i, 1, &j, petscValues, INSERT_VALUES);
}

void
SparseMatrixPetsc::addValuesBlocked( int i, int j, Matrix M  )
{
    int blockSize;
    MatGetBlockSize(_mat,&blockSize);
    if(blockSize!=M.getNumberOfRows() || blockSize!=M.getNumberOfColumns())
        throw CdmathException("SparseMatrixPetsc::addValuesBlocked : matrix size is different from sparse matrix block structure");
    double petscValues[blockSize*blockSize];
    for (int k=0; k<M.getNumberOfRows(); k++)
        for (int l=0; l<M.getNumberOfColumns(); l++)
            petscValues[k*blockSize+l]=M(k,l);
    MatSetValuesBlocked(_mat,1, &i, 1, &j, petscValues, ADD_VALUES);
}

//----------------------------------------------------------------------
double
SparseMatrixPetsc::operator()( int i, int j ) const
//----------------------------------------------------------------------
{
	double res;
	int idxm=i,idxn=j;
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);

	MatGetValues(_mat,1,&idxm,1, &idxn,&res);
	return res;
}

//----------------------------------------------------------------------
SparseMatrixPetsc::~SparseMatrixPetsc()
//----------------------------------------------------------------------
{
	if(&_mat != NULL)
		MatDestroy(&_mat);
	//PetscFinalize();
}

Mat
SparseMatrixPetsc::getPetscMatrix() const
{
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);

	return (_mat);
}

bool
SparseMatrixPetsc::containsPetscMatrix() const
{
	return true;
}
//----------------------------------------------------------------------
const SparseMatrixPetsc&
SparseMatrixPetsc::operator= ( const SparseMatrixPetsc& matrix )
//----------------------------------------------------------------------
{
	_isSparseMatrix=matrix.isSparseMatrix();
	MatDuplicate(matrix.getPetscMatrix(), MAT_COPY_VALUES,&_mat);
	MatGetSize(_mat,&_numberOfRows,&_numberOfColumns);	
	//extract an upper bound for the total number of non zero coefficients
	MatInfo info;
	MatGetInfo(_mat,MAT_LOCAL,&info);
	_numberOfNonZeros = info.nz_allocated;
	return (*this);
}

SparseMatrixPetsc
operator+ (const SparseMatrixPetsc& matrix1, const SparseMatrixPetsc& matrix2)
{
	int numberOfRows = matrix1.getNumberOfRows();
	int numberOfColumns = matrix1.getNumberOfColumns();
	int numberOfRows2 = matrix2.getNumberOfRows();
	int numberOfColumns2 = matrix2.getNumberOfColumns();

	if(numberOfRows2!=numberOfRows || numberOfColumns2!=numberOfColumns)
	{
		string msg="SparseMatrixPetsc::operator()+(const SparseMatrixPetsc& matrix1, const SparseMatrixPetsc& matrix2): number of rows or columns of the matrices is different!";
		throw CdmathException(msg);
	}

	Mat mat1=matrix1.getPetscMatrix();
	Mat mat2=matrix2.getPetscMatrix();
	Mat mat;
	MatDuplicate(mat1, MAT_COPY_VALUES,&mat);
	MatAXPY(mat,1,mat2,DIFFERENT_NONZERO_PATTERN);

	return SparseMatrixPetsc(mat);
}

SparseMatrixPetsc
operator- (const SparseMatrixPetsc& matrix1, const SparseMatrixPetsc& matrix2)
{
	int numberOfRows = matrix1.getNumberOfRows();
	int numberOfColumns = matrix1.getNumberOfColumns();
	int numberOfRows2 = matrix2.getNumberOfRows();
	int numberOfColumns2 = matrix2.getNumberOfColumns();

	if(numberOfRows2!=numberOfRows || numberOfColumns2!=numberOfColumns)
	{
		string msg="SparseMatrixPetsc::operator()-(const SparseMatrixPetsc& matrix1, const SparseMatrixPetsc& matrix2): number of rows or columns of the matrices is different!";
		throw CdmathException(msg);
	}
	Mat mat1=matrix1.getPetscMatrix();
	Mat mat2=matrix2.getPetscMatrix();
	Mat mat;
	MatDuplicate(mat1, MAT_COPY_VALUES,&mat);
	MatAXPY(mat,-1,mat2,DIFFERENT_NONZERO_PATTERN);

	return SparseMatrixPetsc(mat);
}

SparseMatrixPetsc
operator* (double value , const SparseMatrixPetsc& matrix )
{
	Mat mat;
	MatDuplicate(matrix.getPetscMatrix(), MAT_COPY_VALUES,&mat);
	MatScale(mat, value);

	return SparseMatrixPetsc(mat);
}

SparseMatrixPetsc
operator* (const SparseMatrixPetsc& matrix, double value )
{
	Mat mat;
	MatDuplicate(matrix.getPetscMatrix(), MAT_COPY_VALUES,&mat);
	MatScale(mat, value);

	return SparseMatrixPetsc(mat);
}

SparseMatrixPetsc
operator/ (const SparseMatrixPetsc& matrix, double value)
{
	if(value==0.)
	{
		string msg="SparseMatrixPetsc SparseMatrixPetsc::operator()/(const SparseMatrixPetsc& matrix1, const SparseMatrixPetsc& matrix2): division by zero";
		throw CdmathException(msg);
	}
	Mat mat;
	MatDuplicate(matrix.getPetscMatrix(), MAT_COPY_VALUES,&mat);
	MatScale(mat, 1/value);

	return SparseMatrixPetsc(mat);
}

SparseMatrixPetsc
operator*(const SparseMatrixPetsc& matrix1, const SparseMatrixPetsc& matrix2)
{
	Mat mat1=matrix1.getPetscMatrix();
	Mat mat2=matrix2.getPetscMatrix();
	Mat mat;
	MatMatMult(mat1, mat2, MAT_INITIAL_MATRIX,PETSC_DEFAULT,&mat);

	return SparseMatrixPetsc(mat);
}

SparseMatrixPetsc&
SparseMatrixPetsc::operator*= (const SparseMatrixPetsc& matrix)
{
	Mat mat1=matrix.getPetscMatrix();
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);

	Mat mat2;
	MatMatMult(_mat, mat1, MAT_INITIAL_MATRIX,PETSC_DEFAULT,&mat2);

	MatDestroy(&_mat);
	_mat=mat2;
	MatGetSize(_mat,&_numberOfRows,&_numberOfColumns);	
	//extract an upper bound for the total number of non zero coefficients
	MatInfo info;
	MatGetInfo(_mat,MAT_LOCAL,&info);
	_numberOfNonZeros = info.nz_allocated;
	return (*this);
}

Vector
SparseMatrixPetsc::operator* (const Vector& vec) const
{
	int numberOfRows=vec.getNumberOfRows();
	Vec X=vectorToVec(vec);
	Vec Y;
	VecDuplicate (X,&Y);
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);
	MatMult(_mat,X,Y);

	return vecToVector(Y);
}

SparseMatrixPetsc&
SparseMatrixPetsc::operator+= (const SparseMatrixPetsc& matrix)
{
	Mat mat1=matrix.getPetscMatrix();
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);
	MatAXPY(_mat,1,mat1,DIFFERENT_NONZERO_PATTERN);

	//extract an upper bound for the total number of non zero coefficients
	MatInfo info;
	MatGetInfo(_mat,MAT_LOCAL,&info);
	_numberOfNonZeros = info.nz_allocated;

	return (*this);
}

SparseMatrixPetsc&
SparseMatrixPetsc::operator-= (const SparseMatrixPetsc& matrix)
{
	Mat mat1=matrix.getPetscMatrix();
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);
	MatAXPY(_mat,-1,mat1,DIFFERENT_NONZERO_PATTERN);

	//extract an upper bound for the total number of non zero coefficients
	MatInfo info;
	MatGetInfo(_mat,MAT_LOCAL,&info);
	_numberOfNonZeros = info.nz_allocated;

	return (*this);
}

SparseMatrixPetsc&
SparseMatrixPetsc::operator*= (double value)
{
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);
	MatScale(_mat, value);
	return (*this);
}

SparseMatrixPetsc&
SparseMatrixPetsc::operator/= (double value)
{
	if(value==0.)
	{
		string msg="SparseMatrixPetsc SparseMatrixPetsc::operator()/=(const SparseMatrixPetsc& matrix1, const SparseMatrixPetsc& matrix2): division by zero";
		throw CdmathException(msg);
	}
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);
	MatScale(_mat, 1/value);
	return (*this);
}

void
SparseMatrixPetsc::viewMatrix() const 
{
    MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);

	MatView(_mat,PETSC_VIEWER_STDOUT_SELF);
}
double
SparseMatrixPetsc::getMatrixCoeff(int i, int j) const 
{
	double res;
	int idxm=i,idxn=j;
	MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);

	MatGetValues(_mat,1,&idxm,1, &idxn,&res);
	return res;
}

void 
SparseMatrixPetsc::diagonalShift(double lambda)
{
    MatAssemblyBegin(_mat, MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(_mat, MAT_FINAL_ASSEMBLY);

    MatShift(_mat, lambda);
}

void 
SparseMatrixPetsc::zeroEntries()
{
    MatZeroEntries(_mat);
}

Vector
SparseMatrixPetsc::vecToVector(const Vec& vec) const
{
	PetscInt numberOfRows;
	VecGetSize(vec,&numberOfRows);
    double * petscValues;
    VecGetArray(vec,&petscValues);
    
    DoubleTab values (numberOfRows,petscValues);
	Vector result(numberOfRows);
    result.setValues(values);

	return result;
}
Vec
SparseMatrixPetsc::vectorToVec(const Vector& myVector) const
{
	int numberOfRows=myVector.getNumberOfRows();
	const double* values = myVector.getValues().getValues();
	
	Vec result;
	VecCreate(PETSC_COMM_WORLD,&result);
	VecSetSizes(result,PETSC_DECIDE,numberOfRows);
	VecSetBlockSize(result,numberOfRows);
	VecSetFromOptions(result);
	int idx=0;//Index where to add the block of values
	VecSetValuesBlocked(result,1,&idx,values,INSERT_VALUES);

	VecAssemblyBegin(result);
	VecAssemblyEnd(result);

	return result;
}

int 
SparseMatrixPetsc::computeSpectrum(int nev, double ** valP, double ***vecP, double tol)
{
  EPS            eps;         /* eigenproblem solver context */
  EPSType        type;
  PetscReal      error;
  PetscScalar    kr,ki;
  Vec            xr,xi;
  PetscInt       i,maxit,its, nconv;

  if(!isSymmetric(tol))
    {
    cout<<"SparseMatrixPetsc::getEigenvectors() : matrix is not symmetric, tolerance= "<< tol<<endl;
    throw CdmathException("SparseMatrixPetsc::getEigenvectors : Matrix should be symmetric for eigenvalues computation");
    }

  SlepcInitialize(0, (char ***)"", PETSC_NULL, PETSC_NULL);

  MatAssemblyBegin(_mat,MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd(_mat,MAT_FINAL_ASSEMBLY);

  MatCreateVecs(_mat,NULL,&xr);
  MatCreateVecs(_mat,NULL,&xi);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                Create the eigensolver and set various options
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /*
     Create eigensolver context
  */
  EPSCreate(PETSC_COMM_WORLD,&eps);

  /*
     Set operators. In this case, it is a standard eigenvalue problem
  */
  EPSSetOperators(eps,_mat,NULL);
  EPSSetProblemType(eps,EPS_HEP);
  EPSSetWhichEigenpairs(eps,EPS_SMALLEST_MAGNITUDE);//Or EPS_SMALLEST_REAL ?
  EPSSetDimensions(eps,nev,PETSC_DEFAULT,PETSC_DEFAULT);

  /*
     Set solver parameters at runtime
  */
  EPSSetFromOptions(eps);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                      Solve the eigensystem
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  EPSSolve(eps);
  /*
     Optional: Get some information from the solver and display it
  */
  EPSGetIterationNumber(eps,&its);
  PetscPrintf(PETSC_COMM_WORLD," Number of iterations of the method: %D\n",its);
  EPSGetType(eps,&type);
  PetscPrintf(PETSC_COMM_WORLD," Solution method: %s\n\n",type);
  EPSGetDimensions(eps,&nev,NULL,NULL);
  PetscPrintf(PETSC_COMM_WORLD," Number of requested eigenvalues: %D\n",nev);
  EPSGetTolerances(eps,&tol,&maxit);
  PetscPrintf(PETSC_COMM_WORLD," Stopping condition: tol=%.4g, maxit=%D\n",(double)tol,maxit);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                    Display solution and clean up
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /*
     Get number of converged approximate eigenpairs
  */
  EPSGetConverged(eps,&nconv);
  PetscPrintf(PETSC_COMM_WORLD," Number of converged eigenpairs: %D\n\n",nconv);

    *valP=new double[nconv];
    *vecP=new double * [nconv];
    double * myVecp;
    
  if (nconv>0) {
    /*
       Display eigenvalues and relative errors
    */
    PetscPrintf(PETSC_COMM_WORLD,
         "           k          ||Ax-kx||/||kx||\n"
         "   ----------------- ------------------\n");

    for (int i=0;i<nconv;i++) {
      /*
        Get converged eigenpairs: i-th eigenvalue is stored in kr (real part) and
        ki (imaginary part)
      */
      EPSGetEigenpair(eps,i,&kr,&ki,xr,xi);
      /*
         Compute the relative error associated to each eigenpair
      */
      EPSComputeError(eps,i,EPS_ERROR_RELATIVE,&error);

      if (ki!=0.0) {
        PetscPrintf(PETSC_COMM_WORLD," %9f%+9fi %12g\n",(double)kr,(double)ki,(double)error);
      } else {
        PetscPrintf(PETSC_COMM_WORLD,"   %12f       %12g\n",(double)kr,(double)error);
      }
      *(*valP + i)=kr;
      VecGetArray(xr,&myVecp);
      *(*vecP+  i)=new double [_numberOfRows];
      memcpy(*(*vecP+  i),myVecp,_numberOfRows*sizeof(double)) ;
    }
    PetscPrintf(PETSC_COMM_WORLD,"\n");
    /*
     Free work space
    */
    EPSDestroy(&eps);
    VecDestroy(&xr);
    VecDestroy(&xi);
    SlepcFinalize();

    return nconv;
  }
  else
  {
	/*
     Free work space
    */
    EPSDestroy(&eps);
    VecDestroy(&xr);
    VecDestroy(&xi);
    SlepcFinalize();

    throw CdmathException("SparseMatrixPetsc::getEigenvectors : No eigenvector found");	
  }	
}

std::vector< double > 
SparseMatrixPetsc::getEigenvalues(int nev, double tol)
{
	int nconv;
	double * valP;
	double **vecP;

	nconv=computeSpectrum(nev, &valP, &vecP, tol);
	
    std::vector< double > result(nconv);
	
    for (int i=0;i<nconv;i++) 
        result[i]=valP[i];

	delete valP;
    for (int i=0;i<nconv;i++) 
		delete vecP[i];
	delete vecP;	
	
    return result;
}

std::vector< Vector > 
SparseMatrixPetsc::getEigenvectors(int nev, double tol)
{
	int nconv;
	double * valP;
	double **vecP;

	nconv=computeSpectrum(nev, &valP, &vecP, tol);
	
    std::vector< Vector > result(nconv);

    for (int i=0;i<nconv;i++) 
    {
		DoubleTab values (_numberOfRows,vecP[i]);
        Vector myVecP(_numberOfRows);
        myVecP.setValues(values);
        result[i]=myVecP;
	}

	delete valP;
    for (int i=0;i<nconv;i++) 
		delete vecP[i];
	delete vecP;	
	
    return result;
}

MEDCoupling::DataArrayDouble *
SparseMatrixPetsc::getEigenvectorsDataArrayDouble(int nev, double tol)
{
	int nconv;
	double * valP;
	double **vecP;

	nconv=computeSpectrum(nev, &valP, &vecP, tol);
	
	std::vector< int > compoId(1);
	MEDCoupling::DataArrayDouble *arrays=MEDCoupling::DataArrayDouble::New();
	MEDCoupling::DataArrayDouble *array =MEDCoupling::DataArrayDouble::New();
	arrays->alloc(_numberOfRows,nconv);	
	
    for (int i=0;i<nconv;i++) 
    {
		array->useArray(vecP[i],true, MEDCoupling::DeallocType::CPP_DEALLOC, _numberOfRows,1);
		compoId[0]=i;
		arrays->setSelectedComponents(array,compoId);
		arrays->setInfoOnComponent(i,std::to_string(valP[i]));
	}
	delete valP;
    for (int i=0;i<nconv;i++) 
		delete vecP[i];
	delete vecP;	
	
    return arrays;
}
bool SparseMatrixPetsc::isSymmetric(double tol) const
{
	//Check that the matrix is symmetric
	PetscBool isSymetric;
	MatIsSymmetric(_mat,tol,&isSymetric);

	return isSymetric;
}
