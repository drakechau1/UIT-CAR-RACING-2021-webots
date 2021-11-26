"""Test controller."""

# You may need to import some classes of the controller module. Ex:
#  from controller import Robot, Motor, DistanceSensor
from controller import Robot
from controller import Motor
from controller import DistanceSensor
from controller import Camera
from controller import LED
from controller import Supervisor
import math

# create the Robot instance
robot = Robot()

# get the time step of the current world
timestep = 32

# You should insert a getDevice-like function in order to get the
# instance of a device of the robot. Something like:
#  motor = robot.getDevice('motorname')
#  ds = robot.getDevice('dsname')
#  ds.enable(timestep)
robot.step(timestep)
        
# Camera
cam = robot.getDevice("camera")
cam.enable(64)
    
# Leff motor
lm = robot.getDevice("left wheel motor")
lm.setPosition(float("inf"))
lm.setVelocity(0)
lm_max_speed = lm.getMaxVelocity()

# Right motor
rm = robot.getDevice("right wheel motor")
rm.setPosition(float("inf"))
rm.setVelocity(0)
rm_max_speed = rm.getMaxVelocity()

# Sensors
NB_GROUND_SENS = 8
gs = []
gsNames = [
    'gs0', 'gs1', 'gs2', 'gs3',
    'gs4', 'gs5', 'gs6', 'gs7'
]
for i in range(NB_GROUND_SENS):
    gs.append(robot.getDevice(gsNames[i]))
    gs[i].enable(timestep)

# LEDs
NB_LEDS = 5
leds = []
led_Names = [
    'led0', 'led1', 'led2', 'led3', 'led4'
]
for i in range(NB_LEDS):
    leds.append(robot.getDevice(led_Names[i]))

# Waiting for completing initialization
initTime = robot.getTime()
while robot.step(timestep) != -1:
    if (robot.getTime() - initTime) * 1000.0 > 200:
        break

### Private Functions ###
# You should declare all your functions to get data, process data and
# control your robot outputs such as: motors, LEDs, etc in this field.

# Function to read data from ground sensors
def getSensors():
    gsValues = []
    for i in range(NB_GROUND_SENS):
        gsValues.append(gs[i].getValue())
    #print(*gsValues, sep = '\t')
    return gsValues

# Function to control LEDs
def LED_Alert():
    if (robot.getTime() - initTime)*1000 % 3000 >= 2000:
        #leds[1].set(not(leds[1].get()))
        leds[1].set(1)
        #for i in range(NB_LEDS):
            #leds[i].set(not(leds[i].get()))
    return

# Function to control Motors
def Line_Following_Module():
    if (robot.getTime() - initTime) * 1000.0 < 5000:
        lm.setVelocity((10 / 1000.0) * lm_max_speed)
        rm.setVelocity((10 / 1000.0) * rm_max_speed)
    else:
        lm.setVelocity((0 / 1000.0) * lm_max_speed)
        rm.setVelocity((0 / 1000.0) * rm_max_speed)
    return

# Main loop:
# - perform simulation stegs until Webots is stopping the controller
while robot.step(timestep) != -1:
    # Read the sensors:
    # Enter here functions to read sensor data, like:
    #  val = ds.getValue()
    sensors_Data = getSensors()
    print(*sensors_Data, sep = '\t')
    # Process sensor data here.
    LED_Alert()
    # Enter here functions to send actuator commands, like:
    #  motor.setPosition(10.0)
    Line_Following_Module()
    pass

# Enter here exit cleanup code.
