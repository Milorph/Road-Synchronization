# About:

There is a two-lane road with one lane closed, and vehicles are 
approaching from the North and South directions. Due to the traffic lights, the cars arrive in bursts. When 
a car reaches the construction area, there is an 80% chance that another car will follow it. However, if no 
car comes, there will be a 20-second gap (utilizing the provided pthread_sleep function) before any new 
car arrives. 
<br />
<br />
During the intervals where no cars are at either end, the flag person will rest. However, when a car arrives 
at either end, the flag person will wake up and manage the traffic flow from that side, until there are no 
more cars from that side or until there are 10 or more cars waiting in the opposite direction. If there are 10 
or more cars on the opposite side, the flag person must allow those cars to pass first. 
Each car takes one second (using the provided pthread_sleep function) to travel through the construction 
area. Your task is to build a simulation of these events, ensuring that a deadlock never occurs. A deadlock 
can  be  defined  as  a  situation  where  the  flag  person  neither  permits  nor  blocks  traffic  from  either  side 
(which allows vehicles to move from both sides, leading to a collision). 
Road Synchronization


### Total threads: 3

## [Thread #1]
  --> Task: Producer that produce
  <br />
  --> Thread Function: northTraffic

## [Thread #2]
  --> Task: Producer that produces car (object)
  <br />
  --> Thread Function: southTraffic

## [Thread #3]
—> Task: Consumer that consumes the car (object)
<br />
—> Thread Function: worker
..............
#Semaphores
Number of semaphores:
## [Semaphore #1]
  --> Variable: sem_north
  <br />
  --> Initial value:1
  <br />
  --> Purpose: make northTraffic blocked upon on events car arrives from the North

## [Semaphores#2]
 --> Variable: sem_south
 <br />
 --> Initial value: 1
 <br />
 --> Purpose: make southTraffic blocked upon on events car arrives from the South
..........

# Mutex lock 
<br />
Number of mutex locks:1

## [Mutex lock #1]
  --> Variable:locker
  <br />
  --> Purpose: avoid race condition on shared data structure of the northTraffic and southTraffic accepting cars waiting and the process where north and south passes when they are removed from the queue.


