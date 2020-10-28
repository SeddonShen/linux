#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/uio.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define NEMUDISK_MAJOR 230

#define ND_BUF_SR 0
#define ND_ADDR_SR 4
#define ND_LEN_SR 8
#define ND_OP_SR 12

#define ND_OP_RD 0
#define ND_OP_WR 1

#define NEMUDISK_ADDR ((volatile unsigned *)0xbfe97000)

#define ND_BUFSZ 1024

struct nemudisk_dev {
	size_t disk_size;
	struct cdev cdev;
	struct mutex mutex;
	uint8_t ndbuf[ND_BUFSZ]; // make sure to be shared
};

static struct nemudisk_dev nd_shared_data;

static int nemudisk_open(struct inode *inode, struct file *filp)
{
	filp->private_data =
		container_of(inode->i_cdev, struct nemudisk_dev, cdev);
	return 0;
}

static ssize_t nemudisk_read(struct file *filp, char __user *buf, size_t len,
			     loff_t *ppos)
{
	int ret;
	size_t i;
	loff_t pos = *ppos;
	struct nemudisk_dev *devp = filp->private_data;

	if (pos < 0)
		return -EINVAL;
	if (pos >= devp->disk_size)
		return 0;

	/* avoid ops + len overflow */
	if (len >= devp->disk_size - pos)
		len = devp->disk_size - pos;

	for (i = 0; i < len; i += ND_BUFSZ) {
		size_t curl = len - i;
		if (curl > ND_BUFSZ)
			curl = ND_BUFSZ;

		mutex_lock(&nd_shared_data.mutex);
		writel((unsigned long)devp->ndbuf, NEMUDISK_ADDR);
		writel(pos, NEMUDISK_ADDR + 1);
		writel(len, NEMUDISK_ADDR + 2);
		writel(ND_OP_RD, NEMUDISK_ADDR + 3);
		ret = copy_to_user(buf + i, devp->ndbuf, curl);
		mutex_unlock(&nd_shared_data.mutex);

		if (ret)
			return -EFAULT;
	}

	*ppos += len;
	return len;
}

static ssize_t nemudisk_write(struct file *filp, const char __user *buf,
			      size_t len, loff_t *ppos)
{
	int ret;
	size_t i;
	loff_t pos = *ppos;
	struct nemudisk_dev *devp = filp->private_data;
	if (pos < 0)
		return -EINVAL;
	if (pos >= devp->disk_size)
		return 0;

	if (len >= devp->disk_size - pos)
		len = devp->disk_size - pos;

	for (i = 0; i < len; i += ND_BUFSZ) {
		size_t curl = len - i;
		if (curl > ND_BUFSZ)
			curl = ND_BUFSZ;

		mutex_lock(&nd_shared_data.mutex);
		ret = copy_from_user(devp->ndbuf, buf + i, curl);

		if (ret) {
			mutex_unlock(&nd_shared_data.mutex);
			return -EFAULT;
		}

		writel((unsigned long)devp->ndbuf, NEMUDISK_ADDR);
		writel(pos, NEMUDISK_ADDR + 1);
		writel(len, NEMUDISK_ADDR + 2);
		writel(ND_OP_WR, NEMUDISK_ADDR + 3);
		mutex_unlock(&nd_shared_data.mutex);
	}

	*ppos += len;
	return len;
}

static loff_t nemudisk_llseek(struct file *filp, loff_t offset, int orig)
{
	struct nemudisk_dev *devp = filp->private_data;
	switch (orig) {
	case SEEK_SET:
		if (offset < 0)
			return -EINVAL;
		if (offset > devp->disk_size)
			return -EINVAL;
		filp->f_pos = (unsigned int)offset;
		return offset;
	case SEEK_CUR:
		if ((filp->f_pos + offset) > devp->disk_size)
			return -EINVAL;
		if ((filp->f_pos + offset) < 0)
			return -EINVAL;
		filp->f_pos += offset;
		break;
	case SEEK_END:
		filp->f_pos = devp->disk_size;
		break;
	default:
		return -EINVAL;
	}
	return filp->f_pos;
}

static long nemudisk_ioctl(struct file *filp, unsigned int cmd,
			   unsigned long arg)
{
	return -EINVAL;
}

static const struct file_operations nemudisk_ops = {
	.owner = THIS_MODULE,
	.open = nemudisk_open,
	.read = nemudisk_read,
	.write = nemudisk_write,
	.llseek = nemudisk_llseek,
	.unlocked_ioctl = nemudisk_ioctl,
#if 0
  .ioctl = xx,
#endif
};

static int __init nemudisk_init(void)
{
	int err_code = 0;
	dev_t dev = MKDEV(NEMUDISK_MAJOR, 0);

	err_code = register_chrdev_region(dev, 1, "nemudisk");
	if (err_code < 0)
		goto error_register_region;

	cdev_init(&nd_shared_data.cdev, &nemudisk_ops);
	nd_shared_data.cdev.owner = THIS_MODULE;

	nd_shared_data.disk_size = readl(NEMUDISK_ADDR + (ND_LEN_SR >> 2));
	printk("nemudisk size is %u\n", nd_shared_data.disk_size);

	err_code = cdev_add(&nd_shared_data.cdev, dev, 1);
	if (err_code < 0)
		goto error_cdev_add;

	mutex_init(&nd_shared_data.mutex);

	return 0;

error_cdev_add:
	printk("Fail to invoke cdev_add\n");

error_register_region:
	return err_code;
}

static void __exit nemudisk_exit(void)
{
	cdev_del(&nd_shared_data.cdev);
	unregister_chrdev_region(MKDEV(NEMUDISK_MAJOR, 0), 1);
}

module_init(nemudisk_init);
module_exit(nemudisk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wierton");
MODULE_DESCRIPTION("A simple global memory");
