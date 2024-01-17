
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux_fileio.h>

int
linux_kernel_file_write (char *filename, int flags, void *buf, int size)
{
	int ret = 0;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	/* change to KERNEL_DS address limit */
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file to write */
	fp = filp_open(filename, flags, 0664);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		printk("%s: filp_open failed for file <%s>, err = %d\n",
			__FUNCTION__, filename, ret);
		goto exit;
	}

	/* Write buf to file */
	ret = vfs_write(fp, (char*) buf, size, &pos);
	if (ret < 0) {
		printk("%s: vfs_write failed for file <%s>, err = %d\n",
			__FUNCTION__, filename, ret);
		goto exit;
	}
	ret = 0;
exit:
	/* close file before return */
	if (!IS_ERR(fp))
		filp_close(fp, NULL);

	/* restore previous address limit */
	set_fs(old_fs);

	return ret;
}

/*
 * empty out a file (open in for writing/truncating & close it immediately)
 */
void
linux_kernel_file_rewind (char *filename)
{
	struct file *fp = NULL;
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(get_ds());
	fp = filp_open(filename, (O_CREAT | O_WRONLY | O_TRUNC), 0644);
	if (IS_ERR(fp)) {
		printk("%s: filp_open failed for <%s>, err = %ld\n,",
			__FUNCTION__, filename, PTR_ERR(fp));
	} else {
		filp_close(fp, NULL);
	}
	set_fs(oldfs);
}

