
static char help[] = "Tests MatHYPRE\n";

#include <petscmat.h>
#include <petsc/private/matimpl.h>
/* there's no getter API to extract the ParCSR yet */
#include <../src/mat/impls/hypre/mhypre.h>
#include <_hypre_IJ_mv.h>
#include <../src/mat/impls/aij/mpi/mpiaij.h>

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **args)
{
  Mat            A,B,C,D;
  Vec            x,x2,y,y2;
  PetscReal      err;
  PetscInt       i,j,N = 6, M = 6;
  PetscErrorCode ierr;
  PetscBool      flg;
  char           file[256];

  ierr = PetscInitialize(&argc,&args,(char*)0,help);if (ierr) return ierr;
  ierr = PetscOptionsGetString(NULL,NULL,"-f",file,256,&flg);CHKERRQ(ierr);
  ierr = MatCreate(PETSC_COMM_WORLD,&A);CHKERRQ(ierr);
  if (!flg) { /* Create a matrix and test MatSetValues */
    ierr = MatCreate(PETSC_COMM_WORLD,&B);CHKERRQ(ierr);
    ierr = PetscOptionsGetInt(NULL,NULL,"-M",&M,NULL);CHKERRQ(ierr);
    ierr = PetscOptionsGetInt(NULL,NULL,"-N",&N,NULL);CHKERRQ(ierr);
    if (N < 6) SETERRQ(PETSC_COMM_WORLD,PETSC_ERR_SUP,"Matrix has to have more than 6 columns");
    ierr = MatSetSizes(A,PETSC_DECIDE,PETSC_DECIDE,M,N);CHKERRQ(ierr);
    ierr = MatSetType(A,MATAIJ);CHKERRQ(ierr);
    ierr = MatSeqAIJSetPreallocation(A,9,NULL);CHKERRQ(ierr);
    ierr = MatMPIAIJSetPreallocation(A,9,NULL,9,NULL);CHKERRQ(ierr);
    ierr = MatSetSizes(B,PETSC_DECIDE,PETSC_DECIDE,M,N);CHKERRQ(ierr);
    ierr = MatSetType(B,MATHYPRE);CHKERRQ(ierr);
    ierr = MatHYPRESetPreallocation(B,9,NULL,9,NULL);CHKERRQ(ierr);
    for (i=0; i<M; i++) {
      PetscInt    cols[] = {0,1,2,3,4,5};
      PetscScalar vals[] = {0,1./2.,2./3.,3./4.,4./.5,5./6.};
      for (j=i-2; j<i+1; j++) {
        if (j >= N) {
          ierr = MatSetValue(A,i,N-1,(1.*j*N+i)/(3.*N*N),ADD_VALUES);CHKERRQ(ierr);
          ierr = MatSetValue(B,i,N-1,(1.*j*N+i)/(3.*N*N),ADD_VALUES);CHKERRQ(ierr);
        } else if (i > j) {
          ierr = MatSetValue(A,i,j,(1.*j*N+i)/(2.*N*N),ADD_VALUES);CHKERRQ(ierr);
          ierr = MatSetValue(B,i,j,(1.*j*N+i)/(2.*N*N),ADD_VALUES);CHKERRQ(ierr);
        } else {
          ierr = MatSetValue(A,i,j,-1.-(1.*j*N+i)/(4.*N*N),ADD_VALUES);CHKERRQ(ierr);
          ierr = MatSetValue(B,i,j,-1.-(1.*j*N+i)/(4.*N*N),ADD_VALUES);CHKERRQ(ierr);
        }
      }
      ierr = MatSetValues(A,1,&i,6,cols,vals,ADD_VALUES);CHKERRQ(ierr);
      ierr = MatSetValues(B,1,&i,6,cols,vals,ADD_VALUES);CHKERRQ(ierr);
    }
    /* MAT_FLUSH_ASSEMBLY currently not supported */
    if (0) { /* HYPRE_IJMatrix does not support INSERT_VALUES with off-proc entries */
      PetscInt    cols[2],rows[2];
      PetscScalar vals[] = {0,0,0,0};
      ierr = MatAssemblyBegin(A,MAT_FLUSH_ASSEMBLY);CHKERRQ(ierr);
      ierr = MatAssemblyEnd(A,MAT_FLUSH_ASSEMBLY);CHKERRQ(ierr);
      ierr = MatAssemblyBegin(B,MAT_FLUSH_ASSEMBLY);CHKERRQ(ierr);
      ierr = MatAssemblyEnd(B,MAT_FLUSH_ASSEMBLY);CHKERRQ(ierr);
      ierr = MatGetOwnershipRange(A,&rows[0],&rows[1]);CHKERRQ(ierr);
      ierr = MatGetOwnershipRangeColumn(A,&cols[0],&cols[1]);CHKERRQ(ierr);
      if (rows[1] > rows[0]) rows[1] = rows[1]-1;
      else rows[1] = rows[0];
      if (cols[1] > cols[0]) cols[1] = cols[1]-1;
      else cols[1] = cols[0];
      ierr = MatSetValues(B,2,rows,2,cols,vals,INSERT_VALUES);CHKERRQ(ierr);
      ierr = MatSetValues(A,2,rows,2,cols,vals,INSERT_VALUES);CHKERRQ(ierr);
    }
    ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
    ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
    ierr = MatAssemblyBegin(B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
    ierr = MatAssemblyEnd(B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

    /* MatAXPY_Basic further exercises MatSetValues_HYPRE */
    ierr = MatAXPY(B,-1.,A,SAME_NONZERO_PATTERN);CHKERRQ(ierr);
    ierr = MatConvert(B,MATAIJ,MAT_INITIAL_MATRIX,&C);CHKERRQ(ierr);
    ierr = MatNorm(C,NORM_INFINITY,&err);CHKERRQ(ierr);
    if (err > PETSC_SMALL) SETERRQ1(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error MatSetValues %g",err);
    ierr = MatDestroy(&B);CHKERRQ(ierr);
    ierr = MatDestroy(&C);CHKERRQ(ierr);
  } else {
    PetscViewer viewer;
    ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,file,FILE_MODE_READ,&viewer);CHKERRQ(ierr);
    ierr = MatSetFromOptions(A);CHKERRQ(ierr);
    ierr = MatLoad(A,viewer);CHKERRQ(ierr);
    ierr = PetscViewerDestroy(&viewer);CHKERRQ(ierr);
    ierr = MatGetSize(A,&M,&N);CHKERRQ(ierr);
  }
  /* check conversion routines */
  ierr = MatConvert(A,MATHYPRE,MAT_INITIAL_MATRIX,&B);CHKERRQ(ierr);
  ierr = MatConvert(A,MATHYPRE,MAT_REUSE_MATRIX,&B);CHKERRQ(ierr);
  ierr = MatConvert(B,MATIS,MAT_INITIAL_MATRIX,&D);CHKERRQ(ierr);
  ierr = MatConvert(B,MATIS,MAT_REUSE_MATRIX,&D);CHKERRQ(ierr);
  ierr = MatConvert(B,MATAIJ,MAT_INITIAL_MATRIX,&C);CHKERRQ(ierr);
  ierr = MatConvert(B,MATAIJ,MAT_REUSE_MATRIX,&C);CHKERRQ(ierr);
  ierr = MatAXPY(C,-1.,A,SAME_NONZERO_PATTERN);CHKERRQ(ierr);
  ierr = MatNorm(C,NORM_INFINITY,&err);CHKERRQ(ierr);
  if (err > PETSC_SMALL) SETERRQ1(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error Mat AIJ %g",err);
  ierr = MatDestroy(&C);CHKERRQ(ierr);
  ierr = MatISGetMPIXAIJ(D,MAT_INITIAL_MATRIX,&C);CHKERRQ(ierr);
  ierr = MatAXPY(C,-1.,A,SAME_NONZERO_PATTERN);CHKERRQ(ierr);
  ierr = MatNorm(C,NORM_INFINITY,&err);CHKERRQ(ierr);
  if (err > PETSC_SMALL) SETERRQ1(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error Mat IS %g",err);
  ierr = MatDestroy(&C);CHKERRQ(ierr);
  ierr = MatDestroy(&D);CHKERRQ(ierr);

  /* check MatCreateFromParCSR */
  {
    Mat_HYPRE *hB = (Mat_HYPRE*)(B->data);
    hypre_ParCSRMatrix *parcsr = (hypre_ParCSRMatrix*)hypre_IJMatrixObject(hB->ij);
    ierr = MatCreateFromParCSR(parcsr,MATAIJ,PETSC_COPY_VALUES,&D);CHKERRQ(ierr);
    ierr = MatDestroy(&D);CHKERRQ(ierr);
    ierr = MatCreateFromParCSR(parcsr,MATHYPRE,PETSC_USE_POINTER,&C);CHKERRQ(ierr);
  }

  /* check matmult */
  ierr = MatCreateVecs(A,&x,&y);CHKERRQ(ierr);
  ierr = MatCreateVecs(A,&x2,&y2);CHKERRQ(ierr);
  ierr = VecSet(x,1.);CHKERRQ(ierr);
  ierr = MatMult(A,x,y);CHKERRQ(ierr);
  ierr = MatMult(B,x,y2);CHKERRQ(ierr);
  ierr = VecAXPY(y,-1.,y2);CHKERRQ(ierr);
  ierr = VecNorm(y,NORM_2,&err);CHKERRQ(ierr);
  if (err > PETSC_SMALL) SETERRQ1(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error MatMult B %g",err);
  ierr = MatMult(C,x,y);CHKERRQ(ierr);
  ierr = VecAXPY(y,-1.,y2);CHKERRQ(ierr);
  ierr = VecNorm(y,NORM_2,&err);CHKERRQ(ierr);
  if (err > PETSC_SMALL) SETERRQ1(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error MatMult C %g",err);
  ierr = VecSet(y,1.);CHKERRQ(ierr);
  ierr = MatMultTranspose(A,y,x);CHKERRQ(ierr);
  ierr = MatMultTranspose(B,y,x2);CHKERRQ(ierr);
  ierr = VecAXPY(x,-1.,x2);CHKERRQ(ierr);
  ierr = VecNorm(x,NORM_2,&err);CHKERRQ(ierr);
  if (err > PETSC_SMALL) SETERRQ1(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error MatMultTranspose C %g",err);
  ierr = MatMultTranspose(C,y,x);CHKERRQ(ierr);
  ierr = VecAXPY(x,-1.,x2);CHKERRQ(ierr);
  ierr = VecNorm(x,NORM_2,&err);CHKERRQ(ierr);
  if (err > PETSC_SMALL) SETERRQ1(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error MatMultTranspose C %g",err);

  /* check PtAP */
  if (M == N) {
    Mat       pP,hP;
    PetscReal norm;

    /* PETSc PtAP */
    ierr = MatPtAP(A,A,MAT_INITIAL_MATRIX,PETSC_DEFAULT,&pP);CHKERRQ(ierr);
    ierr = MatNorm(pP,NORM_INFINITY,&norm);CHKERRQ(ierr);

    /* MatPtAP_HYPRE_HYPRE */
    ierr = MatPtAP(C,B,MAT_INITIAL_MATRIX,PETSC_DEFAULT,&hP);CHKERRQ(ierr);
    ierr = MatConvert(hP,MATAIJ,MAT_INITIAL_MATRIX,&D);CHKERRQ(ierr);
    ierr = MatAXPY(D,-1.,pP,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);
    ierr = MatNorm(D,NORM_INFINITY,&err);CHKERRQ(ierr);
    if (err/norm > PETSC_SMALL) SETERRQ2(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error MatPtAP %g %g",err,norm);
    ierr = MatDestroy(&hP);CHKERRQ(ierr);
    ierr = MatDestroy(&D);CHKERRQ(ierr);

    /* MatPtAP_AIJ_HYPRE */
    ierr = MatPtAP(A,B,MAT_INITIAL_MATRIX,PETSC_DEFAULT,&hP);CHKERRQ(ierr);
    ierr = MatConvert(hP,MATAIJ,MAT_INITIAL_MATRIX,&D);CHKERRQ(ierr);
    ierr = MatAXPY(D,-1.,pP,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);
    ierr = MatNorm(D,NORM_INFINITY,&err);CHKERRQ(ierr);
    if (err/norm > PETSC_SMALL) SETERRQ2(PetscObjectComm((PetscObject)A),PETSC_ERR_PLIB,"Error MatPtAP mixed %g %g",err,norm);
    ierr = MatDestroy(&D);CHKERRQ(ierr);
    ierr = MatDestroy(&hP);CHKERRQ(ierr);

    ierr = MatDestroy(&pP);CHKERRQ(ierr);
  }

  ierr = VecDestroy(&x);CHKERRQ(ierr);
  ierr = VecDestroy(&x2);CHKERRQ(ierr);
  ierr = VecDestroy(&y);CHKERRQ(ierr);
  ierr = VecDestroy(&y2);CHKERRQ(ierr);
  ierr = MatDestroy(&A);CHKERRQ(ierr);
  ierr = MatDestroy(&B);CHKERRQ(ierr);
  ierr = MatDestroy(&C);CHKERRQ(ierr);

  ierr = PetscFinalize();
  return ierr;
}
