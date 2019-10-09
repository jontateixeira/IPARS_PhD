C  TTDATA.F - IMPLICIT SINGLE PHASE FLOW MODEL TRANSIENT DATA

C  ROUTINES IN THIS MODULE:

C  SUBROUTINE TTDATA (NERR)
C  SUBROUTINE TQUIT  (NERR)

C  CODE HISTORY:

C  JOHN WHEELER     4/29/97   SKELETON ROUTINE
C  BAHAREH MOMKEN   2/15/99   modified for single phase flow
C  JOHN WHEELER     4/03/99   IMPLICIT SINGLE PHASE MODEL
C*********************************************************************
      SUBROUTINE TTDATA (NERR)
C*********************************************************************

C  Input single phase flow model transient data

C  NERR = ERROR KEY STEPPED BY ONE FOR EACH ERROR
C         (INPUT AND OUTPUT, INTEGER)

C*********************************************************************
      IMPLICIT NONE
      INCLUDE 'control.h'

      INCLUDE 'tbaldat.h'
      LOGICAL ONCE
      DATA ONCE/.TRUE./
      INTEGER NDUM,NERR,CVTO

C  INPUT NEWTONIAN CONVERGENCE TOLERANCE

      CVTO=CVTOL
      CALL GETVAL('CVTOL ',CVTOL,'R8',0,0,0,0,NDUM,NERR)
      CALL GETVAL('S_CVTOL ',CVTOL,'R8',0,0,0,0,NDUM,NERR)
      IF (LEVELC.AND.(CVTO.NE.CVTOL.OR.ONCE)) WRITE (NFOUT,1) CVTOL
    1 FORMAT(' CONVERGENCE TOLERANCE (CVTOL)',T49,G11.4)
      ONCE=.FALSE.

      END
C*********************************************************************
      SUBROUTINE TQUIT (NERR)
C*********************************************************************

C  Terminate single phase implicit model

C*********************************************************************
      INCLUDE 'control.h'
      INTEGER NERR

      NERR=NERR

! bag8 - report final discrete-in-time errors
      IF (ITEST.GT.0) CALL TERRCALC_FINAL()

      END
