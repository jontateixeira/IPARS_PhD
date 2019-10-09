C  TBALDAT.H -  IMPLICIT SINGLE PHASE FLOW MODEL BALANCE DATA

C  CODE HISTORY:

C  BAHAREH MOMKEN 02/02/99  Hydroloy-IMPES gbaldat.dh is used as template
C  JOHN WHEELER   04/03/99  IMPLICIT SINGLE PHASE MODEL

C*********************************************************************
      INTEGER TREADIN,TMODEL
      INTEGER IRM1,JRM1,KRM1,
ctm
     & COMPUTE_NORM
ctm
      REAL*8 CURRENT,WELI,WELP,WELIPC,WELVEC,CVTOL1,CVTOL2,CVTOL,WELIS,
     & RESIDM,RESIDT,RMA1,QFLOC,ELERATE,FLUXOM,FLITNP,
ctm   TAMEEM
     & F_TOLERANCE
ctm   TAMEEM

      COMMON /TBALDAT/ CURRENT, RESIDM, RESIDT, FLUXOM, FLITNP, WELIS,
     & CVTOL1, CVTOL2, WELI(50), WELP(50),
     & WELIPC(3,50), WELVEC(6,50),RMA1,QFLOC(50),
     & ELERATE(100,50),
ctm
     & F_TOLERANCE,
ctm
     & CVTOL, IRM1, JRM1, KRM1, TREADIN, COMPUTE_NORM, TMODEL

C*********************************************************************

C  TMODEL = DENSITY TYPE (Affects TPROP, TBDPROP, TIVDAT, TWELL)
C          0 = Usual linearization with exponential density
C          1 = Linearization used for Mandel problem

C  TREADIN = FLAG TO SKIP EQUILIBRIUM CALCULATIONS IN TINIT

C  CURRENT = TOTAL FLUID CURRENTLY IN THE SYSTEM, LB (PROCESSOR 0 ONLY)

C  RESIDM = MAXIMUM RESIDUAL (ABSOLUTE VALUE)

C  RESIDT = TOTAL RESIDUAL

C  WELIS = NET FLUID INJECTED TO ALL WELLS DURING A TIME STEP, LB

C  WELI(n) = TOTAL FLUID INJECTED TO WELL n, LB

C  WELP(n) = TOTAL FLUID PRODUCED FROM WELL n, LB

C  WELIPC(m,n) = WELL INJECTION/PRODUCTION DATA FOR THE TIME STEP
C  WELIPC(1,n) = FLUID INJECTED TO WELL n, LB
C  WELIPC(2,n) = FLUID PRODUCED FROM WELL n, LB
C  WELIPC(3,n) = BOTTOM HOLE PRESSURE OF WELL n, PSI
C  WELVEC(m,n) = WELL QUANITIES USED IN SUMMATIONS.

C  QFLOC(n)= NET FLUID PRODUCTION PREVENTS REVERSE FLOW IN PRODUCTION WELLS

C  RMA1, IRM1 = MAXIMUM RESIDUAL AND ITS LOCATION
C  JRM1, KRM1

C  CVTOL = RELATIVE NEWTONIAN CONVERGENCE TOLLERANCE

C  CVTOL1 = MAX RESIDUAL CONVERGENCE TOLLERANCE

C  CVTOL2 = TOTAL RESIDUAL CONVERGENCE TOLLERANCE

C  ELERATE() = WELL RATES TO ELEMENTS

C  FLUXOM = MASS FLUX TO OTHER PHYSICAL MODELS DURING TIME STEP

C  FLITNP = TOTAL FLUID INJECTED VIA BC's AT END OF STEP, LB
