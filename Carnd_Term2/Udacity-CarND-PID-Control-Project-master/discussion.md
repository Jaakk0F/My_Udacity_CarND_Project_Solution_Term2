# CarND-Controls-PID
Self-Driving Car Engineer Nanodegree Program

## Introduction
---
The purpose of this project is to build a PID controller and tune the PID hyperparameters, Describe the effect each of the P, I, D components had in implementation, and choose the final hyperparameters, to drive a car round a track surface.

# Rubic Discussion Points

## Describe the effect each of the P, I, D components had in your implementation
---

The P, or "proportional" component  causes the car to steer proportional (and opposite) to the car's distance from the lane center (its also known as the CTE) - if the car is off far to the right, it steers hard to the left and if the car is off slightly to the left it steers slightly to the right.

The I, or "Integral", component eliminates a CTE bias which could prevent the error from being eliminated (from reaching the centre line). the bias takes many forms eg steering drift.

The D, or "differential", component counteracts the P component's tendency to ring and overshoot the center line by smoothening the car's approach to it.

## Describe how the final hyperparameters were chosen
---
The parameters were chosen manually by try and error. First, started with the PID parameters given in the exercise, then continued tuning till i got a satisfactory result of (0.15, 0.00003, 3).