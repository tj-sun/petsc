!
!  $Id: pc.h,v 1.27 1999/01/27 19:50:07 bsmith Exp bsmith $;
!
!  Include file for Fortran use of the PC (preconditioner) package in PETSc
!
#define PC           PetscFortranAddr
#define PCNullSpace  PetscFortranAddr
#define PCSide       integer
#define PCASMType    integer
#define PCType       character*(80)
!
!  Various preconditioners
!
#define PCNONE      'none'
#define PCJACOBI    'jacobi'
#define PCSOR       'sor'
#define PCLU        'lu'
#define PCSHELL     'shell'
#define PCBJACOBI   'bjacobi'
#define PCMG        'mg'
#define PCEISENSTAT 'eisenstat'
#define PCILU       'ilu'
#define PCICC       'icc'
#define PCASM       'asm'
#define PCSLES      'sles'
#define PCCOMPOSITE 'composite'

!
!  PCSide
!
      integer PC_LEFT, PC_RIGHT, PC_SYMMETRIC 

      parameter (PC_LEFT=0, PC_RIGHT=1, PC_SYMMETRIC=2)

      integer USE_PRECONDITIONER_MATRIX, USE_TRUE_MATRIX
      parameter (USE_PRECONDITIONER_MATRIX=0, USE_TRUE_MATRIX=1)

!
! PCASMType
!
      integer PC_ASM_BASIC, PC_ASM_RESTRICT, PC_ASM_INTERPOLATE
      integer PC_ASM_NONE

      parameter (PC_ASM_BASIC = 3, PC_ASM_RESTRICT = 1)
      parameter (PC_ASM_INTERPOLATE = 2,PC_ASM_NONE = 0)
!
!  End of Fortran include file for the PC package in PETSc

