/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * The ANSI C Implementation of The File Subsystem
 *  - On most platforms, we use this stdio-based code for file access.
 *  - On some platforms, we use alternative implementations instead of this code:
 *    - Android: ndkfile.c (Resources)
 *    - Wasm: emfile.c (Emscripten's FS)
 *    - Unity: PolarisEngineScript.cs (StreamingAssets)
 */

/* Base */
#include "polarisengine.h"

/*
 * Replace POSIX strcasecmp() to DOS _stricmp() on MSVC.
 */
#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/*
 * fcntl.h is required on Win32.
 */
#ifdef POLARIS_ENGINE_TARGET_WIN32
#include <fcntl.h>
#endif

/* Obfuscation Key */
#include "key.h"

/*
 * "Obfuscattion Key" is stored with obfuscation.
 */
static volatile uint64_t key_obfuscated =
	~((((OBFUSCATION_KEY >> 56) & 0xff) << 0) |
	  (((OBFUSCATION_KEY >> 48) & 0xff) << 8) |
	  (((OBFUSCATION_KEY >> 40) & 0xff) << 16) |
	  (((OBFUSCATION_KEY >> 32) & 0xff) << 24) |
	  (((OBFUSCATION_KEY >> 24) & 0xff) << 32) |
	  (((OBFUSCATION_KEY >> 16) & 0xff) << 40) |
	  (((OBFUSCATION_KEY >> 8)  & 0xff) << 48) |
	  (((OBFUSCATION_KEY >> 0)  & 0xff) << 56));
static volatile uint64_t key_reversed;
static volatile uint64_t *key_ref = &key_reversed;

/*
 * These keys are not secrets.
 */
#define NEXT_MASK1	0xafcb8f2ff4fff33f
#define NEXT_MASK2	0xfcbfaff8f2f4f3f0

/*
 * File read stream
 */
struct rfile {
	/* Is a packaged file? */
	bool is_packaged;

	/* Is obfuscated? */
	bool is_obfuscated;

	/* stdio FILE pointer */
	FILE *fp;

	/* Obfuscation parameters */
	uint64_t next_random;
	uint64_t prev_random;

	/* Effective for a packaged file: */
	uint64_t index;
	uint64_t size;
	uint64_t offset;
	uint64_t pos;
};

/*
 * File write stream.
 */
struct wfile {
	FILE *fp;
	uint64_t next_random;
};

/* File entries in the package. */
static struct file_entry entry[FILE_ENTRY_SIZE];

/* File entry count. */
static uint64_t entry_count;

/* Package file path. */
static char *package_path;

/*
 * Use utf-8 to utf-16 conversion on Win32.
 */
#ifdef POLARIS_ENGINE_TARGET_WIN32
const wchar_t *conv_utf8_to_utf16(const char *s);
#endif

/*
 * Forward declarations.
 */
static void warn_file_name_case(const char *dir, const char *file);
static void ungetc_rfile(struct rfile *rf, char c);
static void set_random_seed(uint64_t index, uint64_t *next_random);
static char get_next_random(uint64_t *next_random, uint64_t *prev_random);
static void rewind_random(uint64_t *next_random, uint64_t *prev_random);

/*
 * Initialization
 */

/*
 * Initialize the file subsystem.
 */
bool init_file(void)
{
#if defined(USE_EDITOR)
	/* Disable the feature to open packages on the editor apps. */
	return true;
#else
	FILE *fp;
	uint64_t i, next_random;
	int j;

	/* Get the actual path to "data01.arc". */
	package_path = make_valid_path(NULL, PACKAGE_FILE);
	if (package_path == NULL)
		return false;

	/* Try opening a package file "data01.arc". */
#ifdef POLARIS_ENGINE_TARGET_WIN32
	_fmode = _O_BINARY;
	fp = _wfopen(conv_utf8_to_utf16(package_path), L"r");
#else
	fp = fopen(package_path, "r");
#endif
	if (fp == NULL) {
		/* Failed to open. */
		free(package_path);
		package_path = NULL;
#if defined(POLARIS_ENGINE_TARGET_IOS) || defined(POLARIS_ENGINE_TARGET_WASM)
		/* Fail: On iOS and Wasm, we need to open "data01.arc". */
		return false;
#else
		/* On other platforms, we don't need "data01.arc". */
		return true;
#endif
	}

	/* Read the number of the file entries. */
	if (fread(&entry_count, sizeof(uint64_t), 1, fp) < 1) {
		log_package_file_error();
		fclose(fp);
		return false;
	}
	if (entry_count > FILE_ENTRY_SIZE) {
		log_package_file_error();
		fclose(fp);
		return false;
	}

	/* Read the file entries. */
	for (i = 0; i < entry_count; i++) {
		if (fread(&entry[i].name, FILE_NAME_SIZE, 1, fp) < 1)
			break;
		set_random_seed(i, &next_random);
		for (j = 0; j < FILE_NAME_SIZE; j++)
			entry[i].name[j] ^= get_next_random(&next_random, NULL);
		if (fread(&entry[i].size, sizeof(uint64_t), 1, fp) < 1)
			break;
		if (fread(&entry[i].offset, sizeof(uint64_t), 1, fp) < 1)
			break;
	}
	if (i != entry_count) {
		log_package_file_error();
		fclose(fp);
		return false;
	}

	/* Close the package for now; we will open one FILE pointer per an input stream.*/
	fclose(fp);

	return true;
#endif
}

/*
 * Cleanup the file subsystem.
 */
void cleanup_file(void)
{
	free(package_path);
}

/*
 * Read
 */

/*
 * Check whether a file exists.
 */
bool check_file_exist(const char *dir, const char *file)
{
	char entry_name[FILE_NAME_SIZE];
	FILE *fp;
	uint64_t i;

#if !defined(POLARIS_ENGINE_TARGET_IOS) || !defined(POLARIS_ENGINE_TARGET_WASM)
	/* Check a file on the real file system first. */
	char *real_path;
	real_path = make_valid_path(dir, file);
	if (real_path == NULL) {
		log_memory();
		return NULL;
	}
#ifdef POLARIS_ENGINE_TARGET_WIN32
	_fmode = _O_BINARY;
	fp = _wfopen(conv_utf8_to_utf16(real_path), L"r");
#else
	fp = fopen(real_path, "r");
#endif
	if (fp != NULL) {
		/* File exists. */
		fclose(fp);
		return true;
	}
#endif

	/* Check whether a package entry exists. */
	snprintf(entry_name, FILE_NAME_SIZE, "%s/%s", dir, file);
	for (i = 0; i < entry_count; i++) {
		if (strcasecmp(entry[i].name, entry_name) == 0){
			/* Entry exists. */
			return true;
		}
	}

	/* File does not exist. */
	return false;
}

/*
 * Open a read file stream.
 */
struct rfile *open_rfile(
	const char *dir,
	const char *file,
	bool save_data)
{
	char entry_name[FILE_NAME_SIZE];
	char *real_path;
	struct rfile *rf;
	uint64_t i;

	warn_file_name_case(dir, file);

	/* Allocate a rfile struct. */
	rf = malloc(sizeof(struct rfile));
	if (rf == NULL) {
		log_memory();
		return NULL;
	}

	/* Make an effective path on the real file system. */
	real_path = make_valid_path(dir, file);
	if (real_path == NULL) {
		log_memory();
		free(rf);
		return NULL;
	}

	/* Try a real file first. */
#ifdef POLARIS_ENGINE_TARGET_WIN32
	_fmode = _O_BINARY;
	rf->fp = _wfopen(conv_utf8_to_utf16(real_path), L"r");
#else
	rf->fp = fopen(real_path, "r");
#endif
	if (rf->fp != NULL) {
		/* Opened: use a real file. */
		free(real_path);
		rf->is_packaged = false;
		if (save_data) {
			rf->is_obfuscated = true;
			set_random_seed(0, &rf->next_random);
		} else {
			rf->is_obfuscated = false;
		}
		return rf;
	}
	free(real_path);

	/* If the file is save data, we don't search from the package. */
	if (save_data) {
		free(rf);
		return NULL;
	}

	/* If we don't have a package, we failed to open a file. */
	if (package_path == NULL) {
		log_dir_file_open(dir, file);
		free(rf);
		return NULL;
	}

	/* Search a file entry on the package. */
	snprintf(entry_name, FILE_NAME_SIZE, "%s/%s", dir, file);
	for (i = 0; i < entry_count; i++) {
		if (strcasecmp(entry[i].name, entry_name) == 0)
			break;
	}
	if (i == entry_count) {
		/* Not found. */
		log_dir_file_open(dir, file);
		free(rf);
		return NULL;
	}

	/* Found: open a new FILE pointer to "data01.arc". */
#ifdef POLARIS_ENGINE_TARGET_WIN32
	_fmode = _O_BINARY;
	rf->fp = _wfopen(conv_utf8_to_utf16(package_path), L"r");
#else
	rf->fp = fopen(package_path, "r");
#endif
	if (rf->fp == NULL) {
		log_file_open(PACKAGE_FILE);
		free(rf);
		return NULL;
	}

	/* Seek to the offset. */
	if (fseek(rf->fp, (long)entry[i].offset, SEEK_SET) != 0) {
		log_package_file_error();
		fclose(rf->fp);
		free(rf);
		return 0;
	}

	/* Setup the rfile struct. */
	rf->is_packaged = true;
	rf->is_obfuscated = true;
	rf->index = i;
	rf->size = entry[i].size;
	rf->offset = entry[i].offset;
	rf->pos = 0;
	set_random_seed(i, &rf->next_random);
	rf->prev_random = 0;

	/* Return a pointer to a rfile. */
	return rf;
}

/* Warn if a file name contains any upper case ASCII character. */
static void warn_file_name_case(const char *dir, const char *file)
{
	const char *c;
	bool after_dot;

	c = file;
	after_dot = false;
	while (*c != '\0') {
		/* Decode utf-8. */
		if (((*c) & 0x80) == 0) {
			/* 1-byte length */
			if (!after_dot) {
				if (*c == '.')
					after_dot = true;
			} else {
				/* 1-byte */
				if (*c >= 'A' && *c <= 'Z') {
					log_file_name_case(dir, file);
					return;
				}
			}
			c++;
		} else if(((*c) & 0xe0) == 0xc0) {
			/* 2-byte length */
			c += 2;
		} else if(((*c) & 0xf0) == 0xe0) {
			/* 3-byte length */
			c += 3;
		} else if (((*c) & 0xf8) == 0xf0) {
			/* 4-byte length */
			c += 4;
		} else {
			/* Decode Error */
			return;
		}
	}
}

/*
 * Get a file size.
 */
size_t get_rfile_size(struct rfile *rf)
{
	long pos, len;

	/* If the rfile points to a real file. */
	if (!rf->is_packaged) {
		/* Return the file size. */
		pos = ftell(rf->fp);
		fseek(rf->fp, 0, SEEK_END);
		len = ftell(rf->fp);
		fseek(rf->fp, pos, SEEK_SET);
		return (size_t)len;
	}

	/* If the rfile points to a package entry. */
	return (size_t)rf->size;
}

/*
 * Read bytes from a read file stream.
 */
size_t read_rfile(struct rfile *rf, void *buf, size_t size)
{
	size_t len, obf;

	assert(rf != NULL);
	assert(rf->fp != NULL);

	if (!rf->is_packaged) {
		/*
		 * For the case the rfile points to a real file.
		 */

		/* Read. */
		len = fread(buf, 1, size, rf->fp);

		/* If the file is a save file, do obfuscation decode. */
		if (rf->is_obfuscated) {
			for (obf = 0; obf < len; obf++)
				*(((char *)buf) + obf) ^= get_next_random(&rf->next_random, NULL);
		}
	} else {
		/*
		 * For the case the rfile points to a package entry.
		 */

		/* Read. */
		if (rf->pos + size > rf->size)
			size = (size_t)(rf->size - rf->pos);
		if (size == 0)
			return 0;
		len = fread(buf, 1, size, rf->fp);
		rf->pos += len;

		/* Do obfuscation decode. */
		for (obf = 0; obf < len; obf++)
			*(((char *)buf) + obf) ^= get_next_random(&rf->next_random, &rf->prev_random);
	}

	return len;
}

/*
 * Read a line from a read file stream.
 */
const char *gets_rfile(struct rfile *rf, char *buf, size_t size)
{
	char *ptr;
	size_t len, ret;
	char c;

	assert(rf != NULL);
	assert(rf->fp != NULL);
	assert(buf != NULL);
	assert(size > 0);

	ptr = buf;
	for (len = 0; len < size - 1; len++) {
		ret = read_rfile(rf, &c, 1);
		if (ret != 1) {
			*ptr = '\0';
			return len == 0 ? NULL : buf;
		}
		if (c == '\n' || c == '\0') {
			*ptr = '\0';
			return buf;
		}
		if (c == '\r') {
			if (read_rfile(rf, &c, 1) != 1) {
				*ptr = '\0';
				return buf;
			}
			if (c == '\n') {
				*ptr = '\0';
				return buf;
			}
			ungetc_rfile(rf, c);
			*ptr = '\0';
			return buf;
		}
		*ptr++ = c;
	}
	*ptr = '\0';
	if (len == 0)
		return NULL;
	return buf;
}

/* Push back a character to a read file stream. */
static void ungetc_rfile(struct rfile *rf, char c)
{
	assert(rf != NULL);
	assert(rf->fp != NULL);

	if (!rf->is_packaged) {
		/* If a real file. */
		ungetc(c, rf->fp);
	} else {
		/* If a package entry. */
		assert(rf->pos != 0);
		ungetc(c, rf->fp);
		rf->pos--;
		rewind_random(&rf->next_random, &rf->prev_random);
	}
}

/*
 * Close a read file stream.
 */
void close_rfile(struct rfile *rf)
{
	assert(rf != NULL);
	assert(rf->fp != NULL);

	fclose(rf->fp);
	free(rf);
}

/*
 * Rewind a read file stream.
 */
void rewind_rfile(struct rfile *rf)
{
	assert(rf != NULL);
	assert(rf->fp != NULL);

	if (!rf->is_packaged) {
		/* If a real file. */
		rewind(rf->fp);
		rf->pos = 0;
		return;
	}

	/* If a package entry. */
	fseek(rf->fp, (long)rf->offset, SEEK_SET);
	rf->pos = 0;
	set_random_seed(rf->index, &rf->next_random);
	rf->prev_random = 0;
}

/* Set a random seed. */
static void set_random_seed(uint64_t index, uint64_t *next_random)
{
	uint64_t i, next, lsb;

	/* The key is shuffled so that decompilers cannot read it directly. */
	key_reversed = ((((key_obfuscated >> 56) & 0xff) << 0) |
			(((key_obfuscated >> 48) & 0xff) << 8) |
			(((key_obfuscated >> 40) & 0xff) << 16) |
			(((key_obfuscated >> 32) & 0xff) << 24) |
			(((key_obfuscated >> 24) & 0xff) << 32) |
			(((key_obfuscated >> 16) & 0xff) << 40) |
			(((key_obfuscated >> 8)  & 0xff) << 48) |
			(((key_obfuscated >> 0)  & 0xff) << 56));
	next = ~(*key_ref);
	for (i = 0; i < index; i++) {
		/* This XOR mask is not a secret. */
		next ^= NEXT_MASK1;
		lsb = next >> 63;
		next = (next << 1) | lsb;
	}

	*next_random = next;
}

/* Get a next random mask. */
static char get_next_random(uint64_t *next_random, uint64_t *prev_random)
{
	uint64_t next;
	char ret;

	/* For ungetc(). */
	if (prev_random != NULL)
		*prev_random = *next_random;

	ret = (char)(*next_random);
	next = *next_random;
	next = (((~(*key_ref) & 0xff00) * next + (~(*key_ref) & 0xff)) %
		~(*key_ref)) ^ NEXT_MASK2;
	*next_random = next;

	return ret;
}

/* Go back to the previous random mask. */
static void rewind_random(uint64_t *next_random, uint64_t *prev_random)
{
	*next_random = *prev_random;
	*prev_random = 0;
}

/*
 * Write
 */

/*
 * Open a write file stream.
 */
struct wfile *open_wfile(const char *dir, const char *file)
{
	char *path;
	struct wfile *wf;

	/* Allocate wfile struct. */
	wf = malloc(sizeof(struct wfile));
	if (wf == NULL) {
		log_memory();
		return NULL;
	}

	/* Make a real file path. */
	path = make_valid_path(dir, file);
	if (path == NULL) {
		log_memory();
		free(wf);
		return NULL;
	}

	/* Open a real file. */
#ifdef POLARIS_ENGINE_TARGET_WIN32
	_fmode = _O_BINARY;
	wf->fp = _wfopen(conv_utf8_to_utf16(path), L"w");
#else
	wf->fp = fopen(path, "w");
#endif
	if (wf->fp == NULL) {
		log_file_open(path);
		free(path);
		free(wf);
		return NULL;
	}
	free(path);

	/* Initialize the random seed. */
	set_random_seed(0, &wf->next_random);

	return wf;
}

/*
 * Write bytes to a write file stream.
 */
size_t write_wfile(struct wfile *wf, const void *buf, size_t size)
{
	char obf[1024];
	const char *src;
	size_t block_size, out, total, i;

	assert(wf != NULL);
	assert(wf->fp != NULL);

	src = buf;
	total = 0;
	while (size > 0) {
		/* Get the block size. (Max. 1024 bytes) */
		if (size > sizeof(obf))
			block_size = sizeof(obf);
		else
			block_size = size;

		/* Obfuscate the block. */
		for (i = 0; i < block_size; i++)
			obf[i] = *src++ ^ get_next_random(&wf->next_random, NULL);

		/* Write the block to the stream. */
		out = fwrite(obf, 1, block_size, wf->fp);
		if (out != block_size)
			return total + out;
		total += out;
		size -= out;
	}
	return total;
}

/*
 * Close a write file stream.
 */
void close_wfile(struct wfile *wf)
{
	assert(wf != NULL);
	assert(wf->fp != NULL);

	fflush(wf->fp);
	fclose(wf->fp);
	free(wf);
}

/*
 * Remove a real file.
 */
void remove_file(const char *dir, const char *file)
{
	char *path;

	/* Make a real path. */
	path = make_valid_path(dir, file);
	if (path == NULL) {
		log_memory();
		return;
	}

	/* Remove the file from the file system. */
	remove(path);

	/* Free the memory of the path. */
	free(path);
}
