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

void sendStopToUI()
{
}

void sendRunToUI()
{
}

void sendResetToUI()
{
}

void sendPauseToUI()
{
}

void sendTimeToUI(int Time)
{
	
}

int main() {

  Supervisor *supervisor = new Supervisor();
  
  supervisor->wwiSendText("Hello World 1!");
  supervisor->wwiSendText("Hello World 2!");
  	
  double  time = supervisor->getTime();
  
  // Main loop:
  while (supervisor->step(TIME_STEP) != -1) {
    time = supervisor->getTime();
    supervisor->wwiSendText("T" + to_string(time));
  };
  
  // Enter here exit cleanup code.
  delete supervisor;
  return 0;
}
