/**
 * @file remove_gps_latency.cpp
 * @brief This node corrects the latency of the GPS/EKF by subtracting a constant value to the message timestamps
 * @author Xavier Ruiz (xavier.ruiz@earthrover.farm)
 */

#include <ros/ros.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/Imu.h>
#include <nav_msgs/Odometry.h>
#include <iostream>
#include <string>


class RemoveLatency
 {
public:
     RemoveLatency(const ros::NodeHandle &node_handle, const ros::NodeHandle &private_node_handle)
     : nh_(node_handle), pnh_(private_node_handle)
     {
         this->init();
     }
     ~RemoveLatency() = default;

     void init();
     void gpsCallback(const sensor_msgs::NavSatFixConstPtr& msg);
     void headingCallback(const sensor_msgs::ImuConstPtr& heading_msg);
     void ekfCallback(const nav_msgs::OdometryConstPtr& ekf_msg);
     // Initialize Variables
     double gps_latency_{0.0};
     double ekf_latency_{0.0};
     // public and private ros node handle
     ros::NodeHandle nh_;
     ros::NodeHandle pnh_;
     // Subscribers and publishers
     ros::Subscriber gps_sub_;
     ros::Subscriber heading_sub_;
     ros::Subscriber ekf_sub_;
     ros::Publisher gps_pub_;
     ros::Publisher heading_pub_;
     ros::Publisher ekf_pub_;
};

void RemoveLatency::init()
{
      // Load params
      nh_.getParam("/remove_latency/gps_latency", gps_latency_);
      nh_.getParam("/remove_latency/ekf_latency", ekf_latency_);

      gps_sub_ = nh_.subscribe("/piksi_receiver/navsatfix_best_fix", 1000, &RemoveLatency::gpsCallback, this);
      heading_sub_ = nh_.subscribe("/heading", 1000, &RemoveLatency::headingCallback, this);
      gps_pub_ = nh_.advertise<sensor_msgs::NavSatFix>("/piksi_receiver/navsatfix_best_fix/corrected_latency", 1000);
      heading_pub_ = nh_.advertise<sensor_msgs::Imu>("/heading/corrected_latency", 1000);

      if (ekf_latency_!=0){
          ekf_sub_ = nh_.subscribe("/odometry/filtered/global", 1000, &RemoveLatency::ekfCallback, this);
          ekf_pub_ = nh_.advertise<nav_msgs::Odometry>("/odometry/filtered/global/corrected_latency", 1000);
      }

}


void RemoveLatency::gpsCallback(const sensor_msgs::NavSatFixConstPtr& gps_msg)
{
      if (gps_latency_==0.0){
          gps_pub_.publish(*gps_msg);
      }
      else{
          sensor_msgs::NavSatFix gps_latency_msg;
          gps_latency_msg = *gps_msg;
          ros::Duration gps_latency_2rosdur(gps_latency_*0.001);
          gps_latency_msg.header.stamp = gps_msg->header.stamp - gps_latency_2rosdur;
          gps_pub_.publish(gps_latency_msg);
      }
}

void RemoveLatency::headingCallback(const sensor_msgs::ImuConstPtr& heading_msg)
{
      if (gps_latency_==0.0){
        heading_pub_.publish(*heading_msg);
      }
      else{
         sensor_msgs::Imu heading_latency_msg;
         heading_latency_msg = *heading_msg;
         ros::Duration gps_latency_2rosdur(gps_latency_*0.001);
         heading_latency_msg.header.stamp = heading_msg->header.stamp - gps_latency_2rosdur;
         heading_pub_.publish(heading_latency_msg);
     }
}

void RemoveLatency::ekfCallback(const nav_msgs::OdometryConstPtr& ekf_msg)
{
      nav_msgs::Odometry ekf_latency_msg;
      ekf_latency_msg = *ekf_msg;
      ros::Duration ekf_latency_2rosdur(ekf_latency_*0.001);
      ekf_latency_msg.header.stamp = ekf_msg->header.stamp - ekf_latency_2rosdur;
      ekf_pub_.publish(ekf_latency_msg);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "remove_latency");
    ros::NodeHandle nh;
    ros::NodeHandle nh_private("~");
    RemoveLatency node(nh, nh_private);
    ros::spin();

    return 0;
}
