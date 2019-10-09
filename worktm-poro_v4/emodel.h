!  EMODEL.H - ELASTIC MODEL SPECIFIC CONTROL DATA

!********************************************************************
      INCLUDE  'moddefs.h'

      REAL*8   PEPVTOL
      INTEGER  PEFLOW,NERRC,DISPOUT,MECH_BC_NCASE
      LOGICAL  PROPOUT,STRSOUT,STRNOUT,ESTRSOUT,BODYFORCE_INPUT,               &
     &         PSTRNOUT,PSTATEOUT,NO_ELASTIC_NEWTON

      LOGICAL  SDPM,PLOT_PERM_CHANGE
      INTEGER*4  TYPESDP
      REAL*8  COEFB,COEFM,COEFN,RESIDP,RESIDO

      COMMON /ECONTRL/                                                         &
     &         PEPVTOL,                                                        &
     &         PEFLOW,NERRC,DISPOUT,                                           &
     &         PROPOUT,STRSOUT,STRNOUT,ESTRSOUT,BODYFORCE_INPUT,               &
     &         PSTRNOUT,PSTATEOUT,                                             &
     &         SDPM,PLOT_PERM_CHANGE,TYPESDP,MECH_BC_NCASE,                    &
     &         NO_ELASTIC_NEWTON

      INTEGER TOTAL_CRACKED_FACE
      LOGICAL COUPLE_FLAG,GRAVITY_FLAG,INITIAL_FLAG

      COMMON /POROHEX/                                                         &
     &    TOTAL_CRACKED_FACE,                                                  &
     &    COUPLE_FLAG,GRAVITY_FLAG,INITIAL_FLAG
!
!RUIJIE
!
      INTEGER EP_CUTBACK_FLAG, MAX_ITERATION_GL, MAX_ITERATION_LOC,            &
     &        MODEL_EP,EP_SOLVER_FLAG,NLOADSTEPS,CR_TYPE
      REAL*8  EP_TOL,MAT_TOL
      COMMON /EPSOLVER/                                                        &
     &      EP_CUTBACK_FLAG, MAX_ITERATION_GL, MAX_ITERATION_LOC,              &
     &      MODEL_EP,EP_TOL,MAT_TOL,EP_SOLVER_FLAG,NLOADSTEPS,                 &
     &      CR_TYPE

      COMMON /EMODEL/                                                          &
     &         COEFB,COEFM,COEFN,RESIDP,RESIDO

! Variables for Mandel problem
      REAL*8 PRESVAL,STRSSYY,UXVAL,UYVAL,EPSXX,EPSYY,FINERR,                   &
     &       SKEMPTON,MANDEL_FORCE,MANDEL_NUU,MANDEL_NU,MANDEL_E,              &
     &       MANDEL_PERM,MANDEL_XSIZE,MANDEL_YSIZE,MANDEL_MU
      COMMON /MANDEL/PRESVAL,STRSSYY,UXVAL,UYVAL,EPSXX,EPSYY,FINERR,           &
     &       SKEMPTON,MANDEL_FORCE,MANDEL_NUU,MANDEL_NU,MANDEL_E,              &
     &       MANDEL_PERM,MANDEL_XSIZE,MANDEL_YSIZE,MANDEL_MU

!**********************************************************************
! MODEL_EP         = ELASTOPLASTIC MODEL
!                    0: LINEAR ELASTICITY MODEL
!                    1: DRUCKER-PRAGER PLASTICITY MODEL
! CONV_RESVOL      = UNIT CONVERSION FACTOR FOR RESERVOIR VOLUME
! CONV_INCHES      = UNIT CONVERSION FACTOR FOR DISPLACEMENTS
! NDIM_ELASTIC     = NUMBER OF DEGREES OF FREEDOM PER DIPLACEMENT NODE
! NERRC            = ERROR FLAG PASSED FROM WORK ROUTINES
! DISPOUT          = FLAG FOR DISPLACEMENT OUTPUT
!                  = 1 AVERAGE DISPLACEMENT AT ELEMETN CENTER
!                  = 2 AVERAGE DISPLACEMENT AT ELEMENT TOP (DEFAULT)
!                  = 3 AVERAGE DISPLACEMENT AT ELEMENT BOTTOM
! PVKONVG          = FLAG OF CONVERGENCE IN ITERMS OF PORE VOLUE ERROR
!                  = 1 CONVERGE
!                  = 2 NOT CONVERGE
! PROPOUT          = FLAG OF ELASTIC PROPERTY OUTPUT
!                  = .TRUE. OUTPUT ELASTIC PROPERTIES AT ELEMENT CENTERS
! STRSOUT          = FLAG OF TOTAL STRESS OUTPUT
!                  = .TRUE. OUTPUT TOTAL STRESS AT ELEMENT CENTERS
! STRNOUT          = FLAG OF STRAIN OUTPUT
!                  = .TRUE. OUTPUT STRAIN VALUES AT ELEMENT CENTERS
! PSTRNOUT         = FLAG OF PLASTIC STRAIN OUTPUT
!                  = .TRUE. OUTPUT ELEMENT CENTERS
! PSTATEOUT        = FLAG OF PLASTIC RELATED STATE VARAIABLE OUTPUT
!                  = .TRUE. OUTPUT ELEMENT CENTERS
! STINIT           = INITIAL PRINCIPLE STRESSES AT A GIVEN DEPTH (PSI)
! STDEPTH          = INITIAL PRINCIPLE STRESS DEPTH (FT)
! STGRAD           = INITIAL PRINCIPLE STRESS GRADIENTS (PSI/FT)
! INIT_STRESS      = FLAG OF INITIAL STRESS INPUT TYPE
!                  = 1 STRESS AT A GIVEN DEPTH
!                  = 2 GRID ELEMENT ARRAY INPUT
! NBAND            = BANDWIDTH OF ELASTIC STIFFNESS MATRIX
! NDIM_BEFORE      = NUMBER OF STENCIL POINTS FOR THE LOWER TRIANGULAR PART OF
!                    THE STIFFNESS MATRIX
! INTEGRATION_NODES = NUMBER OF INTEGRATION NODES IN EACH ELEMENT
! NDIM_PCONN        = NUMBER OF INTEGRATION NODES FOR PRESSURE AND
!                     VOLUMETRIC STRAIN
! NODES_PER_ELEMENT = NUMBER OF DISPLACEMENT NODES PER ELEMENT
! IOFFSET           = OFFSET VECTOR FOR GLOBAL STIFFNESS.
! MXBAND            = MAXIMUM BANDWIDTH  OF GLOBAL STIFFNESS MATRIX
! IMAP              = MAPPINGS FROM LOCAL ELEMENT TO GLOBAL STIFFNESS MATRIX
! IESOLVE           = LINEAR SOLVER TYPE FOR ELASTICITY SYSTEM
!                   = 0 DIRECTOR SOLVER (DEFAULT)
!                   = 1 PCG
!                   = 2 BICGS
!                   = 3 LSOR
!                   = 4 CG
! EP_CUTBACK_FLAG   = TIME STEP CUTTING BACK FLAG 0: NO CUTTING BACK >0: CUTBACK
! MAX_ITERATION_GL  = MAXIMUM ITERATION NUMBER FOR SOLID
! MAX_ITERATION_LOC = MAXIMU ITERATION NUMBER FOR MATERIAL INTEGRATOR
! MAT_GPT_TOL       = TOLERANCE CONTROL FOR MATERIAL INTEGRATOR
! SOLVER_SOLID_TOL  = SOLID SOLVER TOLERANCE
! EP_SOLVER_FLAG    = ELASTO-PLATICITY SOLVER FLAG
!                     0: ITERATIVE SOLVER (HYPER DEFAULT)
!                     1: SUPER_LU
! NLOADSTEPS        = NUMBER OF LOADING STEPS
! CR_TYPE           = TYPE OF ROCK COMPRESSIBILITY CALCULATION
!                     0 : CONSTANT FROM INPUT FILE
!                     1 : DISP_COMP/EPV_FLOW (see EPROP and EFLOWCR)
! NO_ELASTIC_NEWTON = FLAG TO CHANGE NEWTON BEHAVIOR WHEN MODEL_EP=0
!                     F : TAKE >=1 STEPS TO CONVERGE NEWTON BELOW EP_TOL
!                     T : ALWAYS TAKE ONLY 1 NEWTON STEP
! MECH_BC_NCASE     = TIME-DEPENDENT MECHANICS BOUNDARY CONDITIONS
!                     0: NONE
!                     OTHERWISE, CALLS CHANGE_MECH_BC BEFORE PRE_PRCSS2
