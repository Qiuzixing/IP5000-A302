#!/bin/sh
spi_switch_gpio_init()
{
    reg_value=`io 0 0x1E6E2090 | awk '{printf $3}'`
    reg_value=0x$reg_value
    bit_clear=0xffffff3f
    reg_value=`printf "%x" $((reg_value&bit_clear))`
    io 1 0x1E6E2090 $reg_value

    reg_value=`io 0 0x1E6E2094 | awk '{printf $3}'`
    reg_value=0x$reg_value
    bit_clear=0xffffffdf
    reg_value=`printf "%x" $((reg_value&bit_clear))`
    io 1 0x1E6E2094 $reg_value

    reg_value=`io 0 0x1e780024 | awk '{printf $3}'`
    reg_value=0x$reg_value
    bit_set=0x03000000
    reg_value=`printf "%x" $((reg_value|bit_set))`
    io 1 0x1e780024 $reg_value
}

spi_switch_to_mx25u1632f()
{
    reg_value=`io 0 0x1e780020 | awk '{printf $3}'`
    reg_value=0x$reg_value
    bit_set=0x03000000
    reg_value=`printf "%x" $((reg_value|bit_set))`
    io 1 0x1e780020 $reg_value
}

spi_switch_to_arm_flash()
{
    reg_value=`io 0 0x1e780020 | awk '{printf $3}'`
	reg_value=0x$reg_value
	bit_clear=0xfcffffff
    reg_value=`printf "%x" $((reg_value&bit_clear))`
    io 1 0x1e780020 $reg_value
}

update()
{
    #pull down typec_chip reset
    #spi_switch_to_mx25u1632f    # Switching SPI_switch
    echo "update start"
    echo "erase..."
    echo 0 > /sys/devices/platform/m25p80/mx25u1632f_earse_all # Erase
    echo "erase success,write..."
    dd if=/share/typec.bin of=/sys/devices/platform/m25p80/mx25u1632f_write_file bs=256 # Write mx25u1632f Flash
    echo "write success"
    #spi_switch_to_arm_flash
    #pull up typec_chip reset
}
#spi_switch_gpio_init
update