#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "hixer_camera.hpp"
#include <vector>

using namespace std;


class Game : public olc::PixelGameEngine{ 
    public:
	Game()
	    {
	    	sAppName = "Momentum";
	    }

    hixer::sCamera cam;
    olc::vi2d bar_size;

    bool OnUserCreate() override 
    {   
        bar_size = {ScreenWidth(), 150};
        cam.cameraLen ={ScreenWidth(),ScreenHeight()-bar_size.y};
        cam.cameraPos ={-cam.cameraLen.x/2,cam.cameraLen.y/2};
        cam.scaling = 1;
        cam.screen_pos = {0,0};
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::BLACK);
        
        if(GetKey(olc::W).bHeld) cam.cameraPos.y += 200*fElapsedTime;
        if(GetKey(olc::S).bHeld) cam.cameraPos.y -= 200*fElapsedTime;
        if(GetKey(olc::A).bHeld) cam.cameraPos.x -= 200*fElapsedTime;
        if(GetKey(olc::D).bHeld) cam.cameraPos.x += 200*fElapsedTime;
        
        if(GetKey(olc::UP).bHeld) {
            olc::vf2d bpos = cam.camToWorld({cam.cameraLen.x/2, cam.cameraLen.y/2});
            cam.scaling *= (1.005);
            olc::vf2d apos = cam.camToWorld({cam.cameraLen.x/2, cam.cameraLen.y/2});
            cam.cameraPos += (bpos-apos);          
            }
        if(GetKey(olc::DOWN).bHeld) {
            olc::vf2d bpos = cam.camToWorld({cam.cameraLen.x/2, cam.cameraLen.y/2});
            cam.scaling *=(0.995);
            olc::vf2d apos = cam.camToWorld({cam.cameraLen.x/2, cam.cameraLen.y/2});
            cam.cameraPos += (bpos-apos);        
        }
        
        
        //draw in cam
        if(cam.isViewing({0,0})){
            FillCircle(cam.camToScreen(cam.worldToCam({0,0})), 10*cam.scaling );
        }
        if(cam.isViewing({30,10})){
            FillCircle(cam.camToScreen(cam.worldToCam({30,10})), 10*cam.scaling, olc::RED);
        }
        //draw in screen
        FillRect({0,(cam.screen_pos+cam.cameraLen).y}, {ScreenWidth(), bar_size.y}, olc::Pixel{100,100,200});
        
        //rect rappresenting camera
        DrawRect(cam.screen_pos, (cam.cameraLen-olc::vi2d{1,0}));
        //                     ^^^ to show the border

        
        return true;
    }
    
};



int main()
{
	Game demo;
	if (demo.Construct(1600, 1000, 1, 1, 0, 1))
		demo.Start();

	return 0;
}