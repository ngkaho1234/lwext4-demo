/*
 * Copyright (c) 2013 Grzegorz Kostka (kostka.grzegorz@gmail.com)
 *
 *
 * HelenOS:
 * Copyright (c) 2012 Martin Sucha
 * Copyright (c) 2012 Frantisek Princ
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup lwext4
 * @{
 */
/**
 * @file  ext4_types.h
 * @brief Ext4 data structure definitions.
 */

#ifndef EXT4_TYPES_H_
#define EXT4_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ext4_config.h"
#include "ext4_blockdev.h"

#include <stddef.h>
#include <stdint.h>

#define EXT4_MIN_BLOCK_SIZE 1024  /* 1 KiB */
#define EXT4_MAX_BLOCK_SIZE 65536 /* 64 KiB */
#define EXT4_REV0_INODE_SIZE 128

#define EXT4_INODE_BLOCK_SIZE 512

#define EXT4_INODE_DIRECT_BLOCK_COUNT 12
#define EXT4_INODE_INDIRECT_BLOCK EXT4_INODE_DIRECT_BLOCK_COUNT
#define EXT4_INODE_DOUBLE_INDIRECT_BLOCK (EXT4_INODE_INDIRECT_BLOCK + 1)
#define EXT4_INODE_TRIPPLE_INDIRECT_BLOCK (EXT4_INODE_DOUBLE_INDIRECT_BLOCK + 1)
#define EXT4_INODE_BLOCKS (EXT4_INODE_TRIPPLE_INDIRECT_BLOCK + 1)
#define EXT4_INODE_INDIRECT_BLOCK_COUNT                                        \
	(EXT4_INODE_BLOCKS - EXT4_INODE_DIRECT_BLOCK_COUNT)

#pragma pack(push, 1)

/*
 * Structure of an inode on the disk
 */
struct ext4_inode {
	uint16_t mode;		    /* File mode */
	uint16_t uid;		    /* Low 16 bits of owner uid */
	uint32_t size_lo;	   /* Size in bytes */
	uint32_t access_time;       /* Access time */
	uint32_t change_inode_time; /* I-node change time */
	uint32_t modification_time; /* Modification time */
	uint32_t deletion_time;     /* Deletion time */
	uint16_t gid;		    /* Low 16 bits of group id */
	uint16_t links_count;       /* Links count */
	uint32_t blocks_count_lo;   /* Blocks count */
	uint32_t flags;		    /* File flags */
	uint32_t unused_osd1;       /* OS dependent - not used in HelenOS */
	uint32_t blocks[EXT4_INODE_BLOCKS]; /* Pointers to blocks */
	uint32_t generation;		    /* File version (for NFS) */
	uint32_t file_acl_lo;		    /* File ACL */
	uint32_t size_hi;
	uint32_t obso_faddr; /* Obsoleted fragment address */

	union {
		struct {
			uint16_t blocks_high;
			uint16_t file_acl_high;
			uint16_t uid_high;
			uint16_t gid_high;
			uint16_t checksum_lo; /* crc32c(uuid+inum+inode) LE */
			uint16_t reserved2;
		} linux2;
		struct {
			uint16_t reserved1;
			uint16_t mode_high;
			uint16_t uid_high;
			uint16_t gid_high;
			uint32_t author;
		} hurd2;
	};

	uint16_t extra_isize;
	uint16_t checksum_hi;	/* crc32c(uuid+inum+inode) BE */
	uint32_t ctime_extra; /* Extra change time (nsec << 2 | epoch) */
	uint32_t mtime_extra; /* Extra Modification time (nsec << 2 | epoch) */
	uint32_t atime_extra; /* Extra Access time (nsec << 2 | epoch) */
	uint32_t crtime;      /* File creation time */
	uint32_t
	    crtime_extra;    /* Extra file creation time (nsec << 2 | epoch) */
	uint32_t version_hi; /* High 32 bits for 64-bit version */
};

#pragma pack(pop)

#define EXT4_INODE_MODE_FIFO 0x1000
#define EXT4_INODE_MODE_CHARDEV 0x2000
#define EXT4_INODE_MODE_DIRECTORY 0x4000
#define EXT4_INODE_MODE_BLOCKDEV 0x6000
#define EXT4_INODE_MODE_FILE 0x8000
#define EXT4_INODE_MODE_SOFTLINK 0xA000
#define EXT4_INODE_MODE_SOCKET 0xC000
#define EXT4_INODE_MODE_TYPE_MASK 0xF000

/*
 * Inode flags
 */
#define EXT4_INODE_FLAG_SECRM 0x00000001     /* Secure deletion */
#define EXT4_INODE_FLAG_UNRM 0x00000002      /* Undelete */
#define EXT4_INODE_FLAG_COMPR 0x00000004     /* Compress file */
#define EXT4_INODE_FLAG_SYNC 0x00000008      /* Synchronous updates */
#define EXT4_INODE_FLAG_IMMUTABLE 0x00000010 /* Immutable file */
#define EXT4_INODE_FLAG_APPEND 0x00000020  /* writes to file may only append */
#define EXT4_INODE_FLAG_NODUMP 0x00000040  /* do not dump file */
#define EXT4_INODE_FLAG_NOATIME 0x00000080 /* do not update atime */

/* Compression flags */
#define EXT4_INODE_FLAG_DIRTY 0x00000100
#define EXT4_INODE_FLAG_COMPRBLK                                               \
	0x00000200			   /* One or more compressed clusters */
#define EXT4_INODE_FLAG_NOCOMPR 0x00000400 /* Don't compress */
#define EXT4_INODE_FLAG_ECOMPR 0x00000800  /* Compression error */

#define EXT4_INODE_FLAG_INDEX 0x00001000  /* hash-indexed directory */
#define EXT4_INODE_FLAG_IMAGIC 0x00002000 /* AFS directory */
#define EXT4_INODE_FLAG_JOURNAL_DATA                                           \
	0x00004000			  /* File data should be journaled */
#define EXT4_INODE_FLAG_NOTAIL 0x00008000 /* File tail should not be merged */
#define EXT4_INODE_FLAG_DIRSYNC                                                \
	0x00010000 /* Dirsync behaviour (directories only) */
#define EXT4_INODE_FLAG_TOPDIR 0x00020000    /* Top of directory hierarchies */
#define EXT4_INODE_FLAG_HUGE_FILE 0x00040000 /* Set to each huge file */
#define EXT4_INODE_FLAG_EXTENTS 0x00080000   /* Inode uses extents */
#define EXT4_INODE_FLAG_EA_INODE 0x00200000  /* Inode used for large EA */
#define EXT4_INODE_FLAG_EOFBLOCKS 0x00400000 /* Blocks allocated beyond EOF */
#define EXT4_INODE_FLAG_RESERVED 0x80000000  /* reserved for ext4 lib */

#define EXT4_DIRECTORY_FILENAME_LEN 255

/**@brief   Directory entry types. */
enum {
	EXT4_DE_UNKNOWN = 0,
	EXT4_DE_REG_FILE,
	EXT4_DE_DIR,
	EXT4_DE_CHRDEV,
	EXT4_DE_BLKDEV,
	EXT4_DE_FIFO,
	EXT4_DE_SOCK,
	EXT4_DE_SYMLINK
};

#ifdef __cplusplus
}
#endif

#endif /* EXT4_TYPES_H_ */

/**
 * @}
 */
