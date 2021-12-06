"""my_python_controller controller."""

# You may need to import some classes of the controller module. Ex:
#  from controller import Robot, Motor, DistanceSensor
from controller import Robot
from controller import Motor
from controller import DistanceSensor
from controller import Camera
from controller import LED
from controller import Supervisor
import sys
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
        # leds[1].set(not(leds[1].get()))
        leds[1].set(1)
        # for i in range(NB_LEDS):
        # leds[i].set(not(leds[i].get()))
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
LEFT = 1
RIGHT = 2
FULL_SIGNAL = 3
BLANK_SIGNAL = 4
ROUNDABOUT_SIGNAL = 5
# Điều chỉnh tốc độ phù hợp
MAX_SPEED = 20

threshold = [300, 300, 300, 300, 300, 300, 300, 300]
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
    if (filted == 0b00010000 or filted == 0b00001000 or filted == 0b00011000):
        return MID
    elif (filted == 0b10000000 or filted == 0b11000000 or filted == 0b01100000 or filted == 0b00110000):
        return RIGHT
    elif (filted == 0b00000001 or filted == 0b00000011 or filted == 0b00000110 or filted == 0b00001100):
        return LEFT
    elif (filted == 0b11111111 or filted == 0b01111111 or filted == 0b11111110 or filted == 0b01111110):
        return FULL_SIGNAL
    elif(filted == 0b11100111 or filted == 0b11000111 or filted == 0b11100011):
        return ROUNDABOUT_SIGNAL
    elif filted == 0b00000000:
        return BLANK_SIGNAL
    return NOP


def GoStraight(filted):
    if filted == 0b00010000:
        # return left_ratio, right_ratio
        return 0.95, 1.0
    if filted == 0b00001000:
        return 1.0, 0.95
    if filted == 0b00011000:
        return 1.0, 1.0
    return 1.0, 1.0


def TurnRight(filted):
    if filted == 0b00001100:
        # return left_ratio, right_ratio
        return 0.9, 0.7
    if filted == 0b00000110:
        return 0.8, 0.55
    if filted == 0b00000011:
        return 0.7, 0.3
    if filted == 0b00000001:
        return 0.9, 0.2
    return 1.0, 1.0


def TurnLeft(filted):
    if filted == 0b00110000:
        # return left_ratio, right_ratio
        return 0.7, 0.9
    if filted == 0b01100000:
        return 0.55, 0.8
    if filted == 0b11000000:
        return 0.3, 0.7
    if filted == 0b10000000:
        return 0.2, 0.9
    return 1.0, 1.0


def TurnLeftCorner():
    # return left_ratio, right_ratio
    return 0.1, 0.75


def TurnRightCorner():
    # return left_ratio, right_ratio
    return 0.75, 0.1


def contrain(value, min, max):
    if value < min:
        return min
    if value > max:
        return max
    return value


def SetVelocity(left_ratio, right_ratio):
    left_ratio = contrain(left_ratio, 0, 1)
    right_ratio = contrain(right_ratio, 0, 1)
    lm.setVelocity(left_ratio * MAX_SPEED)
    rm.setVelocity(right_ratio * MAX_SPEED)


stop = 0
chk = 99

# Main loop:
# - perform simulation stegs until Webots is stopping the controller
# Chương trình sẽ được lặp lại vô tận
while robot.step(timestep) != -1:
    # Đọc giá trị của sensor
    filted = ReadSensors()
    # In ra màn hình giá trị của filted ở dạng nhị phân
    #print('Position: ' + str(format(filted, '08b')), sep='\t')
    # Xác định vị trí của xe so với làn đường
    pos = DeterminePosition(filted)
    # Xác định hướng rẽ của ngã tư
    if (filted == 0b11110000 or filted == 0b11111000):
        intersectionDirect = LEFT
       # print("\nLEFT")
    elif (filted == 0b00001111 or filted == 0b00011111):
        intersectionDirect = RIGHT
     #   print("\nRIGHT")
    elif (filted == 0b01110000):
        pos = 10
    #    print("\nROUNDABOUT")

    # Xác định tỉ lệ tốc độ của mỗi động cơ -> Điều khiển xe
    if pos == MID:
        stop = 0
        left_ratio, right_ratio = GoStraight(filted)
     #   print('\tGo Straight')
    elif pos == LEFT:
        stop = 0
        left_ratio, right_ratio = TurnRight(filted)
      #  print('\tLEFT')
    elif pos == RIGHT:
        stop = 0
        left_ratio, right_ratio = TurnLeft(filted)
     #   print('\tRIGHT')
    elif pos == 10:
        stop = 0
        left_ratio, right_ratio = TurnRightCorner()
      #  print('\tEXIT')
        intersectionDirect = MID
    elif pos == BLANK_SIGNAL:
        if (preFilted == 0b11000000 or preFilted == 0b11100000 or preFilted == 0b11110000 or preFilted == 0b11111000 or preFilted == 0b10000000):
            # Điều khiển xe rẽ góc vuông trái
            left_ratio, right_ratio = TurnLeftCorner()
        #    print('Turn left corner')
            # -----------------------
        elif (preFilted == 0b00000011 or preFilted == 0b00000111 or preFilted == 0b00001111 or preFilted == 0b00011111 or preFilted == 0b00000001):
            # Điều khiển xe rẽ góc vuông phải
            left_ratio, right_ratio = TurnRightCorner()
            #print('Turn right corner')
            # -----------------------
        elif (preFilted == 0b00011000 or preFilted == 0b00010000 or preFilted == 0b00001000 or preFilted == 0b00000000):
            # Điều khiển xe mất line
        #    print("Lost of road markings\n")
            if(stop == 0):
                stop = robot.getTime()
                #print(robot.getTime())
            if (robot.getTime()-stop)*1000 < 500:
                #print(robot.getTime()-stop)
                GoStraight(0b00001100)
            else:
                left_ratio, right_ratio = 0, 0
        elif(preFilted == 0b11111111 or preFilted == 0b11000011):
       #     print('END')
            left_ratio, right_ratio = 0, 0
        intersectionDirect = MID
    elif pos == FULL_SIGNAL:
        stop = 0
        if intersectionDirect == LEFT:
            # Điều khiển xe rẽ ngã tư trái
       #     print("Turn left intersection\n")
            left_ratio, right_ratio = TurnLeftCorner()
            # -----------------------
        elif intersectionDirect == RIGHT:
            # Điều khiển xe rẽ ngã tư phải
        #    print("Turn right intersection\n")
            left_ratio, right_ratio = TurnRightCorner()
        else:
         #   print("Pre_ROUNDABOUT\n")
            left_ratio, right_ratio = 1, 1
    elif pos == ROUNDABOUT_SIGNAL:
     #   print("ROUNDABOUT\n")
        left_ratio, right_ratio = TurnRightCorner()
        # elif (preFilted3 == 0b11111111):
        #     print("END\n")
        #     left_ratio, right_ratio = 0,0
        # -----------------------
    # Giới hạn tỉ lệ tốc độ của động cơ
    # left_ratio = contrain(left_ratio, 0, 1)
    # right_ratio = contrain(right_ratio, 0, 1)
    # Điều chỉnh tốc độ động cơ
    # lm.setVelocity(left_ratio * MAX_SPEED)
    # rm.setVelocity(right_ratio * MAX_SPEED)
    SetVelocity(left_ratio, right_ratio)
    if(filted != 0b00000000):
        preFilted = filted
    pass
pass


# Enter here exit cleanup code.
