// C++17 / CLI simulation
#include "core.hpp"
#include <iostream>
int main(){
 dpx::Camera camera{{0,0,0},90,1280,720};
 dpx::Settings settings;
 std::vector<dpx::Player> players{{1,{0,0,10},100,50,true},{2,{2,0,12},80,25,false}};
 auto target=dpx::selectTarget(players,camera,settings);
 std::cout<<"DPX-EXTERN simulation target: "<<(target?std::to_string(*target):"none")<<"\n";
 return 0;
}
