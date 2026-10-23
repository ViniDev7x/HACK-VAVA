// C++17 / core tests
#include "core.hpp"
#include <cassert>
#include <cmath>
int main(){
 using namespace dpx;
 assert(clampHealth(-2)==0 && clampHealth(170)==100);
 Camera c{{0,0,0},90,1280,720};
 auto v=project({0,0,10},c);
 assert(v && std::abs(v->x-640)<1e-6 && std::abs(v->y-360)<1e-6);
 assert(!project({0,0,-1},c));
 auto s=smoothStep({0,0},{100,100},10);
 assert(std::abs(s.x-10)<1e-6);
 std::vector<Player> ps{{1,{0,0,10},100,0,true},{2,{0,0,9},100,0,false}};
 Settings cfg;cfg.visibleOnly=true;
 auto id=selectTarget(ps,c,cfg);assert(id && *id==1);
 return 0;
}
