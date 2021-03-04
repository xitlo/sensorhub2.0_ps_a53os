#!/bin/bash

#1, check param
if [ $# -lt 2 ] || [ $1 -lt 1 ] || [ $1 -gt 5 ] ; then
    echo "Usage: $0 <app_id> <turn_on_1_off_0>"
    echo "app_id: 1, r5 control, 1-on, 2-reset, 0-off"
    echo "app_id: 2, timesync, 1-on, 0-off"
    echo "app_id: 3, ptp, 1-on, 0-off"
    echo "app_id: 4, task-state, 1-on, 0-off"
    echo "app_id: 5, task-data, 1-on, 0-off"
    echo " e.g.: $0 1 1"
    exit 1;
fi

# 1, app r5 control
if [ $1 -eq 1 ] ; then
    if [ $2 -eq 1 ]; then
        echo ">>>1, app r5 on!"
        /etc/common/r5control.sh 1 sensorhub.elf
    elif [ $2 -eq 2 ]; then
        echo ">>>1, app r5 reset!"
        /etc/common/r5control.sh 0 && /etc/common/r5control.sh 1 sensorhub.elf
    else
        echo ">>>1, app r5 off!"
        /etc/common/r5control.sh 0
    fi
fi

# 2, app timesync control
if [ $1 -eq 2 ] ; then
    if [ $2 -eq 1 ]; then
        echo ">>>2, app timesync on!"
        /etc/common/test_time.sh > /dev/null &
    else
        echo ">>>2, app timesync off!"
        ps -ef | grep timesync-app | grep -v grep | awk '{print $1}' | xargs kill -9
    fi
fi

# 3, app r5 control
if [ $1 -eq 3 ] ; then
    if [ $2 -eq 1 ]; then
        echo ">>>3, app ptp4l on!"
        ptp4l -i eth0 -S -m >> /data/bsplog/ptp.log &
    else
        echo ">>>3, app ptp4l off!"
        ps -ef | grep ptp4l | grep -v grep | awk '{print $1}' | xargs kill -9
    fi
fi

# 4, app task-state control
if [ $1 -eq 4 ] ; then
    if [ $2 -eq 1 ]; then
        echo ">>>4, app task-state on!"
        task-state > /dev/null &
    else
        echo ">>>4, app task-state off!"
        ps -ef | grep task-state | grep -v grep | awk '{print $1}' | xargs kill -9
    fi
fi

# 5, app task-data control
if [ $1 -eq 5 ] ; then
    if [ $2 -eq 1 ]; then
        echo ">>>5, app task-data on!"
        task-data > /dev/null &
    else
        echo ">>>5, app task-data off!"
        ps -ef | grep task-data | grep -v grep | awk '{print $1}' | xargs kill -9
    fi
fi

exit 0;