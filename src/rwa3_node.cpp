// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include <algorithm>
#include <vector>
#include <ros/ros.h>

#include <nist_gear/LogicalCameraImage.h>
#include <nist_gear/Order.h>
#include <nist_gear/Proximity.h>
#include <sensor_msgs/LaserScan.h>
#include <sensor_msgs/Range.h>
#include <std_msgs/Float32.h>
#include <std_msgs/String.h>
#include <std_srvs/Trigger.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h> //--needed for tf2::Matrix3x3

#include "competition.h"
#include "utils.h"
#include "gantry_control.h"

#include <tf2/LinearMath/Quaternion.h>

#define MAX_NUMBER_OF_CAMERAS 17
std::array<std::array<part, 20>, 20>  parts_from_camera_main ;
std::vector<std::vector<std::vector<master_struct> > > master_vector_main (10,std::vector<std::vector<master_struct> >(10,std::vector <master_struct>(20)));
bool part_placed = false;
int k = 0;
part faulty_part;


int main(int argc, char ** argv) {

    ros::init(argc, argv, "rwa3_node");
    ros::NodeHandle node;
    ros::AsyncSpinner spinner(8);
    spinner.start();

    Competition comp(node);

    //Array of Logical Camera Subscribers
    ros::Subscriber logical_camera_subscriber_ [MAX_NUMBER_OF_CAMERAS];
    std::ostringstream otopic;
    std::string topic;

    for (int idx = 0; idx < MAX_NUMBER_OF_CAMERAS; idx++){
        otopic.str("");
        otopic.clear();
        otopic << "/ariac/logical_camera_" << idx;
        topic = otopic.str();
        logical_camera_subscriber_[idx] = node.subscribe<nist_gear::LogicalCameraImage>
                (topic, 10, boost::bind(&Competition::logical_camera_callback, &comp, _1, idx));
    }

    comp.init();


    std::string c_state = comp.getCompetitionState();
    comp.getClock();

    GantryControl gantry(node);
    gantry.init();

    parts_from_camera_main = comp.get_parts_from_camera();
    master_vector_main = comp.get_master_vector();

    for(int i=0; i < 10;  i++) {

        for (int j = 0; j < 10; j++) {
            LOOP:while(k< 20)
            {
//            for (int k = 0; k < 20; k++){
                ROS_INFO_STREAM("NEWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW part");
                ROS_INFO_STREAM(i << j << k);
                if((master_vector_main[i][j][k].type == "pulley_part_red") || (master_vector_main[i][j][k].type == "pulley_part_blue") || (master_vector_main[i][j][k].type == "pulley_part_green")|| (master_vector_main[i][j][k].type == "disk_part_blue")|| (master_vector_main[i][j][k].type == "disk_part_red")|| (master_vector_main[i][j][k].type == "disk_part_green")|| (master_vector_main[i][j][k].type == "piston_part_blue")|| (master_vector_main[i][j][k].type == "piston_part_green")|| (master_vector_main[i][j][k].type == "piston_part_red")|| (master_vector_main[i][j][k].type == "gasket_part_blue")|| (master_vector_main[i][j][k].type == "gasket_part_red")|| (master_vector_main[i][j][k].type == "gasket_part_green"))
                {

                    part_placed = false;
                    for (int l = 0 ; l < parts_from_camera_main.size(); l++)
                    {
                        for (int m = 0; m < parts_from_camera_main[i].size(); m++)
                        {
//                            parts_from_camera_main = comp.get_parts_from_camera();
                            if ((master_vector_main[i][j][k].type == parts_from_camera_main[l][m].type) && (parts_from_camera_main[l][m].faulty == false) &&(parts_from_camera_main[l][m].picked == false))
                            {
                                ROS_INFO_STREAM("Part found in environment");
                                ROS_INFO_STREAM(parts_from_camera_main[l][m].type);
                                if (master_vector_main[i][j][k].type == "disk_part_blue")
                                {
                                    ROS_INFO_STREAM(master_vector_main[i][j][k].place_part_pose);
                                    ROS_INFO_STREAM(parts_from_camera_main[l][m].pose);
                                    std::string location = "bin_13";
                                    gantry.goToPresetLocation(gantry.start_);
                                    ROS_INFO_STREAM("Start location reached");
                                    gantry.goToPresetLocation(gantry.bin13_);
                                    ROS_INFO_STREAM("bin13 location reached");
                                    parts_from_camera_main[l][m].picked = true;
                                    gantry.pickPart(parts_from_camera_main[l][m]);
                                    ROS_INFO_STREAM("Part picked");
                                    gantry.goToPresetLocation(gantry.bin13_);
                                    ROS_INFO_STREAM("bin13 location reached");

                                    gantry.goToPresetLocation(gantry.start_);
                                    ROS_INFO_STREAM("Start location reached");

                                    part part_in_tray;
                                    part_in_tray.type = master_vector_main[i][j][k].type;
                                    part_in_tray.pose.position.x = master_vector_main[i][j][k].place_part_pose.position.x;
                                    part_in_tray.pose.position.y = master_vector_main[i][j][k].place_part_pose.position.y;
                                    part_in_tray.pose.position.z = master_vector_main[i][j][k].place_part_pose.position.z;
                                    part_in_tray.pose.orientation.x = master_vector_main[i][j][k].place_part_pose.orientation.x;
                                    part_in_tray.pose.orientation.y = master_vector_main[i][j][k].place_part_pose.orientation.y;
                                    part_in_tray.pose.orientation.z = master_vector_main[i][j][k].place_part_pose.orientation.z;
                                    part_in_tray.pose.orientation.w = master_vector_main[i][j][k].place_part_pose.orientation.w;

                                    gantry.placePart(part_in_tray, master_vector_main[i][j][k].agv_id);
                                    ROS_INFO_STREAM("Part placed");

                                    k++;
                                    if(master_vector_main[i][j][k].agv_id == "agv2")
                                    {
                                        gantry.goToPresetLocation(gantry.agv2_);
                                        ROS_INFO_STREAM("AGV2 location reached");
                                    }

                                    faulty_part = comp.get_quality_sensor_status();
                                    ROS_INFO_STREAM("Status of faulty part = ");
                                    ROS_INFO_STREAM(faulty_part.faulty);
                                    if(faulty_part.faulty == true)
                                    {
                                        part faulty_part;
                                        faulty_part.pose = gantry.getTargetWorldPose_dummy(faulty_part.pose, master_vector_main[i][j][k].agv_id);
                                        ROS_INFO_STREAM("Black sheep location");
                                        ROS_INFO_STREAM(faulty_part.pose);
                                        faulty_part.type = parts_from_camera_main[l][m].type;
                                        faulty_part.pose.position.x = faulty_part.pose.position.x;
                                        faulty_part.pose.position.y = faulty_part.pose.position.y;
                                        faulty_part.pose.position.z = faulty_part.pose.position.z+0.03;
                                        faulty_part.pose.orientation.x = faulty_part.pose.orientation.x;
                                        faulty_part.pose.orientation.y = faulty_part.pose.orientation.y;
                                        faulty_part.pose.orientation.z = faulty_part.pose.orientation.z;
                                        faulty_part.pose.orientation.w = faulty_part.pose.orientation.w;
                                        gantry.pickPart(faulty_part);
                                        gantry.goToPresetLocation(gantry.agv2_drop_);
                                        gantry.deactivateGripper("left_arm");
                                        ROS_INFO_STREAM("BLack Sheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeep MEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEHHHHHHHHHHHHHHHHHHHHH");
                                    }
                                    goto LOOP;

                                } else if (master_vector_main[i][j][k].type == "disk_part_green")
                                {
                                    ROS_INFO_STREAM(master_vector_main[i][j][k].place_part_pose);
                                    ROS_INFO_STREAM(parts_from_camera_main[l][m].pose);
                                    std::string location = "bin_16";
                                    gantry.goToPresetLocation(gantry.start_);
                                    ROS_INFO_STREAM("Start location reached");
                                    gantry.goToPresetLocation(gantry.bin16_);
                                    ROS_INFO_STREAM("bin16 location reached");
                                    parts_from_camera_main[l][m].picked = true;
                                    gantry.pickPart(parts_from_camera_main[l][m]);
                                    ROS_INFO_STREAM("Part picked");
                                    gantry.goToPresetLocation(gantry.bin16_);
                                    ROS_INFO_STREAM("bin16 location reached");

                                    gantry.goToPresetLocation(gantry.start_);
                                    ROS_INFO_STREAM("Start location reached");

                                    part part_in_tray;
                                    part_in_tray.type = master_vector_main[i][j][k].type;
                                    part_in_tray.pose.position.x = master_vector_main[i][j][k].place_part_pose.position.x;
                                    part_in_tray.pose.position.y = master_vector_main[i][j][k].place_part_pose.position.y;
                                    part_in_tray.pose.position.z = master_vector_main[i][j][k].place_part_pose.position.z;
                                    part_in_tray.pose.orientation.x = master_vector_main[i][j][k].place_part_pose.orientation.x;
                                    part_in_tray.pose.orientation.y = master_vector_main[i][j][k].place_part_pose.orientation.y;
                                    part_in_tray.pose.orientation.z = master_vector_main[i][j][k].place_part_pose.orientation.z;
                                    part_in_tray.pose.orientation.w = master_vector_main[i][j][k].place_part_pose.orientation.w;

                                    gantry.placePart(part_in_tray, master_vector_main[i][j][k].agv_id);
                                    ROS_INFO_STREAM("Part placed");
                                    k++;
                                    if(master_vector_main[i][j][k].agv_id == "agv2")
                                    {
                                        gantry.goToPresetLocation(gantry.agv2_);
                                        ROS_INFO_STREAM("AGV2 location reached");
                                    }

                                    faulty_part = comp.get_quality_sensor_status();
                                    ROS_INFO_STREAM("Status of faulty part = ");
                                    ROS_INFO_STREAM(faulty_part.faulty);
                                    if(faulty_part.faulty == true)
                                    {
                                        part faulty_part;
                                        faulty_part.pose = gantry.getTargetWorldPose_dummy(faulty_part.pose, master_vector_main[i][j][k].agv_id);
                                        ROS_INFO_STREAM("Black sheep location");
                                        ROS_INFO_STREAM(faulty_part.pose);
                                        faulty_part.type = parts_from_camera_main[l][m].type;
                                        faulty_part.pose.position.x = faulty_part.pose.position.x;
                                        faulty_part.pose.position.y = faulty_part.pose.position.y;
                                        faulty_part.pose.position.z = faulty_part.pose.position.z+0.03;
                                        faulty_part.pose.orientation.x = faulty_part.pose.orientation.x;
                                        faulty_part.pose.orientation.y = faulty_part.pose.orientation.y;
                                        faulty_part.pose.orientation.z = faulty_part.pose.orientation.z;
                                        faulty_part.pose.orientation.w = faulty_part.pose.orientation.w;
                                        gantry.pickPart(faulty_part);
                                        gantry.goToPresetLocation(gantry.agv2_drop_);
                                        gantry.deactivateGripper("left_arm");
                                        ROS_INFO_STREAM("BLack Sheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeep MEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEHHHHHHHHHHHHHHHHHHHHH");
                                    }
                                    goto LOOP;
                                }
                                else if (master_vector_main[i][j][k].type == "pulley_part_red")
                                {
                                    parts_from_camera_main[l][m].picked = true;
                                    ROS_INFO_STREAM(master_vector_main[i][j][k].place_part_pose);
                                    ROS_INFO_STREAM(parts_from_camera_main[l][m].pose);
                                    std::string location = "shelf5";
                                    gantry.goToPresetLocation(gantry.start_);
                                    ROS_INFO_STREAM("Start location reached");
                                    gantry.goToPresetLocation(gantry.waypoint_1_);
                                    ROS_INFO_STREAM("waypont1 location reached");
                                    gantry.goToPresetLocation(gantry.waypoint_2_);
                                    ROS_INFO_STREAM("waypoint2 location reached");
                                    gantry.goToPresetLocation(gantry.waypoint_3_);
                                    ROS_INFO_STREAM("waypoint3 location reached");
                                    gantry.goToPresetLocation(gantry.waypoint_4_);
                                    ROS_INFO_STREAM("waypoint4 location reached");

                                    gantry.pickPart(parts_from_camera_main[l][m]);
                                    ROS_INFO_STREAM("Part picked");

                                    gantry.goToPresetLocation(gantry.waypoint_4_);
                                    ROS_INFO_STREAM("waypoint4 location reached");
                                    gantry.goToPresetLocation(gantry.waypoint_3_);
                                    ROS_INFO_STREAM("waypoint3 location reached");
                                    gantry.goToPresetLocation(gantry.waypoint_2_);
                                    ROS_INFO_STREAM("waypoint2 location reached");
                                    gantry.goToPresetLocation(gantry.waypoint_1_);
                                    ROS_INFO_STREAM("waypoint1 location reached");
                                    gantry.goToPresetLocation(gantry.start_);
                                    ROS_INFO_STREAM("Start location reached");

                                    part part_in_tray;
                                    part_in_tray.type = master_vector_main[i][j][k].type;
                                    part_in_tray.pose.position.x = master_vector_main[i][j][k].place_part_pose.position.x;
                                    part_in_tray.pose.position.y = master_vector_main[i][j][k].place_part_pose.position.y;
                                    part_in_tray.pose.position.z = master_vector_main[i][j][k].place_part_pose.position.z;
                                    part_in_tray.pose.orientation.x = master_vector_main[i][j][k].place_part_pose.orientation.x;
                                    part_in_tray.pose.orientation.y = master_vector_main[i][j][k].place_part_pose.orientation.y;
                                    part_in_tray.pose.orientation.z = master_vector_main[i][j][k].place_part_pose.orientation.z;
                                    part_in_tray.pose.orientation.w = master_vector_main[i][j][k].place_part_pose.orientation.w;

                                    gantry.placePart(part_in_tray, master_vector_main[i][j][k].agv_id);
                                    ROS_INFO_STREAM("Part placed");
                                    k++;
                                    if(master_vector_main[i][j][k].agv_id == "agv2")
                                    {
                                        gantry.goToPresetLocation(gantry.agv2_);
                                        ROS_INFO_STREAM("AGV2 location reached");
                                    }

                                    faulty_part = comp.get_quality_sensor_status();
                                    ROS_INFO_STREAM("Status of faulty part = ");
                                    ROS_INFO_STREAM(faulty_part.faulty);
                                    if(faulty_part.faulty == true)
                                    {
                                        part faulty_part;
                                        faulty_part.pose = gantry.getTargetWorldPose_dummy(faulty_part.pose, master_vector_main[i][j][k].agv_id);
                                        ROS_INFO_STREAM("Black sheep location");
                                        ROS_INFO_STREAM(faulty_part.pose);
                                        faulty_part.type = parts_from_camera_main[l][m].type;
                                        faulty_part.pose.position.x = faulty_part.pose.position.x;
                                        faulty_part.pose.position.y = faulty_part.pose.position.y;
                                        faulty_part.pose.position.z = faulty_part.pose.position.z+0.03;
                                        faulty_part.pose.orientation.x = faulty_part.pose.orientation.x;
                                        faulty_part.pose.orientation.y = faulty_part.pose.orientation.y;
                                        faulty_part.pose.orientation.z = faulty_part.pose.orientation.z;
                                        faulty_part.pose.orientation.w = faulty_part.pose.orientation.w;
                                        gantry.pickPart(faulty_part);
                                        gantry.goToPresetLocation(gantry.agv2_drop_);
                                        gantry.deactivateGripper("left_arm");
                                        ROS_INFO_STREAM("BLack Sheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeep MEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEHHHHHHHHHHHHHHHHHHHHH");
                                    }
                                    goto LOOP;
                                }

                            }
                        }
                    }
                    ROS_INFO_STREAM("Second for loop katham");
                }
                k++;
            }
        }
    }

//    gantry.goToPresetLocation(gantry.start_);
//    gantry.goToPresetLocation(gantry.bin3_);
//
//    //--You should receive the following information from a camera
//    part my_part;
//    my_part.type = "pulley_part_red";
//    my_part.pose.position.x = 4.365789;
//    my_part.pose.position.y = 1.173381;
//    my_part.pose.position.z = 0.728011;
//    my_part.pose.orientation.x = 0.012;
//    my_part.pose.orientation.y = -0.004;
//    my_part.pose.orientation.z = 0.002;
//    my_part.pose.orientation.w = 1.000;
//
//    //--get pose of part in tray from /ariac/orders
//    part part_in_tray;
//    part_in_tray.type = "pulley_part_red";
//    part_in_tray.pose.position.x = -0.12;
//    part_in_tray.pose.position.x = -0.2;
//    part_in_tray.pose.position.x = 0.0;
//    part_in_tray.pose.orientation.x = 0.0;
//    part_in_tray.pose.orientation.y = 0.0;
//    part_in_tray.pose.orientation.z = 0.0;
//    part_in_tray.pose.orientation.w = 1.0;

    //--Go pick the part
//    gantry.pickPart(my_part);
    //--Go place the part
//    gantry.placePart(part_in_tray, "agv2");

    ROS_INFO_STREAM("Mangathaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa DaWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");

    comp.endCompetition();
    spinner.stop();
    ros::shutdown();
    return 0;
}