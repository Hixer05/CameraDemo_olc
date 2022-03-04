#ifndef HIXER_CAMERA_HPP
#define HIXER_CAMERA_HPP
#include "olcPixelGameEngine.h"
namespace hixer{
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
}

#endif