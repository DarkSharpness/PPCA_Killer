#include "src/cpu.h"

template <class integer>
void writelineInt(integer num) {
    if(!num) return (void) puts("0");
    if(num < 0) num = -num,putchar('-');
    static char ch[3 * sizeof(integer) + 1] = {0};
    int top = 3 * sizeof(integer) - 1;
    while(num) {
        ch[--top] = '0' ^ (num % 10);
        num /= 10;
    } puts(ch + top);
}


signed main() {
    // freopen("Ignore/docs/testcases/qsort.data","r",stdin);
    // freopen("Ignore/Hastin/my.out","w",stdout);
    dark::cpu intel_13900KF;    /* For fun LOL */
    intel_13900KF.init();       /* Init data.  */
    while(intel_13900KF.work());
    uint8_t result = intel_13900KF.a0;
    writelineInt(result);
    return 0;
}
