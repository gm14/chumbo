#!/usr/bin/env python3
import asyncio
import math
import time
from mavsdk import System
from mavsdk.mocap import (
    VisionPositionEstimate,
    PositionBody,
    AngleBody,
    Covariance
)

async def main():
    drone = System()
    # Listen for PX4 telemetry coming to this computer (e.g. PX4 → 192.168.x.x:14550)
    await drone.connect(system_address="udpin://0.0.0.0:14571")

    print("Waiting for connection...")
    async for state in drone.core.connection_state():
        if state.is_connected:
            print("-- Connected to drone")
            break

    print("-- Streaming vision yaw as VisionPositionEstimate")
    while True:
        # Replace with your vision tracker yaw (radians)
        yaw = get_yaw_from_tracker()

        now_s = time.time()          # seconds (float, PX4 expects seconds)
        pose_cov = [
            1e6, 0, 0, 0, 0, 0,             # row 1
            1e6, 0, 0, 0, 0,                # row 2
            1e6, 0, 0, 0,                   # row 3
            1e6, 0, 0,                      # row 4
            1e6, 0,                         # row 5
            (3 * math.pi / 180) ** 2        # row 6 (yaw variance)
        ]
        msg = VisionPositionEstimate(
            time_usec=int(now_s * 1e6),
            position_body=PositionBody(0.0, 0.0, 0.0),
            angle_body=AngleBody(0.0, 0.0, yaw),
            pose_covariance=Covariance(pose_cov)
        )

        print(f"Sending vision yaw: {yaw:.2f} rad")
        await drone.mocap.set_vision_position_estimate(msg)
        await asyncio.sleep(0.05)    # 20 Hz

def get_yaw_from_tracker() -> float:
    """Stub for camera-based yaw toward the target (radians)."""
    return 0.3 * math.sin(time.time() * 0.5)

if __name__ == "__main__":
    asyncio.run(main())
