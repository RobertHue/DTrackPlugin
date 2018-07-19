# DTrackPlugin Dev-Notes

## Table of Contents
1. [About](#about)
4. [Additional Infos](#Additional Infos)

## About

## DTrack-Plugin Class Architecture

The Class Architecture of the DTrack-Plugin, which controls an Actor, is mainly divided into three parts:

* The `DTrackPollThread`'s task is to pick up DTrack data coming from the DTrack SDK, hence the A.R.T. track controller. It then Converts the DTrack-Space (mm, RHS) into Unreal's Space (cm, LHS) with the use of the FCoordinateConverter (SpaceConverter).
After this is done it places ("injects") the newly converted data into a data-buffer (see #Data-Buffer). In case of the quality being Zero (0), it doesn't convert or inject anything. Hence the PollThread is there to ease the game loop off being overloaded too much by doing that kind of processing. 

* The `FDTrackPlugin` itself, to `start_up` (register) or `remove` (unregister) clients (Actors) to be notified, to store the converted DTrack data received by the `DTrackPollThread` (see #Data-Buffer) and to forward that newly "injected" data coming from the `DTrackPollThread` to the `UDTrackComponent`.

* The `UDTrackComponent` attached to the Actor, that is being controlled, which enables the `FDTrackPlugin` to tick and does callback the corresponding handler of the Actor-Implemented `IDTrackInterface`-Method.




![DTrack-Plugin UML Class Diagram](../../images/DTrackPlugin_UML_ClassDiagram.png)

## Data-Buffer

To store the data coming from DTrack for the different Game-Actors to pick up<sup>1</sup>, there needs to be a shared memory between DTrackPollThread and the game itself.

### Double-Buffering

The DTrackPollThread does not write directly into the currently used data-buffer of the UDTrackComponent. It instead writes into a second data-buffer<sup>2</sup> (called: `m_injected`). This has the advantage that the Plugin can still process the data on the first data-buffer (here: m_front). Meanwhile the DTrackPollThread can inject its new data into the m_injected data-buffer.

To swap out the two buffers, so that the Plugin can use the new data, there has to be some form of mutual exclusion to avoid swapping of the buffers, while the m_front is still being used. To ensure this, there has been introduced a mutex on the data-buffers (here: swapping_mutex). Each time after the DTrackPollThread receives new data, converts it and injects it into the m_injected data-buffer, it tries to Lock access on the buffers by locking a mutex. It then can safely swap m_front and m_injected.

<sup>1</sup>   In reality the Actor does not pick up anything, whereas the DTrackComponent does a callback on the Actor's implemented IDTrackInterface-Method

<sup>2</sup>   There has been a bug, where targets not being visible made the connected object inside unreal wiggle around. This bug got solved by not writing new data into the injected data-buffer.

### Data (Buffer) Structure

To get a overview of how the structure looks like, see:

![DTrack-Plugin DataBuffer Structure](../../images/DataBufferStructure.png)







## Space Conversion

# Rotations
For converting between spaces a similar concept from https://www.geometrictools.com/Documentation/ConvertingBetweenCoordinateSystems.pdf has been used.

To convert from right-handed coordinate system (DTrack) to left-handed coordinate system (Unreal-Engine 4).
This is done by flipping one axis direction, namely the z-axis (-Z).

# Locations
Also the units of DTrack and Unreal differ. In DTrack the unit of [cm] is used, whereas in Unreal the unit [mm] is used.





![Unreal model of a human hand right](../../images/UE_Hand_Right.png)


## Fingertracking

As depicted in the image below, the DTrack model of a human left hand.
![DTrack model of a human hand left](../../images/DTrack_model_of_human_hand_left.png)


You can see the DTrack model of a human right hand as depicted in ....
It doesn't matter whether left or right hand the positive X-Axis is always orientated along the finger.
Here, for the back of the hand, the positive Z-Axis is pointing down. Whereas, for the phalanxes the positive Z-Axis is pointing upwards.

![DTrack model of a human hand right](../../images/DTrack_model_of_human_right.png)


### Applying the DTrack provided angles to all the effectors in the kinematic chain

One approach is to get the different angles between the finger bones and apply them to the unreal skeleton. A Disadvantage in this solution is, that when your fingers don't have the lengths of the unreal skeleton fingers, then your end-effector position may be off (even though DTrack uses an IK-Method and statistics about fingers internally).

Only problem is, that DTrack does not provide the angle between inner phalanx to backOfHand. These can be calculated as follows:

`
FRotator rotatorFingerTip = m_coord_converter.from_dtrack_rotation(hand->finger[j].rot);
finger.m_rotation = FRotator(rotatorFingerTip);
FQuat fingerTipOrientation(finger.m_rotation);

finger.m_middle_outer_phalanx_angle = hand->finger[j].anglephalanx[0];  // gamma
finger.m_inner_middle_phalanx_angle = hand->finger[j].anglephalanx[1];	// beta

FQuat rotationGammaQuat(FVector::ForwardVector, -finger.m_middle_outer_phalanx_angle * DEG_TO_RAD); 
FQuat rotationBetaQuat(FVector::ForwardVector,  -finger.m_inner_middle_phalanx_angle * DEG_TO_RAD);

FQuat quatOuter = fingerTipOrientation;
FQuat quatMiddle = quatOuter  * rotationGammaQuat;		// first apply rotationGammaQuat and then quatOuter
FQuat quatInner  = quatMiddle * rotationBetaQuat;		// first apply rotationBetaQuat  and then quatMiddle

// add to proper UE4-rotation part: (is needed because forward is pointing to the negative side upwards the finger bones)
FRotator rotatorInner = quatInner.Rotator();
finger.m_hand_inner_phalanx_angle_pitch = -rotatorInner.Roll - 180.f;
finger.m_hand_inner_phalanx_angle_yaw = rotatorInner.Yaw - 180.f;
finger.m_hand_inner_phalanx_angle_roll = rotatorInner.Pitch; // should be zero (unchanged), because its not possible to do a finger twist.
`

The thumb is a challenge in itself, due to its initial default rotation in UE4. Therefore I added some offset -180.f to the angles in the base bone hand_inner_phalanx_angle.

### Applying DTrack end effectors position & rotation and letting unreal do the IK

Another approach is to use the DTrack position and rotation for the finger tips (the end effectors) and letting unreal do the Inverse Kinematics (IK) calculations.

Every manufacturer can define their bone-lengths (hence the joint-locations) at their own demand. Also some artists can define their bones with offsets in their custom Skeleton.
 
So a challenge here is, that in DTrack the fingerTip locations are relative to the back of the hand, whose coordinate system is placed where the index finger begins, as shown in the picture below:

![DTrack model of a human right hand](../Resources/../path/to/DTrack model of a human right hand.png?raw=true "DTrack model of a human right hand")

But for Unreal that location is at the base of the hand. 

So to get the same location corresponding to the DTrack one in Unreal you need to know Unreal's offset from base to index finger. Following code does that:

`FVector relativeDBackOfHand = locationIndexFingerBase - locationBackOfHand;`
 
The position of the end effector (here: tip of the index finger) is calculate as following:

`FVector relativeDTipOfIndexFinger = relLocationOfFingerTip + relativeDBackOfHand;`


## Additional Infos:

https://wiki.beyondunreal.com/Quaternion


