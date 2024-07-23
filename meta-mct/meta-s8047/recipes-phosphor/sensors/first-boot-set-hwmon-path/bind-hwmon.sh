#!/bin/sh -eu

power_status() {
    st=$(busctl get-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState | cut -d"." -f6)
    if [ "$st" == "On\"" ]; then
        echo "on"
    else
        echo "off"
    fi
}

read_dimm_temp_file() {
    success_count=0
    for temp_file in /sys/devices/platform/ahb/ahb:apb/ahb:apb:bus@1e78a000/1e78a300.i2c-bus/i2c-5/5-003c/hwmon/hwmon*/temp{1..8}_input; do
        if [ -r "$temp_file" ]; then
            temp_value=$(cat "$temp_file" 2>/dev/null || echo "error")
            if [ "$temp_value" != "error" ]; then
                success_count=$((success_count + 1))
            fi
        fi
    done
    echo $success_count
}

bind_hwmon() {
    echo 5-004c > /sys/bus/i2c/drivers/sbtsi/bind
    sleep 1
    echo 1e610000.pwm-tacho-controller > /sys/bus/platform/drivers/aspeed_g6_pwm_tacho/bind
    sleep 1
    echo 4-0061 > /sys/bus/i2c/drivers/pmbus/bind
    sleep 1
    echo 4-0062 > /sys/bus/i2c/drivers/pmbus/bind
    sleep 1
    echo 4-0063 > /sys/bus/i2c/drivers/pmbus/bind
    sleep 1
    echo 19-006a > /sys/bus/i2c/drivers/lm75/bind || true
    sleep 1
    echo 20-006a > /sys/bus/i2c/drivers/lm75/bind || true
    sleep 1
    echo 5-003c > /sys/bus/i2c/drivers/sbrmi/bind
    sleep 1

    echo "Initial bind done, checking DIMM temperature inputs..."
    initial_success_count=$(read_dimm_temp_file)
    echo "Initial success count: $initial_success_count"

    while [ $initial_success_count -eq 0 ]; do
        echo "No valid temperature input found, retrying in 10 seconds"
        sleep 10
        initial_success_count=$(read_dimm_temp_file)
        if [ $initial_success_count -gt 0 ]; then
            echo "Vaild DIMM temperature found success count: $initial_success_count"
            echo 5-003c > /sys/bus/i2c/drivers/sbrmi/unbind
            sleep 1
            echo 5-003c > /sys/bus/i2c/drivers/sbrmi/bind
        fi
    done

    for i in $(seq 1 30); do
        sleep 10
        new_success_count=$(read_dimm_temp_file)
        if [ $new_success_count -gt $initial_success_count ]; then
            initial_success_count=$new_success_count
            echo "New success count: $new_success_count, re-binding sbrmi"
            echo 5-003c > /sys/bus/i2c/drivers/sbrmi/unbind
            sleep 1
            echo 5-003c > /sys/bus/i2c/drivers/sbrmi/bind
        fi
    done

    echo "No change in success count after 5 minutes, exiting"
}

initial_status=$(power_status)
echo 5-004c > /sys/bus/i2c/drivers/sbtsi/unbind || true
sleep 1
echo 1e610000.pwm-tacho-controller > /sys/bus/platform/drivers/aspeed_g6_pwm_tacho/unbind || true
sleep 1

if [ "$initial_status" == "on" ]; then
    echo 4-0061 > /sys/bus/i2c/drivers/pmbus/unbind || true
    sleep 1
    echo 4-0062 > /sys/bus/i2c/drivers/pmbus/unbind || true
    sleep 1
    echo 4-0063 > /sys/bus/i2c/drivers/pmbus/unbind || true
    sleep 1
    echo 19-006a > /sys/bus/i2c/drivers/lm75/unbind || true
    sleep 1
    echo 20-006a > /sys/bus/i2c/drivers/lm75/unbind || true
    sleep 1
    echo 5-003c > /sys/bus/i2c/drivers/sbrmi/unbind || true
    sleep 1
    
    bind_hwmon
    exit 0

elif [ "$initial_status" == "off" ]; then
    dbus-monitor --system "type='signal',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged',path='/xyz/openbmc_project/state/chassis0',arg0='xyz.openbmc_project.State.Chassis'" | \
    while read -r line; do
        if echo "$line" | grep -q "xyz.openbmc_project.State.Chassis.PowerState.On"; then
            dbus_monitor_pid=$(systemctl status bind-hwmon.service | grep dbus-monitor | awk '{print $1}' | cut -c 3-)
            echo "Host has powered on, executing bind_hwmon"
            bind_hwmon
            echo "$dbus_monitor_pid" | head -n 1 | xargs kill
            exit 0
        fi
    done
fi
