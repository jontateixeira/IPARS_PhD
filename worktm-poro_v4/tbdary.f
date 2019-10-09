C  TBDARY.F - IMPLICIT SINGLE PHASE MODEL BOUNDARY CONDITION ROUTINES

C  ROUTINES IN THIS MODULE:

C SUBROUTINE TBDINTAB
C SUBROUTINE TBDPROP
c SUBROUTINE COMPPRDEN
C SUBROUTINE TBDPROPW
c SUBROUTINE TBDTRAN
c SUBROUTINE TBDTRANW
c SUBROUTINE TBDBAL

C  CODE HISTORY:

C  M. PESZYNSKA,      1/01  INITIAL VERSION
c
c   Routines in this file are written for boundary conditions
c   of type = bound_type(region):
c    type =  1: value = PRESS (pressure) [psi]
c    type =  2: value = PRESS (pressure) [psi], used at BOUND_DEP
c    type =  3: value = PRESS (pressure) [psi], supplied by user code
c                             defined in TBDMOD(region)
c    type =  4: value = PRESHEAD [ft] : interpreted as pressure head.
c    type = -1: value = BFLUX (rate of inflow per unit area) [lb/day*sq-ft]
c
c   Other boundary conditions, if specified, are ignored.
c   Geometrical information about regions is read and processed
c   by the framework.
C*********************************************************************
      SUBROUTINE TBDINTAB(NTIME,NERR)
C*********************************************************************
c input tables with boundary values
C*********************************************************************
      IMPLICIT NONE
      INTEGER NTIME, NERR

      INCLUDE 'unitsex.h'
      INCLUDE 'boundary.h'
      INCLUDE 'control.h'
      INCLUDE 'layout.h'

      integer i,nt,nerr0

c space for getblk

      INTEGER LOCTA(60),LENTA(60)
      CHARACTER*1 BLKBUF(200000)
      CHARACTER*50 TABTIT

c ---------------------------------------------
c read in the boundary values for each region

      DO I=1,60
         LENTA(I)=0
      ENDDO
      CALL GETBLK('TBOUND ',BLKBUF,200000,NBND_REG,
     &     LOCTA,LENTA,NERR)

      DO I=1,NBND_REG
         IF (LENTA(I).GT.0) THEN

            NT = NBND_TYPE(I)

c pressure or reference pressure condition
            IF(NT.EQ.1.OR.NT.EQ.2)  THEN

               TABTIT='PRESS FOR BDARY REGION[psi]'
               CALL MAKTIT(TABTIT,50,I)
               CALL TABUNT (EXTTIME,EXTPRES)
               CALL TABLE(BLKBUF(LOCTA(I)),LENTA(I),
     &              'TIME[day] ','BPRES[psi] ',TABTIT,
     &              NTABBND(I,1),NERR)

c pressure head
            ELSE IF(NT.EQ.4)  THEN

               TABTIT='PRESS HEAD FOR BDARY REGION[ft]'
               CALL MAKTIT(TABTIT,50,I)
               CALL TABUNT (EXTTIME,EXTDIST)
               CALL TABLE(BLKBUF(LOCTA(I)),LENTA(I),
     &              'TIME[day] ','BPHEAD[ft] ',TABTIT,
     &              NTABBND(I,1),NERR)

c flux bdary condition

            ELSE IF(NT.EQ.-1)  THEN

               TABTIT='FLUX FOR BDARY REGION[lb/day sq-ft]'
               CALL MAKTIT(TABTIT,50,I)
               CALL TABUNT (EXTTIME,'[lb/day sq-ft] ')
               CALL TABLE(BLKBUF(LOCTA(I)),LENTA(I),
     &              'TIME[day] ','BFLUX[lb/day sq-ft] ',TABTIT,
     &              NTABBND(I,1),NERR)

            ENDIF
         ENDIF
      ENDDO

c read and compile program for bdary modification

      DO I=1,60
         LENTA(I)=0
      ENDDO
      CALL GETBLK('TBDMOD ',BLKBUF,200000,NBND_REG,
     &     LOCTA,LENTA,NERR)

      DO I=1,NBND_REG
         IF (LENTA(I).GT.0.AND.NBND_TYPE(I).EQ.3) THEN

            NERR0=NERR
            NBDPROG(I) = 0

            CALL BDPROG(BLKBUF(LOCTA(I)),LENTA(I),NBDPROG(I),NERR)

            IF(NERR.NE.NERR0) RETURN
         ENDIF
      ENDDO

      END

C*********************************************************************
      SUBROUTINE TBDPROP()
C*********************************************************************
c executive for TBDPROPW
C*********************************************************************
      IMPLICIT NONE
      INCLUDE 'tarydat.h'
      INCLUDE 'blkary.h'

      INTEGER IBDPROP(5)
      DATA IBDPROP / 5*0 /
      EXTERNAL TBDPROPW
      LOGICAL ONCEONLY
      DATA ONCEONLY /.TRUE./

      IF(ONCEONLY) THEN
         ONCEONLY = .FALSE.
         IBDPROP(1) = 4
         IBDPROP(2) = N_PRES
         IBDPROP(3) = N_FLDEN
         IBDPROP(4) = N_DEPTH
         IBDPROP(5) = N_UPMOBPROD
      ENDIF

      CALL CALLWORK(TBDPROPW,IBDPROP)
      END

C*********************************************************************
      SUBROUTINE TBDPROPW(IDIM,JDIM,KDIM,LDIM,IL1,IL2,JL1V,JL2V,KL1,
     &           KL2,KEYOUT,NBLK,PRES,FLDEN,DEPTH,UPMOBPROD)
C*********************************************************************
C  ROUTINE EVALUATES properties (density) on the bdary.
C  PRES(I,J,K) = FLUID PRESSURE, PSI (INPUT, REAL*8)
C  FLDEN(I,J,K) = FLUID DENSITY, LB/CU-FT (OUTPUT, REAL*8)
C  DEPTH(I,J,K) = DEPTH OF BDARY SURFACE ELEMENT
C*********************************************************************
      implicit none
C      INCLUDE 'msjunk.h'
      INCLUDE 'control.h'
      INCLUDE 'boundary.h'
      INCLUDE 'layout.h'

      INCLUDE 'tfluidsc.h'
      include 'tbaldat.h'

      INTEGER IDIM,JDIM,KDIM,LDIM,IL1,IL2,KL1,KL2,
     &     IOFF,JOFF,KOFF,NBLK
      INTEGER JL1V(KDIM),JL2V(KDIM),    KEYOUT(IDIM,JDIM,KDIM)
      REAL*8 PRES(IDIM,JDIM,KDIM),      DEPTH(IDIM,JDIM,KDIM),
     &     FLDEN(IDIM,JDIM,KDIM),       RESID(IDIM,JDIM,KDIM),
     &     UPMOBPROD(IDIM,JDIM,KDIM,3)

      REAL*8 TE,PQ,DPQ,XB,YB,ZB,depb
      INTEGER I,J,K,L,IW,MERR,IB,NDIR,NFOFF,NTYPE,KG,IG,JG

      REAL*4 MINUSHEAD
c--------------------------------------------

      IF (NBND_REG.EQ.0.OR.NBEL.EQ.0) RETURN

c for single phase flow, bd. cond values need only be found once per newtonian
c iteration. For other models and outflow conditions, situation may vary.

      IF(NEWT.NE.1) RETURN

C  EVALUATE boundary TERMS AT TIME N + 1

      TE=TIM+DELTIM

C  GET LOCAL TO GLOBAL INDEX OFFSETS

      CALL BLKOFF(NBLK,IOFF,JOFF,KOFF,MERR)

C  LOOP OVER THE bdary condition regions

      DO 1 IB=1,NBND_REG

         NTYPE = NBND_TYPE(IB)

c properties and values set only for Dirichlet condition, type >0

         IF(NTYPE.LE.0) GOTO 1

         IF(NBLK.EQ.1.AND.IB.EQ.1) NFOFF=1
         IF(NBLK.GT.1.AND.IB.EQ.1) NFOFF=LOFFBND(NBND_REG,NBLK-1)+1
         IF(IB.NE.1) NFOFF=LOFFBND(IB-1,NBLK)+1

c for types 1, 2 get the current value from table

         IF(NTYPE.EQ.1.OR.NTYPE.EQ.2.OR.NTYPE.EQ.4)
     &        CALL LOOKUP(NTABBND(IB,1),TE,PQ,DPQ)

c loop over all bdary elements in this region

         DO 2 L=NFOFF,LOFFBND(IB,NBLK)
            I = LOCBND(1,L)
            J = LOCBND(2,L)
            K = LOCBND(3,L)

            NDIR = LOCBND(4,L)

            IF(NTYPE.EQ.1) THEN
c ----------
c copy the current value from table to primary variable (pressure)

               PRES(I,J,K) = PQ
               IF (TMODEL.EQ.0) THEN
                 FLDEN(I,J,K) = STFLDEN*EXP(FLCMP*(PQ))
               ELSEIF (TMODEL.EQ.1) THEN
                 FLDEN(I,J,K) = STFLDEN
               ENDIF

            ELSE IF (NTYPE.EQ.2) THEN
c ---------
c compute pressure and density from values at ref. depth

               CALL COMPPRDEN(BND_DEP(IB),PQ,DEPTH(I,J,K),
     &              PRES(I,J,K),FLDEN(I,J,K))

            ELSE IF (NTYPE.EQ.3) THEN
c ---------
c execute user supplied code to get the values pres(x,y,z)

               PQ = 0.D0
               KG=K+KOFF
               JG=J+JOFF
               IG=I+IOFF

               XB=.5D0*(XREC(IG,NBLK)+XREC(IG+1,NBLK))
               YB=.5D0*(YREC(JG,NBLK)+YREC(JG+1,NBLK))
               ZB=.5D0*(ZREC(KG,NBLK)+ZREC(KG+1,NBLK))

               IF(NDIR.EQ.1) XB=XREC(IG+1,NBLK)
               IF(NDIR.EQ.2) XB=XREC(IG,NBLK)
               IF(NDIR.EQ.3) YB=YREC(JG+1,NBLK)
               IF(NDIR.EQ.4) YB=YREC(JG,NBLK)
               IF(NDIR.EQ.5) ZB=ZREC(KG+1,NBLK)
               IF(NDIR.EQ.6) ZB=ZREC(KG,NBLK)

               DEPB = DEPTH(I,J,K)
               CALL BDMOD(NBDPROG(IB),XB,YB,ZB,DEPB,PQ)

               PRES(I,J,K) = PQ
               IF (TMODEL.EQ.0) THEN
                 FLDEN(I,J,K) = STFLDEN*EXP(FLCMP*(PQ))
               ELSEIF (TMODEL.EQ.1) THEN
                 FLDEN(I,J,K) = STFLDEN
               ENDIF

            ELSE IF (NTYPE.EQ.4) THEN
c ---------
c transform value of head to value of press at depth=0, assume small compr

c pressure is zero at -head value where density is stflden
               MINUSHEAD=-PQ
c guess for pressure and density assuming incompr.
               PRES(I,J,K) = PQ* GRAV * STFLDEN
               IF (TMODEL.EQ.0) THEN
                 FLDEN(I,J,K) = STFLDEN*EXP(FLCMP*PRES(I,J,K))
               ELSEIF (TMODEL.EQ.1) THEN
                 FLDEN(I,J,K) = STFLDEN
               ENDIF

               CALL COMPPRDEN(MINUSHEAD,0.0D0,DEPTH(I,J,K),
     &              PRES(I,J,K),FLDEN(I,J,K))

            ENDIF

! bag8 - update mobility array on external edges
            IF (NDIR.EQ.1) THEN
              UPMOBPROD(I+1,J,K,1) = 0.5D0*(FLDEN(I,J,K)+FLDEN(I+1,J,K))
     &                               /FLVIS
            ELSEIF (NDIR.EQ.2) THEN
              UPMOBPROD(I,J,K,1) = 0.5D0*(FLDEN(I-1,J,K)+FLDEN(I,J,K))
     &                               /FLVIS
            ELSEIF (NDIR.EQ.3) THEN
              UPMOBPROD(I,J+1,K,2) = 0.5D0*(FLDEN(I,J,K)+FLDEN(I,J+1,K))
     &                               /FLVIS
            ELSEIF (NDIR.EQ.4) THEN
              UPMOBPROD(I,J,K,2) = 0.5D0*(FLDEN(I,J-1,K)+FLDEN(I,J,K))
     &                               /FLVIS
            ELSEIF (NDIR.EQ.5) THEN
              UPMOBPROD(I,J,K+1,3) = 0.5D0*(FLDEN(I,J,K)+FLDEN(I,J,K+1))
     &                               /FLVIS
            ELSEIF (NDIR.EQ.6) THEN
              UPMOBPROD(I,J,K,3) = 0.5D0*(FLDEN(I,J,K-1)+FLDEN(I,J,K))
     &                               /FLVIS
            ENDIF

    2 CONTINUE

    1 CONTINUE

      END

c ************************************************
      SUBROUTINE COMPPRDEN(REFD,REFP,DEP,PRES,DEN)
c ************************************************
c computes pressure <pres> and density <den> at depth <dep>
c using as reference pressure <refp> at depth <refd>
c ************************************************
      IMPLICIT NONE
      INCLUDE 'tfluidsc.h'
      INCLUDE 'layout.h'
      INCLUDE 'tbaldat.h'

      REAL*4 REFD
      REAL*8 REFP,DEP,PRES,DEN
      REAL*8 GRD2,DP,DENB,F,DF,DENA
      INTEGER N
c-----------------------------------
      IF (TMODEL.EQ.0) THEN
        DENA = STFLDEN*EXP(FLCMP*REFP)
      ELSEIF (TMODEL.EQ.1) THEN
        DENA = STFLDEN
      ENDIF
      GRD2=.5D0*GRAV*DENA*(DEP-REFD)
      DP=2.D0*GRD2/(1.D0-GRD2*FLCMP)

      DO  N=1,3
         DENB=EXP(FLCMP*DP)
         F=DP-GRD2*(1.D0+DENB)
         DF=1.D0-GRD2*FLCMP*DENB
         DP=DP-F/DF
      ENDDO

      PRES = REFP+DP
      IF (TMODEL.EQ.0) THEN
        DEN = STFLDEN*EXP(FLCMP*PRES)
      ELSEIF (TMODEL.EQ.1) THEN
        DEN = STFLDEN
      ENDIF
      END

c *********************************************************
      SUBROUTINE TBDBAL()
c *********************************************************
c evaluates total contribution of fluxes across boundary
c and adds it to the flitnp
c *********************************************************

      IMPLICIT NONE
      INCLUDE 'control.h'
      INCLUDE 'tbaldat.h'
      INCLUDE 'boundary.h'
      REAL*8   TBDARYFLUX
      INTEGER I
C      include 'mmodel.h'
c------------------------

      CALL SUMIT(NBND_REG,BND_FLUX(1,1))

      TBDARYFLUX =  0.D0
      DO I=1,NBND_REG
C         IF(BNDRMOD(I).EQ.SINGLEI) THEN
          TBDARYFLUX =  TBDARYFLUX + BND_FLUX(I,1)
C         ENDIF
      ENDDO

      FLITNP=FLITNP + TBDARYFLUX

      END


CGUS, BAG8 ADDING BOUNDARY CONDITIONS

c======================================================================
      subroutine TBDTRAN
c======================================================================
      implicit none
c
c   MPFA boundary subroutines.
c AinvF1 = Ainv*(-FGrav)
c AinvF1 = AinvF1 + Ainv*(Fdir + F1Neum)
c LRHS   = B'*Ainv*( - FGrav)
c LRHS   =  LRHS + B'*Ainv*(Fdir + F1Neum ) - F2Neum
c
      include 'blkary.h'
      include 'tarydat.h'
      include 'mpfaary.h'
c
      integer kerr,ibdry(16)
      logical onceonly
      external calcbdry,DEBUG_GEA_MPFA
      data ibdry/16*0/, onceonly/.true./

      if (onceonly) then
         onceonly = .false.

         ibdry(1) = 15
         ibdry(2) = n_xc
         ibdry(3) = n_yc
         ibdry(4) = n_zc
         ibdry(5) = n_depth
         ibdry(6) = N_AINVF
         ibdry(7) = n_vprop
         ibdry(8) = n_fprop
         ibdry(9) = n_vdim
         ibdry(10) = n_fdim
         ibdry(11) = n_perminv
         ibdry(12) = n_resid
         ibdry(13) = n_pres
         ibdry(14) = n_kcr
         ibdry(15) = N_AINV
         ibdry(16) = N_UPMOBPROD
      endif

      call callwork(calcbdry,ibdry)

      end

c======================================================================
      subroutine calcbdry(idim,jdim,kdim,ldim,il1,il2,jl1v,jl2v,
     &    kl1,kl2,keyout,nblk,xc,yc,zc,depth,AINVF1,
     &    volprop,faceprop,voldim,facedim,perminv,
     &    resid,pres,keyoutcr,AINV,UPMOBPROD)
c======================================================================
      implicit none
C
C Guangri Xue 2008-2011
C Original Code
C WARNING! NDIR is different from expected face numbering nomenclature
C
C Gurpreet Singh 2012
C (1)  Modified to include internal boundary condition and fractures
C specification
C
      include 'boundary.h'
      include 'control.h'
cgus
c Convergence criteria crude implementation
      include 'tbaldat.h'
      include 'wells.h'
cgus

      integer idim,jdim,kdim,ldim,il1,il2,jl1v(kdim),jl2v(kdim),
     &   kl1,kl2,keyout(idim,jdim,kdim),nblk
      integer volprop(idim+1,jdim+1,kdim+1,8),
     &   faceprop(idim+1,jdim+1,kdim+1,12),
     &   voldim(idim+1,jdim+1,kdim+1),
     &   facedim(idim+1,jdim+1,kdim+1)
      real*8 perminv(3,3,8,idim,jdim,kdim),resid(idim,jdim,kdim),
     &   pres(idim,jdim,kdim),AINVF1(12,IDIM+1,JDIM+1,KDIM+1),
     &   DEPTH(IDIM,JDIM,KDIM),
     &   AINV(12,12,IDIM+1,JDIM+1,KDIM+1),
     &   UPMOBPROD(IDIM,JDIM,KDIM,3)
      real*8 xc(idim+1,jdim+1,kdim+1),yc(idim+1,jdim+1,kdim+1),
     &       zc(idim+1,jdim+1,kdim+1)
      INTEGER I,J,K,IB,NTYPE,NDIR,NFOFF,MPFA_BTYPE,ITEMP,JTEMP,KTEMP,
     &   L,IOFF,JOFF,KOFF,IERR
      REAL*8 PQ,DPQ,TE,XB,YB,ZB,DEPB,PT(3)
      INTEGER jl1,jl2, keyoutcr(idim+1,jdim+1,kdim+1),NUMIBC,NUMIBCF
      INTEGER port
      REAL*8 DUM1

      CALL BLKOFF(NBLK,IOFF,JOFF,KOFF,IERR)

      IF (NBND_REG.EQ.0.OR.NBEL.EQ.0) RETURN
      TE = TIM + DELTIM
c
c  LOOP OVER THE bdary condition regions
c  NTYPE = 1: constant Dirichlet, 2: Dirichlet with gravity
c          3: user specified diri function, 0: no-flow,
c        < 0: non-zero flux
c
c convert IPARS boundary type to MPFA boundary type

      DO 100 IB=1,NBND_REG

      NTYPE = NBND_TYPE(IB)

      IF (NTYPE.LT.0) MPFA_BTYPE = 1
      IF (NTYPE.GT.0) MPFA_BTYPE = 2
      IF (NTYPE.EQ.0) MPFA_BTYPE = 3

      IF(NBLK.EQ.1.AND.IB.EQ.1) NFOFF=1
      IF(NBLK.GT.1.AND.IB.EQ.1) NFOFF=LOFFBND(NBND_REG,NBLK-1)+1
      IF(IB.NE.1) NFOFF=LOFFBND(IB-1,NBLK)+1

!c for types 1, 2 or 4 get the current value from table
!      IF((NTYPE.NE.3).AND.(NTYPE.NE.-2)) THEN
!      CALL LOOKUP(NTABBND(IB,1),TE,PQ,DPQ)
!      ENDIF

c loop over all bdary elements in this region
      DO 200 L=NFOFF,LOFFBND(IB,NBLK)

      I = LOCBND(1,L)
      J = LOCBND(2,L)
      K = LOCBND(3,L)
      NDIR = LOCBND(4,L)

      PQ = PRES(I,J,K)    ! This was set in TBDPROP

!      IF ((NTYPE.EQ.3).OR.(NTYPE.EQ.-2)) THEN
!
!      call FaceCenterCorner(i,j,k,ndir,XB,YB,ZB,XC,YC,ZC,
!     &                      idim,jdim,kdim)
!
!         IF (NTYPE.EQ.-2) THEN
!           PQ = 0.D0
!           DEPB = DEPTH(I,J,K)
!           CALL BDMOD(NBDPROG(IB),XB,YB,ZB,DEPB,PQ)
!         ELSEIF (NTYPE.EQ.3) THEN
!           PT(1) = XB
!           PT(2) = YB
!           PT(3) = ZB
!CGUS           PQ = PEXACT(PT)
!           STOP 'PEXACT NOT SPECIFIED'
!         ENDIF
!
!      ENDIF

C
C  Change Neumann physical data to reference one
C
      IF (MPFA_BTYPE.EQ.1)
     &    CALL NEUMTOREF(PQ,XC,YC,ZC,I,J,K,NDIR,IDIM,JDIM,KDIM)

cgus: WARNING: NDIR is different from expected

c x- and x+  faces
      IF((NDIR.EQ.1).or.(NDIR.EQ.2)) THEN
        IF (NDIR.EQ.1) ITEMP = I + 1
        IF (NDIR.EQ.2) ITEMP = I

c ITEMP,j,k,11,
c ITEMP,j+1,k,9,
c ITEMP,j+1,k+1,1,
c ITEMP,j,k+1,3,

      call getRHSbdAbove(AINV(1,1,ITEMP,j,k),ITEMP,j,k,11,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,ITEMP,j+1,k),ITEMP,j+1,k,9,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,ITEMP,j+1,k+1),ITEMP,j+1,k+1,1,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,ITEMP,j,k+1),ITEMP,j,k+1,3,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)

c y- and y+ faces
      ELSEIF((NDIR.EQ.3).or.(NDIR.EQ.4)) THEN
        IF(NDIR.EQ.3) JTEMP = J + 1
        IF(NDIR.EQ.4) JTEMP = J

c i,JTEMP,k,10,
c i+1,JTEMP,k,12,
c i+1,JTEMP,k+1,4,
c i,JTEMP,k+1,2,

      call getRHSbdAbove(AINV(1,1,i,JTEMP,k),i,JTEMP,k,10,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,i+1,JTEMP,k),i+1,JTEMP,k,12,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,i+1,JTEMP,k+1),i+1,JTEMP,k+1,4,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,i,JTEMP,k+1),i,JTEMP,k+1,2,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)

c z- and z+  faces
      ELSEIF((NDIR.EQ.5).or.(NDIR.EQ.6)) THEN
        IF (NDIR.EQ.5) KTEMP = K + 1
        IF (NDIR.EQ.6) KTEMP = K

c i,j,KTEMP,7,
c i+1,j,KTEMP,8,
c i+1,j+1,KTEMP,5,
c i,j+1,KTEMP,6,

      call getRHSbdAbove(AINV(1,1,i,j,KTEMP),i,j,KTEMP,7,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,i+1,j,KTEMP),i+1,j,KTEMP,8,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,i+1,j+1,KTEMP),i+1,j+1,KTEMP,5,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)
      call getRHSbdAbove(AINV(1,1,i,j+1,KTEMP),i,j+1,KTEMP,6,
     &           keyoutcr,PQ,MPFA_BTYPE,volprop,faceprop,voldim,
     &           facedim,perminv,AinvF1,pres,resid,idim,jdim,kdim,
     &           UPMOBPROD)

      ELSE
        STOP 'MPFA_TRAN.F:NDIR (EXTERNAL BC) IS WRONG'
      ENDIF

 200  CONTINUE
 100  CONTINUE

      END

c======================================================================
      subroutine getRHSbdAbove(AINVL,i,j,k,FNUM,
     &             keyoutcr,PQ,MPFA_BTYPE,volprop,
     &             faceprop,voldim,facedim,perminv,AinvF1,
     &             pres,resid,idim,jdim,kdim,UPMOBPROD)
c======================================================================
      implicit none
c
c Guangri Xue 10/26/09 11/14/09
c
      include 'control.h'
c
      INTEGER i,j,k,FNUM,MPFA_BTYPE,idim,jdim,kdim
      integer volprop(idim+1,jdim+1,kdim+1,8),
     &     faceprop(idim+1,jdim+1,kdim+1,12),
     &     voldim(idim+1,jdim+1,kdim+1),
     &     facedim(idim+1,jdim+1,kdim+1),
     &     keyoutcr(idim+1,jdim+1,kdim+1)
      REAL*8  AINVL(12,12),PQ,
     &     perminv(3,3,8,idim,jdim,kdim),
     &     AINVF1(12,IDIM+1,JDIM+1,KDIM+1),
     &     resid(idim,jdim,kdim),pres(idim,jdim,kdim),
     &     UPMOBPROD(IDIM,JDIM,KDIM,3)
C
      integer m,vprop(8),fprop(12),VINDEX(8),
     &        FINDEX(12)
      real*8 BVAL(12),pn(8),UPMOB(12)
C

cgus
c Modifying for internal boundary condition
      if ((keyoutcr(i,j,k).eq.-2).or.(keyoutcr(i,j,k).eq.-1)) goto 55

      CALL INITARYR8(BVAL,12,0.0D0)
      BVAL(FNUM) = PQ

      do m = 1, 8
         vprop(m) = volprop(i,j,k,m)
      enddo
      do m = 1, 12
         fprop(m) = faceprop(i,j,k,m)
      enddo

      CALL GETDOFINDEX(I,J,K,VOLPROP(I,J,K,:),FACEPROP(I,J,K,:),
     &            VOLDIM(I,J,K),FACEDIM(I,J,K),VINDEX,FINDEX)
      CALL getCornerLocal(PN,I,J,K,PRES,IDIM,JDIM,KDIM,VPROP)
      CALL GETUPMOB(I,J,K,FACEDIM(I,J,K),FINDEX,UPMOBPROD,
     &                IDIM,JDIM,KDIM,UPMOB)

      call getRHSbd(i,j,k,BVAL,MPFA_BTYPE,vprop,fprop,
     &             voldim(i,j,k),facedim(i,j,k),
     &             perminv,AinvF1(1,i,j,k),pn,resid,
     &             idim,jdim,kdim,AINVL,UPMOB)

   55 continue

      return
      end


c======================================================================
      SUBROUTINE getRHSbd(i,j,k,BVAL,MPFA_BTYPE,vprop,fprop,
     &                 vdim,fdim,perminv,LAinvF1,pn,resid,
     &                 idim,jdim,kdim,AINV,UPMOB)
c======================================================================
      IMPLICIT NONE
      include 'control.h'

      INTEGER i,j,k,MPFA_BTYPE,VDIM,FDIM,idim,jdim,kdim,ind
      integer vprop(8),fprop(12)
      REAL*8  BVAL(12),
     &     perminv(3,3,8,idim,jdim,kdim),LAinvF1(12),
     &     resid(idim,jdim,kdim),pn(8)
C
      integer VINDEX(VDIM),FINDEX(FDIM),info,ipiv(fdim),m,l
      real*8 LCOF(VDIM,VDIM),A(fdim,fdim),
     &       B(fdim,vdim),work(fdim,vdim),F2Neum(vdim),F1Neum(fdim),
     &       LAinvF1s(fdim),FDir(fdim),LRHS(vdim),AINV(12,12),
     &       TMPAINV(FDIM,FDIM),TMPF1(FDIM),UPMOB(FDIM)
C

      CALL INITARYR8(LRHS,VDIM,0.0D0)
      CALL INITARYR8(LAinvF1s,fdim,0.0D0)

      LRHS    = 0.D0
      TMPAINV = 0.D0

      CALL GETDOFINDEX(I,J,K,VPROP,FPROP,VDIM,FDIM,VINDEX,FINDEX)

      DO L = 1,FDIM
      DO M = 1,FDIM
         TMPAINV(L,M) = AINV(FINDEX(L),FINDEX(M))
      ENDDO
      ENDDO

      CALL GETB(B,FDIM,VDIM,FINDEX,VINDEX)

      IF (MPFA_BTYPE.EQ.1) THEN
        CALL buildF2Neum(F2Neum,VDIM,VINDEX,FPROP,BVAL)
        CALL buildF1Neum(F1Neum,FDIM,I,J,K,FINDEX,VPROP,FPROP,
     &                   BVAL,PERMINV,IDIM,JDIM,KDIM)
        CALL AddaxToby(-1.0D0,F2Neum,1.0d0,LRHS,VDIM)
        CALL AddaxToby(1.0d0,F1Neum,1.0d0,LAinvF1s,fdim)

      ELSEIF((MPFA_BTYPE.EQ.2).OR.(MPFA_BTYPE.EQ.5)) THEN
        CALL buildFDIR(FDir,FDIM,FPROP,VPROP,BVAL)
        CALL AddaxToby(1.0d0,FDir,1.0d0,LAinvF1s,fdim)
      ENDIF

      TMPF1 = LAINVF1S
      if (fdim.ne.0) then
        CALL DGEMV('N',FDIM,FDIM,1.D0,TMPAINV,FDIM,TMPF1,1,
     &              0.D0,LAINVF1S,1)
      endif

      do 950 m = 1,vdim
      do 950 l = 1,fdim
         LRHS(m) = LRHS(m) + UPMOB(L)*B(l,m)*LAinvF1s(l)
 950  continue

      do m = 1,fdim
         ind = fIndex(m)
         LAinvF1(ind) = LAinvF1(ind) + LAinvF1s(m)
      enddo

      CALL ScaMulMat(DELTIM,LRHS,VDIM,1)

      CALL storeRHS(RESID,LRHS,VDIM,VINDEX,idim,jdim,kdim,
     &                  i,j,k,vprop)

      end

c====================================================================
      SUBROUTINE buildF2Neum(F2Neum,VDIM,VINDEX,FPROP,FVAL)
c====================================================================
      IMPLICIT NONE
      INCLUDE 'mpfaary.h'

      INTEGER VDIM,VINDEX(VDIM),FPROP(12)
      REAL*8 F2Neum(VDIM),FVAL(12)
C
      INTEGER VOLN,FACEM
      REAL*8 F2Neum_val
C
      CALL INITARYR8(F2Neum,VDIM,0.0D0)

      do 930 voln = 1, vdim
      do 930 facem = 1, 12
      if (fprop(facem).eq.BCNEUMANN) then
         F2Neum_val = 0.0d0
         call getF2Neum(vIndex(voln),facem,F2Neum_val,fval)
         F2Neum(voln) = F2Neum(voln) + F2Neum_val
      endif
 930  continue

      END

c====================================================================
      subroutine getF2Neum(testvol,trialface,F2Neum,fval)
c======================================================================
      implicit none
c
c getF2Nuem computes the neuman face (as a trial) functions
c contribution to the right hand side to the vol test function
c
c   INPUTs:
c     RHS        : right hand side of pressure system
c     gX,gY,gZ   : vertex boundary condition data
c     hx,hy,hz   : cell sizes in each direction
c     nx,ny,nz   : number of cells in each direction
c     i,j,k      : vertex index (i,j,k)
c     testvol    : test volume number associated with the vertex
c     trialface  : trial face number associated with the vertex
c
c    OUTPUT:
c     F2Neum: neumann trial face contribution to test vol
c
c   TODO:
c     This subroutine is simplification of storeF2NeumRHS.
c     Compared to storeG2RHS, the indices i j k in hx,hy,hz
c     and gX,gY,gZ and RHS are different.There might be errors
c     in storeG2RHS.
c
      integer testvol,trialface
c
      real*8 F2Neum,fval(12)
c
      real*8 IntFactor

      IntFactor = 0.25d0

c
c We loop over trial face which is on the boundary
c

c
c Faces 1 3 11 9 follow counter clock-wise direction in j-k plane
c
      if (trialface.eq.1)then
         if (testvol.eq.2) F2Neum = fval(1)*IntFactor
         if (testvol.eq.1) F2Neum = fval(1)*IntFactor
c
      elseif (trialface.eq.3)then
         if (testvol.eq.3) F2Neum = fval(3)*IntFactor
         if (testvol.eq.4) F2Neum = fval(3)*IntFactor
c
      elseif (trialface.eq.11)then
         if (testvol.eq.7) F2Neum = fval(11)*IntFactor
         if (testvol.eq.8) F2Neum = fval(11)*IntFactor
c
      elseif (trialface.eq.9)then
         if (testvol.eq.6) F2Neum = fval(9)*IntFactor
         if (testvol.eq.5) F2Neum = fval(9)*IntFactor
c
c Faces 4 2 10 12 follow counter clock-wise direction in i-k plane
c
      elseif (trialface.eq.4) then
         if (testvol.eq.4) F2Neum = fval(4)*IntFactor
         if (testvol.eq.1) F2Neum = fval(4)*IntFactor
c
      elseif (trialface.eq.2) then
         if (testvol.eq.3) F2Neum = fval(2)*IntFactor
         if (testvol.eq.2) F2Neum = fval(2)*IntFactor
c
      elseif (trialface.eq.10) then
         if (testvol.eq.7) F2Neum = fval(10)*IntFactor
         if (testvol.eq.6) F2Neum = fval(10)*IntFactor
c
      elseif (trialface.eq.12) then
         if (testvol.eq.8) F2Neum = fval(12)*IntFactor
         if (testvol.eq.5) F2Neum = fval(12)*IntFactor
c
c Faces 5,6,7,8 follow counter clock-wise direction in i-j plane
c
      elseif (trialface.eq.5)then
         if (testvol.eq.5) F2Neum = fval(5)*IntFactor
         if (testvol.eq.1) F2Neum = fval(5)*IntFactor
c
      elseif (trialface.eq.6)then
         if (testvol.eq.6) F2Neum = fval(6)*IntFactor
         if (testvol.eq.2) F2Neum = fval(6)*IntFactor
c
      elseif (trialface.eq.7)then
         if (testvol.eq.7) F2Neum = fval(7)*IntFactor
         if (testvol.eq.3) F2Neum = fval(7)*IntFactor
c
      elseif (trialface.eq.8)then
         if (testvol.eq.8) F2Neum = fval(8)*IntFactor
         if (testvol.eq.4) F2Neum = fval(8)*IntFactor
c
      else
         STOP 'buildMPFA.f: invalid triaface'
      endif

      return
      end


c====================================================================
      SUBROUTINE buildF1Neum(F1Neum,FDIM,I,J,K,FINDEX,VPROP,FPROP,
     &                       FVAL,PERMINV,IDIM,JDIM,KDIM)
c====================================================================
      IMPLICIT NONE
      INCLUDE 'mpfaary.h'
C
      INTEGER FDIM,I,J,K,FINDEX(FDIM),VPROP(8),FPROP(12),
     &        IDIM,JDIM,KDIM
      REAL*8 F1Neum(FDIM),FVAL(12),
     &       PERMINV(3,3,8,IDIM,JDIM,KDIM)
C
      INTEGER FACEL,FACEM
      REAL*8 F1Neum_val
C
      CALL INITARYR8(F1Neum,FDIM,0.0D0)

      do 920 facel = 1,fdim
      do 920 facem = 1,12
         if (fprop(facem).eq.BCNEUMANN)then
c            write(*,*)'F1Neum is called'
c            stop
            call getF1Neum(i,j,k,fIndex(facel),facem,vprop,
     &           fval,idim,jdim,kdim,perminv,F1Neum_val)
            F1Neum(facel) = F1Neum(facel) + F1Neum_val
         endif
 920  continue

      END

c====================================================================
      subroutine getF1Neum(i,j,k,testface,trialface,vprop,fval,
     &     idim,jdim,kdim,perminv,F1Neum)
c====================================================================
      implicit none
c
c     getF1Nuem computes the neuman face (as a trial) functions
c     contribution to the right hand side corresponding to
c     the face test function.
c
c   INPUTs:
c     i,j,k: vertex index
c     testface: test face number associated with the vertex
c     trialface: trial face number associated with the vertex
c
c   OUTPUT:
c     F1Neum: neumann trial face contribution to test face
c
c   NOTE:
c     Numbering, bInv(l,m,i,j,k):
c     (i,j,k) identifies the particular sub-cube embedded in
c     the reference unit cube
c     l identifies entry in permeability array such that
c     l=1 => bXX, l=2 => bXY, l=3 => bXZ, l=4 => bYY,
c     l=5 => bYZ, l=6 => bZZ
c          [K11(x) K12(x) K13(x)]   [bInv(1,...) bInv(2,...) bInv(3,...)]
c     bInv=[K12(x) K22(x) K23(x)] = [bInv(2,...) bInv(4,...) bInv(5,...)]
c          [K13(x) K23(x) K33(x)]   [bInv(3,...) bInv(5,...) bInv(6,...)]
c
c     m identifies the particular corner to evaluate
c     the permeability function in the particular sub-cube
c     In particular, start with smallest x, y, and z value
c     and move in counter-clockwise direction (in xy-plane).
c     (values 1 through 4). Next, continue numbering with
c     smallest x and y value and largest z-value again moving
c     in a counter-clockwise direction (5 through 8)
c
      integer idim,jdim,kdim,vprop(8)
      real*8 perminv(3,3,8,idim,jdim,kdim),fval(12)
      integer i,j,k,testface,trialface
c
      real*8 F1Neum
c

c      call blkoff(nblk,ioff,joff,koff,ierr)
c      call blkdim(nblk,nx,ny,nz,merr
cgxue: i->i+ioff,0->1,nx->nx+1, (similarly for y,z directions)

c
c     Here we loop over trial faces (while in getFDir we loop
c     over test faces). Trial faces are on the boundary, which
c     makes coding convenient.
c

c                    normal direction    quad point
c                        i  j  k           index
c p1: (i-1,j-1,k-1)      1  4  5             7
c p2: (i  ,j-1,k-1)      1  2  6             8
c p3: (i  ,j  ,k-1)      3  2  7             5
c p4: (i-1,j  ,k-1)      3  4  8             6
c p5: (i-1,j-1,k)        9  12 5             3
c p6: (i  ,j-1,k)        9  10 6             4
c p7: (i  ,j  ,k)        11 10 7             1
c p8: (i-1,j  ,k)        11 12 8             2

      F1Neum = 0.0d0
c
c Faces 1 3 11 9 follow counter clock-wise direction in j-k plane
c
      if (trialface.eq.1) then
         if (vprop(2).eq.0) then
c====================================================================
            call calcF1Neum(2,6,i,j-1,k-1,8, 2,1, 3,1,
     &             fval(1),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(1).eq.0) then
            call calcF1Neum(4,5,i-1,j-1,k-1,7, 2,1, 3,1,
     &           fval(1),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.3)then
         if (vprop(3).eq.0)then
            call calcF1Neum(2,7, i,j,k-1,5, 2,1,  3,1,
     &           fval(3),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(4).eq.0)then
            call calcF1Neum(4,8, i-1,j,k-1,6, 2,1,  3,1,
     &           fval(3),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.11)then
         if (vprop(7).eq.0)then
            call calcF1Neum(10,7, i,j,k,1, 2,1,  3,1,
     &           fval(11),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(8).eq.0) then
            call calcF1Neum(12,8, i-1,j,k,2, 2,1,  3,1,
     &           fval(11),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.9)then
         if (vprop(6).eq.0)then
            call calcF1Neum(10,6, i,j-1,k,4, 2,1,  3,1,
     &           fval(9),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(5).eq.0) then
            call calcF1Neum(12,5, i-1,j-1,k,3, 2,1,  3,1,
     &           fval(9),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
c Faces 4 2 10 12 follow counter clock-wise direction in i-k plane
c
      elseif (trialface.eq.4)then
         if (vprop(4).eq.0)then
            call calcF1Neum(3,8, i-1,j,k-1,6, 1,2,  3,2,
     &           fval(4),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(1).eq.0) then
            call calcF1Neum(1,5, i-1,j-1,k-1,7, 1,2,  3,2,
     &           fval(4),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.2)then
         if (vprop(3).eq.0)then
            call calcF1Neum(3,7, i,j,k-1,5, 1,2,  3,2,
     &           fval(2),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(2).eq.0) then
            call calcF1Neum(1,6, i,j-1,k-1,8,  1,2,  3,2,
     &           fval(2),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.10)then
         if (vprop(7).eq.0)then
            call calcF1Neum(11,7, i,j,k,1, 1,2,  3,2,
     &           fval(10),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(6).eq.0) then
            call calcF1Neum(9,6, i,j-1,k,4, 1,2,  3,2,
     &           fval(10),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.12)then
         if (vprop(8).eq.0)then
            call calcF1Neum(11,8, i-1,j,k,2, 1,2,  3,2,
     &           fval(12),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(5).eq.0) then
            call calcF1Neum(9,5, i-1,j-1,k,3, 1,2,  3,2,
     &           fval(12),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
c     Faces 5,6,7,8 follow counter clock-wise direction in i-j plane
c
      elseif (trialface.eq.5)then

         if (vprop(5).eq.0)then
            call calcF1Neum(9,12, i-1,j-1,k,3, 1,3,  2,3,
     &           fval(5),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(1).eq.0) then
            call calcF1Neum(1,4, i-1,j-1,k-1,7, 1,3,  2,3,
     &           fval(5),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.6)then
         if (vprop(6).eq.0)then
            call calcF1Neum(9,10, i,j-1,k,4, 1,3,  2,3,
     &           fval(6),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(2).eq.0) then
            call calcF1Neum(1,2, i,j-1,k-1,8, 1,3,  2,3,
     &           fval(6),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.7)then
         if (vprop(7).eq.0)then
            call calcF1Neum(11,10, i,j,k,1, 1,3,  2,3,
     &           fval(7),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(3).eq.0) then
            call calcF1Neum(3,2, i,j,k-1,5, 1,3,  2,3,
     &           fval(7),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif (trialface.eq.8)then
         if (vprop(8).eq.0)then
            call calcF1Neum(11,12, i-1,j,k,2, 1,3,  2,3,
     &           fval(8),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
            F1Neum = -F1Neum
         elseif (vprop(4).eq.0) then
            call calcF1Neum(3,4, i-1,j,k-1,6, 1,3,  2,3,
     &           fval(8),
     &           perminv,idim,jdim,kdim,F1Neum,trialface,testface)
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      else
         STOP 'buildMPFA.f: invalid index'
      endif

      end


c======================================================================
      subroutine calcF1Neum(nf1,nf2,i,j,k,index,row1,col1,row2,col2,
     &     fvalu,perminv,idim,jdim,kdim,F1Neum,
     &     trialface,testface)
c======================================================================
      implicit none
c
c  In K-orthogonal grid, F1Neum is always zero.
c  To test this subroutine, one need to come up with non k-orthogonal
c  case.
c
      integer i,j,k,nf1,nf2,row1,col1,row2,col2,index,idim,jdim,kdim,
     &     trialface,testface
      real*8 perminv(3,3,8,idim,jdim,kdim),fvalu,F1Neum
c
      real*8 IntFactor

      IntFactor = 0.125d0

      if (testface.eq.nf1)then
         F1Neum = perminv(row1,col1,index,i,j,k)*IntFactor*fvalu
      elseif (testface.eq.nf2)then
         F1Neum = perminv(row2,col2,index,i,j,k)*IntFactor*fvalu
      endif

      end

c======================================================================
      SUBROUTINE buildFDIR(FDir,FDIM,FPROP,VPROP,FVAL)
c======================================================================
      IMPLICIT NONE
C
C Guangri Xue 2008-2011
c Original code
c
c Gurpreet Singh 2012
c Modifications to include internal boundary condition augmentation
C
      INCLUDE 'mpfaary.h'
C
      INTEGER FDIM,FPROP(12),VPROP(8)
      REAL*8 FDIR(FDIM),FVAL(12),IntFactor
C
      INTEGER TEMPL,FACEL,i,j
C

      IntFactor = 0.25D0
      CALL INITARYR8(FDir,FDIM,0.0D0)

cgus
c ---------------------------Convention-------------------------------
c IBCPRES = 5
c RHS/Residual modification due to internal boundary condition
c Counting faces from 1 to 12 the negative side of IBC face is counted
c first followed by positive side
c
c Example
c If 2 is the IBC face
c FDir(2)  = y- face and FDir(3) = y+ face
c ---------------------------------------------------------------------

      templ = 0
      do 910 facel = 1,12
        if ((fprop(facel).eq.0).or.
     &      (fprop(facel).eq.BCDIRICHLET).or.
     &      (fprop(facel).eq.IBCPRES)) then
           templ = templ + 1
           if (fprop(facel).eq.BCDIRICHLET) then
              call getFDir(facel,FDir(templ),vprop,fval)
           elseif (fprop(facel).eq.IBCPRES) then
              templ = templ + 1
              FDir(templ-1)= fval(facel)*IntFactor
              FDir(templ)= -FDir(templ-1)
           endif
        endif
 910  continue

      if (templ.ne.fdim) then
         STOP 'setupMPFARHS: templ is wrong'
      endif

      END

c======================================================================
      subroutine getFDir(testface,Dir,vprop,fval)
c======================================================================
c
c     getFDir computes dirichlet boundary contribution to
c     the test face function
c
c   INPUTs:
c     gX,gY,gZ: boundary condtion data
c     hx,hy,hz: cell size
c     nx,ny,nz: number of cells in each direction
c     i,j,k   : vertex index
c     testface: Test face number associated with the vertex
c
c   OUTPUT:
c     Dir: Integral of dirichlet data on the testface
c
      implicit none
      integer testface,vprop(8)
      real*8 Dir,fval(12)
c
      real*8 IntFactor
c
      IntFactor = 0.25d0
c
c We loop over test faces which are in the boundary
c
c
c Faces 1 3 11 9 follow counter clock-wise direction in j-k plane
c

      if (testface.eq.1)then
         if (vprop(1).eq.0) then
            Dir =  fval(1)*IntFactor
         elseif (vprop(2).eq.0) then
            Dir = -fval(1)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.3)then
         if (vprop(4).eq.0) then
            Dir =  fval(3)*IntFactor
         elseif (vprop(3).eq.0) then
            Dir = -fval(3)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.11)then
         if (vprop(8).eq.0) then
            Dir =  fval(11)*IntFactor
         elseif (vprop(7).eq.0) then
            Dir = -fval(11)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.9)then
         if (vprop(5).eq.0) then
            Dir =  fval(9)*IntFactor
         elseif (vprop(6).eq.0) then
            Dir = -fval(9)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
c Faces 4 2 10 12 follow counter clock-wise direction in i-k plane
c
      elseif(testface.eq.4)then
         if (vprop(1).eq.0) then
            Dir =  fval(4)*IntFactor
         elseif (vprop(4).eq.0) then
            Dir = -fval(4)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.2)then
         if (vprop(2).eq.0) then
            Dir =  fval(2)*IntFactor
         elseif (vprop(3).eq.0) then
            Dir = -fval(2)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.10)then
         if (vprop(6).eq.0) then
            Dir =  fval(10)*IntFactor
         elseif (vprop(7).eq.0) then
            Dir = -fval(10)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.12)then
         if (vprop(5).eq.0) then
            Dir =  fval(12)*IntFactor
         elseif (vprop(8).eq.0) then
            Dir = -fval(12)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
c Faces 5,6,7,8 follow counter clock-wise direction in i-j plane
c
      elseif(testface.eq.5)then
         if (vprop(1).eq.0) then
            Dir =  fval(5)*IntFactor
         elseif (vprop(5).eq.0) then
            Dir = -fval(5)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.6)then
         if (vprop(2).eq.0) then
            Dir =  fval(6)*IntFactor
         elseif (vprop(6).eq.0) then
            Dir = -fval(6)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.7)then
         if (vprop(3).eq.0) then
            Dir =  fval(7)*IntFactor
         elseif (vprop(7).eq.0) then
            Dir = -fval(7)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      elseif(testface.eq.8)then
         if (vprop(4).eq.0) then
            Dir =  fval(8)*IntFactor
         elseif (vprop(8).eq.0) then
            Dir = -fval(8)*IntFactor
         else
            STOP 'buildMPFA.f: invalid index'
         endif
c
      else
         STOP 'buildMPFA.f: invalid index for vprop'
      endif

      end

c======================================================================
      SUBROUTINE NEUMTOREF(PQ,XC,YC,ZC,I,J,K,NDIR,IDIM,JDIM,KDIM)
c======================================================================
      IMPLICIT NONE
C
C     Input: PQ is Neumann Data
C     output: PQ is reference Neumann data
C             PQ = facearea * PQ
C
      INTEGER IDIM,JDIM,KDIM,I,J,K,NDIR
      REAL*8 PQ
      REAL*8 XC(IDIM+1,JDIM+1,KDIM+1),YC(IDIM+1,JDIM+1,KDIM+1),
     &       ZC(IDIM+1,JDIM+1,KDIM+1)
C
      REAL*8 Tri_AREA
C
      INTEGER ITEMP,JTEMP,KTEMP, DIM
      REAL*8 P1(3),P2(3),P3(3),P4(3),Pc(3),area
C

c x- and x+  faces
      IF((NDIR.EQ.1).or.(NDIR.EQ.2)) THEN
        IF (NDIR.EQ.1) ITEMP = I + 1
        IF (NDIR.EQ.2) ITEMP = I

c ITEMP,j,k,11,
c ITEMP,j+1,k,9,
c ITEMP,j+1,k+1,1,
c ITEMP,j,k+1,3,

        P1(1) = XC(ITEMP,j,k)
        P1(2) = YC(ITEMP,j,k)
        P1(3) = ZC(ITEMP,j,k)

        P2(1) = XC(ITEMP,j+1,k)
        P2(2) = YC(ITEMP,j+1,k)
        P2(3) = ZC(ITEMP,j+1,k)

        P3(1) = XC(ITEMP,j+1,k+1)
        P3(2) = YC(ITEMP,j+1,k+1)
        P3(3) = ZC(ITEMP,j+1,k+1)

        P4(1) = XC(ITEMP,j,k+1)
        P4(2) = YC(ITEMP,j,k+1)
        P4(3) = ZC(ITEMP,j,k+1)

c y- and y+ faces
      ELSEIF((NDIR.EQ.3).or.(NDIR.EQ.4)) THEN
        IF(NDIR.EQ.3) JTEMP = J + 1
        IF(NDIR.EQ.4) JTEMP = J

c i,JTEMP,k,10,
c i+1,JTEMP,k,12,
c i+1,JTEMP,k+1,4,
c i,JTEMP,k+1,2,


        P1(1) = XC(i,JTEMP,k)
        P1(2) = YC(i,JTEMP,k)
        P1(3) = ZC(i,JTEMP,k)

        P2(1) = XC(i+1,JTEMP,k)
        P2(2) = YC(i+1,JTEMP,k)
        P2(3) = ZC(i+1,JTEMP,k)

        P3(1) = XC(i+1,JTEMP,k+1)
        P3(2) = YC(i+1,JTEMP,k+1)
        P3(3) = ZC(i+1,JTEMP,k+1)

        P4(1) = XC(i,JTEMP,k+1)
        P4(2) = YC(i,JTEMP,k+1)
        P4(3) = ZC(i,JTEMP,k+1)

c z- and z+  faces
      ELSEIF((NDIR.EQ.5).or.(NDIR.EQ.6)) THEN
        IF (NDIR.EQ.5) KTEMP = K + 1
        IF (NDIR.EQ.6) KTEMP = K

c i,j,KTEMP,7,
c i+1,j,KTEMP,8,
c i+1,j+1,KTEMP,5,
c i,j+1,KTEMP,6,

        P1(1) = XC(i,j,KTEMP)
        P1(2) = YC(i,j,KTEMP)
        P1(3) = ZC(i,j,KTEMP)

        P2(1) = XC(i+1,j,KTEMP)
        P2(2) = YC(i+1,j,KTEMP)
        P2(3) = ZC(i+1,j,KTEMP)

        P3(1) = XC(i+1,j+1,KTEMP)
        P3(2) = YC(i+1,j+1,KTEMP)
        P3(3) = ZC(i+1,j+1,KTEMP)

        P4(1) = XC(i,j+1,KTEMP)
        P4(2) = YC(i,j+1,KTEMP)
        P4(3) = ZC(i,j+1,KTEMP)

      ELSE
c         CALL ParcelCrash('MPFA_TRAN.F: NDIR IS WRONG',-222)
      ENDIF

      do 100 dim = 1,3
         Pc(dim) = 0.25d0*(P1(DIM)+P2(DIM)+P3(DIM)+P4(DIM))
  100 continue

      AREA =   Tri_AREA(Pc,P1,P2)
     &       + Tri_AREA(Pc,P2,P3)
     &       + Tri_AREA(Pc,P3,P4)
     &       + Tri_AREA(Pc,P4,P1)

      PQ = AREA * PQ

      end

c======================================================================
      subroutine FaceCenterCorner(i,j,k,ndir,XB,YB,ZB,XC,YC,ZC,
     &                            idim,jdim,kdim)
c======================================================================
      IMPLICIT NONE
c
      integer i,j,k,ndir,idim,jdim,kdim
      real*8 XB,YB,ZB
      real*8 xc(idim+1,jdim+1,kdim+1),yc(idim+1,jdim+1,kdim+1),
     &       zc(idim+1,jdim+1,kdim+1)
c
      integer Itemp,Jtemp,Ktemp
c
      IF(NDIR.EQ.1)THEN
      Itemp = I+1
      CALL GETFACECENTER(Itemp,J,K, Itemp,J+1,K,
     &                   Itemp,J+1,K+1, Itemp,J,K+1,
     &                   XB,YB,ZB,XC,YC,ZC,IDIM,JDIM,KDIM)
      ELSEIF(NDIR.EQ.2)THEN
      Itemp = I
      CALL GETFACECENTER(Itemp,J,K, Itemp,J+1,K,
     &                   Itemp,J+1,K+1, Itemp,J,K+1,
     &                   XB,YB,ZB,XC,YC,ZC,IDIM,JDIM,KDIM)
      ELSEIF(NDIR.EQ.3)THEN
      Jtemp = J+1
      CALL GETFACECENTER(I,Jtemp,K, I+1,Jtemp,K,
     &                   I+1,Jtemp,K+1, I,Jtemp,K+1,
     &                   XB,YB,ZB,XC,YC,ZC,IDIM,JDIM,KDIM)
      ELSEIF(NDIR.EQ.4)THEN
      Jtemp = J
      CALL GETFACECENTER(I,Jtemp,K, I+1,Jtemp,K,
     &                   I+1,Jtemp,K+1, I,Jtemp,K+1,
     &                   XB,YB,ZB,XC,YC,ZC,IDIM,JDIM,KDIM)
      ELSEIF(NDIR.EQ.5)THEN
      Ktemp = K+1
      CALL GETFACECENTER(I,J,Ktemp, I+1,J,Ktemp,
     &                   I+1,J+1,Ktemp, I,J+1,Ktemp,
     &                   XB,YB,ZB,XC,YC,ZC,IDIM,JDIM,KDIM)
      ELSEIF(NDIR.EQ.6)THEN
      Ktemp = K
      CALL GETFACECENTER(I,J,Ktemp, I+1,J,Ktemp,
     &                   I+1,J+1,Ktemp, I,J+1,Ktemp,
     &                   XB,YB,ZB,XC,YC,ZC,IDIM,JDIM,KDIM)
      ELSE
         STOP 'MPFA_TRAN.DF: NDIR IS WRONG'
      ENDIF

      end

