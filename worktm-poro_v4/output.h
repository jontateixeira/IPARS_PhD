C  OUTPUT.H - I/O DATA

      COMMON /OUTCOM/ MFILE(6),I1AP(10,3),I2AP(10,3),
     & ISAP(10,3),J1AP(10,3),J2AP(10,3),
     & JSAP(10,3),K1AP(10,3),K2AP(10,3),
     & KSAP(10,3),NUMREG(10),NARS,NEORS,NERRP

      COMMON /OUTCOM8/ PBUF8A(480),PBUF8B(480),PBUF8C(480),RSFOUT

      REAL*8 PBUF8A,PBUF8B,PBUF8C,RBUFR8(3*480)
      REAL*4 PBUF4A(480),PBUF4B(480),PBUF4C(480),RBUFR4(3*480)
      INTEGER IPBUF4A(480),IPBUF4B(480),IPBUF4C(480),
     & RBUFI4(3*480)
      CHARACTER*60 RSFOUT
      CHARACTER*1 RSFOUT1(60)
      EQUIVALENCE (PBUF8A,PBUF4A,IPBUF4A,RBUFR8,RBUFR4,RBUFI4),
     & (PBUF8B,PBUF4B,IPBUF4B),(PBUF8C,PBUF4C,IPBUF4C),(RSFOUT,RSFOUT1)
      INTEGER I1AP,I2AP,ISAP,J1AP,J2AP,JSAP,K1AP,K2AP,KSAP,NUMREG,
     & NARS,NEORS,NERRP,MFILE

C*********************************************************************

C  MFILE  = USER OUTPUT FILE MAP FOR COMP.FOR

C  I1AP,I2AP,ISAP,J1AP,J2AP,JSAP,K1AP,K2AP,KSAP = INDEX RANGES FOR GRID
C  ELEMENT ARRAY PRINTOUT.  INDEXED BY FAULT BLOCK AND NUMBER OF PRINT
C  REGIONS IN EACH FAULT BLOCK.  THESE ARE GLOBAL INDEXES.

C  NUMREG(i) = NUMBER OF PRINT REGIONS IN FAULT BLOCK i

C  NARS = GRID-ELEMENT ARRAY NUMBER USED IN RESTART OUTPUT

C  NEORS = NUMBER OF GRID-ELEMENTS OUTPUT TO RESTART FILE (PROCESSOR 0)

C  NERRP = FLAG USED BY FRAMEWORK TO PASS ERROR CONDITION FROM WORK ROUTINES

C  PBUF8A,PBUF8B,PBUF8C = MESSAGE, PRINT, AND RESTART BUFFERS

C  RSFOUT = RESTART OUTPUT FILE BASE NAME
