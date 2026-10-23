// C++17 / Windows 10-11 / controlled FPS simulator
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <tlhelp32.h>
#include <cwchar>
#include <algorithm>
#include <cmath>
#include <vector>
#include "core.hpp"
namespace {
struct Actor { dpx::Player player; double phase; };
std::vector<Actor> actors;
dpx::Settings settings;
bool showEsp=true,showHealth=true,showAim=true,paused=false;
double t=0;
dpx::Vec2 aim{640,360};
HWND windowHandle=nullptr;
bool valorantRunning(){
 HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
 if(snapshot==INVALID_HANDLE_VALUE)return false;
 PROCESSENTRY32W pe{};pe.dwSize=sizeof(pe);
 bool found=false;
 if(Process32FirstW(snapshot,&pe)){
  do{
   if(_wcsicmp(pe.szExeFile,L"VALORANT-Win64-Shipping.exe")==0){found=true;break;}
  }while(Process32NextW(snapshot,&pe));
 }
 CloseHandle(snapshot);
 return found;
}
int canvasW=1280,canvasH=720;
void line(HDC dc,int x,int y,int xx,int yy,COLORREF color,int width=1){HPEN p=CreatePen(PS_SOLID,width,color);auto old=SelectObject(dc,p);MoveToEx(dc,x,y,nullptr);LineTo(dc,xx,yy);SelectObject(dc,old);DeleteObject(p);}
void rect(HDC dc,int x,int y,int w,int h,COLORREF color,int width=1){HPEN p=CreatePen(PS_SOLID,width,color);auto old=SelectObject(dc,p);auto b=SelectObject(dc,GetStockObject(NULL_BRUSH));Rectangle(dc,x,y,x+w,y+h);SelectObject(dc,b);SelectObject(dc,old);DeleteObject(p);}
void fill(HDC dc,int x,int y,int w,int h,COLORREF color){RECT r{x,y,x+w,y+h};HBRUSH b=CreateSolidBrush(color);FillRect(dc,&r,b);DeleteObject(b);}
void label(HDC dc,int x,int y,const wchar_t* text,COLORREF color){SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);TextOutW(dc,x,y,text,lstrlenW(text));}
void reset(){actors.clear();for(int i=0;i<8;i++)actors.push_back({{i+1,{(i-4)*1.6,0,8+i*2.2},100-i*11,i%3?25:50,i%3!=0},i*.85});t=0;aim={canvasW/2.,canvasH/2.};}
void draw(HDC dc){
 RECT cr;GetClientRect(windowHandle,&cr);canvasW=cr.right;canvasH=cr.bottom;
 fill(dc,0,0,canvasW,canvasH,RGB(12,16,25));
 for(int x=0;x<canvasW;x+=64)line(dc,x,0,x,canvasH,RGB(25,32,44));
 for(int y=0;y<canvasH;y+=64)line(dc,0,y,canvasW,y,RGB(25,32,44));
 int cx=canvasW/2,cy=canvasH/2;
 dpx::Camera cam{{0,0,0},90,canvasW,canvasH};
 std::vector<dpx::Player> ps;
 for(const auto& a:actors)ps.push_back(a.player);
 auto selected=dpx::selectTarget(ps,cam,settings);
 if(showAim){HPEN p=CreatePen(PS_DOT,1,RGB(83,132,207));auto old=SelectObject(dc,p);auto b=SelectObject(dc,GetStockObject(NULL_BRUSH));Ellipse(dc,cx-(int)settings.aimFovPixels,cy-(int)settings.aimFovPixels,cx+(int)settings.aimFovPixels,cy+(int)settings.aimFovPixels);SelectObject(dc,b);SelectObject(dc,old);DeleteObject(p);}
 for(const auto& a:actors){auto pos=dpx::project(a.player.position,cam);if(!pos)continue;int x=(int)pos->x,y=(int)pos->y;int h=std::clamp((int)(1250/a.player.position.z),38,165),w=h/2;
  COLORREF c=a.player.visible?RGB(87,214,179):RGB(218,134,85);
  if(showEsp){rect(dc,x-w/2,y-h/2,w,h,c,2);line(dc,x,y-h/2,x,y+h/2,c);line(dc,x-w/2,y,x+w/2,y,c);}
  if(showHealth){int hp=dpx::clampHealth(a.player.health);fill(dc,x-w/2-10,y-h/2,5,h,RGB(55,57,65));fill(dc,x-w/2-10,y+h/2-(h*hp/100),5,h*hp/100,hp>60?RGB(65,220,117):hp>30?RGB(241,190,60):RGB(245,76,76));}
  wchar_t name[100];wsprintfW(name,L"BOT %02d  HP:%d  ARM:%d",a.player.id,a.player.health,a.player.armor);// Stagger labels to reduce overlap between nearby simulated targets.
  const int labelY=y-h/2-22-(a.player.id%4)*18;
  label(dc,x-w/2,labelY,name,RGB(222,227,239));
  if(selected&&*selected==a.player.id&&showAim){rect(dc,x-w/2-4,y-h/2-4,w+8,h+8,RGB(249,220,98),2);aim=dpx::smoothStep(aim,*pos,settings.smooth);}
 }
 if(!selected)aim=dpx::smoothStep(aim,{(double)cx,(double)cy},settings.smooth);
 if(showAim){line(dc,(int)aim.x-9,(int)aim.y,(int)aim.x+9,(int)aim.y,RGB(255,218,105),2);line(dc,(int)aim.x,(int)aim.y-9,(int)aim.x,(int)aim.y+9,RGB(255,218,105),2);}
 line(dc,cx-12,cy,cx+12,cy,RGB(220,230,240));line(dc,cx,cy-12,cx,cy+12,RGB(220,230,240));
 fill(dc,12,12,std::max(0,std::min(canvasW-24,760)),105,RGB(25,33,49));
 label(dc,24,20,L"DPX-EXTERN v0.6.1 | OFFLINE TRAINING + GAME STATUS",RGB(111,225,202));
 label(dc,24,43,L"F1 ESP  F2 HEALTH  F3 AIM GUIDE  F4 VISIBLE ONLY  SPACE PAUSE",RGB(220,225,240));
 label(dc,24,65,L"UP/DOWN FOV  LEFT/RIGHT SMOOTH  R RESET  ESC EXIT",RGB(220,225,240));
 wchar_t status[150];wsprintfW(status,L"FOV: %d px   SMOOTH: %d   VISIBLE ONLY: %s",(int)settings.aimFovPixels,(int)settings.smooth,settings.visibleOnly?L"ON":L"OFF");label(dc,24,88,status,RGB(255,207,109));
 label(dc,24,121,valorantRunning()?L"VALORANT: RUNNING (NO GAME ACCESS)":L"VALORANT: NOT DETECTED",RGB(111,225,202));
}
LRESULT CALLBACK proc(HWND w,UINT msg,WPARAM wp,LPARAM lp){
 switch(msg){case WM_CREATE:windowHandle=w;reset();SetTimer(w,1,16,nullptr);return 0;
 case WM_TIMER:if(!paused){t+=.016;for(auto& a:actors){a.player.position.x=std::sin(t*.8+a.phase)*4.5;a.player.position.y=std::cos(t*.6+a.phase)*.75;}}InvalidateRect(w,nullptr,FALSE);return 0;
 case WM_KEYDOWN:switch(wp){case VK_F1:showEsp=!showEsp;break;case VK_F2:showHealth=!showHealth;break;case VK_F3:showAim=!showAim;break;case VK_F4:settings.visibleOnly=!settings.visibleOnly;break;case VK_UP:settings.aimFovPixels=std::min(450.,settings.aimFovPixels+15);break;case VK_DOWN:settings.aimFovPixels=std::max(20.,settings.aimFovPixels-15);break;case VK_LEFT:settings.smooth=std::max(1.,settings.smooth-1);break;case VK_RIGHT:settings.smooth=std::min(30.,settings.smooth+1);break;case VK_SPACE:paused=!paused;break;case 'R':reset();break;case VK_ESCAPE:DestroyWindow(w);break;}InvalidateRect(w,nullptr,FALSE);return 0;
 case WM_PAINT:{PAINTSTRUCT p;HDC dc=BeginPaint(w,&p);RECT r;GetClientRect(w,&r);int width=std::max(1L,r.right),height=std::max(1L,r.bottom);HDC mem=CreateCompatibleDC(dc);if(mem){HBITMAP bmp=CreateCompatibleBitmap(dc,width,height);if(bmp){auto old=SelectObject(mem,bmp);if(old&&old!=HGDI_ERROR){draw(mem);BitBlt(dc,0,0,width,height,mem,0,0,SRCCOPY);SelectObject(mem,old);}DeleteObject(bmp);}DeleteDC(mem);}EndPaint(w,&p);return 0;}
 case WM_DESTROY:KillTimer(w,1);PostQuitMessage(0);return 0;
 }return DefWindowProcW(w,msg,wp,lp);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR,int show){
 WNDCLASSW cls{};cls.lpfnWndProc=proc;cls.hInstance=instance;cls.lpszClassName=L"DPXExternSimulator";cls.hCursor=LoadCursor(nullptr,IDC_CROSS);cls.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
 if(!RegisterClassW(&cls))return 1;
 HWND w=CreateWindowExW(0,cls.lpszClassName,L"DPX-EXTERN v0.6.1 | OFFLINE TRAINING",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1300,760,nullptr,nullptr,instance,nullptr);
 if(!w)return 2;ShowWindow(w,show);UpdateWindow(w);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;
}
