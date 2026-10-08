#include <stdio.h>
#include "escape.h"

int main(void)
{
    /* EL(0): erase from the cursor to the end of the line. */
    printf("KEPT:  GONE");
    fputs(EST_CSI_CHA_SSTR(8), stdout);   /* cursor to column 8 ("GONE") */
    fputs(EST_CSI_EL_SSTR(0), stdout);    /* erase "GONE" */
    printf("\n");

    /* EL(2): erase the whole line, then rewrite from column 1. */
    printf("This line will be replaced");
    fputs(EST_CSI_EL_SSTR(2), stdout);    /* erase whole line */
    fputs(EST_CSI_CHA_SSTR(1), stdout);   /* back to column 1 */
    printf("Replaced by EL(2)\n");

    /* ED(2) would clear the whole screen; kept commented to preserve output. */
    /* fputs(EST_CSI_ED_SSTR(2), stdout); */

    /* Also available: ED, ICH, DCH, ECH, IL, DL, SU, SD, REP. */
    printf("ED / ICH / DCH / ECH / IL / DL / SU / SD / REP are also available.\n");

    return 0;
}
