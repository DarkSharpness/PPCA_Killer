/* This program is for test use only! Windows only! */
#include "src/cpu.h"
#include <iomanip>

bool check(const std::string &str) {
    return str.size() > 5 && str.substr(str.size() - 5) == ".data";
}

std::string get_name(const std::string &str) {
    size_t i = str.size() - 5;
    while(i-- && (str[i] != '\\' || str[i] == '/'));
    ++i;
    return str.substr(i,str.size() - 5 - i);
}

signed main(int argc,const char ** argv) {
    std::string path = "F:/Code/Github/PPCA_Killer/Ignore/result.md";
    freopen(path.data(),"w",stdout);
    std::cout << "| Test Case | Total branches | Success Rate | Total CPU clock |\n";
    std::cout << "| :-------: | :------------: | :----------: | :-------------: |\n";
    freopen("CON","w",stdout);
    for(int i = 0 ; i != argc ; ++i) {
        std::string str = argv[i];

        if(!check(str)) continue;

        std::string name = get_name(str);
        freopen(str.data(),"r",stdin);
        freopen(path.data(),"a",stdout);

        auto clk = clock();
        dark::cpu intel_13900KF;    /* For fun LOL */
        intel_13900KF.init();       /* Init data.  */
        while(intel_13900KF.work());
        auto delta = clock() - clk;
        std::cout << "| "  << name
                  << " | " << intel_13900KF.branches()
                  << " | " << std::fixed << std::setprecision(6)
                           << intel_13900KF.get_accuracy() 
                  << " | " << intel_13900KF.clock << " |\n";

        freopen("CON","r",stdin);
        freopen("CON","w",stdout);

        std::cout << name << " is done!\n";
        std::cout << "Time: " << delta << "ms\n";
    }
    std::cout << "All done!\n";
    system("pause");
    return 0;
}

