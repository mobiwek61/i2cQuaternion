#include "MathHelper.h"

/** This version machine-assembled by AI march 27 2026
 * Converts Quat9 data to a formatted Euler string.
 * Includes a 90-degree Z-axis rotation to reset orientation.
 */
std::string getEulerString(double q1, double q2, double q3) {
    // 1. Recover Q0 (Assumes unit quaternion)
    // Guard against noise causing a negative number inside sqrt (prevents NaN)
    double q0_sq = 1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3));
    double q0 = sqrt(q0_sq > 0 ? q0_sq : 0);

    // 2. RESET ORIENTATION: Rotate +90 degrees around Z-axis
    // Rotation quaternion for +90 deg around Z: [w:0.7071, x:0, y:0, z:0.7071]
    const double rw = 0.70710678;
    const double rz = 0.70710678;

    // Apply Hamilton Product: rotatedQ = rotationQ * originalQ
    double nq0 = (rw * q0) - (rz * q3);
    double nq1 = (rw * q1) + (rz * q2);
    double nq2 = (rw * q2) - (rz * q1);
    double nq3 = (rw * q3) + (rz * q0);

    // 3. Calculate Euler Angles (Radians) using the rotated quaternions
    // Roll (x-axis rotation)
    double roll = atan2(2.0 * (nq0 * nq1 + nq2 * nq3), 1.0 - 2.0 * (nq1 * nq1 + nq2 * nq2));

    // Pitch (y-axis rotation) with Gimbal Lock Guard
    double p_sin = 2.0 * (nq0 * nq2 - nq3 * nq1);
    if (p_sin > 1.0) p_sin = 1.0;
    else if (p_sin < -1.0) p_sin = -1.0;
    double pitch = asin(p_sin);

    // Yaw (z-axis rotation)
    double yaw = atan2(2.0 * (nq0 * nq3 + nq1 * nq2), 1.0 - 2.0 * (nq2 * nq2 + nq3 * nq3));

    // 4. Convert to Degrees
    float rDeg = (float)(roll * 180.0 / M_PI);
    float pDeg = (float)(pitch * 180.0 / M_PI);
    float yDeg = (float)(yaw * 180.0 / M_PI);

    char buf[64];
    snprintf(buf, sizeof(buf), "ROLL:%7.2f, PITCH:%7.2f, YAW:%7.2f", rDeg, pDeg, yDeg);

    return std::string(buf);
}
