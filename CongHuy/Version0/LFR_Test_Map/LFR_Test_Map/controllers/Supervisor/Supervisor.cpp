
#include <webots/Supervisor.hpp>
#include <webots/Node.hpp>
using namespace webots;

#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <cassert>
using namespace std;

#define TIME_STEP 32

// Coord for checkPoint
typedef struct {
  double x, z, dx, dz;
} Coord;


// CheckPoint
Coord CP[11] = {{-0.44, 0.6, 0.03, 0.081}, {1.15, 0.6, 0.081, 0.03}, {1.17, -0.99, 0.081, 0.03},
                {0.87, 0.06,0.03, 0.081}, {0.51, -0.29, 0.03, 0.081},{-0.15, 0.06, 0.03, 0.081},
              {-0.68, -0.89, 0.03, 0.081}, {-1.35, -0.34, 0.081, 0.03}, {-1.35, 0.56, 0.081, 0.03},
              {-1.35, 1.03, 0.081, 0.02}, {-1.35, 1.25, 0.081, 0.03}};

//Start point of every line
Coord CL[29] = {{1.09, 1.18, 0.04, 0.081},
              {-0.4, 1.18, 0.02, 0.081}, {-0.4, 0.6, 0.02, 0.081}, {0.035,0.6, 0.02, 0.081},
              {0.275, 0.835, 0.02, 0.081}, {0.508, 0.6, 0.02, 0.081}, {0.745, 0.835, 0.02, 0.081},
              {0.985, 0.6, 0.02, 0.081}, {1.45, 0.6, 0.02, 0.081}, {1.45, -1.08, 0.081, 0.02},
              {1.17, -1.08, 0.081, 0.02},{1.17, 0.06, 0.081, 0.02}, {0.51, 0.06, 0.02, 0.081}, 
              {0.51, -0.5, 0.081, 0.02}, {0.65, -0.64, 0.02, 0.081}, {0.65, -0.92, 0.02, 0.081},
              {0.09, -0.92, 0.02, 0.081}, {0.1, -0.64, 0.02, 0.081}, {0.25, -0.5, 0.081, 0.02},
              {0.25, -0.08, 0.081, 0.02}, {0.11, 0.06, 0.02, 0.081}, {-0.53, 0.06, 0.02, 0.081},
              {-0.53, -0.89, 0.081, 0.02}, {-1.21, -0.89, 0.02, 0.081}, {-1.35, -0.75, 0.081, 0.02},
              {-1.35, -0.14, 0.081, 0.02}, {-1.35, 0.44, 0.081, 0.02}, {-1.35, 1.03, 0.081, 0.02},
              {-1.35, 1.24, 0.081, 0.02}};
int LineType[28] = {0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 3, 4, 0, 5, 6, 0, 7, 0, 0, 0, 8, 0, 9, 0, 0};              
// Rigt and Left Sensor translation from center of car
Coord LPoint0 = {-0.0449, -0.1288};
Coord RPoint0 = {0.0449, -0.1288};
Coord Trans0  = {1.18842, 1.18029};
bool CheckPointCMP(const double *trans, int count){
      return (trans[0] >= CP[count - 1].x - CP[count - 1].dx && trans[0] <=  CP[count - 1].x + CP[count - 1].dx && trans[2] >= CP[count - 1].z - CP[count - 1].dz && trans[2] <= CP[count - 1].z + CP[count - 1].dz);
}



Coord Rotation(const double *trans, Coord coord ,const double *rotat) {
    Coord  Result;
    if(rotat[1] < 0) {
      Result.x = trans[0]  + coord.x*cos(rotat[3]) - coord.z*sin(rotat[3]);
      Result.z = trans[2]  + coord.x*sin(rotat[3]) + coord.z*cos(rotat[3]);
    }
    else {
      Result.x = trans[0]  + coord.x*cos(-rotat[3]) - coord.z*sin(-rotat[3]);
      Result.z = trans[2]  + coord.x*sin(-rotat[3]) + coord.z*cos(-rotat[3]);
    }
    
    return Result;
}

//Duong thang
bool StraightLine(int index, Coord L, Coord R, double d) {
     double a = CL[index-1].z - CL[index].z;
     double b = CL[index].x - CL[index-1].x;
     double c = -CL[index-1].x * a - CL[index-1].z * b;
     
     if ((abs(a*L.x + b*L.z + c) / sqrt(a*a + b*b)) > d || (abs(a*R.x + b*R.z + c) / sqrt(a*a + b*b)) > d)
          return true;
     return false;
}

//Duong cong
bool CurveLine(int index, Coord L, Coord R, double Ia, double Ib, double R1, double R2) {
        if ( (((L.x - Ia) * (L.x - Ia) + (L.z - Ib) * (L.z - Ib)) > (R1*R1) &&
              ((L.x - Ia) * (L.x - Ia) + (L.z - Ib) * (L.z - Ib)) < (R2*R2))
             &&
             (((R.x - Ia) * (R.x - Ia) + (R.z - Ib) * (R.z - Ib)) > (R1*R1) &&
              ((R.x - Ia) * (R.x - Ia) + (R.z - Ib) * (R.z - Ib)) < (R2*R2))
             ) {
             //cout<< "CheckCurver:  " << ((L.x - Ia) * (L.x - Ia) + (L.z - Ib) * (L.z - Ib)) << "    " << ((R.x - Ia) * (R.x - Ia) + (R.z - Ib) * (R.z - Ib)) << "\n";
             return false;
             }
        return true;
}
bool CheckOutLine(Coord LP, Coord RP, int numLine) {
    switch (LineType[numLine-1]) {
    case 0: 
      return StraightLine(numLine, LP, RP, 0.13);
      
    case 1:
      return CurveLine(numLine, LP, RP, -0.4, 0.89, 0.16, 0.42);
      
    case 2:
      return CurveLine(numLine, LP, RP, 1.31, -1.08, 0.01, 0.27);
      
     case 3:
      return CurveLine(numLine, LP, RP, 0.65, -0.5, 0.01, 0.27);
      
     case 4:
      return CurveLine(numLine, LP, RP, 0.65, -0.78, 0.01, 0.27);
      
     case 5:
      return CurveLine(numLine, LP, RP, 0.11, -0.78, 0.01, 0.27);
      
     case 6:
      return CurveLine(numLine, LP, RP, 0.11, -0.5, 0.01, 0.27);
      
     case 7: 
      return CurveLine(numLine, LP, RP, 0.11, -0.08, 0.01, 0.27);
      
     case 8: 
      return CurveLine(numLine, LP, RP, -1.21, -0.75, 0.01, 0.27);
      
     case 9: 
      return CurveLine(numLine, LP, RP, -1.35, 0.15, 0.16, 0.42);
     default: return false;     
  }
}
//Chuyen sang line khac
bool ForwardLine(Coord trans, int count) {
  if ((trans.x >= (CL[count].x - CL[count].dx)) && (trans.x <= (CL[count].x + CL[count].dx))  && (trans.z >= (CL[count].z -CL[count].dz)) && (trans.z <= (CL[count].z + CL[count].dz)))
    return true;
  else return false;
} 

//void rot(const double *trans) {
//  cout << trans[1] << "\t\t" << trans[3] << "\n";
//  return;
//}
int main(int argc, char **argv) {

  
  // init Supervisor
  Supervisor *supervisor = new Supervisor();
  Node *IRCar = supervisor->getFromDef("Speed_Line_Follower_Robot_V4");
  Field *IRCar_trans_field = IRCar->getField("translation");
  Field *IRCar_rotat_field = IRCar->getField("rotation");
  //cout << "Orientation:" << IRCar->getOrientation() << "\n";
  
  double CP_time[11];
  int Next_CP = 1;
  int Next_Line = 1;

  
  while (supervisor->step(TIME_STEP) != -1) {
    Coord LPoint = Rotation(IRCar_trans_field->getSFVec3f(), LPoint0, IRCar_rotat_field->getSFRotation());
    Coord RPoint = Rotation(IRCar_trans_field->getSFVec3f(), RPoint0, IRCar_rotat_field->getSFRotation());
    Coord CtPoint = {(LPoint.x + RPoint.x)/2 , (LPoint.z + RPoint.z)/2, 0, 0}; 
    const double* EndPoint = IRCar_trans_field->getSFVec3f();
    
//CheckOutLine
    //Forward Line
    if (Next_Line < 27) {          
      if(ForwardLine(CtPoint, Next_Line)){
          cout <<"\nCheckLine" << Next_Line << "\n\n";
          Next_Line++;  
      }
    //else cout << "Next" << Next_Line << "\n"; 
    //Check    
      if(CheckOutLine(LPoint, RPoint, Next_Line)) {
        //cout << "\nL\t" << LPoint.x << " : " << LPoint.z << "\t\t\t" << "R\t" << RPoint.x << " : " << RPoint.z << "\t\t\t" << "C\t" << CtPoint.x << " : " << CtPoint.z << "\n";
         supervisor->simulationSetMode(webots::Supervisor::SimulationMode::SIMULATION_MODE_PAUSE);  
      }   
    }
    //rot(IRCar_rotat_field->getSFRotation());
//CheckPoint
    if (Next_CP < 10){
      if(CheckPointCMP(IRCar_trans_field->getSFVec3f(), Next_CP)) {
        CP_time[Next_CP] = supervisor->getTime();
        supervisor->wwiSendText("C.0" + to_string(Next_CP) + "." + to_string(CP_time[Next_CP]));
        cout <<"CheckPoint\t" << "C.0" + to_string(Next_CP) + "." + to_string(CP_time[Next_CP]) << "\n" ;
        
        Next_CP++;
      }
    }
    else {
      if(CheckPointCMP(IRCar_trans_field->getSFVec3f(), Next_CP))
          {
          CP_time[10] = supervisor->getTime();
          cout << "\nCP10:" << CP_time[10] << "\n";
          }
      if (CP_time[10] > 0) {
        if ((supervisor->getTime() - CP_time[10] < 3) && (EndPoint[2] > CP[10].z || EndPoint[0] < -1.45 || EndPoint[0] > -1.25)) {
             supervisor->simulationSetMode(webots::Supervisor::SimulationMode::SIMULATION_MODE_PAUSE);
        }
        else if ((supervisor->getTime() - CP_time[10] > 3) && (EndPoint[3] < CP[8].z)){
              supervisor->wwiSendText("C.10." + to_string(CP_time[10])); 
              cout <<"CheckPoint\t"<< "C.10." + to_string(CP_time[10]) << "\n";
           }
        }  
      }      
  }

  // Enter here exit cleanup code.


  delete supervisor;
  return 0;
}
