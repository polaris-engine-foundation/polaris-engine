/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Polaris Engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

#ifndef POLARIS_ENGINE_FILE_H
#define POLARIS_ENGINE_FILE_H

#include "types.h"

/*
 * File System Abstraction
 *  - file.c, the common implementation of file.h, uses stdio library.
 *  - Following ports use separate implementations, not file.c:
 *    - engine-android
 *    - engine-unity
 *    - engine-wasm
 */

/*
 * [Archive File Design]
 *
 * struct header {
 *     u64 file_count;
 *     struct file_entry {
 *         u8  file_name[256]; // Obfuscated
 *         u64 file_size;
 *         u64 file_offset;
 *     } [file_count];
 * };
 * u8 file_body[file_count][file_length]; // Obfuscated
 */

/* Package file name. */
#define PACKAGE_FILE		"data01.arc"

/* Maximum entries in a package. */
#define FILE_ENTRY_SIZE		(65536)

/* File name length for an entry. */
#define FILE_NAME_SIZE		(256)

/*
 * File entry.
 */
struct file_entry {
	/* File name. */
	char name[FILE_NAME_SIZE];

	/* File size. */
	uint64_t size;

	/* Offset in the package file. */
	uint64_t offset;
};

/* File read stream. */
struct rfile;

/* File write stream. */
struct wfile;

/* Initialize the file subsystem. */
bool init_file(void);

/* Cleanup the file subsystem. */
void cleanup_file(void);

/* Check whether file exists. */
bool check_file_exist(const char *dir, const char *file);

/* Open file read stream. */
struct rfile *open_rfile(const char *dir, const char *file, bool save_data);

/* Get a file size. */
size_t get_rfile_size(struct rfile *rf);

/* Read from a file stream. */
size_t read_rfile(struct rfile *rf, void *buf, size_t size);

/* Read a line from a file stream. */
const char *gets_rfile(struct rfile *rf, char *buf, size_t size);

/* Close a file stream. */
void close_rfile(struct rfile *rf);

/* Go back to the top of a file stream. */
void rewind_rfile(struct rfile *rf);

/* Open a write file stream. */
struct wfile *open_wfile(const char *dir, const char *file);

/* Write to a file stream. */
size_t write_wfile(struct wfile *wf, const void *buf, size_t size);

/* Close a write file stream. */
void close_wfile(struct wfile *wf);

/* Remove a file. */
void remove_file(const char *dir, const char *file);

#endif
