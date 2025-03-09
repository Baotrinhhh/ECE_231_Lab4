#!/bin/sh
config-pin -q P9_14
config-pin P9_14 pwm
cd /sys/class/pwm/pwmchip4/pwm-4\:0
echo 1000000000 > period
echo 500000000 > duty_cycle
echo 1 > enable
echo 'PWM Configured & Started'