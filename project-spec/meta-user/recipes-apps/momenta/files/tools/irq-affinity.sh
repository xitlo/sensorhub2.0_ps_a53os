#!/bin/bash
SCRIPT_VERSION=v1.1

echo "irq-affinity, VER: $SCRIPT_VERSION"

#1, irq network, set affinity to CPU1
net0_irq_id=`cat /proc/interrupts | grep eth0 | awk '{print $1}' | cut -d : -f 1`
net0_aff_file=/proc/irq/${net0_irq_id}/smp_affinity

if [ -f ${net0_aff_file} ]; then
	echo 2 > ${net0_aff_file}
	echo ">>>>1, irq network to CPU1! ${net0_aff_file}"
	cat ${net0_aff_file}
else
	echo ">>>>1, can't find ${net0_aff_file}"
fi

#2, irq ipi, for openamp, set affinity to CPU3
amp_irq_id=`cat /proc/interrupts | grep zynqmp_ipi1 | awk '{print $1}' | cut -d : -f 1`
amp_aff_file=/proc/irq/${amp_irq_id}/smp_affinity

if [ -f ${amp_aff_file} ]; then
	echo 8 > ${amp_aff_file}
	echo ">>>>2, irq openamp to CPU3! ${amp_aff_file}"
	cat ${amp_aff_file}
else
	echo ">>>>2, can't find ${amp_aff_file}"
fi

echo "irq-affinity, exit"
exit 0;
