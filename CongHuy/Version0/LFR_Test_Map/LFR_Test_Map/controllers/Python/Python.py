"""my_python_controller controller."""

# You may need to import some classes of the controller module. Ex:
#  from controller import Robot, Motor, DistanceSensor
from controller import Robot
from controller import Motor
from controller import DistanceSensor
from controller import Camera
from controller import LED
from controller import Supervisor
import math


### Phần code không nên chỉnh sửa ### 
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

# Right motor
rm = robot.getDevice("right wheel motor")
rm.setPosition(float("inf"))
rm.setVelocity(0)

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

### Private Functions ###
# Function to control LEDs
def LED_Alert():
    if (robot.getTime() - initTime)*1000 % 3000 >= 2000:
        #leds[1].set(not(leds[1].get()))
        leds[1].set(1)
        #for i in range(NB_LEDS):
            #leds[i].set(not(leds[i].get()))
    return

# Waiting for completing initialization
initTime = robot.getTime()
while robot.step(timestep) != -1:
    if (robot.getTime() - initTime) * 1000.0 > 200:
        break


### Phần code được phép chỉnh sửa cho phù hợp ###

# Định nghĩa các tín hiệu của xe
NOP = -1
MID = 0
LEFT_1 = 1
LEFT_2 = 2
LEFT_3 = 3
LEFT_4 = 4
LEFT_5 = 5
LEFT_6 = 6
RIGHT_1 = -1
RIGHT_2 = -2
RIGHT_3 = -3
RIGHT_4 = -4
RIGHT_5 = -5
RIGHT_6 = -6
FULL_SIGNAL  = 8
BLANK_SIGNAL = -8
LEFT = 9
RIGHT = -9
LEFT_inter = 10
RIGHT_inter = -10
MAX_SPEED = 20

threshold = [350, 350, 350, 350, 350, 350, 350, 350]
#threshold = [370, 370, 370, 370, 370, 370, 370, 350]
#threshold = [400, 400, 400, 400, 400, 400, 400, 400]
preFilted = 0b00000000

# Biến lưu giá trị tỉ lệ tốc độ của động cơ
left_ratio = 0.0
right_ratio = 0.0

# Hàm đọc giá trị của sensors
def ReadSensors():
    gsValues = []
    filted = 0x00
    for i in range(NB_GROUND_SENS):
        gsValues.append(gs[i].getValue())
        if gsValues[i] > threshold[i]:
            filted |= (0x01 << (NB_GROUND_SENS - i - 1))
    # print(*gsValues, sep = '\t')
    return filted

# Phần code điều khiển xe
def DeterminePosition(filted):
    #giữ
    if (filted == 0b00010000 or filted == 0b00001000 or filted == 0b00011000):
        return MID
    #sửa
    elif (filted == 0b00110000 or filted == 0b00100000):
        return RIGHT_1
    elif (filted == 0b00001100 or filted == 0b00000100):
        return LEFT_1
    elif (filted == 0b01100000):
        return RIGHT_2
    elif (filted == 0b00000110):
        return LEFT_2
    elif (filted == 0b01000000 or filted == 0b00111000):
        return RIGHT_3
    elif (filted == 0b00000010 or filted == 0b00011100):
        return LEFT_3
    elif (filted == 0b10000000 or filted == 0b11000000 or filted == 0b01110000):
        return RIGHT_4
    elif (filted == 0b00000001 or filted == 0b00000011 or filted == 0b00001110):
        return LEFT_4
    elif (filted == 0b11100000 or filted == 0b11110000 or filted == 0b01111000):
        return RIGHT_5
    elif (filted == 0b00000111 or filted == 0b00001111 or filted == 0b00011110):
        return LEFT_5
    elif (filted == 0b11111000 or filted == 0b11111100):
        return RIGHT_6
    elif (filted == 0b00011111 or filted == 0b00111111):
        return LEFT_6   
   
    elif (filted == 0b11111111 or filted == 0b01111111 or filted == 0b11111110 or filted == 0b01111110):
        return FULL_SIGNAL    
    elif filted == 0b00000000:
        return BLANK_SIGNAL
    else:
        return NOP
    
def GoStraight(filted):
    #print("Go straight\n")
    if filted == 0b00010000:
        return 0.9, 1.0
    if filted == 0b00001000:
        return 1.0, 0.9
    if filted == 0b00011000:
        return 1.0, 1.0
    return 1.0, 1.0

def TurnLeft(filted):
    #print("turn left\n")
    pos = DeterminePosition(filted)
    if pos == RIGHT_1:
        #print("1")
        return 0.6, 1.0
    if pos == RIGHT_2:
        #print("2")
        return 0.5, 1.0
    if pos == RIGHT_3:
        #print("3")
        return 0.4, 1.0
    if pos == RIGHT_4:
        #print("4")
        return 0.0, 1.0
    return 0.0, 1.0
    
def TurnRight(filted):
    #print("turn right\n")
    pos = DeterminePosition(filted)
    if pos == LEFT_1:
        return 1.0, 0.6
    if pos == LEFT_2:
        return 1.0, 0.5
    if pos == LEFT_3:
        return 1.0, 0.4
    if pos == LEFT_4:
        return 1.0, 0.0
    return 1.0, 0.0

def contrain(value, min, max):
    if value < min:
        return min
    if value > max:
        return max
    return value
    
def TurnInter(interSignal):
    #print("turninter fuction is calling\n")
    if interSignal == LEFT_inter:
        #print("LEFT\n")
        return -4.0, 4.0
    elif interSignal == RIGHT_inter:
        #print("RIGHT\n")
        return 4.0, -4.0
        
timeCounter = 0
#lastPos = 0        
# Main loop:
# - perform simulation stegs until Webots is stopping the controller
# Chương trình sẽ được lặp lại vô tận 
while robot.step(timestep) != -1:
    filted = ReadSensors()
    #print('Position: ' + str(format(filted, '08b')), sep = '\t')

    #print(robot.getTime()*1000.0)print(lastPos)
    pos = DeterminePosition(filted)
   
    #lastPos = lastPosition(filted)
    #lastpos setup
    
    if (pos == LEFT_1 or pos == LEFT_2 or pos == LEFT_3 or pos == LEFT_4  or pos == LEFT_5  or pos == LEFT_6):
        lastPos = LEFT
    elif (pos == RIGHT_1 or pos == RIGHT_2 or pos == RIGHT_3 or pos == RIGHT_4 or pos == RIGHT_5 or pos == RIGHT_6):
        lastPos = RIGHT
    elif (pos == MID):
        lastPos= MID
    
    #intersection signal setup
    if (pos == RIGHT_5 or pos == RIGHT_6):
        #print("a left intersection a head\n")
        interSignal = LEFT_inter
        currentTime = robot.getTime()
        #print("inter: LEFT_inter\n")
    elif (pos == LEFT_5 or pos == LEFT_6):
        #print("a right intersection a head\n")
        interSignal = RIGHT_inter
        currentTime = robot.getTime()
        #print("inter: RIGHT_inter\n" )  
       
    #to go straight
    if pos == MID:
        left_ratio, right_ratio = GoStraight(filted)
        timeCounter = 0
    #to turn right
    elif (pos == LEFT_1 or pos == LEFT_2 or pos == LEFT_3 or pos == LEFT_4):
        left_ratio, right_ratio = TurnRight(filted)
    #to turn left
    elif (pos == RIGHT_1 or pos == RIGHT_2 or pos == RIGHT_3 or pos == RIGHT_4):
        left_ratio, right_ratio = TurnLeft(filted)
    #
    elif pos == BLANK_SIGNAL:
        #mat line
        #cua tron`
        #print("###blank signal\n")
        #left_ratio,right_ratio = GoStraight(0b00011000)
        #print("inter: " + str(interSignal))
        
        if lastPos == LEFT:
            left_ratio, right_ratio = TurnRight(0b00011111)
        elif lastPos == RIGHT:
            left_ratio, right_ratio = TurnLeft(0b11111000)
        elif lastPos == MID:
            left_ratio,right_ratio = GoStraight(0b00011000)
        #cua 90
        elif interSignal == LEFT_inter:#leftInter = 10
            #print("turning left\n")
            left_ratio, right_ratio = TurnInter(interSignal)
        elif interSignal == RIGHT_inter:#rightInter = -10
            #print("turning right \n")
            left_ratio, right_ratio = TurnInter(interSignal)
        
    elif pos == FULL_SIGNAL:
        #print("###FULL SIGNAL\n")
        timer = robot.getTime() - currentTime
        if timer*1000.0 < 1200:
            if  (interSignal == LEFT_inter):#leftInter = 10
                #print("turning left intersection\n")
                left_ratio, right_ratio = TurnInter(interSignal)
            elif (interSignal == RIGHT_inter):#rightInter = -10
                #print("turning right intersection\n")
                left_ratio, right_ratio = TurnInter(interSignal)
        else:
            left_ratio, right_ratio = GoStraight(0b00011000)
            timeCounter += 1
    elif pos == NOP:
        pos = lastPos
     
    #print("time counter =" +str(timeCounter))   
    if timeCounter >= 15:
        left_ratio, right_ratio = 0.0, 0.0        
    # Giới hạn tỉ lệ tốc độ của động cơ
    #left_ratio = contrain(left_ratio, 0, 1)
    #right_ratio = contrain(right_ratio, 0, 1)
    # Điều chỉnh tốc độ động cơ
    lm.setVelocity(left_ratio * MAX_SPEED)
    rm.setVelocity(right_ratio * MAX_SPEED)
    
    preFilted = filted
    
    pass

# Enter here exit cleanup code.