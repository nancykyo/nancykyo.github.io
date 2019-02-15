
相关的操作，开启关闭log
cat /sys/module/edac_core/parameters/edac_mc_log_ue
cat /sys/module/edac_core/parameters/edac_mc_log_ce


监控error数量方法
root@n24-029-138:~# cat /sys/devices/system/edac/mc/mc0/ce_count 