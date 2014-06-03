#include <string>
#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <sensor_msgs/JointState.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Bool.h>

ros::Publisher pub, pub2, pub3;

double tmin, tmax, pmin, pmax;
double tsmin, tsmax, psmin, psmax;
float pcur, tcur;
bool mode = true;

double max_speed = 0.300; // m/second
double max_turn = 60.0*M_PI/180.0; // rad/second

void posRcvd(const sensor_msgs::JointState::ConstPtr& msg) {
    pcur = msg->position[0];
    tcur = msg->position[1];
}

void joyRcvd(const sensor_msgs::Joy::ConstPtr& msg) {
    float x = msg->axes[0], y = msg->axes[1];
    float pan, pvel, tilt, tvel;
    float dead = 0.25;
    if (msg->buttons[1]) {
        mode = !mode;
    }

    if (msg->buttons[1]) {
        std_msgs::Bool brake_cmd;
        brake_cmd.data = true;
        pub3.publish(brake_cmd);
    }


    if (msg->buttons[3]) {
        std_msgs::Bool brake_cmd;
        brake_cmd.data = false;
        pub3.publish(brake_cmd);
    }

		geometry_msgs::Twist cmdvel;
		float speed, turn;
		if(x<dead && x>-dead)
			turn = 0;
		else
			turn = max_turn * x;
		if(y<dead && y>-dead)
			speed = 0;
		else
			speed = max_speed * y;
		cmdvel.linear.x = speed;
        cmdvel.angular.z = turn;
		pub2.publish(cmdvel);
    
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "b21_teleop");
    ros::NodeHandle n;

    int hz = 30;
    ros::Rate loop_rate(hz);
    pub = n.advertise<sensor_msgs::JointState>("/ptu/cmd", 1);
    pub2 = n.advertise<geometry_msgs::Twist>("/b21/cmd_vel", 1);
    pub3 = n.advertise<std_msgs::Bool>("/b21/cmd_brake_power", 1);
    n.param("/ptu/max_pan", pmax, 90.);
    n.param("/ptu/min_pan", pmin, -90.);
    n.param("/ptu/max_tilt", tmax, 30.);
    n.param("/ptu/min_tilt", tmin, -30.);
    n.param("/ptu/min_tilt_speed", tsmin, 4.);
    n.param("/ptu/max_tilt_speed", tsmax, 140.);
    n.param("/ptu/min_pan_speed", psmin, 4.);
    n.param("/ptu/max_pan_speed", psmax, 140.);
    pcur = tcur = 0;

    ros::Subscriber sub =
        n.subscribe<sensor_msgs::Joy>("joy", 1, joyRcvd);
    ros::Subscriber sub2=
        n.subscribe<sensor_msgs::JointState>("/ptu/state", 1, posRcvd);

    while (ros::ok()) {

        // Process a round of subscription messages
        ros::spinOnce();

        // This will adjust as needed per iteration
        loop_rate.sleep();
    }

    return 0;
}
