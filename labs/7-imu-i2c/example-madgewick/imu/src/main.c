//
//  main.c
//  madgwickFilter
//
//  Created by Blake Johnson on 4/28/20.
//
#include "madgwick.h"

#define DELTA_T 0.01f // 100Hz sampling frequency

void notmain(void) {
    float roll = 0.0, pitch = 0.0, yaw = 0.0;

    kmalloc_init(32);
    struct mgos_imu_madgwick *m = mgos_imu_madgwick_create();


    // flat upright position to resolve the graident descent problem
    printf("\n\n Upright to solve gradient descent problem\n");
    for(float i = 0; i<1000; i++){
        mgos_imu_madgwick_update(m, 0,0,0, 0.05, 0.05, 0.9, 0,0,0);
        // mgos_imu_madgwick_updateIMU(m, 0,0,0, 0.05, 0.05, 0.9);
        mgos_imu_madgwick_get_angles(m, &roll, &pitch, &yaw);
        // eulerAngles(q_est, &roll, &pitch, &yaw);
        printf("Time (s): %f, roll: %f, pitch: %f, yaw: %f\n", i * DELTA_T, roll, pitch, yaw);
    }

#if 0

    // Angular Rotation around Z axis (yaw) at 100 degrees per second
    printf("\n\nYAW 100 Degrees per Second\n");
    for(float i = 0; i<1000; i++){
        imu_filter(0, 0, 1, 0, 0, -100 * PI / 180);
        eulerAngles(q_est, &roll, &pitch, &yaw);
        printf("Time (s): %f, roll: %f, pitch: %f, yaw: %f\n", i * DELTA_T, roll, pitch, yaw);
    }

    // Angular Rotation around Z axis (yaw) at -20 degrees per second
    printf("\n\nYAW -20 Degrees per Second\n");
    for(float i = 0; i<1000; i++){
        imu_filter(0, 0, 1, 0, 0, 20 * PI / 180);
        eulerAngles(q_est, &roll, &pitch, &yaw);
        printf("Time (s): %f, roll: %f, pitch: %f, yaw: %f\n", i * DELTA_T, roll, pitch, yaw);
    }
#endif
}

int main(int argc, const char * argv[]) {
    notmain();
    return 0;
}

