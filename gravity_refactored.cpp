#define HIXER_CAMERA_DOUBLE_ON
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "hixer_camera.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <map>
using std::vector, std::cout, std::endl;
int temp=1;

class Body{
    
    public:
    Body(double mass, double radius, olc::vd2d position, olc::vd2d velocity, size_t key)
        :m_mass(mass), m_radius(radius), m_position(position), m_velocity(velocity), m_key(key)
    {
    }

    void drawSelf(olc::PixelGameEngine* pge, hixer::sCamera& cam){
        pge->FillCircle(cam.worldToCam(m_position), m_radius, olc::WHITE);
        for(olc::vd2d pos : positions){
            pge->Draw(cam.worldToCam(pos), olc::RED);
        }
    }

    void applyImpulse(olc::vd2d force, double time){
        // f = ma
        m_velocity += (force/m_mass)*time;
        m_position += m_velocity*time;
        push_to_queue(m_position);
        
    }
    size_t getKey(){
        return m_key;
    }

    olc::vd2d getPosition(){return m_position;}
    olc::vd2d getVelocity(){return m_velocity;}
    double getMass(){return m_mass;}
    double getRadius(){return m_radius;}
    int test(){return positions.size();}

    private:

    void push_to_queue(olc::vd2d pos){
        if(positions.size()==MAX_POS_CAP){
            positions.erase(positions.begin());
            positions.push_back(pos);
        }else if(positions.size()< MAX_POS_CAP){
            positions.push_back(pos);
        }
    }
    size_t m_key;
    double m_mass{0};
    double m_radius{0};
    olc::vd2d m_position{0,0};
    olc::vd2d m_velocity{0,0};
    vector<olc::vd2d> positions;
    const size_t MAX_POS_CAP=10000; // maximum positions capability
};

namespace _calculator {
    static bool run = false;
    static bool pop_back = false;
    std::thread* t;
    static size_t MAX_CALC;
    vector<vector<Body>>* r_bodies; 

    static void _calc(){
        const double G = 6.674 * pow(10, -11);
        const double G_multiplier=10000000; // bias
        double delta_time = (double)(1.0/60.0)*temp; //simulation at 60 frames per second
        auto distance = [](olc::vf2d pos1, olc::vf2d pos2){
            return sqrt(pow(pos1.x-pos2.x,2)+pow(pos1.y-pos2.y,2));
        };

        size_t num  = 0;

        while(run){
            delta_time = (double)(1.0/60.0)*temp;
            if(r_bodies->size()< MAX_CALC){
                std::map<size_t, olc::vd2d> forces2apply;
                
                vector<Body> bodies = r_bodies->at((r_bodies->size()-1));// make copy (?)

                for(Body b: bodies){
                    forces2apply[b.getKey()] = olc::vd2d{0,0};
                    for(Body b2: bodies){
                        if(b.getKey() == b2.getKey()) continue;
                        double dist;
                        if((dist=distance(b.getPosition(), b2.getPosition())) == 0) continue;
                        olc::vd2d force = b2.getPosition() - b.getPosition();
                        force = force.norm();
                        double force_ammount = G*(b.getMass() * b2.getMass())/(dist);
                        force *= force_ammount * G_multiplier;
                        forces2apply[b.getKey()] += force;
                    }
                }

                for(Body& b : bodies){
                    b.applyImpulse(forces2apply[b.getKey()], delta_time);
                }

                r_bodies->push_back(bodies);
               
            }
            if(pop_back){
                    pop_back=false;
                    if(!r_bodies->empty()){
                        r_bodies->erase(r_bodies->begin());
                    }
                }
            
        }
        
    }

    void start(const size_t maximum_ammount_to_calc, vector<vector<Body>>* bodies){
        if(!run){
            MAX_CALC = maximum_ammount_to_calc;
            r_bodies = bodies;
            run = true;
            
            t = new std::thread(&_calculator::_calc); 

            
        }
    }
    void stop(){
        run = false;
        t->join();
        delete t;
    }

    void do_pop_back(){
        pop_back = true;
    }
    
};

class Simulation : public olc::PixelGameEngine {

    vector<vector<Body>>* bodies;
    hixer::sCamera cam;

    public:
    Simulation(){
        sAppName = "Gravity";
    }

    bool OnUserCreate() override{
        cam = {{ScreenWidth(), ScreenHeight()}, {-ScreenWidth()/2, ScreenHeight()/2},1};
        
        bodies = new vector<vector<Body>>(1);

        bodies->at(0).push_back(Body(10.0,10.0,{100.0,0.0}, {0,5}, 0));
        bodies->at(0).push_back(Body(100000.0,20.0,{0.0,0.0}, {0,0}, 1));
        
        _calculator::start(100, bodies);
        
        return true;
    };

    bool OnUserUpdate(float fElapsedTime) override{
        Clear(olc::BLACK);

        for(Body& b: bodies->at(0)){
            b.drawSelf(this, cam);

        }
        _calculator::do_pop_back();

        if(GetKey(olc::D).bPressed) temp+=10;
        if(GetKey(olc::A).bPressed) temp=1;

        //cout << bodies->at(0).at(0).getPosition() << endl; // TODO: _Calculator does not apply IMPULSE! Obj position NOT CHANGING!!!!
        return true;
    };

};

int main(){
    Simulation sim;
    if(sim.Construct(800,600,1,1,0,1)){
        sim.Start();
    }
}