#ifndef NOMINMAX
#define NOMINMAX
#endif
#pragma once
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
namespace dpx {
struct Vec2 { double x,y; };
struct Vec3 { double x,y,z; };
struct Player { int id; Vec3 position; int health; int armor; bool visible; };
struct Camera { Vec3 position; double fovDegrees; int width; int height; };
struct Settings { double aimFovPixels=180.0; double smooth=8.0; bool visibleOnly=true; };
inline int clampHealth(int hp){return std::clamp(hp,0,100);}
inline std::optional<Vec2> project(Vec3 world, Camera cam) {
 const double z=world.z-cam.position.z;
 if(z<=0.01 || cam.width<=0 || cam.height<=0) return std::nullopt;
 const double f=(cam.width*0.5)/std::tan(cam.fovDegrees*3.141592653589793/360.0);
 return Vec2{cam.width*.5+(world.x-cam.position.x)*f/z,cam.height*.5-(world.y-cam.position.y)*f/z};
}
inline Vec2 smoothStep(Vec2 current,Vec2 target,double smooth) {
 const double s=std::max(1.0,smooth);
 return {current.x+(target.x-current.x)/s,current.y+(target.y-current.y)/s};
}
inline std::optional<int> selectTarget(const std::vector<Player>& players,Camera cam,Settings settings){
 const double cx=cam.width/2.,cy=cam.height/2.;
 double best=settings.aimFovPixels*settings.aimFovPixels;
 std::optional<int> id;
 for(const auto& p:players){
  if(p.health<=0 || (settings.visibleOnly&&!p.visible))continue;
  auto pos=project(p.position,cam);if(!pos)continue;
  double dx=pos->x-cx,dy=pos->y-cy,d=dx*dx+dy*dy;
  if(d<=best){best=d;id=p.id;}
 }
 return id;
}
}
