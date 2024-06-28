#!/bin/bash

set +e

POWER_CMD_OFF="busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis RequestedPowerTransition s xyz.openbmc_project.State.Chassis.Transition.Off"
POWER_CMD_ON="busctl set-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis RequestedPowerTransition s xyz.openbmc_project.State.Chassis.Transition.On"
IMAGE_FILE=$1
if [[ "$IMAGE_FILE" != /* ]]; then
        IMAGE_FILE="$(pwd)/$IMAGE_FILE"
fi

GPIOCHIP=816
GPIOB6=$((${GPIOCHIP} + 14))
GPIOD3=$((${GPIOCHIP} + 27))
GPIOG1=$((${GPIOCHIP} + 49))

SPI_DEV="1e630000.spi"
SPI_PATH="/sys/bus/platform/drivers/aspeed-smc"

power_status() {
	st=$(busctl get-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState | cut -d"." -f6)
	if [ "$st" == "On\"" ]; then
		echo "on"
	else
		echo "off"
	fi
}

set_gpio_to_bmc()
{
    echo "switch bios GPIO to bmc"
    if [ ! -d /sys/class/gpio/gpio$GPIOB6 ]; then
        cd /sys/class/gpio
        echo $GPIOB6 > export
        cd gpio$GPIOB6
    else
        cd /sys/class/gpio/gpio$GPIOB6
    fi
    direc=`cat direction`
    if [ $direc == "in" ]; then
        echo "out" > direction
    fi
    data=`cat value`
    if [ "$data" == "1" ]; then
        echo 0 > value
    fi

    if [ ! -d /sys/class/gpio/gpio$GPIOG1 ]; then
        cd /sys/class/gpio
        echo $GPIOG1 > export
        cd gpio$GPIOG1
    else
        cd /sys/class/gpio/gpio$GPIOG1
    fi
    direc=`cat direction`
    if [ $direc == "in" ]; then
        echo "out" > direction
    fi
    echo 1 > value
    sleep 1
    echo 0 > value
    sleep 1

    if [ ! -d /sys/class/gpio/gpio$GPIOD3 ]; then
        cd /sys/class/gpio
        echo $GPIOD3 > export
        cd gpio$GPIOD3
    else
        cd /sys/class/gpio/gpio$GPIOD3
    fi
    direc=`cat direction`
    if [ $direc == "in" ]; then
        echo "out" > direction
    fi
    data=`cat value`
    if [ "$data" == "0" ]; then
        echo 1 > value
    fi

    return 0
}
set_gpio_to_host()
{
    echo "switch bios GPIO to host"
    if [ ! -d /sys/class/gpio/gpio$GPIOB6 ]; then
        cd /sys/class/gpio
        echo $GPIOB6 > export
        cd gpio$GPIOB6
    else
        cd /sys/class/gpio/gpio$GPIOB6
    fi
    direc=`cat direction`
    if [ $direc == "in" ]; then
        echo "out" > direction
    fi
    data=`cat value`
    if [ "$data" == "1" ]; then
        echo 0 > value
    fi
    echo "in" > direction
    echo $GPIOB6 > /sys/class/gpio/unexport

    if [ ! -d /sys/class/gpio/gpio$GPIOG1 ]; then
        cd /sys/class/gpio
        echo $GPIOG1 > export
        cd gpio$GPIOG1
    else
        cd /sys/class/gpio/gpio$GPIOG1
    fi
    direc=`cat direction`
    if [ $direc == "in" ]; then
        echo "out" > direction
    fi
    data=`cat value`
    if [ "$data" == "1" ]; then
        echo 0 > value
    fi
    echo "in" > direction
    echo $GPIOG1 > /sys/class/gpio/unexport

    if [ ! -d /sys/class/gpio/gpio$GPIOD3 ]; then
        cd /sys/class/gpio
        echo $GPIOD3 > export
        cd gpio$GPIOD3
    else
        cd /sys/class/gpio/gpio$GPIOD3
    fi
    direc=`cat direction`
    if [ $direc == "in" ]; then
        echo "out" > direction
    fi
    data=`cat value`
    if [ "$data" == "1" ]; then
        echo 0 > value
    fi
    echo "in" > direction
    echo $GPIOD3 > /sys/class/gpio/unexport

    return 0
}
flash_image_to_mtd() {
        if [ -z "$IMAGE_FILE" ]; then
                echo "No image file specified."
                return 1
        fi

	if [ -e "$IMAGE_FILE" ];
	then
		echo "Bios image is $IMAGE_FILE"
		for d in mtd6 mtd7 ; do
			if [ -e "/dev/$d" ]; then
				mtd=`cat /sys/class/mtd/$d/name`
				if [ $mtd == "pnor" ]; then
					echo "Flashing bios image to $d..."
					flash_eraseall /dev/$d
					dd if=$IMAGE_FILE of=/dev/$d bs=4096
					if [ $? -eq 0 ]; then
						echo "bios updated successfully..."
					else
						echo "bios update failed..."
					fi
					break
				fi
				echo "$d is not a pnor device"
			fi
			echo "$d not available"
		done
	else
		echo "Bios image $IMAGE_FILE doesn't exist"
	fi
	popd
}

echo "Bios upgrade started at $(date)"
power_state=$(power_status)
echo "Current Host state is $power_state"

if [ "$power_state" != "off" ];
then
#Power off host server.
echo "Power off host server"
$POWER_CMD_OFF
sleep 10
fi

if [ $(power_status) != "off" ];
then
    echo "Host server didn't power off"
    echo "Bios upgrade failed"
    exit -1
fi
echo "Host server powered off"


#Flip GPIO to access SPI flash used by host.
echo "Set GPIO $GPIO to access SPI flash from BMC used by host"
set_gpio_to_bmc

#Bind spi driver to access flash
echo "bind aspeed-smc spi driver"
if [ -d $SPI_PATH/$SPI_DEV ]; then
    echo -n $SPI_DEV > $SPI_PATH/unbind    
fi
echo -n $SPI_DEV > $SPI_PATH/bind
if [ $? -eq 0 ];
then
    echo "SPI Driver Bind Successful"
else
    echo "SPI Driver Bind Failed.Run micron_v2.ini or micron_v3.ini using DediProg."
    set_gpio_to_host
    sleep 5
    exit -1
fi
sleep 1

#Flashcp image to device.
flash_image_to_mtd

#Unbind spi driver
sleep 1
echo "Unbind aspeed-smc spi driver"
echo -n $SPI_DEV > $SPI_PATH/unbind
sleep 1

#Flip GPIO back for host to access SPI flash
echo "Set GPIO back for host to access SPI flash"
set_gpio_to_host
sleep 1

if [ "$power_state" != "off" ];
then
$POWER_CMD_ON
sleep 1
fi
exit 0
