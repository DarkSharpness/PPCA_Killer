#include "src/cpu.h"


signed main() {
    dark::cpu intel_13900KF;    /* For fun LOL */
    intel_13900KF.init();       /* Init data.  */
    while(intel_13900KF.work());
    uint32_t result  = (uint8_t)intel_13900KF.a0;
    printf("%u",result);
    return 0;
}
