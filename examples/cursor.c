#include <stdio.h>
#include "escape.h"

int main(void)
{
    /* Absolute position: row 3, column 1. */
    fputs(EST_CSI_CUP_SSTR(3, 1), stdout);
    printf("CUP(3, 1) - absolute position");

    /* Relative moves. */
    fputs(EST_CSI_CUD_SSTR(2), stdout);   /* down 2 rows */
    printf("CUD(2)    - down 2 rows");

    fputs(EST_CSI_CUF_SSTR(4), stdout);   /* right 4 columns */
    printf("CUF(4)    - right 4 columns");

    /* Absolute column / row. */
    fputs(EST_CSI_CHA_SSTR(1), stdout);   /* column 1 */
    printf("CHA(1)    - back to column 1");

    fputs(EST_CSI_VPA_SSTR(8), stdout);   /* row 8 */
    printf("VPA(8)    - jump to row 8");

    /* Save and restore the cursor. */
    fputs(EST_CSI_SCP_SSTR(), stdout);    /* save position */
    fputs(EST_CSI_CUP_SSTR(12, 1), stdout);
    printf("SCP saved here, now at row 12");
    fputs(EST_CSI_RCP_SSTR(), stdout);    /* restore position */
    printf("RCP       - restored");

    printf("\n");
    return 0;
}
