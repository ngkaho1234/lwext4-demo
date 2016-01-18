#ifndef _BLOCKDEV_H
#define _BLOCKDEV_H

int io_raw_get(char *file_name, struct ext4_blockdev **pbdev);
void io_raw_put(struct ext4_blockdev *bdev);

#endif