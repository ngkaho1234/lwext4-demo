#include <Windows.h>

#include "../lwext4/lwext4/ext4.h"

#include "blockdev.h"
#include "test_lwext4.h"

#define bail(...) { \
	printf(__VA_ARGS__); \
	exit(EXIT_FAILURE); \
}

static bool verbose = true;
static int rw_size = 4096;
static int rw_count = 10;
static int dir_cnt = 500;

int main(int argc, char **argv)
{
	bool ret;
	struct ext4_blockdev *bdev;
	if (argc != 2)
		bail("Insufficient argument.\n");

	ret = io_raw_get(argv[1], &bdev);
	if (ret != EOK)
		bail("ret = %d.\n", ret);

	if (verbose)
		ext4_dmask_set(DEBUG_ALL);

	if (!(ret = test_lwext4_mount(bdev, NULL)))
		bail("ret = %d.\n", ret);

	test_lwext4_cleanup();

	test_lwext4_mp_stats();

	test_lwext4_dir_ls("/mp/");
	fflush(stdout);
	if (!(ret = test_lwext4_dir_test(dir_cnt)))
		bail("ret = %d.\n", ret);

	fflush(stdout);
	uint8_t *rw_buff = malloc(rw_size);
	if (!rw_buff)
		bail("Insufficient resources.\n");

	if (!(ret = test_lwext4_file_test(rw_buff, rw_size, rw_count)))
		bail("ret = %d.\n", ret);

	free(rw_buff);

	fflush(stdout);
	test_lwext4_dir_ls("/mp/");

	test_lwext4_mp_stats();

	test_lwext4_cleanup();

	if (!(ret = test_lwext4_umount()))
		bail("ret = %d.\n", ret);

	io_raw_put(bdev);

	printf("\ntest finished\n");
	return EXIT_SUCCESS;
}