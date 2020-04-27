/*------------------------------------------------------------------------------
-- --
--       This software is confidential and proprietary and may be used --
--        only as expressly authorized by a licensing agreement from --
-- --
--                            Verisilicon. --
-- --
--                   (C) COPYRIGHT 2014 VERISILICON --
--                            ALL RIGHTS RESERVED --
-- --
--                 The entire notice above must be reproduced --
--                  on all copies and should not be removed. --
-- --
--------------------------------------------------------------------------------
--
--  Description : Locking semaphore for hardware sharing
--
------------------------------------------------------------------------------*/

#ifndef __EWL_LINUX_LOCK_H__
#define __EWL_LINUX_LOCK_H__

// Please select right lock for your platform
#ifndef ANDROID_EWL
#define USE_SYS_V_IPC
#else
#define USE_POSIX_SEMAPHORE
#endif
#include <semaphore.h>

/*SYSTEM V*/
#ifdef USE_SYS_V_IPC
#include <sys/types.h>
#include <sys/ipc.h>

int binary_semaphore_allocation(key_t key, int nsem, int sem_flags);
int binary_semaphore_deallocate(int semid);
int binary_semaphore_wait(int semid, int sem_num);
int binary_semaphore_post(int semid, int sem_num);
int binary_semaphore_initialize(int semid, int sem_num);
#endif

#endif /* __EWL_LINUX_LOCK_H__ */
