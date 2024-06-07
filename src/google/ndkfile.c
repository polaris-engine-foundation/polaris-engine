/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * x-engine
 * Copyright (C) 2024, The Authors. All rights reserved.
 */

/*
 * Android App Assets and Save Data Access
 *  - This file is used by engine-android, but not used by pro-android.
 */

/* Base */
#include "xengine.h"

/* HAL */
#include "ndkmain.h"

/* パスの長さ */
#define PATH_SIZE	(1024)

/*
 * 前方参照
 */
static void ungetc_rfile(struct rfile *rf, char c);

/*
 * 初期化
 */

/*
 * ファイル読み込みの初期化処理を行う
 */
bool init_file(void)
{
	return true;
}

/*
 * ファイル読み込みの終了処理を行う
 */
void cleanup_file(void)
{
}

/*
 * 読み込み
 */

/*
 * ファイルがあるか調べる
 */
bool check_file_exist(const char *dir, const char *file)
{
	char path[PATH_SIZE];
	struct rfile *rf;
	jclass cls;
	jmethodID mid;
	jboolean ret;

	/* パスを生成する */
	snprintf(path, sizeof(path), "%s/%s", dir, file);

	/* ファイルの内容を取得する */
	cls = (*jni_env)->FindClass(jni_env, "com/xengine/engineandroid/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeCheckFileExists", "(Ljava/lang/String;)Z");
	ret = (*jni_env)->CallBooleanMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, path));
	if (ret) {
		/* ファイルが存在する */
		return true;
	}

	/* ファイルが存在しない */
	return false;
}

/*
 * ファイル読み込みストリームを開く
 */
struct rfile *open_rfile(const char *dir, const char *file, bool save_data)
{
	char path[PATH_SIZE];
	struct rfile *rf;
	jclass cls;
	jmethodID mid;
	jobject ret;

	/* パスを生成する */
	snprintf(path, sizeof(path), "%s/%s", dir, file);

	/* ファイルの内容を取得する */
	cls = (*jni_env)->FindClass(jni_env, "com/xengine/engineandroid/MainActivity");
	mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeGetFileContent", "(Ljava/lang/String;)[B");
	ret = (*jni_env)->CallObjectMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, path));
	if (ret == NULL) {
		if (!save_data)
			log_dir_file_open(dir, file);
		return NULL;
	}

	/* rfile構造体のメモリを確保する */
	rf = malloc(sizeof(struct rfile));
	if (rf == NULL) {
		log_memory();
		return NULL;
	}

	/* rfile構造体の中身をセットする */
	rf->array = (jbyteArray)(*jni_env)->NewGlobalRef(jni_env, ret);
	rf->buf = (char *)(*jni_env)->GetByteArrayElements(jni_env, rf->array, JNI_FALSE);
	rf->size = (uint64_t)(*jni_env)->GetArrayLength(jni_env, rf->array);
	rf->pos = 0;

	return rf;
}

/*
 * ファイルのサイズを取得する
 */
size_t get_rfile_size(struct rfile *rf)
{
	return (size_t)rf->size;
}

/*
 * ファイル読み込みストリームから読み込む
 */
size_t read_rfile(struct rfile *rf, void *buf, size_t size)
{
	if (rf->pos + size > rf->size)
		size = (size_t)(rf->size - rf->pos);
	if (size == 0)
		return 0;
	memcpy(buf, rf->buf + rf->pos, size);
	rf->pos += size;
	return size;
}

/* ファイル読み込みストリームに1文字戻す */
static void ungetc_rfile(struct rfile *rf, char c)
{
	/* HINT: cを戻していない */
	assert(rf->pos != 0);
	rf->pos--;
}

/*
 * ファイル読み込みストリームから1行読み込む
 */
const char *gets_rfile(struct rfile *rf, char *buf, size_t size)
{
	char *ptr;
	size_t len;
	char c;

	ptr = (char *)buf;

	for (len = 0; len < size - 1; len++) {
		if (read_rfile(rf, &c, 1) != 1) {
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
	return buf;
}

/*
 * ファイル読み込みストリームを先頭まで巻き戻す
 */
void rewind_rfile(struct rfile *rf)
{
	rf->pos = 0;
}

/*
 * ファイル読み込みストリームを閉じる
 */
void close_rfile(struct rfile *rf)
{
#ifndef XENGINE_TARGET_ANDROID
	(*jni_env)->ReleaseByteArrayElements(jni_env, rf->array, (jbyte *)rf->buf, JNI_ABORT);
	(*jni_env)->DeleteGlobalRef(jni_env, rf[i]->array);
	free(delayed_rfile_free_slot);
#else
	/*
	 * AndroidでOpenSLのコールバックから呼ばれる際、
	 * jni_env == NULL であるため、参照の削除を遅延させて、
	 * メインスレッドで処理させる
	 */
	if (jni_env != NULL) {
		/* メインスレッドである */
		(*jni_env)->ReleaseByteArrayElements(jni_env, rf->array, (jbyte *)rf->buf, JNI_ABORT);
		(*jni_env)->DeleteGlobalRef(jni_env, rf->array);
		free(rf);
	} else {
		/* サウンドスレッドである */
		post_delayed_remove_rfile_ref(rf);
	}
#endif
}

/*
 * 書き込み
 */

/*
 * ファイル書き込みストリームを開く
 */
struct wfile *open_wfile(const char *dir, const char *file)
{
	assert(strcmp(dir, SAVE_DIR) == 0);

	/* wfile構造体のメモリを確保する */
	struct wfile *wf = malloc(sizeof(struct wfile));
	if (wf == NULL) {
		log_memory();
		return NULL;
	}

	/* ファイルをオープンする */
	jclass cls = (*jni_env)->FindClass(jni_env, "com/xengine/engineandroid/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeOpenSaveFile", "(Ljava/lang/String;)Ljava/io/OutputStream;");
	jobject ret = (*jni_env)->CallObjectMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, file));
	if (ret == NULL) {
		log_dir_file_open(dir, file);
		free(wf);
		return NULL;
	}
	wf->os = (*jni_env)->NewGlobalRef(jni_env, ret);

	return wf;
}

/*
 * ファイル書き込みストリームに書き込む
 */
size_t write_wfile(struct wfile *wf, const void *buf, size_t size)
{
	jclass cls = (*jni_env)->FindClass(jni_env, "com/xengine/engineandroid/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeWriteSaveFile", "(Ljava/io/OutputStream;I)Z");

	size_t i;
	for (i = 0; i < size; i++) {
		int c = (unsigned char)*((char *)buf + i);
		jboolean ret = (*jni_env)->CallBooleanMethod(jni_env, main_activity, mid, wf->os, c);
		if (ret != JNI_TRUE)
			return i;
	}

	/* 解放しないとlocal reference tableが溢れる */
	(*jni_env)->DeleteLocalRef(jni_env, cls);

	return i;
}

/*
 * ファイル読み込みストリームを閉じる
 */
void close_wfile(struct wfile *wf)
{
	jclass cls = (*jni_env)->FindClass(jni_env, "com/xengine/engineandroid/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeCloseSaveFile", "(Ljava/io/OutputStream;)V");
	(*jni_env)->CallVoidMethod(jni_env, main_activity, mid, wf->os);
	(*jni_env)->DeleteGlobalRef(jni_env, wf->os);
	free(wf);
}

/*
 * ファイルを削除する
 */
void remove_file(const char *dir, const char *file)
{
	jclass cls = (*jni_env)->FindClass(jni_env, "com/xengine/engineandroid/MainActivity");
	jmethodID mid = (*jni_env)->GetMethodID(jni_env, cls, "bridgeRemoveSaveFile", "(Ljava/lang/String;)V;");
	(*jni_env)->CallObjectMethod(jni_env, main_activity, mid, (*jni_env)->NewStringUTF(jni_env, file));
}
