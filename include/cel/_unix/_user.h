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
#ifndef __CEL_SYS_USER_UNIX_H__
#define __CEL_SYS_USER_UNIX_H__

#include "cel/types.h"
#include <pwd.h>
#include <grp.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//struct group
typedef struct _OsGroupInfo
{
    char *name;
    char *passwd;
    uid_t gid;
    char **members;
}OsGroupInfo;

//struct passwd
typedef struct _OsUserInfo
{
    char *name;
    char *passwd;
    uid_t uid;
    gid_t gid;
    char *gecos;
    char *dir;
    char *shell;
}OsUserInfo;

typedef int (* OsGroupEachFunc) (OsGroupInfo *grp, void *user_data);
typedef int (* OsUserEachFunc) (OsUserInfo *usr, void *user_data);


BOOL os_groupexists(TCHAR *groupname);
int os_groupadd(OsGroupInfo *group);
int os_groupadduser(TCHAR *groupname, TCHAR *username);
int os_groupdel(TCHAR *groupname);
int os_groupdeluser(TCHAR *groupname, TCHAR *username);
OsGroupInfo *os_getgroupinfo(TCHAR *groupname);
#define os_freegroupinfo(gi) cel_free(gi)
#define os_groupforeach(each_func, user_data) \
    os_usergroupforeach(NULL, each_func, user_data)
int os_groupuserforeach(TCHAR *groupname, 
                        OsUserEachFunc each_func, void *user_data);

BOOL os_userexists(TCHAR *username);
int os_useradd(OsUserInfo *user);
int os_userdel(TCHAR *username);
int os_userpswd(TCHAR *username, TCHAR *oldpassword, TCHAR *newpassword);
OsUserInfo *os_getuserinfo(TCHAR *username);
#define os_freeuserinfo(ui) cel_free(ui)
/* int os_userforeach(OsUserEachFunc each_func, void *user_data); */
#define os_userforeach(each_func, user_data) \
    os_groupuserforeach(NULL, each_func, user_data);
int os_usergroupforeach(TCHAR *username,
                        OsGroupEachFunc each_func, void *user_data);

#ifdef __cplusplus
}
#endif

#endif
