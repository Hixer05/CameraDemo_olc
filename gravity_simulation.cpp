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
#include "hixer_camera.hpp"
#include <vector>
#include <string> 
#include <map>
#include <ctime>
using namespace std;
using namespace hixer;


struct sBody{
    size_t mass;
    size_t radius;
    olc::vf2d position; 
    olc::vf2d velocity;
    string name = "Unnamed";
    olc::Pixel color = olc::WHITE;
    void applyImpulse(olc::vf2d force, float time){
        
        velocity += (force/mass)*time;
        position += velocity*time;
    }
};

float distance(olc::vf2d pos1, olc::vf2d pos2){
        return sqrt(pow(pos1.x-pos2.x,2)+pow(pos1.y-pos2.y,2));
}

int pBrightness(float scaling){
    if(scaling > 1){
        return 255;
    }else{
        return scaling*255;
    }
    
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
const int gridSpacing = 200;

// CELESTIAL BODIES
map<size_t, sBody>elements;
size_t tick = 0;
const float G = 6.674 * pow(10, -11);
float time_multiplier = 2;
float gravity_multiplier = 1000000;
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

        elements[0] = sBody{20000000, 30, olc::vf2d{0,0}};
        elements[1] = sBody{8600, 20, olc::vf2d{-200,0}, olc::vf2d{0,20}};
		return true;
	}

public:
    
	bool OnUserUpdate(float fElapsedTime) override
	{
        //cam1.cameraPos +=  elements[1].position - cam1.camToWorld({ScreenWidth()/2, ScreenHeight()/2});
        tick++;

        if(GetKey(olc::Key::C).bPressed){
            elements.clear();
        }
        if(GetKey(olc::MINUS).bPressed){
            if(time_multiplier>1){
                time_multiplier -=1;
            }
        }
        if(GetKey(olc::EQUALS).bPressed){
            time_multiplier +=1;
        }

        float physics_time = fElapsedTime * time_multiplier;

        //clear
		Clear(olc::BACK);

        DrawString(GetMousePos() + olc::vi2d{50, 0}, to_string(cam1.camToWorld(GetMousePos()).x) + " " + to_string(cam1.camToWorld(GetMousePos()).y));


        

        

        //__CAMERA_MOV__
            
        cam_speed = (1/cam1.scaling) * bcam_speed;
        if(GetKey(olc::Key::A).bHeld) cam1.cameraPos.x -= cam_speed * fElapsedTime;
        if(GetKey(olc::Key::D).bHeld) cam1.cameraPos.x += cam_speed * fElapsedTime;
        if(GetKey(olc::Key::W).bHeld) cam1.cameraPos.y += cam_speed* fElapsedTime;
        if(GetKey(olc::Key::S).bHeld) cam1.cameraPos.y -= cam_speed* fElapsedTime;

        if(GetKey(olc::UP).bHeld) {
            olc::vf2d bpos = cam1.camToWorld({ScreenWidth()/2, ScreenHeight()/2});
            cam1.scaling *= (1.005);
            olc::vf2d apos = cam1.camToWorld({ScreenWidth()/2, ScreenHeight()/2});
            cam1.cameraPos += (bpos-apos);          
            }
        if(GetKey(olc::DOWN).bHeld) {
            olc::vf2d bpos = cam1.camToWorld({ScreenWidth()/2, ScreenHeight()/2});
            cam1.scaling *=(0.995);
            olc::vf2d apos = cam1.camToWorld({ScreenWidth()/2, ScreenHeight()/2});
            cam1.cameraPos += (bpos-apos);        
        }
        

        //Check for collisions, though only one per frame is dealt withs, thogh with many fast particles does not work
        {
        bool stop = false;
            for(auto&[key, body] : elements){
                for(auto&[key2, body2]: elements){
                    if(key == key2) continue;
                    
                    if(distance(body.position, body2.position) <= body.radius + body2.radius){
                        body.color = olc::RED;
                        sBody new_body = {(body.mass + body2.mass), (body.radius +body2.radius), (body.mass >= body2.mass ? body.position : body2.position), 
                        (body.velocity * body.mass + body2.velocity * body2.mass)/(body.mass + body2.mass)};
                        elements.erase(body.mass >= body2.mass ? key2: key);
                        elements[key] = new_body;
                        
                        stop = true;
                        break;
                    }
                }
                if(stop){
                    break;
                }
            }
        }

        
        //__CALC FORCES BETWEEN BODIES__
        map<size_t, olc::vf2d> forces2apply; // <key, resulting forces>
        for(auto&[key,body] : elements){
            forces2apply[key] = olc::vf2d{0,0};
            for(auto&[key2, body2] : elements){
                if(key2 == key) continue;
                if(distance(body.position, body2.position) == 0) continue; //they'll get  checked out as colliding, so skip
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

        //__DRAW_GRID__

        int pixel_brightness = pBrightness(cam1.scaling);


        // horizontal:
        int scarto = (int)(cam1.cameraPos.y) % gridSpacing; // eg pos.x=1988; scarto=38
        for(int i = cam1.camToWorld({0,0}).y-scarto; i>cam1.camToWorld(cam1.cameraLen).y; i-=gridSpacing){
                DrawLine({0,cam1.worldToCam({0,i}).y},{cam1.cameraLen.x,cam1.worldToCam({0,i}).y}, olc::Pixel{255,255,255,pixel_brightness});
                
        }
        //vertical:
        
        scarto = (int)(cam1.cameraPos.x) % gridSpacing;
        for(int i = cam1.camToWorld({0,0}).x-scarto; i<cam1.camToWorld(cam1.cameraLen).x; i+=gridSpacing){
                DrawLine({cam1.worldToCam({i,0}).x, 0},{cam1.worldToCam({i,0}).x,cam1.cameraLen.y}, olc::Pixel{255,255,255,pixel_brightness});
        }

        //__DRAW BODIES__  
        for(auto& [key, o] : elements){
            FillCircle(cam1.worldToCam(o.position), o.radius*cam1.scaling, o.color);
        }


        //__INSERT_MODE__
        {
            
            if(GetKey(olc::I).bPressed){
                imode = !imode;
            }
            if(imode){
                DrawString(olc::vi2d{0,0} , "Mass Level: "+ std::to_string(imode_mass_lvl) +"\nPress O and P to adjust", olc::WHITE, 2);
                DrawString(olc::vi2d{0,35}, "Press C to cancel the scene", olc::WHITE, 2);
                if(GetMouse(1).bHeld){
                    elements[tick] = sBody{imode_mass_lvl, (imode_mass_lvl/70)+1, cam1.camToWorld(GetMousePos()), {0,0}};
                }
                if(GetMouse(0).bPressed){
                    imode_release = true;
                    tmp_pos = GetMousePos();
                }
                else if(GetKey(olc::P).bPressed) imode_mass_lvl +=100;
                else if (GetKey(olc::O).bPressed && imode_mass_lvl>100) imode_mass_lvl -=100;
            }else{
                DrawString(olc::vi2d{0,0}, "Press C to cancel the scene", olc::WHITE, 2);
            }
            
            

            if(imode_release){
                    FillCircle(tmp_pos, (imode_mass_lvl/70)*cam1.scaling);
                    DrawLine(tmp_pos, GetMousePos());
                    if(GetMouse(0).bReleased){  
                        elements[tick]=sBody{imode_mass_lvl, (imode_mass_lvl/70)+1, cam1.camToWorld(tmp_pos) ,(cam1.camToWorld(tmp_pos)-cam1.camToWorld(GetMousePos()))*imode_bias_multiply_velocity};
                        imode_release=false;
                    }
                
            }

        }

        
        

        DrawString({0,ScreenHeight()-10}, "Realised by using Javidx9 pixelGameEngine");
        DrawString({0,ScreenHeight()-20}, to_string(cam1.scaling));
        
        return true;
	}
};


int main()
{
	Example demo;
	if (demo.Construct(1920, 1080, 1, 1, 1, 1))
		demo.Start();

	return 0;
}
