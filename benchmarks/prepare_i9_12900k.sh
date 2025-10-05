#!/bin/bash

set -e

if [ "$EUID" -ne 0 ]; then
    echo "Execute as root: sudo $0"
    exit 1
fi

echo "Disabling Hyperthreading..."
THREADS_TO_DISABLE=(1 3 5 7 9 11 13 15)
for cpu in "${THREADS_TO_DISABLE[@]}"; do
    echo 0 > /sys/devices/system/cpu/cpu$cpu/online
done

echo "Setting CPU Governor to performance..."
echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null

echo "Configuring GRUB for P-core isolation..."
cp /etc/default/grub /etc/default/grub.backup

GRUB_PARAMS="isolcpus=0-15 nohz_full=0-15 rcu_nocbs=0-15 intel_pstate=disable processor.max_cstate=1 intel_idle.max_cstate=0"

if grep -q "isolcpus=" /etc/default/grub; then
    sed -i 's/isolcpus=[^ "]*//g' /etc/default/grub
    sed -i 's/nohz_full=[^ "]*//g' /etc/default/grub
    sed -i 's/rcu_nocbs=[^ "]*//g' /etc/default/grub
    sed -i 's/intel_pstate=[^ "]*//g' /etc/default/grub
    sed -i 's/processor.max_cstate=[^ "]*//g' /etc/default/grub
    sed -i 's/intel_idle.max_cstate=[^ "]*//g' /etc/default/grub
    sed -i 's/  */ /g' /etc/default/grub
fi

sed -i "s/GRUB_CMDLINE_LINUX=\"\(.*\)\"/GRUB_CMDLINE_LINUX=\"\1 ${GRUB_PARAMS}\"/" /etc/default/grub
sed -i 's/GRUB_CMDLINE_LINUX=" /GRUB_CMDLINE_LINUX="/' /etc/default/grub

update-grub

echo ""
echo "Configuration applied:"
echo "  HT: disabled"
echo "  Active CPUs: $(nproc)"
echo "  Governor: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)"
echo "  Isolation: cores 0-15"
echo "  Intel P-state: disabled"
echo "  C-states: disabled (max_cstate=0/1)"
echo ""
echo "GRUB parameters added:"
echo "  ${GRUB_PARAMS}"
echo ""
echo "Reboot to activate all optimizations: sudo reboot"
echo "After reboot, use: taskset -c 0 ./benchmark"