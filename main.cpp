/*
License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018 - 2022 OneLoneCoder.com
	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:
	1. Redistributions or derivations of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	2. Redistributions or derivative works in binary form must reproduce the above
	copyright notice. This list of conditions and the following	disclaimer must be
	reproduced in the documentation and/or other materials provided with the distribution.
	3. Neither the name of the copyright holder nor the names of its contributors may
	be used to endorse or promote products derived from this software without specific
	prior written permission.
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS	"AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
	SHALL THE COPYRIGHT	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL,	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
	SUCH DAMAGE.
*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <vector>
#include <string> 
#include <map>
#include <ctime>
using namespace std;

struct sCamera{
    olc::vf2d cameraLen;
    olc::vf2d cameraPos;
    double scaling;
    olc::vf2d worldToCam(olc::vf2d world_pos){
        //(x-cam.x)k, -(y, cam.y)k
        return {(world_pos.x - cameraPos.x)*scaling, (-(world_pos.y - cameraPos.y))*scaling};
    };
    olc::vf2d camToWorld(olc::vi2d position){
        //olc::vf2d translate_vec = -cameraPos; 
        return {(position.x/scaling)+cameraPos.x,-(position.y/scaling)+cameraPos.y};
    }
    bool isViewing(olc::vf2d obj_pos){
        return ((obj_pos.x > cameraPos.x && obj_pos.x < cameraPos.x+cameraLen.x)
            &&( obj_pos.y < cameraPos.y && obj_pos.y > cameraPos.y-cameraLen.y));
    }
};

struct sBody{
    size_t mass;
    size_t radius;
    olc::vf2d position; 
    olc::vf2d velocity;
    void applyImpulse(olc::vf2d force, float time){
        velocity += (force/mass)*time;
        position += velocity*time;
    }
    
};

float distance(olc::vf2d pos1, olc::vf2d pos2){
        return sqrt(pow(pos1.x-pos2.x,2)+pow(pos1.y-pos2.y,2));
}
float v_module(olc::vf2d vec){
    return sqrt(pow(vec.x, 2)+pow(vec.y, 2));
}

class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		sAppName = "Example";
	}

private:
vector<olc::vf2d> toDraw;
const double bcam_speed = 400;
double cam_speed = bcam_speed;
sCamera cam1;
const int gridSpacing = 50;

// CELESTIAL BODIES
map<string, sBody>elements;
size_t tick = 0;
const float G = 6.674 * pow(10, -11);
float time_multiplier = 1;
float gravity_multiplier = 10000;
//imode
bool imode = false;
bool imode_release = false;;
olc::vf2d tmp_pos;
double imode_bias_multiply_velocity=1.0f;
size_t imode_mass_lvl=100;

public:
	bool OnUserCreate() override
	{
        cam1.cameraLen = {ScreenWidth(), ScreenHeight()};
        cam1.cameraPos= {-cam1.cameraLen.x/2, cam1.cameraLen.y/2};
        cam1.scaling = 1;
        //toDraw.push_back({100,500});
        //toDraw.push_back({ScreenWidth()/2,ScreenHeight()/2});

        elements["Pianeta1"] = sBody{10000000000, 10, olc::vf2d{0,0}};

		return true;
	}

public:

	bool OnUserUpdate(float fElapsedTime) override
	{
        //cout << cam1.cameraPos << endl;
        tick++;
        float physics_time = fElapsedTime * time_multiplier;

        if(GetKey(olc::Key::C).bPressed){
            elements.clear();
        }

        //clear
		Clear(olc::BACK);

        //__INSERT_MODE__
        
        {
            
            if(GetKey(olc::I).bPressed){
                imode = !imode;
            }
            if(imode){
                DrawString(olc::vi2d{0,0} , "Mass Level: "+ std::to_string(imode_mass_lvl) +"\nPress O and P to adjust", olc::WHITE, 2);
                DrawString(olc::vi2d{0,35}, "Press C to cancel the scene", olc::WHITE, 2);
                if(GetMouse(0).bPressed){
                    imode_release = true;
                    tmp_pos = cam1.camToWorld(GetMousePos());
                }else if(GetKey(olc::P).bPressed) imode_mass_lvl +=100;
                else if (GetKey(olc::O).bPressed && imode_mass_lvl>100) imode_mass_lvl -=100;
            }else{
                DrawString(olc::vi2d{0,0}, "Press C to cancel the scene", olc::WHITE, 2);
            }
            
            

            if(imode_release){
                    FillCircle(cam1.worldToCam(tmp_pos), (imode_mass_lvl/10)*cam1.scaling);
                    DrawLine(cam1.worldToCam(tmp_pos), GetMousePos());
                    if(GetMouse(0).bReleased){  
                        elements["pianeta"+to_string(tick)]=sBody{imode_mass_lvl/100, imode_mass_lvl/10, tmp_pos ,(tmp_pos-cam1.camToWorld(GetMousePos()))*imode_bias_multiply_velocity};
                        imode_release=false;
                    }
                
            }

        }

        

        //__CAMERA_MOV__

        cam_speed = (1/cam1.scaling) * bcam_speed;
        if(GetKey(olc::Key::A).bHeld) cam1.cameraPos.x -= cam_speed * fElapsedTime;
        if(GetKey(olc::Key::D).bHeld) cam1.cameraPos.x += cam_speed * fElapsedTime;
        if(GetKey(olc::Key::W).bHeld) cam1.cameraPos.y += cam_speed* fElapsedTime;
        if(GetKey(olc::Key::S).bHeld) cam1.cameraPos.y -= cam_speed* fElapsedTime;

        if(GetKey(olc::Key::Q).bHeld) cam1.scaling *= 1.005;
        if(GetKey(olc::Key::E).bHeld) cam1.scaling *= 0.995;
         
        //__CALC FORCES BETWEEN BODIES__

        map<string, olc::vf2d> forces2apply; // <key, resulting forces>
        for(auto&[key,body] : elements){
            forces2apply[key] = olc::vf2d{0,0};
            for(auto&[key2, body2] : elements){
                if(key2 == key) continue;
                olc::vf2d force = body2.position - body.position; //calc direction
                force = force.norm(); //normalize
                float force_ammount = G*(body.mass * body2.mass)/(distance(body.position, body2.position)); 
                force *= force_ammount*gravity_multiplier;
                forces2apply[key] += force;
            }
        }    
        //__APPLY FORCES__
        for(auto& [key, f] : forces2apply){
            elements[key].applyImpulse(f, physics_time);
        };


        //__DRAW BODIES__  
        for(auto& [key, o] : elements){
            DrawCircle(cam1.worldToCam(o.position), o.radius*cam1.scaling);
        }

        
        //__DRAW_GRID__
        // horizontal:
        int scarto = (int)(cam1.cameraPos.y) % gridSpacing; // eg pos.x=1988; scarto=38
        for(int i = cam1.camToWorld({0,0}).y-scarto; i>cam1.camToWorld(cam1.cameraLen).y; i-=gridSpacing){
                DrawLine({0,cam1.worldToCam({0,i}).y},{cam1.cameraLen.x,cam1.worldToCam({0,i}).y}, olc::GREY);
                
        }
        //vertical:
        
        scarto = (int)(cam1.cameraPos.x) % gridSpacing;
        for(int i = cam1.camToWorld({0,0}).x-scarto; i<cam1.camToWorld(cam1.cameraLen).x; i+=gridSpacing){
                DrawLine({cam1.worldToCam({i,0}).x, 0},{cam1.worldToCam({i,0}).x,cam1.cameraLen.y}, olc::GREY);
        }

        //__DRAW_OBJS
        /*
        for(int i = 0; i< toDraw.size(); i++){
            if(cam1.isViewing(toDraw.at(i))){
                FillCircle(cam1.worldToCam(toDraw.at(i)), 50);
            }    
        }
        */

        DrawString({0,ScreenHeight()-10}, "Realised by using Javidx9 pixelGameEngine");
        DrawString({0,ScreenHeight()-20}, to_string(GetMousePos().x)+" "+to_string(GetMousePos().y));
		DrawString({0,ScreenHeight()-30}, to_string(cam1.camToWorld(GetMousePos()).x)+" "+to_string(cam1.camToWorld(GetMousePos()).y));
        return true;
	}
};


int main()
{
	Example demo;
	if (demo.Construct(800, 600, 1, 1, 0, 1))
		demo.Start();

	return 0;
}
