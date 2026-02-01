
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

int main(int argc, char** argv)
{
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    Mavsdk mavsdk{Mavsdk::Configuration{ComponentType::CompanionComputer}};
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

    Offboard::VelocityBodyYawspeed stay{};
    offboard.set_velocity_body(stay);

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
    std::cout << "Armed, starting offboard\n";

    offboard.set_velocity_body(stay);

    Offboard::Result offboard_result_2 = offboard.start();
    if (offboard_result_2 != Offboard::Result::Success) {
        std::cerr << "Offboard start failed: " << offboard_result_2 << '\n';
        return 1;
    }

    std::cout << "\nClimb\n";
    Offboard::VelocityBodyYawspeed setpoint{};
    setpoint.down_m_s = -0.5f;
    setpoint.yawspeed_deg_s = 0.0f;
    offboard.set_velocity_body(setpoint);

    while (telemetry.altitude().altitude_relative_m < 1.0f) {
        sleep_for(milliseconds(100));
        offboard.set_velocity_body(setpoint);
    }

    std::cout << "\nDescend\n";
    setpoint.down_m_s = 0.5f;
    setpoint.yawspeed_deg_s = 0.0f;
    offboard.set_velocity_body(setpoint);
    
    while (telemetry.altitude().altitude_relative_m > 0.2f) {
        sleep_for(milliseconds(100));
        offboard.set_velocity_body(setpoint);
    }

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
