 
#include <webots/Supervisor.hpp>
#include <webots/Node.hpp>
using namespace webots;

#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <cassert>
using namespace std;

#define TIME_STEP 32

// Coord
typedef struct {
  double x, z;
} Coord;

typedef struct {
  double x, z, dx, dz;
} StartLine;
//CheckPoint
Coord CP[3] = {{1.45, -0.4}, {-0.89, -0.965}, {0.635, -0.378}};

//Start point of every line
StartLine SP[] = {{1.45, 1.09,0.075, 0.01},{1.45,-0.438,0.075, 0.01}};


bool CheckPointCMP(const double *trans, int count){
  switch(count) {
    case 0: 
      return (trans[0] >= CP[0].x - 0.075 && trans[0] <=  CP[0].x + 0.075 && trans[2] >= CP[0].z - 0.01 && trans[2] <= CP[0].z + 0.01);
    case 1:
      return (trans[0] >= CP[1].x - 0.01 && trans[0] <=  CP[1].x + 0.01 && trans[2] >= CP[1].z - 0.075 && trans[2] <= CP[1].z + 0.075);
    case 2:
      return (trans[0] >= CP[2].x - 0.01 && trans[0] <=  CP[2].x + 0.01 && trans[2] >= CP[2].z - 0.075 && trans[2] <= CP[2].z + 0.075);
    default: return false;
  }

}

Coord LPoint0 = {-0.0449, -0.129};
Coord RPoint0 = {0.0449, -0.129};

Coord Rotation(const double *trans, Coord coord ,const double *rotat) {
    Coord a;
    a.x = trans[0] + coord.x*cos(rotat[3]) - coord.z*sin(rotat[3]);
    a.z = trans[2] + coord.x*sin(rotat[3]) + coord.z*cos(rotat[3]);
    return a;
}
bool CheckOutLine(Coord LP, Coord RP, int count) {
  switch(count) {
    case 0:{
        return (!(LP.x >= 1.33) && (RP.x <= 1.57));
      }
    default: return false ;
  }
}
//Chuyen sang line khac
bool MoveLine(const double *trans, int count) {
  return (trans[0] >= SP[count+1].x - SP[count+1].dx && trans[0] <=  SP[count+1].x + SP[count+1].dx  && trans[2] >= SP[count+1].z - SP[count+1].dz && trans[2] <= SP[count+1].z + SP[count+1].dz );
} 

int main(int argc, char **argv) {

  
  // init Supervisor
  Supervisor *supervisor = new Supervisor();
  Node *IRCar = supervisor->getFromDef("Speed_Line_Follower_Robot_V4");
  Field *IRCar_trans_field = IRCar->getField("translation");
  Field *IRCar_rotat_field = IRCar->getField("rotation");
  
  int Next_CP = 0, Next_Line = 0;
  double CP_time[3];
  while (supervisor->step(TIME_STEP) != -1) {
    Coord LPoint = Rotation(IRCar_trans_field->getSFVec3f(), LPoint0, IRCar_rotat_field->getSFRotation());
    Coord RPoint = Rotation(IRCar_trans_field->getSFVec3f(), RPoint0, IRCar_rotat_field->getSFRotation());
    //cout << "\nL\t" << LPoint.x << " : " << LPoint.z << "\t\t\t" << "R\t" << RPoint.x << " : " << RPoint.z;
    
//CheckOutLine
    if(MoveLine(IRCar_trans_field->getSFVec3f(), Next_Line))
        Next_Line++;
        
    if(CheckOutLine(LPoint, RPoint, Next_Line)) {
       supervisor->simulationSetMode(webots::Supervisor::SimulationMode::SIMULATION_MODE_PAUSE);
    }   
    
    
//CheckPoint
    if(CheckPointCMP(IRCar_trans_field->getSFVec3f(), Next_CP)) {
      CP_time[Next_CP] = supervisor->getTime();
      cout << "\nCheckPoint " << Next_CP << ": \t" << CP_time[Next_CP] << "\n";
      Next_CP++;

    }
  }

  // Enter here exit cleanup code.


  delete supervisor;
  return 0;
}
