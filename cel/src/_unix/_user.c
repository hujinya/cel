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
//#ifndef _BSD_SOURCE
//#define _BSD_SOURCE
//#endif

/* crypt */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* fgetgrent_r fgetpwrent_r */
#ifndef _SVID_SOURCE 
#define _SVID_SOURCE
#endif

//#define _XOPEN_SOURCE    500 /* or any value > 500 */

#include "cel/sys/user.h"
#ifdef _CEL_UNIX
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/file.h"
#include "cel/datetime.h"

/* Group file */
#define GROUP           "/etc/group"
#define GROUP_SWAP      "/etc/.group.swap"
#define GROUP_EACH      "/etc/.group.each"
#define GSHADOW         "/etc/gshadow"
#define GSHADOW_SWAP    "/etc/.gshadow.swap"
#define GSHADOW_EACH    "/etc/.gshadow.each"
/* User file */
#define PASSWD          "/etc/passwd"
#define PASSWD_SWAP     "/etc/.passwd.swap"
#define PASSWD_EACH     "/etc/.passwd.each"
#define SHADOW          "/etc/shadow"
#define SHADOW_SWAP     "/etc/.shadow.swap"
#define SHADOW_EACH     "/etc/.shadow.each"
/* Home dir */
#define HOMEDIR         "/home"

#define GRPINFO_BUF_SIZE 4096
#define USRINFO_BUF_SIZE 4096

static char *cel_makecryptsalt(void)
{
    struct timeval tv;
    static char result[40];

    result[0] = '\0';
    /*
     * Generate 8 chars of salt, the old crypt()will use only first 2.
     */
    gettimeofday(&tv, (struct timezone *)0);
    strcat(result, "$6$");
    strcat(result, l64a(tv.tv_usec));
    strcat(result, l64a(tv.tv_sec + getpid ()+ clock ()));
    
    if (strlen (result) > 3 + 3 + 8)    /* magic+salt */
        result[11] = '\0';
    strcat(result, "$");

    return result;
}

BOOL os_groupexists(TCHAR *groupname)
{
    FILE *fp;
    size_t len;
    char buf[CEL_LINELEN];

    if ((fp = cel_fopen(GROUP, "r")) == NULL) 
        return -1;
    len = strlen(groupname);
    while (fgets(buf, CEL_LINELEN, fp) != NULL)
    {
        /* Check group exists */
        if (memcmp(groupname, buf, len) == 0 && buf[len] == ':')
        {
            cel_fclose(fp);
            return TRUE;
        }
    }
    cel_fclose(fp);
    return FALSE;
}

int os_groupadd(OsGroupInfo *group)
{
    FILE *fp;
    size_t len;
    int i, gid1, gid2 = 2000;
    char buf[CEL_LINELEN];

    if (group->name == NULL 
        || (len = strlen(group->name)) > CEL_GNLEN || len == 0)
    {
        CEL_SETERR((CEL_ERR_LIB,  "Group name \"%s\" invalid.", group->name));
        return -1;
    }
    cel_fsync(GROUP_SWAP, GROUP);
    if ((fp = cel_fopen(GROUP, "a")) == NULL) 
        return -1;
    gid1 = 0;
    while (fgets(buf, CEL_LINELEN, fp) != NULL)
    {
        /* Check group exists */
        if (memcmp(group->name, buf, len) == 0
            && buf[len] == ':')
        {
            cel_fclose(fp);
            CEL_SETERR((CEL_ERR_LIB,  "Group \"%s\" is already exists.", group->name));
            return -1;
        }
        /* Get new gid */
        i = 0;
        while (buf[i] != '\0')
        {
            if (buf[i] == ':'
                && buf[++i] == 'x'
                && buf[++i] ==  ':')
            {
                if (buf[++i] != '\0' 
                    && (gid1 = (gid_t)atoi(&buf[i])) == gid2)
                    gid2 = (gid1 + 1);
                break;
            }
            i++;
        }
    }
    if (fprintf(fp, "%s:x:%d:\n", group->name, gid2) == -1)
    {
        cel_fclose(fp);
        cel_fsync(GROUP, GROUP_SWAP);
        CEL_SETERR((CEL_ERR_LIB,  "Write group file \"%s\" failed", GROUP));
        return -1;
    }
    cel_fclose(fp);

    return gid2;
}

int os_groupdel(char *groupname)
{
    int c;
    size_t i, len;
    FILE *fp, *fp_;
    char buf[CEL_GNLEN + 1];

    if (groupname == NULL 
        || (len = strlen(groupname)) > CEL_GNLEN || len == 0)
    {
        CEL_SETERR((CEL_ERR_LIB,  "Group name \"%s\" invalid.", groupname));
        return -1;
    }
    /* Delete group info */ 
    cel_fsync(GROUP_SWAP, GROUP);
    if ((fp_ = cel_fopen(GROUP_SWAP, "r")) == NULL 
        || (fp = cel_fopen(GROUP, "w")) == NULL)
    {
        if (fp_ != NULL) cel_fclose(fp_);
        return -1;
    }
    while (!feof(fp_))
    {
        i = 0;
        /* Get group name */
        while (i < CEL_GNLEN && (c = fgetc(fp_)) != EOF)
        {
            buf[i++] = (char)c;
            if (c == ':') 
                break;
        }
        /* Compare the group name */
        if (len == (i - 1) 
            && strncmp(groupname, buf, len) == 0
            && strncmp("root", buf, 4) != 0)
        {
            while ((c = fgetc(fp_)) != EOF && c != '\n');
        }
        else
        {
            fwrite(buf, i, 1, fp);
            while ((c = fgetc(fp_)) != EOF)
            {
                fputc(c, fp);
                if (c == '\n') break;
            }
        }
    }
    cel_fclose(fp);
    cel_fclose(fp_);

    return 0;
}

int os_groupadduser(char *groupname, char *username)
{
    int c, exists = 0;
    size_t i, glen, ulen;
    FILE *fp, *fp_;
    char buf[CEL_GNLEN + 1];

    if (groupname == NULL || username == NULL
        || (glen = strlen(groupname)) > CEL_GNLEN || glen == 0
        || (ulen = strlen(username)) > CEL_UNLEN || ulen == 0
        || getgrnam(groupname) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB,  "Name is null or group \"%s\" not exists", groupname));
        return -1;
    }
    
    cel_fsync(GROUP_SWAP, GROUP);
    if ((fp_ = cel_fopen(GROUP_SWAP, "r")) == NULL 
        || (fp = cel_fopen(GROUP, "w")) == NULL)
    {
        if (fp_ != NULL) cel_fclose(fp_);
        return -1;
    }
    while (!feof(fp_))
    {
        i = 0;
        /* Get group name */
        while (i < CEL_GNLEN && (c = fgetc(fp_)) != EOF)
        {
            buf[i++] = (char) c;
            if (c == ':')break;
        }
        fwrite(buf, i, 1, fp);
        /* Compare the group name */
        if (glen == (i - 1) && strncmp(groupname, buf, glen) == 0)
        {
            i = 0;
            while ((c = fgetc(fp_)) != EOF)
            {
                if (c == ':' || c == ',' || c == '\n')
                {
                    fwrite(buf, i, 1, fp);
                    /* Check user already exists in group*/
                    if (c != ':' && ulen == (i - 1)
                        && strncmp(username, buf + 1, ulen) == 0)
                    {
                        cel_seterr(CEL_ERR_LIB, "User \"%s\" already exists in group \"%s\"", 
                            username, groupname);
                        exists = 1;
                    }
                    /* Add user to group list */
                    if (c == '\n')
                    {
                        if (exists == 0)
                        {
                            if (i > 1 && buf[i - 1] != ':')
                                fputc(',', fp);
                            fwrite(username, ulen, 1, fp);
                        }
                        fputc(c, fp);
                        break;
                    }
                    i = 0;
                }
                buf[i++] = (char) c;
            }
        }
        else
        {
            while ((c = fgetc(fp_)) != EOF)
            {
                fputc(c, fp);
                if (c == '\n')break;
            }
        }
    }
    cel_fclose(fp);
    cel_fclose(fp_);

    return 0;
}

int os_groupdeluser(char *groupname, char *username)
{
    int c;
    size_t i, glen, ulen;
    FILE *fp, *fp_;
    char buf[CEL_GNLEN + 1];

    if (groupname == NULL || username == NULL
        || (glen = strlen(groupname)) > CEL_GNLEN || glen == 0
        || (ulen = strlen(username)) > CEL_UNLEN || ulen == 0
        || getgrnam(groupname) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB,  "Name is null or group \"%s\" not exists", groupname));
        return -1;
    }
    cel_fsync(GROUP_SWAP, GROUP);
    if ((fp_ = cel_fopen(GROUP_SWAP, "r")) == NULL 
        || (fp = cel_fopen(GROUP, "w")) == NULL)
    {
        if (fp_ != NULL) cel_fclose(fp_);
        return -1;
    }
    while (!feof(fp_))
    {
        i = 0;
        /* Get group name */
        while (i < CEL_GNLEN && (c = fgetc(fp_)) != EOF)
        {
            buf[i++] = (char) c;
            if (c == ':')break;
        }
        fwrite(buf, i, 1, fp);
        /* Compare the group name */
        if (glen == (i - 1) && strncmp(groupname, buf, glen) == 0)
        {
            i = 0;
            /* Delete user from group list */
            while ((c = fgetc(fp_)) != EOF)
            {
                if (c == ':')
                {
                    fwrite(buf, i, 1, fp);
                    i = 0;
                }
                else if (c == ',' || c == '\n')
                {
                    if (ulen == (i - 1) && strncmp(username, buf + 1, ulen) == 0)
                    {
                        if (buf[0] == ':')fputc(buf[0], fp);
                        if (c == ',')
                        { 
                            i = 0; continue; 
                        }
                    }
                    else
                    {
                        fwrite(buf, i, 1, fp);
                    }
                    i = 0;
                    if (c == '\n'){ fputc(c, fp); break; }
                }
                buf[i++] = (char) c;
            }
        }
        else
        {
            while ((c = fgetc(fp_)) != EOF)
            {
                fputc(c, fp);
                if (c == '\n')break;
            }
        }
    }
    cel_fclose(fp);
    cel_fclose(fp_);

    return 0;
}

OsGroupInfo *cel_getgroupinfo(TCHAR *groupname)
{
    OsGroupInfo *gi1, *gi2;

    if ((gi1 = (OsGroupInfo *)cel_malloc(GRPINFO_BUF_SIZE)) != NULL)
    {
        if (getgrnam_r(groupname, (struct group *)gi1, 
            (char *)(gi1 + 1), GRPINFO_BUF_SIZE - sizeof(struct group),
            (struct group **)&gi2) == 0
            && gi2 != NULL)
            return gi2;
        cel_free(gi1);
    }
    return NULL;
}

static int os_useradd_time(void)
{
    CelDateTime dt1, dt2;

    cel_datetime_init_value(&dt1, 1970, 0, 0, 0, 0, 0);
    cel_datetime_init_now(&dt2);

    return (int)(cel_datetime_diffseconds(&dt2, &dt1) / (60 * 60 * 24));
}

BOOL os_userexists(TCHAR *username)
{
    FILE *fp;
    size_t len;
    char buf[CEL_LINELEN];

    if ((fp = cel_fopen(PASSWD, "r")) == NULL) 
        return -1;
    len = strlen(username);
    while (fgets(buf, CEL_LINELEN, fp) != NULL)
    {
        /* Check user exists */
        if (memcmp(username, buf, len) == 0 && buf[len] == ':')
        {
            cel_fclose(fp);
            return TRUE;
        }
    }
    cel_fclose(fp);
    return FALSE;
}

int os_useradd(OsUserInfo *user)
{
    FILE *fp;
    size_t i, len;
    uid_t uid;
    char buf[CEL_LINELEN];

    if (user->name == NULL 
        || (len = strlen(user->name)) > CEL_UNLEN || len == 0)
    {
        CEL_SETERR((CEL_ERR_LIB,  "User name \"%s\" is null or user already exists",
            user->name));
        return -1;
    }
    if (user->passwd == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB,  "User \"%s\" password is null", user->name));
        return -1;
    }
    if (getgrgid(user->gid) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB,  "Group id \"%d\" is not exists.",user->gid));
        return -1;
    }
    /* Write passwd file */
    cel_fsync(PASSWD_SWAP, PASSWD);
    if ((fp = cel_fopen(PASSWD, "r+")) == NULL) 
        return -1;
    user->uid = 2000;
    while (fgets(buf, CEL_LINELEN, fp) != NULL)
    {
        /* Check user exists */
        if (memcmp(user->name, buf, len) == 0
            && buf[len] == ':')
        {
            cel_fclose(fp);
            CEL_SETERR((CEL_ERR_LIB,  "User \"%s\" is already exists.", user->name));
            return -1;
        }
        /* Get new uid */
        i = 0;
        while (buf[i] != '\0')
        {
            if (buf[i] == ':'
                && buf[++i] == 'x'
                && buf[++i] == ':')
            {
                if (buf[++i] != '\0' 
                    && (uid = (uid_t)atoi(&buf[i])) == user->uid)
                    user->uid = (uid + 1);
                /*printf("%s uid = %d, user uid = %d \r\n", 
                    &buf[i], uid, user->uid);*/
                break;
            }
            i++;
        }
    }
    if (fprintf(fp, "%s:x:%d:%d:%s:/home/%s:%s\n", 
        user->name, user->uid, user->gid,
        user->name, user->name, user->shell) == -1)
    {
        cel_fclose(fp);
        cel_fsync(PASSWD, PASSWD_SWAP);
        CEL_SETERR((CEL_ERR_LIB,  "Write passwd file \"%s\" failed", PASSWD));
        return -1;
    }
    cel_fclose(fp);
    /* Write shadow file */
    cel_fsync(SHADOW_SWAP, SHADOW);
    if ((fp = cel_fopen(SHADOW, "a")) == NULL)
    {
        cel_fsync(PASSWD, PASSWD_SWAP);
        return -1;
    }
    //cel_datetime_init_now(&dt);
    if (fprintf(fp, "%s:%s:%d:0:99999:7:::\n", 
        user->name, crypt(user->passwd, cel_makecryptsalt()), 
        os_useradd_time()) == -1)
    {
        cel_fclose(fp);
        cel_fsync(PASSWD, PASSWD_SWAP);
        cel_fsync(SHADOW, SHADOW_SWAP);
        CEL_SETERR((CEL_ERR_LIB,  "Write shadow file \"%s\" failed", SHADOW));
        return -1;
    }
    cel_fclose(fp);
    /* Create home dir */
    if (user->dir != NULL)
    {
        if (cel_mkdir_a(user->dir, S_ISUID|S_ISGID|S_IRWXU|S_IRWXG) == -1 
            && errno != EEXIST)
        {
            cel_fsync(PASSWD, PASSWD_SWAP);
            cel_fsync(SHADOW, SHADOW_SWAP);
            CEL_SETERR((CEL_ERR_LIB,  "Create home dir \"%s\" failed", user->dir));
            return -1;
        }
        if (chown(user->dir, user->uid, user->gid) == -1)
        {
            CEL_SETERR((CEL_ERR_LIB,  "Add user \"%s\" successed, but chown home dir failed", 
                user->name));
            return -1;
        }
    }
    //_tprintf("step5 %ld\r\n", cel_getticks());

    return 0;
}

int os_userdel(char *username)
{
    int c;
    size_t i, len;
    FILE *fp, *fp_;
    char buf[CEL_PATHLEN];

    if (username == NULL
        || (len = strlen(username)) > CEL_UNLEN || len == 0)
    {
        CEL_SETERR((CEL_ERR_LIB,  "User name \"%s\" is null or user not exists", username));
        return -1;
    }
    /* Delete password info */ 
    cel_fsync(PASSWD_SWAP, PASSWD);
    if ((fp_ = cel_fopen(PASSWD_SWAP, "r")) == NULL 
        || (fp = cel_fopen(PASSWD, "w")) == NULL)
    {
        if (fp_ != NULL)
            cel_fclose(fp_);
        return -1;
    }
    while (!feof(fp_))
    {
        i = 0;
        /* Get user name */
        while (i < CEL_UNLEN && (c = fgetc(fp_)) != EOF)
        {
            buf[i++] = (char) c;
            if (c == ':') 
                break;
        }
        /* Compare the user name */
        if (len == (i - 1)
            && strncmp(username, buf, len) == 0
            && strncmp("root", buf, 4) != 0)
        {
            while ((c = fgetc(fp_)) != EOF && c != '\n');
        }
        else
        {
            fwrite(buf, i, 1, fp);
            while ((c = fgetc(fp_)) != EOF)
            {
                fputc(c, fp);
                if (c == '\n') break;
            }
        }
    }
    cel_fclose(fp);
    cel_fclose(fp_);
    /* Delete shadow info */ 
    cel_fsync(SHADOW_SWAP, SHADOW);
    if ((fp_ = cel_fopen(SHADOW_SWAP, "r")) == NULL 
        || (fp = cel_fopen(SHADOW, "w")) == NULL)
    {
        if (fp_ != NULL) 
            cel_fclose(fp_);
        return -1;
    }
    while (!feof(fp_))
    {
        i = 0;
        /* Get user name */
        while (i < CEL_UNLEN && (c = fgetc(fp_)) != EOF)
        {
            buf[i++] = (char) c;
            if (c == ':')break;
        }
        /* Compare the user name */
        if (len == (i - 1) && strncmp(username, buf, len) == 0)
        {
            while ((c = fgetc(fp_)) != EOF && c != '\n');
        }
        else
        {
            fwrite(buf, i, 1, fp);
            while ((c = fgetc(fp_)) != EOF)
            {
                fputc(c, fp);
                if (c == '\n')break;
            }
        }
    }
    cel_fclose(fp);
    cel_fclose(fp_);
    /* Remove home dir */
    len = snprintf(buf, CEL_PATHLEN, HOMEDIR"/%s", username);
    cel_rmdirs(buf);

    return 0;
}

int os_userpswd(char *username, char *oldpassword, char *newpassword)
{
    int c;
    size_t i, len;
    FILE *fp, *fp_;
    char buf[CEL_UNLEN];

    if (username == NULL 
        || (len = strlen(username)) > CEL_UNLEN || len == 0
        || newpassword == NULL 
        || (len = strlen(newpassword)) > CEL_PWLEN || len == 0
        || getpwnam(username) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB,  "User name \"%s\" is null or user not exists", username));
        return -1;
    }
    cel_fsync(SHADOW_SWAP, SHADOW);
    if ((fp_ = cel_fopen(SHADOW_SWAP, "r")) == NULL 
        || (fp = cel_fopen(SHADOW, "w")) == NULL)
    {
        if (fp_ != NULL)
            cel_fclose(fp_);
        return -1;
    }
    while (!feof(fp_))
    {
        i = 0;
        /* Get user name */
        while (i < CEL_UNLEN && (c = fgetc(fp_)) != EOF)
        {
            buf[i++] = (char) c;
            if (c == ':') break;
        }
        /* Compare the user name */
        if (len == (i - 1) 
            && strncmp(username, buf, len) == 0
            && strncmp("root", buf, 4) != 0)
        {
            while ((c = fgetc(fp_)) != EOF && c != '\n');
            fprintf(fp, "%s:%s:%d:0:99999:7:::\n", 
                username, crypt(newpassword, cel_makecryptsalt()),
                os_useradd_time());
        }
        else
        {
            fwrite(buf, i, 1, fp);
            while ((c = fgetc(fp_)) != EOF)
            {
                fputc(c, fp);
                if (c == '\n')break;
            }
        }
    }
    cel_fclose(fp);
    cel_fclose(fp_);

    return 0;
}

OsUserInfo *cel_getuserinfo(TCHAR *username)
{
    OsUserInfo *ui1, *ui2;

    if ((ui1 = (OsUserInfo *)cel_malloc(USRINFO_BUF_SIZE)) != NULL)
    {
        if (getpwnam_r(username, (struct passwd *)ui1, 
            (char *)(ui1 + 1), USRINFO_BUF_SIZE - sizeof(struct passwd), 
            (struct passwd **)&ui2) == 0
            && ui2 != NULL)
            return ui2;
        cel_free(ui1);
    }
    return NULL;
}

int os_groupuserforeach(TCHAR *groupname, 
                        OsUserEachFunc each_func, void *user_data)
{
    int ret;
    FILE *fp;
    OsGroupInfo *gi;
    OsUserInfo *ui1, *ui2 = NULL;

    if (groupname == NULL 
        || (gi = (OsGroupInfo *)cel_getgroupinfo(groupname)) != NULL)
    {
        if ((ui1 = (OsUserInfo *)cel_malloc(USRINFO_BUF_SIZE)) != NULL)
        {
            if (cel_fsync(PASSWD_EACH, PASSWD) != -1
                && (fp = fopen(PASSWD_EACH, "r")) != NULL)
            {
                while ((ret = fgetpwent_r(fp,
                    (struct passwd *)ui1, (char *)(ui1 + 1), 
                    USRINFO_BUF_SIZE - sizeof(OsUserInfo), 
                    (struct passwd **)&ui2)) == 0
                    && ui2 != NULL)
                {
                    //puts(ui2->name);
                    if (groupname != NULL && ui2->gid != gi->gid)
                        continue;
                    if ((ret = each_func(ui2, user_data)) != 0)
                        break;
                }
                os_freeuserinfo(gi);
                cel_free(ui1);
                fclose(fp);
                return ret;
            }
            cel_free(ui1);
        }
        os_freeuserinfo(gi);
    }

    return -1;
}

int os_usergroupforeach(TCHAR *username,
                        OsGroupEachFunc each_func, void *user_data)
{
    int ret = 0;
    FILE *fp;
    OsGroupInfo *gi1, *gi2 = NULL;

    if ((gi1 = (OsGroupInfo *)cel_malloc(GRPINFO_BUF_SIZE)) != NULL)
    {
        if (cel_fsync(GROUP_EACH, GROUP) != -1
            && (fp = fopen(GROUP_EACH, "r")) != NULL)
        {
            while (fgetgrent_r(fp, (struct group *)gi1, (char *)gi1 + 1, 
                GRPINFO_BUF_SIZE - sizeof(OsGroupInfo), 
                (struct group **)&gi2) == 0
                && gi2 != NULL)
            {
                if ((ret = each_func(gi2, user_data)) != 0)
                    break;
            }
            os_freegroupinfo(gi1);
            fclose(fp);
            return ret;
        }
        cel_free(gi1);
    }
    return -1;
}

#endif
