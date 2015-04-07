Signal Logger
==============

[![Build Status](http://129.132.38.183:8080/buildStatus/icon?job=signal_logger)](http://129.132.38.183:8080/view/legged_robotics/job/signal_logger/)

These packages provide an interface for a signal logger and some default signal loggers.
A signal logger can log a time series of a signal.

Author(s): Christian Gehring, Dario Bellicoso

Dependencies
----------------------------------------------------------------------------
* [kindr](https://github.com/ethz-asl/kindr) - Kinematics and Dynamics for Robotics
* [kindr_ros](https://github.com/ethz-asl/kindr_ros) - kindr ROS wrapper
* [roco](https://bitbucket.org/ethz-asl-lr/roco) - common interface for robot controllers


Building
----------------------------------------------------------------------------
To build the non-ROS implementation of the signal logger, link the following pacakges in your catkin workspace:

     ln -s ~/git/signal_logger/signal_logger ~/catkin_ws/src
     ln -s ~/git/signal_logger/signal_logger_std ~/catkin_ws/src

Additionally, if you wish to also build the ROS version of the signal logger, link the following pacakges in your catkin workspace:

     ln -s ~/git/signal_logger/signal_logger_msgs ~/catkin_ws/src
     ln -s ~/git/signal_logger/signal_logger_ros ~/catkin_ws/src