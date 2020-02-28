SEC("kprobe/__netif_receive_skb_core")
int bpf_prog1(struct pt_regs *ctx)
{
    /* attaches to kprobe netif_receive_skb,
     * looks for packets on loobpack device and prints them
     */
    char devname[IFNAMSIZ];
    struct net_device *dev;
    struct sk_buff *skb;
    int len;

    /* non-portable! works for the given kernel only */
    skb = (struct sk_buff *) PT_REGS_PARM1(ctx);
    dev = _(skb->dev);
    len = _(skb->len);

    bpf_probe_read(devname, sizeof(devname), dev->name);

    if (devname[0] == 'l' && devname[1] == 'o') {
        char fmt[] = "skb %p len %d\n";
        /* using bpf_trace_printk() for DEBUG ONLY */
        bpf_trace_printk(fmt, sizeof(fmt), skb, len);
    }

    return 0;
}

int main(int ac, char **argv)
{
    FILE *f;
    char filename[256];

    snprintf(filename, sizeof(filename), "%s_kern.o", argv[0]);
//首先通过load_bfp_file 将kernel space的部分装载到dram中。例如本例中对应的就是bpf_prog1
    if (load_bpf_file(filename)) {
        printf("%s", bpf_log_buf);
        return 1;
    }
//当年执行下面的命令时就会调用kernel space的__netif_receive_skb_core，从而触发bpf_prog1
    f = popen("taskset 1 ping -c5 localhost", "r");
    (void) f;
//在bpf_prog1 中我们通过bpf_trace_printk 将log打印到pipe中，通过read_trace_pipe 来读取pipe中的log，也就是bpf_prog1输出的log
    read_trace_pipe();

    return 0;
}