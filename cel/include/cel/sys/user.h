/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com) 
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#ifndef __CEL_SYS_USER_H__
#define __CEL_SYS_USER_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_user.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_user.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef OsGroupInfo CelGroupInfo;
typedef OsUserInfo CelUserInfo;

/* return 0 = continue;-1 = error;1 = break */
typedef OsGroupEachFunc CelGroupEachFunc;
typedef OsUserEachFunc CelUserEachFunc;

/* BOOL os_groupexists(TCHAR *groupname);*/
#define cel_groupexists os_groupexists
/* int cel_groupadd(OsGroupInfo *group); */
#define cel_groupadd os_groupadd
/* int cel_groupadduser(TCHAR *groupname, TCHAR *username); */
#define cel_groupadduser os_groupadduser
/* int cel_groupdel(TCHAR *groupname); */
#define cel_groupdel(groupname) os_groupdel
/* int cel_groupdeluser(TCHAR *groupname, TCHAR *username); */
#define cel_groupdeluser os_groupdeluser
/* OsGroupInfo *os_getgroupinfo(TCHAR *groupname); */
#define cel_getgroupinfo os_getgroupinfo
/* */
#define cel_freegroupinfo os_freegroupinfo
/* */
#define cel_groupforeach os_groupforeach
/* 
 * int cel_groupuserforeach(TCHAR *groupname, 
 *                          CelUserEachFunc each_func, void *user_data);
 */
#define cel_groupuserforeach os_groupuserforeach

/* BOOL os_userexists(TCHAR *username); */
#define cel_userexists os_userexists
/* int cel_useradd(OsUserInfo *user); */
#define cel_useradd os_useradd
/* int cel_userdel(TCHAR *username); */
#define cel_userdel os_userdel
/* 
 * int cel_userpswd(TCHAR *username, TCHAR *oldpassword, TCHAR *newpassword);
 */
#define cel_userpswd os_userpswd
/* OsUserInfo *os_getuserinfo(TCHAR *username); */
#define cel_getuserinfo os_getuserinfo
/* */
#define cel_freeuserinfo os_freeuserinfo
/* */
#define cel_getautologon os_getautologon
/* */
#define cel_setautologon os_setautologon
/* int cel_userforeach(OsUserEachFunc each_func, void *user_data); */
#define cel_userforeach os_userforeach
/* 
 * int cel_usergroupforeach(TCHAR *username, 
 *                          OsGroupEachFunc each_func, void *user_data);
 */
#define cel_usergroupforeach os_usergroupforeach

#ifdef __cplusplus
}
#endif

#endif
