---
layout: post
title: Linux常见内核进程说明 - kswapd和kcompactd
tags: [Linux]
---
### kswapd

kswapd每个node分配一个，该线程每100毫秒起来工作一次，或者由于别的进程分配内存失败，而被唤醒。

如果是kswapd定期唤醒，它的任务就是让每一个zone的空闲内存都超过高水位。 Linux内核维护了3条内存水位线:

`pages_min`

```
#cat /proc/sys/vm/min_free_kbytes
3145728
```
或者

```
#cat /proc/zoneinfo  | grep min
min      63
min      7139
min      779229
```

两者对比验证基本保持一致：

```
#echo "3145728/4" |bc
786432
```

```
#echo "63+7139+779229" |bc
786431
```

`pages_low`

```
#cat /proc/zoneinfo  | grep low
low      78
low      8923
low      974036
```

`pages_high`

```
#cat /proc/zoneinfo  | grep high | grep -v :
high     94
high     10708
high     1168843
```

![image](https://github.com/luohao-brian/luohao-brian.github.io/blob/master/img/kswapd0.png?raw=true)


kswapd用来检查`pages_high`和 `pages_low`，如果可用内存少于 `pages_low`，kswapd 就开始扫描并试图释放 32个页面，并且重复扫描释放的过程直到可用内存大于`pages_high` 为止。扫描的时候检查3件事：

* 如果页面没有修改，把页放到可用内存列表里；
* 如果页面被文件系统修改，把页面内容写到磁盘上；
* 如果页面被修改 了，但不是被文件系统修改的，把页面写到交换空间。

如果kswapd是被唤醒的，说明有另一个进程在分配内存的时候遇到了麻烦。在唤醒kswapd的同时，这个进程还会把它正在试图分配的order等参数提交给kswapd， 除了保证zone内的空闲内存超过高水位，还得保证空闲内存在0~order之间平衡分布。所以如果达不到平衡分布，就还得继续回收以及尝试小order向大order的组装。


### kcompactd

在linux中使用buddy算法解决物理内存的外碎片问题，其把所有空闲的内存，以2的幂次方的形式，分成11个块链表，分别对应为1、2、4、8、16、32、64、128、256、512、1024个页块。

而Linux支持NUMA技术，对于NUMA设备，NUMA系统的结点通常是由一组CPU和本地内存组成，每一个节点都有相应的本地内存，因此buddyinfo 中的Node0表示节点ID；而每一个节点下的内存设备，又可以划分为多个内存区域（zone），因此下面的显示中，对于Node0的内存，又划分类DMA、Normal、HighMem区域。而后面则是表示空闲的区域。

此处以Normal区域进行分析，第二列值为5664，表示当前系统中normal区域，可用的连续两页的内存大小为5664*2*PAGE_SIZE；第三列值为1708，表示当前系统中normal区域，可用的连续四页的内存大小为1708*2^2*PAGE_SIZE

```
# cat /proc/buddyinfo
Node 0, zone      DMA      1      1      0      0      2      1      1      0      1      1      3
Node 0, zone    DMA32      2      5     16     16     14      3      2      2      3      0    355
Node 0, zone   Normal   9518   5664   1708    531    159     77     44     16      6      6  11536
Node 1, zone   Normal   1346    921    437    116    122     79     38     20     14      5  13223
```

传统的`kswapd`负责页面回收，主要满足如下两个目标：

1. zone内的空闲内存超过高水位，这主要是通过回收order=0的页面；
2. 要求zone内的内存在0到给定order之间平衡分布，这主要需要通过内存整理，将连续若干小order的的页面合并为大order页面。

从Linux 4.6开始，`kswapd`将更聚焦于内存回收，从`kswapd`分离出的`kcompactd`将专注于内存整理，旨在解决如下2个问题：

1. 目前内存整理都是由于系统缺少大order内存而被触发执行的，不利于维护整个系统内存在0到给定order之间平衡分布；
2. 大order的内存分配延时很高；


### 参考
* [Timer_go's Memory Blog ](https://www.cnblogs.com/muahao/p/6560175.html)
* [mm, compaction: introduce kcompactd](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=698b1b30642f1ff0ea10ef1de9745ab633031377)