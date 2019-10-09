C  BDARYIN.DF - OUTPUT BOUNDARY CONDITION DATA

C  ROUTINES IN THIS MODULE:

C     SUBROUTINE BND_OUT
c     Malgo Peszynska, 1/01  initial version
C***********************************************************************
      SUBROUTINE BND_OUT()
C***********************************************************************
c if flag bndout is set in input data, routine
c     outputs bdary fluxes to stdout
C***********************************************************************
      INCLUDE 'control.h'
      INCLUDE 'boundary.h'
      INTEGER NPH

      IF(BNDOUT) THEN
         IF(LEVELC)THEN
            DO I=1,NBND_REG
               WRITE(NFOUT,11) 'TIME=',TIM+DELTIM,' BND.REG.=',I,
     &              (' FLUX',NPH,'=',BND_FLUX(I,NPH),NPH=1,13)
            ENDDO
         ENDIF
      ENDIF

 11   FORMAT(A6,F6.2,1X,A10,I2,13(A5,I1,A1,F8.1,1X))


      END

C***********************************************************************
      SUBROUTINE BDCLEARFLUX()
C***********************************************************************
c zero bdary fluxes for the regions over blocks which belong
c to a model which agrees with  the model calling the routine
C***********************************************************************
      implicit none
      INCLUDE 'boundary.h'
      INCLUDE 'control.h'
C      INCLUDE 'mmodel.h'

      INTEGER I,J

      DO I=1,NBND_REG
C         IF(BNDRMOD(I).EQ.CURRENT_MODEL) THEN
        DO J=1,13
           BND_FLUX(I,J)=0.D0
        ENDDO
C         ENDIF
      ENDDO

      END



