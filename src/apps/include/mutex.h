/*
 * Copyright (C) 2005 SerComm Corporation. All Rights Reserved.
 *
 * SerComm Corporation reserves the right to make changes to this document
 * without notice. SerComm Corporation makes no warranty, representation
 * or guarantee regarding the suitability of its products for any
 * particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm
 * Corporation specifically disclaims any and all liability, including
 * without limitation consequential or incidental damages; neither does
 * it convey any license under its patent rights, nor the rights of
 * others.
 */
#ifndef __SC_SEM_H_
#define __SC_SEM_H_
#include <sys/types.h>
#include <sys/sem.h>
enum{
	CREATE_UNLOCKED,
	CREATE_LOCKED
};
enum{
	NO_WAIT,
	WAIT_FOREVER
};
int scSemCreat(key_t keyid, int full);
int scSemDelete(int semid);
int scSemValid(int semid);
int scSemLock(int semid, int type);
int scSemUnlock(int semid);
#endif
