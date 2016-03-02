#!/system/bin/sh
BB=/sbin/bb/busybox
TIMESTAMP=`date +%Y-%m-%d.%H.%M.%S`;
LOG="data/nebula.txt"
LOGS="/sdcard/Synapse/Logs"
export PATH=${PATH}:/sbin:/system/sbin:/system/bin:/system/xbin
###########################################################################################
###          -=[ The Nebula Project: The NebulaKernel Post Boot Script ]=-              ###
### ----------------------------------------------------------------------------------- ###
### UpDated: 01/25/2016    Custom Kernel Settings                                       ###
###                                                                                     ###
###-------------------------------------------------------------------------------------###
###########################################################################################

echo "-----------------------------------------------------------" | tee /dev/kmsg
echo "[NebulaKernel] Boot Script Started" | tee /dev/kmsg
echo "nebula-post_boot.sh Started" `date` > $LOG;

# protect init from oom
echo "-1000" > /proc/1/oom_score_adj;

# Mount root as RW to apply tweaks and settings
$BB mount -o remount,rw /;
$BB mount -o rw,remount /system

# Disable crap
if [ -e /system/bin/mpdecision ]; then
	echo "[NebulaKernel] mpdecison found: Disabled Now" | tee /dev/kmsg
	stop mpdecision;
	mv /system/bin/mpdecision /system/bin/mpdecision-bak;
fi
# disable the PowerHAL since there is now a kernel-side touch boost implemented
if [ -e /system/lib/hw/power.default.so ]; then
	echo "[NebulaKernel] power.default found: Disabled Now" | tee /dev/kmsg
	mv /system/lib/hw/power.default.so /system/lib/hw/power.default.so.bak;
fi

if [ -e /system/bin/thermal-engine ]; then
	echo "[NebulaKernel] thermal-engine found: Disabled Now" | tee /dev/kmsg
	mv /system/bin/thermal-engine /system/bin/thermal-engine-bak;
fi

if [ -e /system/etc/sysctl.conf ]; then
	echo "[NebulaKernel] sysctl.conf found: Disabled Now" | tee /dev/kmsg
	mv /system/etc/sysctl.conf /system/etc/sysctl.conf.bak;
fi

sleep 15


# Cleanup conflicts
$BB rm -f /system/etc/init.d/N4UKM;
$BB rm -f /system/etc/init.d/UKM;
$BB rm -f /system/etc/init.d/UKM_WAKE;
$BB rm -f /system/xbin/uci;
$BB rm -rf /data/UKM;
$BB rm -rf /data/data/leankernel;
if [ -e /system/xbin/zip ]; then
	$BB rm -f /sbin/zip;
fi;
echo "[NebulaKernel] Cleaned all Conflicts" | tee /dev/kmsg

# Make tmp folder
$BB mkdir /tmp;
if [ ! -d /data/.nebula ]; then
	$BB mkdir /data/.nebula/;
fi;
$BB chmod -R 0777 /data/.nebula/;

CRITICAL_PERM_FIX()
{
	# critical Permissions fix
	$BB chown -R root:root /tmp;
	$BB chown -R root:root /res;
	$BB chown -R root:root /sbin;
	$BB chown -R root:root /lib;
	$BB chmod -R 777 /tmp/;
	$BB chmod -R 775 /res/;
	$BB chmod -R 06755 /sbin/nebula/;
	$BB chmod -R 06755 /sbin/bb/;
	$BB chmod -R 06755 /sbin/nebula/;
	$BB chmod -R 06755 /system/xbin/;
	$BB chmod 6755 /res/synapse/actions/*;
$BB chmod -R 777 /res/synapse/files/*;
$BB echo "Boot initiated on $(date)" > /tmp/bootcheck;
echo "[NebulaKernel] Permissions to execute are now set" | tee /dev/kmsg
}
CRITICAL_PERM_FIX;

# make sure we own the device nodes
$BB chown system /sys/devices/system/cpu/cpufreq/ondemand/*
$BB chown system /sys/devices/system/cpu/cpu0/cpufreq/*
$BB chown system /sys/devices/system/cpu/cpu1/online
$BB chown system /sys/devices/system/cpu/cpu2/online
$BB chown system /sys/devices/system/cpu/cpu3/online
$BB chmod 666 /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
$BB chmod 666 /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
$BB chmod 666 /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
$BB chmod 444 /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq
$BB chmod 444 /sys/devices/system/cpu/cpu0/cpufreq/stats/*
$BB chmod 666 /sys/devices/system/cpu/cpufreq/all_cpus/*
$BB chmod 666 /sys/devices/system/cpu/cpu1/online
$BB chmod 666 /sys/devices/system/cpu/cpu2/online
$BB chmod 666 /sys/devices/system/cpu/cpu3/online
$BB chmod 666 /sys/module/msm_thermal/parameters/*
$BB chmod 666 /sys/kernel/intelli_plug/*
$BB chmod 666 /sys/class/kgsl/kgsl-3d0/max_gpuclk
$BB chmod 666 /sys/devices/fdb00000.qcom,kgsl-3d0/devfreq/fdb00000.qcom,kgsl-3d0/governor
$BB chmod 666 /sys/devices/fdb00000.qcom,kgsl-3d0/devfreq/fdb00000.qcom,kgsl-3d0/*_freq
echo "[NebulaKernel] Permissions Inforced.." | tee /dev/kmsg


# disable debugging on some modules
echo "N" > /sys/module/kernel/parameters/initcall_debug;
echo "0" > /sys/devices/fe12f000.slim/debug_mask
echo "0" > /sys/module/smd/parameters/debug_mask
echo "0" > /sys/module/smem/parameters/debug_mask
echo "0" > /sys/module/rpm_regulator_smd/parameters/debug_mask
echo "0" > /sys/module/ipc_router/parameters/debug_mask
echo "0" > /sys/module/event_timer/parameters/debug_mask
echo "0" > /sys/module/smp2p/parameters/debug_mask
echo "0" > /sys/module/msm_serial_hs_lge/parameters/debug_mask
#	echo "0" > /sys/module/msm_hotplug/parameters/debug_mask
#	echo "0" > /sys/module/cpufreq_limit/parameters/debug_mask
echo "0" > /sys/module/rpm_smd/parameters/debug_mask
echo "0" > /sys/module/smd_pkt/parameters/debug_mask
echo "0" > /sys/module/xt_qtaguid/parameters/debug_mask
echo "0" > /sys/module/qpnp_regulator/parameters/debug_mask
echo "0" > /sys/module/binder/parameters/debug_mask
echo "0" > /sys/module/msm_show_resume_irq/parameters/debug_mask
echo "0" > /sys/module/alarm_dev/parameters/debug_mask
echo "0" > /sys/module/mpm_of/parameters/debug_mask
echo "0" > /sys/module/msm_pm/parameters/debug_mask
echo "0" > /sys/module/spm_v2/parameters/debug_mask
echo "0" > /sys/module/alu_t_boost/parameters/debug_mask
echo "0" > /sys/module/lpm_levels/parameters/debug_mask
echo "0" > /sys/module/ipc_router_smd_xprt/parameters/debug_mask
echo "0" > /sys/module/x_tables/parameters/debug_mask
echo "0" > /sys/module/lge_touch_core/parameters/debug_mask
echo "[NebulaKernel] disable debug mask" | tee /dev/kmsg


sleep 15

#
# Stop Google Service and restart it on boot
# This removes high CPU load and ram leak!
#

if [ "$($BB pidof com.google.android.gms | wc -l)" -eq "1" ]; then
	$BB kill $($BB pidof com.google.android.gms);
fi;
if [ "$($BB pidof com.google.android.gms.unstable | wc -l)" -eq "1" ]; then
	$BB kill $($BB pidof com.google.android.gms.unstable);
fi;
if [ "$($BB pidof com.google.android.gms.persistent | wc -l)" -eq "1" ]; then
	$BB kill $($BB pidof com.google.android.gms.persistent);
fi;
if [ "$($BB pidof com.google.android.gms.wearable | wc -l)" -eq "1" ]; then
	$BB kill $($BB pidof com.google.android.gms.wearable);
fi;
echo "[NebulaKernel] Google Services have been temp killed" | tee /dev/kmsg

Google_Services_BatteryFix() {
# Google Services battery drain fixer by Alcolawl@xda
pm enable com.google.android.gms/.update.SystemUpdateActivity
pm enable com.google.android.gms/.update.SystemUpdateService
pm enable com.google.android.gms/.update.SystemUpdateService$ActiveReceiver
pm enable com.google.android.gms/.update.SystemUpdateService$Receiver
pm enable com.google.android.gms/.update.SystemUpdateService$SecretCodeReceiver
pm enable com.google.android.gsf/.update.SystemUpdateActivity
pm enable com.google.android.gsf/.update.SystemUpdatePanoActivity
pm enable com.google.android.gsf/.update.SystemUpdateService
pm enable com.google.android.gsf/.update.SystemUpdateService$Receiver
pm enable com.google.android.gsf/.update.SystemUpdateService$SecretCodeReceiver
echo "[NebulaKernel] Google Services Battery Drain Fixer By Alcolawl@xda." | tee /dev/kmsg
}
Google_Services_BatteryFix;

sleep 15
################################


# backup and replace Host AP Daemon for working Wi-Fi tether on 3.4 kernel Wi-Fi drivers
#$bb [ -e /system/bin/hostapd.dvbak ] || $bb cp /system/bin/hostapd /system/bin/hostapd.dvbak;
#$bb cp -f /sbin/hostapd /system/bin/;
#$bb chown root.shell /system/bin/hostapd;
#$bb chmod 755 /system/bin/hostapd;


## Install Busybox System Wide (Latest Edition)
# $BB sh /sbin/nebula/busybox.sh;

############################
# Tweak Background Writeout
#
echo 200 > /proc/sys/vm/dirty_expire_centisecs
echo 40 > /proc/sys/vm/dirty_ratio
echo 5 > /proc/sys/vm/dirty_background_ratio
echo 10 > /proc/sys/vm/swappiness

############################
# ZRAM Settings
#
#echo "lzo [lz4]" > /sys/block/zram0/comp_algorithm
#echo 3 > /sys/block/zram0/max_comp_stream
#echo 300 > /sys/devices/virtual/block/zram0/disksize

############################
# MSM_Hotplug Settings
#
echo 0 > /sys/module/msm_hotplug/msm_enabled
echo 1 > /sys/module/msm_hotplug/min_cpus_online
echo 2 > /sys/module/msm_hotplug/cpus_boosted
echo 500 > /sys/module/msm_hotplug/down_lock_duration
echo 2500 > /sys/module/msm_hotplug/boost_lock_duration
echo 200 5:100 50:50 350:200 > /sys/module/msm_hotplug/update_rates
echo 100 > /sys/module/msm_hotplug/fast_lane_load
echo 1 > /sys/module/msm_hotplug/max_cpus_online_susp


############################
# INTELLI_PLUG Settings   
#
echo 1 > /sys/kernel/intelli_plug/intelli_plug_active
echo 2500 > /sys/kernel/intelli_plug/boost_lock_duration
echo 722 > /sys/kernel/intelli_plug/cpu_nr_run_threshold
echo 2 > /sys/kernel/intelli_plug/cpus_boosted
echo 268 > /sys/kernel/intelli_plug/def_sampling_ms
echo 2500 > /sys/kernel/intelli_plug/down_lock_duration
echo 0 > /sys/kernel/intelli_plug/full_mode_profile
echo 1 > /sys/kernel/intelli_plug/min_cpus_online
echo 4 > /sys/kernel/intelli_plug/max_cpus_online
echo 1 > /sys/kernel/intelli_plug/max_cpus_online_susp
echo 3 > /sys/kernel/intelli_plug/nr_fshift
echo 8 > /sys/kernel/intelli_plug/nr_run_hysteresis
echo 10 > /sys/kernel/intelli_plug/suspend_defer_time


############################
# MSM Limiter
#
echo 300000 > /sys/kernel/msm_limiter/suspend_min_freq_0
echo 300000 > /sys/kernel/msm_limiter/suspend_min_freq_1
echo 300000 > /sys/kernel/msm_limiter/suspend_min_freq_2
echo 300000 > /sys/kernel/msm_limiter/suspend_min_freq_3
echo 2457600 > /sys/kernel/msm_limiter/resume_max_freq_0
echo 2457600 > /sys/kernel/msm_limiter/resume_max_freq_1
echo 2457600 > /sys/kernel/msm_limiter/resume_max_freq_2
echo 2457600 > /sys/kernel/msm_limiter/resume_max_freq_3
echo 1728000 > /sys/kernel/msm_limiter/suspend_max_freq

###################################################################
### GPU CLK: Set default to 657 ###
# make sure our max gpu clock is set via sysfs
echo "657500000" > /sys/class/kgsl/kgsl-3d0/max_gpuclk
#echo "200000000" > /sys/devices/fdb00000.qcom,kgsl-3d0/devfreq/fdb00000.qcom,kgsl-3d0/min_freq
#echo "657500000" > /sys/devices/fdb00000.qcom,kgsl-3d0/devfreq/fdb00000.qcom,kgsl-3d0/max_freq

############################
# MSM Thermal (Intelli-Thermal v2)
#
echo 0 > /sys/module/msm_thermal/core_control/enabled
echo 1 > /sys/module/msm_thermal/parameters/enabled
echo 0 > /sys/module/msm_thermal/vdd_restriction/enabled

############################
# Alucard Touch Boost Settings
#
echo 1497600 > /sys/module/alu_t_boost/parameters/input_boost_freq

############################
# Power Effecient Workqueues (Enable for battery)
#
echo 1 > /sys/module/workqueue/parameters/power_efficient

############################
# Scheduler and Read Ahead
#
echo zen > /sys/block/mmcblk0/queue/scheduler
echo 1024 > /sys/block/mmcblk0/bdi/read_ahead_kb


sleep 15

############################
# Governor Tunings
#
echo ondemand > /sys/kernel/msm_limiter/scaling_governor_0
echo 95 > /sys/devices/system/cpu/cpufreq/ondemand/up_threshold
echo 50000 > /sys/devices/system/cpu/cpufreq/ondemand/sampling_rate
echo 1 > /sys/devices/system/cpu/cpufreq/ondemand/io_is_busy
echo 4 > /sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor
echo 10 > /sys/devices/system/cpu/cpufreq/ondemand/down_differential
echo 75 > /sys/devices/system/cpu/cpufreq/ondemand/up_threshold_multi_core
echo 3 > /sys/devices/system/cpu/cpufreq/ondemand/down_differential_multi_core
echo 960000 > /sys/devices/system/cpu/cpufreq/ondemand/optimal_freq
echo 960000 > /sys/devices/system/cpu/cpufreq/ondemand/sync_freq
echo 85 > /sys/devices/system/cpu/cpufreq/ondemand/up_threshold_any_cpu_load

echo impulse > /sys/kernel/msm_limiter/scaling_governor_0
echo 20000 1400000:40000 1700000:20000 > /sys/devices/system/cpu/cpufreq/impulse/above_hispeed_delay
echo 95 > /sys/devices/system/cpu/cpufreq/impulse/go_hispeed_load
echo 1190400 > /sys/devices/system/cpu/cpufreq/impulse/hispeed_freq
echo 1 > /sys/devices/system/cpu/cpufreq/impulse/io_is_busy
echo 85 1500000:90 1800000:70 > /sys/devices/system/cpu/cpufreq/impulse/target_loads
echo 40000 > /sys/devices/system/cpu/cpufreq/impulse/min_sample_time
echo 30000 > /sys/devices/system/cpu/cpufreq/impulse/timer_rate
echo 100000 > /sys/devices/system/cpu/cpufreq/impulse/max_freq_hysteresis
echo 30000 > /sys/devices/system/cpu/cpufreq/impulse/timer_slack
echo 1 > /sys/devices/system/cpu/cpufreq/impulse/powersave_bias

echo smartmax > /sys/kernel/msm_limiter/scaling_governor_0
echo smartmax > /sys/kernel/msm_limiter/scaling_governor_1
echo smartmax > /sys/kernel/msm_limiter/scaling_governor_2
echo smartmax > /sys/kernel/msm_limiter/scaling_governor_3
echo 729600 > /sys/devices/system/cpu/cpufreq/smartmax/suspend_ideal_freq
echo 1036800 > /sys/devices/system/cpu/cpufreq/smartmax/awake_ideal_freq
echo 0 > /sys/devices/system/cpu/cpufreq/smartmax/io_is_busy
echo 70 > /sys/devices/system/cpu/cpufreq/smartmax/max_cpu_load
echo 30 > /sys/devices/system/cpu/cpufreq/smartmax/min_cpu_load
echo 1497600 > /sys/devices/system/cpu/cpufreq/smartmax/touch_poke_freq
echo 1497600 > /sys/devices/system/cpu/cpufreq/smartmax/boost_freq

### Default Governor on Boot ###
echo interactive > /sys/kernel/msm_limiter/scaling_governor_0
echo interactive > /sys/kernel/msm_limiter/scaling_governor_1
echo interactive > /sys/kernel/msm_limiter/scaling_governor_2
echo interactive > /sys/kernel/msm_limiter/scaling_governor_3
echo 20000 1400000:40000 1700000:20000 > /sys/devices/system/cpu/cpufreq/interactive/above_hispeed_delay
echo 90 > /sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load
echo 1190400 > /sys/devices/system/cpu/cpufreq/interactive/hispeed_freq
echo 1 > /sys/devices/system/cpu/cpufreq/interactive/io_is_busy
echo 85 1500000:90 1800000:70 > /sys/devices/system/cpu/cpufreq/interactive/target_loads
echo 40000 > /sys/devices/system/cpu/cpufreq/interactive/min_sample_time
echo 30000 > /sys/devices/system/cpu/cpufreq/interactive/timer_rate
echo 100000 > /sys/devices/system/cpu/cpufreq/interactive/max_freq_hysteresis
echo 30000 > /sys/devices/system/cpu/cpufreq/interactive/timer_slack

sleep 15

############################
# LMK Tweaks
#
echo 2560,4096,8192,16384,24576,32768 > /sys/module/lowmemorykiller/parameters/minfree
echo 32 > /sys/module/lowmemorykiller/parameters/cost

############################
# MISC Tweaks
#
echo 0 > /sys/kernel/sched/gentle_fair_sleepers
echo 1 > /sys/module/adreno_idler/parameters/adreno_idler_active
echo 2 > /sys/devices/system/cpu/sched_mc_power_savings

# Allow untrusted apps to read from debugfs
if [ -e /system/lib/libsupol.so ]; then
/system/xbin/supolicy --live \
	"allow untrusted_app debugfs file { open read getattr }" \
	"allow untrusted_app sysfs_lowmemorykiller file { open read getattr }" \
	"allow untrusted_app persist_file dir { open read getattr }" \
	"allow debuggerd gpu_device chr_file { open read getattr }" \
	"allow netd netd capability fsetid" \
	"allow netd { hostapd dnsmasq } process fork" \
	"allow { system_app shell } dalvikcache_data_file file write" \
	"allow { zygote mediaserver bootanim appdomain }  theme_data_file dir { search r_file_perms r_dir_perms }" \
	"allow { zygote mediaserver bootanim appdomain }  theme_data_file file { r_file_perms r_dir_perms }" \
	"allow system_server { rootfs resourcecache_data_file } dir { open read write getattr add_name setattr create remove_name rmdir unlink link }" \
	"allow system_server resourcecache_data_file file { open read write getattr add_name setattr create remove_name unlink link }" \
	"allow system_server dex2oat_exec file rx_file_perms" \
	"allow mediaserver mediaserver_tmpfs file execute" \
	"allow drmserver theme_data_file file r_file_perms" \
	"allow zygote system_file file write" \
	"allow atfwd property_socket sock_file write" \
	"allow debuggerd app_data_file dir search" \
	"allow sensors diag_device chr_file { read write open ioctl }" \
	"allow sensors sensors capability net_raw" \
	"allow init kernel security setenforce" \
	"allow netmgrd netmgrd netlink_xfrm_socket nlmsg_write" \
	"allow netmgrd netmgrd socket { read write open ioctl }"
fi;

# Calibrate display
$BB echo "250 250 255" > /sys/devices/platform/kcal_ctrl.0/kcal
$BB echo 243 > /sys/devices/platform/kcal_ctrl.0/kcal_sat
$BB echo 1515 > /sys/devices/platform/kcal_ctrl.0/kcal_hue
$BB echo 250 > /sys/devices/platform/kcal_ctrl.0/kcal_val

# remount sysfs+sdcard with noatime,nodiratime since that's all they accept
$bb mount -o remount,nosuid,nodev,noatime,nodiratime -t auto /;
$bb mount -o remount,nosuid,nodev,noatime,nodiratime -t auto /proc;
$bb mount -o remount,nosuid,nodev,noatime,nodiratime -t auto /sys;
$bb mount -o remount,nosuid,nodev,noatime,nodiratime -t auto /sys/kernel/debug;
$bb mount -o remount,nosuid,nodev,noatime,nodiratime -t auto /mnt/shell/emulated;
for i in /storage/emulated/*; do
  $bb mount -o remount,nosuid,nodev,noatime,nodiratime -t auto $i;
  $bb mount -o remount,nosuid,nodev,noatime,nodiratime -t auto $i/Android/obb;
done;
echo "[NebulaKernel] Remounted sysfs+sdcard With noatime, nodiratime" | tee /dev/kmsg

# fix permissions for any included governors (and older underlying ramdisks)
governor=reset;
while sleep 3; do
  current=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`;
  if [ $governor != $current ]; then
    governor=$current;
    for i in /sys/devices/system/cpu/cpufreq/*; do
      $bb chown system:system $i/*;
      $bb chmod 664 $i/*;
    done;
  fi;
done&

# lmk tweaks for fewer empty background processes
minfree=6144,8192,12288,16384,24576,40960;
lmk=/sys/module/lowmemorykiller/parameters/minfree;
minboot=`cat $lmk`;
while sleep 1; do
  if [ `cat $lmk` != $minboot ]; then
    [ `cat $lmk` != $minfree ] && echo $minfree > $lmk || exit;
  fi;
done&

# lmk whitelist for common launchers and increase launcher priority
list="com.android.launcher com.google.android.googlequicksearchbox org.adw.launcher org.adwfreak.launcher net.alamoapps.launcher com.anddoes.launcher com.android.lmt com.chrislacy.actionlauncher.pro com.cyanogenmod.trebuchet com.gau.go.launcherex com.gtp.nextlauncher com.miui.mihome2 com.mobint.hololauncher com.mobint.hololauncher.hd com.qihoo360.launcher com.teslacoilsw.launcher com.tsf.shell org.zeam";
while sleep 60; do
  for class in $list; do
    if [ `$bb pgrep $class | head -n 1` ]; then
      launcher=`$bb pgrep $class`;
      $bb echo -17 > /proc/$launcher/oom_adj;
      $bb chmod 100 /proc/$launcher/oom_adj;
      $bb renice -18 $launcher;
    fi;
  done;
  exit;
done&

# wait for systemui and increase its priority
while sleep 3; do
  if [ `$bb pidof com.android.systemui` ]; then
    systemui=`$bb pidof com.android.systemui`;
    $bb renice -18 $systemui;
    $bb echo -17 > /proc/$systemui/oom_adj;
    $bb chmod 100 /proc/$systemui/oom_adj;
    exit;
  fi;
done&

while [ "$(cat /sys/class/thermal/thermal_zone5/temp)" -ge "65" ]; do
		sleep 5;
	done;

dmesg() {
		saved="$LOGS/dmesg-$TIMESTAMP.txt";
		dmesg > $saved;
		}
		
Setup_Synapse() {
#	ln -s /res/synapse/uci /sbin/uci;
if [ ! -d /sdcard/Synapse/crontab ]; then
	$BB mkdir /sdcard/Synapse/crontab/;
fi;
$BB chmod -R 0777 /sdcard/Synapse/crontab/;
sleep 1

# Copy Cron files
cp -af /res/crontab/ /sdcard/Synapse
if [ ! -e /sdcard/Synapse/crontab/custom_jobs ]; then
	touch /sdcard/Synapse/crontab/custom_jobs;
	chmod 777 /sdcard/Synapse/crontab/custom_jobs;
fi;

	$BB mount -t rootfs -o remount,rw rootfs
	sleep 2
	$BB chmod -R 755 /res/synapse
	$BB ln -fs /res/synapse/uci /sbin/uci
	sleep 5
	cd /;
	/sbin/uci start;
		sleep 3
	/sbin/uci restart;
	sleep 3
	/sbin/uci reset;
	sleep 2
	/sbin/uci;
# $BB mount -t rootfs -o remount,ro rootfs
# $BB chmod 0755 /system/sbin/uci
	echo "[NebulaKernel] Synapse Is now Setup." | tee /dev/kmsg
}




############################
# Synapse + UKM Support
#
Setup_Synapse;
/sbin/uci


echo "[NebulaKernel] Boot Script Completed!" | tee /dev/kmsg
echo "-----------------------------------------------------------" | tee /dev/kmsg
echo "nebula-post_boot.sh Finished Loading" `date` >> $LOG;

$BB mount -o remount,ro /system;
sleep 15
/sbin/uci
