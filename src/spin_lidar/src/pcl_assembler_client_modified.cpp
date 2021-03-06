/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

#include <cstdio>
#include <ros/ros.h>
#include<ros/time.h>

// Services
#include "laser_assembler/AssembleScans2.h"

// Messages
#include "sensor_msgs/PointCloud2.h"
#include "dynamixel_msgs/JointState.h"
#include "std_msgs/Time.h"

/* This is a modified periodic_snapshotter.cpp file that acts both as a timer and subscriber, depending on the value of the
assembled_cloud_mode parameter.  It defaults to subscriber.  The node calls a service from the point_cloud2_assembler node
from the laser_assembler package to create the compiled point cloud and then publishes the result.
Timer:
As a timer, the node performs its function every 5 seconds.  To use this function, the assembled_cloud_mode parameter must
be set to time.
Subscriber:
As a subscriber, the node subscribes to start and end times published while tilting the motor and sends those as necessary
to the compilation service to put all of the point clouds together.  One cloud is published for each complete sweep
(i.e. -90 -> +90 -> -90).  This is the default setting of the node.*/

using namespace laser_assembler;

//global variables
ros::Time start;
ros::Time end;
ros::Time init;
ros::Time start_time;
int go = 0;
std::string assembled_cloud_mode;
double scan_time;
ros::Publisher pub_; 
ros::ServiceClient client_;
AssembleScans2 srv;
//callback for start time, saves in global variable

void startTime(const std_msgs::Time &msg) {
    ROS_INFO("Inside start time");
    start = msg.data;
}

//callback for end time, saves in global variable and updates go to start compilation
void endTime(const std_msgs::Time &msg) {
    ROS_INFO("Inside End time ");
}

void compile_point_cloud(sensor_msgs::PointCloud2 cloud){
	srv.request.begin = ros::Time::now()-ros::Duration(13.025);
        srv.request.end   = ros::Time::now();
	if (client_.call(srv)) {
            pub_.publish(srv.response.cloud);
        }
        return;
}


int main(int argc, char **argv)
{
    //initialize and wait for necessary services, etc.
    
    ros::init(argc, argv, "Cloud_Compiler");
    ros::NodeHandle n_;
    
    pub_ = n_.advertise<sensor_msgs::PointCloud2> ("assembled_cloud", 1);
    client_ = n_.serviceClient<AssembleScans2>("assemble_scans2");
    
    ROS_INFO("Waiting for [build_cloud] to be advertised");
    
    //parameter for timer vs. subscriber and length of timer
    ros::service::waitForService("assemble_scans2");
    ROS_INFO_STREAM("Found build_cloud! Starting the Cloud Compiler");
    
    //SUBSCRIBER intialization
    ROS_INFO("Here");
    //Wait for dynamixel servo to init by waiting for /state topic
    ros::topic::waitForMessage<dynamixel_msgs::JointState>("/tilt_controller/state", ros::Duration(20));    
    //subscribes to start and end time published by tilting motor
    ros::Subscriber sub_1=n_.subscribe("/time/start_time", 1, &startTime);
    ros::Subscriber sub_2=n_.subscribe("/time/end_time", 1, &endTime);
    ros::Subscriber sub_3=n_.subscribe("/hokuyo_points", 1, &compile_point_cloud);

    init = ros::Time::now();
    //SUBSCRIBER MAIN LOOP
    while(ros::ok())
    {
    	ros::spinOnce();
    }    
    return 0;

}
