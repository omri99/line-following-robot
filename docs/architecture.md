# System Architecture

## Hardware Components
- Arduino board
- Motor driver (H-bridge)
- 2 DC motors
- 5 IR sensors (QTR / TCRT5000)
- Robot chassis + battery pack

## Data Flow
1. IR sensors read line reflectance
2. QTR library computes line position
3. Error = target position - actual position
4. PID controller calculates correction
5. Motor driver adjusts left/right speeds

## Control Logic
- PID for smooth and accurate tracking
- Calibration on startup
- Safety handling if no line is detected

## Notes
This file documents how the robot operates at a logical and hardware level.
