#ifndef GameUtils_H
#define GameUtils_H
#include<string>
inline char *id2str(int x){
    static char tmp[6]="0000";
    for(int i=3;i>=0;--i){
        tmp[i]='0'+x%10;
        x/=10;
    }
    tmp[4]=0;
    return tmp;
}
inline bool is_int(const std::string &str){
    auto it=str.begin();
    if(*it=='-') ++it;
    if(it==str.end()) return false;
    for(;it!=str.end();++it) if(!isdigit(*it)) return false;
    return true;
}
inline void sep_str(const std::string &str, std::string &s1, std::string &s2, const char c){
    s1.clear();
    s2.clear();
    bool flag=true;
    for(const auto &a:str){
        if(flag){
            if(a==c) flag=false;
            else s1+=a;
        }
        else s2+=a;
    }
}
#endif
