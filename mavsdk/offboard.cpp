
//
// Example that demonstrates offboard control using attitude, velocity control
// in NED (North-East-Down), and velocity control in body (Forward-Right-Down)
// coordinates.
//

#include <chrono>
#include <cmath>
#include <future>
#include <iostream>
#include <thread>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

using namespace mavsdk;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::this_thread::sleep_for;

void usage(const std::string& bin_name)
{
    std::cerr << "Usage : " << bin_name << " <connection_url>\n"
              << "Connection URL format should be :\n"
              << " For TCP server: tcpin://<our_ip>:<port>\n"
              << " For TCP client: tcpout://<remote_ip>:<port>\n"
              << " For UDP server: udp://<our_ip>:<port>\n"
              << " For UDP client: udp://<remote_ip>:<port>\n"
              << " For Serial : serial://</path/to/serial/dev>:<baudrate>]\n"
              << "For example, to connect to the simulator use URL: udpin://0.0.0.0:14540\n";
}

//
// Does Offboard control using body co-ordinates.
// Body coordinates really means world coordinates rotated by the yaw of the
// vehicle, so if the vehicle pitches down, the forward axis does still point
// forward and not down into the ground.
//
// returns true if everything went well in Offboard control.
//
bool offb_ctrl_body(mavsdk::Offboard& offboard)
{
    std::cout << "Starting Offboard velocity control in body coordinates\n";

    // Send it once before starting offboard, otherwise it will be rejected.
    Offboard::VelocityBodyYawspeed stay{};
    offboard.set_velocity_body(stay);

    Offboard::Result offboard_result = offboard.start();
    if (offboard_result != Offboard::Result::Success) {
        std::cerr << "Offboard start failed: " << offboard_result << '\n';
        return false;
    }
    std::cout << "Offboard started\n";

    std::cout << "Turn clock-wise and climb\n";
    Offboard::VelocityBodyYawspeed cc_and_climb{};
    cc_and_climb.down_m_s = -1.0f;
    cc_and_climb.yawspeed_deg_s = 60.0f;
    offboard.set_velocity_body(cc_and_climb);
    sleep_for(seconds(5));

    std::cout << "Turn back anti-clockwise\n";
    Offboard::VelocityBodyYawspeed ccw{};
    ccw.down_m_s = -1.0f;
    ccw.yawspeed_deg_s = -60.0f;
    offboard.set_velocity_body(ccw);
    sleep_for(seconds(5));

    std::cout << "Wait for a bit\n";
    offboard.set_velocity_body(stay);
    sleep_for(seconds(2));

    std::cout << "Fly a circle\n";
    Offboard::VelocityBodyYawspeed circle{};
    circle.forward_m_s = 5.0f;
    circle.yawspeed_deg_s = 30.0f;
    offboard.set_velocity_body(circle);
    sleep_for(seconds(15));

    std::cout << "Wait for a bit\n";
    offboard.set_velocity_body(stay);
    sleep_for(seconds(5));

    std::cout << "Fly a circle sideways\n";
    circle.right_m_s = -5.0f;
    circle.yawspeed_deg_s = 30.0f;
    offboard.set_velocity_body(circle);
    sleep_for(seconds(15));

    std::cout << "Wait for a bit\n";
    offboard.set_velocity_body(stay);
    sleep_for(seconds(8));

    offboard_result = offboard.stop();
    if (offboard_result != Offboard::Result::Success) {
        std::cerr << "Offboard stop failed: " << offboard_result << '\n';
        return false;
    }
    std::cout << "Offboard stopped\n";

    return true;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    Mavsdk mavsdk{Mavsdk::Configuration{ComponentType::GroundStation}};
    ConnectionResult connection_result = mavsdk.add_any_connection(argv[1]);

    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << '\n';
        return 1;
    }

    auto system = mavsdk.first_autopilot(3.0);
    if (!system) {
        std::cerr << "Timed out waiting for system\n";
        return 1;
    }

    // Instantiate plugins.
    auto action = Action{system.value()};
    auto offboard = Offboard{system.value()};
    auto telemetry = Telemetry{system.value()};

    std::cout << "Starting Offboard velocity control in body coordinates\n";

    // Send it once before starting offboard, otherwise it will be rejected.
    Offboard::VelocityBodyYawspeed stay{};
    offboard.set_velocity_body(stay);

    Offboard::Result offboard_result = offboard.start();
    if (offboard_result != Offboard::Result::Success) {
        std::cerr << "Offboard start failed: " << offboard_result << '\n';
        return 1;
    }

    sleep_for(seconds(2));

    while (!telemetry.health().is_armable) {
        std::cout << "Waiting for system to be ready\n";
        Telemetry::Health health = telemetry.health();

        std::cout << "Health: " << health << '\n';

        sleep_for(seconds(1));
    }

    std::cout << "Arming... \n";

    const auto arm_result = action.arm();
    if (arm_result != Action::Result::Success) {
        std::cerr << "Arming failed: " << arm_result << '\n';
        return 1;
    }
    std::cout << "Armed\n";

    sleep_for(seconds(2));

    // std::cout << "Offboard started\n";

    std::cout << "Climb\n";
    Offboard::VelocityBodyYawspeed setpoint{};
    setpoint.down_m_s = -0.5f;
    setpoint.yawspeed_deg_s = 0.0f;
    offboard.set_velocity_body(setpoint);

    std::cout << "Descend" << std::endl;
    setpoint.down_m_s = -0.5f;
    setpoint.yawspeed_deg_s = 0.0f;
    offboard.set_velocity_body(setpoint);
    sleep_for(seconds(4));


    const auto disarm_result = action.disarm();
    if (disarm_result != Action::Result::Success) {
        std::cerr << "Disarming failed: " << disarm_result << '\n';
        return 1;
    }

    // We are relying on auto-disarming but let's keep watching the telemetry for
    // a bit longer.
    sleep_for(seconds(3));
    std::cout << "Finished...\n";

    return 0;
}
