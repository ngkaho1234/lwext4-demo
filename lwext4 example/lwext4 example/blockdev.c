#include "include/ext4_config.h"
#include "include/ext4_blockdev.h"
#include "include/ext4_errno.h"
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <windows.h>
#include <winioctl.h>

/**@brief   IO block size.*/
#define EXT4_IORAW_BSIZE 512

/**@brief   Block device structure.*/
struct block_dev {
	struct ext4_blockdev bdev;
	struct ext4_blockdev_iface bdif;
	unsigned char block_buf[EXT4_IORAW_BSIZE];
	HANDLE dev_file;
};

#define ALLOC_BDEV() calloc(1, sizeof(struct block_dev))
#define FREE_BDEV(p) free(p)

/**********************BLOCKDEV INTERFACE**************************************/
static int io_raw_open(struct ext4_blockdev *bdev);
static int io_raw_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
	uint32_t blk_cnt);
static int io_raw_bwrite(struct ext4_blockdev *bdev, const void *buf,
	uint64_t blk_id, uint32_t blk_cnt);
static int io_raw_close(struct ext4_blockdev *bdev);

/******************************************************************************/
int io_raw_get(char *file_name, struct ext4_blockdev **pbdev)
{
	uint64_t disk_size;
	LARGE_INTEGER FileSize;
	struct block_dev *bdev_full = ALLOC_BDEV();
	HANDLE dev_file;

	assert(bdev_full != NULL);

	dev_file =
		CreateFile(file_name, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
			0, NULL);

	if (dev_file == INVALID_HANDLE_VALUE) {
		FREE_BDEV(bdev_full);
		return EIO;
	}

	FileSize.LowPart = GetFileSize(dev_file, &FileSize.HighPart);
	if (FileSize.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
		FREE_BDEV(bdev_full);
		CloseHandle(dev_file);
		return EIO;
	}

	disk_size = FileSize.QuadPart;

	bdev_full->bdif.ph_bsize = EXT4_IORAW_BSIZE;
	bdev_full->bdif.ph_bcnt = disk_size / EXT4_IORAW_BSIZE;
	bdev_full->bdif.ph_bbuf = bdev_full->block_buf;

	bdev_full->bdif.open = io_raw_open;
	bdev_full->bdif.bread = io_raw_bread;
	bdev_full->bdif.bwrite = io_raw_bwrite;
	bdev_full->bdif.close = io_raw_close;

	bdev_full->bdev.bdif = &bdev_full->bdif;
	bdev_full->bdev.part_offset = 0;
	bdev_full->bdev.part_size = disk_size;

	bdev_full->dev_file = dev_file;

	*pbdev = (struct ext4_blockdev *)bdev_full;

	return EOK;
}

void io_raw_put(struct ext4_blockdev *bdev)
{
	struct block_dev *bdev_full = (struct block_dev *)bdev;
	if (bdev_full->dev_file && bdev_full->dev_file != INVALID_HANDLE_VALUE)
		CloseHandle(bdev_full->dev_file);

	bdev_full->dev_file = NULL;
	FREE_BDEV(bdev);
}

/******************************************************************************/
static int io_raw_open(struct ext4_blockdev *bdev)
{
	return EOK;
}

/******************************************************************************/

static int io_raw_bread(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id,
	uint32_t blk_cnt)
{
	struct block_dev *bdev_full = (struct block_dev *)bdev;
	LONG hipart = blk_id >> (32 - 9);
	LONG lopart = blk_id << 9;
	long err;

	SetLastError(0);
	lopart = SetFilePointer(bdev_full->dev_file, lopart, &hipart, FILE_BEGIN);

	if (lopart == -1 && NO_ERROR != (err = GetLastError())) {
		return EIO;
	}

	DWORD n;

	if (!ReadFile(bdev_full->dev_file, buf, blk_cnt * EXT4_IORAW_BSIZE, &n, NULL)) {
		err = GetLastError();
		return EIO;
	}
	return EOK;
}

/******************************************************************************/
static int io_raw_bwrite(struct ext4_blockdev *bdev, const void *buf,
	uint64_t blk_id, uint32_t blk_cnt)
{
	struct block_dev *bdev_full = (struct block_dev *)bdev;
	LONG hipart = blk_id >> (32 - 9);
	LONG lopart = blk_id << 9;
	long err;

	SetLastError(0);
	lopart = SetFilePointer(bdev_full->dev_file, lopart, &hipart, FILE_BEGIN);

	if (lopart == -1 && NO_ERROR != (err = GetLastError())) {
		return EIO;
	}

	DWORD n;

	if (!WriteFile(bdev_full->dev_file, buf, blk_cnt * 512, &n, NULL)) {
		err = GetLastError();
		return EIO;
	}
	return EOK;
}

/******************************************************************************/
static int io_raw_close(struct ext4_blockdev *bdev)
{
	struct block_dev *bdev_full = (struct block_dev *)bdev;
	if (bdev_full->dev_file && bdev_full->dev_file != INVALID_HANDLE_VALUE)
		CloseHandle(bdev_full->dev_file);

	bdev_full->dev_file = NULL;
	return EOK;
}
