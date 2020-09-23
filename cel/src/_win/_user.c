/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/sys/user.h"
#ifdef _CEL_WIN
#include "cel/error.h"
#include "cel/convert.h"
#include "cel/net/sockaddr.h"
#include <lmaccess.h>
#include <Ntsecapi.h> /* LsaStorePrivateData */
#ifndef STATUS_SUCCESS 
#define STATUS_SUCCESS 0x00000000L
#endif

static DWORD UpdateDefaultPassword(WCHAR * pwszSecret)
{

    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaPolicyHandle = NULL;
    LSA_UNICODE_STRING lusSecretName;
    LSA_UNICODE_STRING lusSecretData;
    USHORT SecretNameLength;
    USHORT SecretDataLength;
    NTSTATUS ntsResult = STATUS_SUCCESS;
    DWORD dwRetCode = ERROR_SUCCESS;

    //  Object attributes are reserved, so initialize to zeros.
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
    //  Get a handle to the Policy object.
    ntsResult = LsaOpenPolicy(
        NULL,    // local machine
        &ObjectAttributes, 
        POLICY_CREATE_SECRET,
        &LsaPolicyHandle);
    if (STATUS_SUCCESS != ntsResult)
    {
        //  An error occurred. Display it as a win32 error code.
        dwRetCode = LsaNtStatusToWinError(ntsResult);
        wprintf(L"Failed call to LsaOpenPolicy %lu\n", dwRetCode);
        return dwRetCode;
    } 
    //  Initialize an LSA_UNICODE_STRING for the name of the
    //  private data ("DefaultPassword").
    SecretNameLength = (USHORT)wcslen(L"DefaultPassword");
    lusSecretName.Buffer = L"DefaultPassword";
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength =
        (SecretNameLength+1) * sizeof(WCHAR);
    //  If the pwszSecret parameter is NULL, then clear the secret.
    if (NULL == pwszSecret)
    {
        wprintf(L"Clearing the secret...\n");
        ntsResult = LsaStorePrivateData(
            LsaPolicyHandle,
            &lusSecretName,
            NULL);
        dwRetCode = LsaNtStatusToWinError(ntsResult);
    }
    else
    {
        wprintf(L"Setting the secret...\n");
        //  Initialize an LSA_UNICODE_STRING for the value
        //  of the private data. 
        SecretDataLength = (USHORT)wcslen(pwszSecret);
        lusSecretData.Buffer = pwszSecret;
        lusSecretData.Length = SecretDataLength * sizeof(WCHAR);
        lusSecretData.MaximumLength =
            (SecretDataLength+1) * sizeof(WCHAR);
        ntsResult = LsaStorePrivateData(
            LsaPolicyHandle,
            &lusSecretName,
            &lusSecretData);
        dwRetCode = LsaNtStatusToWinError(ntsResult);
    }
    LsaClose(LsaPolicyHandle);
    if (dwRetCode != ERROR_SUCCESS)
        wprintf(L"Failed call to LsaStorePrivateData %lu\n", dwRetCode);
    return dwRetCode;
}

int os_groupadduser(TCHAR *groupname, TCHAR *username)
{
    LOCALGROUP_MEMBERS_INFO_3 ugi;
#ifndef _UNICODE
    WCHAR lpszGroupName[CEL_GNLEN], lpszUserName[CEL_UNLEN];

    cel_mb2unicode(groupname, -1, lpszGroupName, CEL_GNLEN);
    cel_mb2unicode(username, -1, lpszUserName, CEL_UNLEN);
#else
    WCHAR *lpszGroupName = groupname, *lpszUserName = username;
#endif
    ugi.lgrmi3_domainandname = lpszUserName;

    return (NetLocalGroupAddMembers(
        cel_gethostname_w(), lpszGroupName, 3, (LPBYTE) &ugi, 1) 
        == NERR_Success ? 0 : -1);
}

int os_groupdel(TCHAR *groupname)
{
#ifndef _UNICODE
    WCHAR lpszGroupName[CEL_GNLEN];

    cel_mb2unicode(groupname, -1, lpszGroupName, CEL_GNLEN);
#else
    WCHAR *lpszGroupName = groupname;
#endif

    return (NetLocalGroupDel(cel_gethostname_w(), lpszGroupName) == NERR_Success ? 0 : -1);
}

int os_groupdeluser(TCHAR *groupname, TCHAR *username)
{
    LOCALGROUP_MEMBERS_INFO_3 ugi;
#ifndef _UNICODE
    WCHAR lpszGroupName[CEL_GNLEN], lpszUserName[CEL_UNLEN];

    cel_mb2unicode(groupname, -1, lpszGroupName, CEL_GNLEN);
    cel_mb2unicode(username, -1, lpszUserName, CEL_UNLEN);
#else
    LPWSTR lpszGroupName, lpszUserName;
    lpszGroupName = groupname;
    lpszUserName = username;
#endif

    return (NetLocalGroupDelMembers(
        cel_gethostname_w(), lpszGroupName, 3, (LPBYTE) &ugi, 1) == NERR_Success ? 0 : -1);
}

OsGroupInfo *cel_getgroupinfo(WCHAR *groupname)
{
    OsGroupInfo *gi;

    if (NetGroupGetInfo(cel_gethostname_w(), groupname, 1, (LPBYTE *)&gi) == NERR_Success
        && gi != NULL)
        return gi;
    //NetApiBufferFree(gi);
    return NULL;
}

int os_usergroupforeach(TCHAR *username, OsGroupEachFunc each_func, void *user_data)
{
    return 0;
}

BOOL os_userexists(TCHAR *username)
{
    USER_INFO_1 *ui;
#ifndef _UNICODE
    wchar_t UserName[UNLEN];
    MultiByteToWideChar(CP_ACP, 0, username, -1, UserName, UNLEN);
#else
    LPWSTR UserName = username;
#endif
    if (NetUserGetInfo(cel_gethostname_w(), UserName, 1, (LPBYTE *)&ui) == NERR_Success
        && ui != NULL)
    {
        NetApiBufferFree(ui);
        return TRUE;
    }
    return FALSE;
}

int os_useradd(OsUserInfo *ui)
{
    ui->priv = USER_PRIV_USER;
    ui->flags = UF_SCRIPT|UF_DONT_EXPIRE_PASSWD;
    if (NetUserAdd(cel_gethostname_w(), 1, (LPBYTE)ui, NULL) != NERR_Success)
        return -1;
    return 0;
}

int os_userdel(TCHAR *username)
{
#ifndef _UNICODE
    WCHAR lpszUserName[CEL_GNLEN];

    cel_mb2unicode(username, -1, lpszUserName, CEL_GNLEN);
#else
    LPWSTR lpszUserName = username;
#endif

    return (NetUserDel(cel_gethostname_w(), lpszUserName) == NERR_Success ? 0 : -1);
}

int os_userpswd(TCHAR *username, TCHAR *oldpassword, TCHAR *newpassword)
{
    DWORD dwRet;
    USER_INFO_1003 ui;
#ifndef _UNICODE
    WCHAR lpszUserName[CEL_UNLEN];
    WCHAR lpszOldPassword[CEL_PWLEN];
    WCHAR lpszNewPassword[CEL_PWLEN];

    cel_mb2unicode(username, -1, lpszUserName, CEL_UNLEN);
    if (oldpassword != NULL)
        cel_mb2unicode(oldpassword, -1, lpszOldPassword, CEL_PWLEN);
    else
        lpszOldPassword[0] = L'\0';
    cel_mb2unicode(newpassword, -1, lpszNewPassword, CEL_PWLEN);
#else
    LPWSTR lpszUserName, lpszOldPassword = NULL, lpszNewPassword;
    lpszUserName = username;
    lpszOldPassword = oldpassword;
    lpszNewPassword = newpassword;
#endif
    if (lpszOldPassword == NULL || lpszOldPassword[0] == L'\0')
    {
        ui.usri1003_password = lpszNewPassword;
        dwRet = NetUserSetInfo(cel_gethostname_w(), lpszUserName, 1003, (LPBYTE)&ui, NULL); 
    }
    else
    {
        dwRet = NetUserChangePassword(
            cel_gethostname_w(), lpszUserName, lpszOldPassword, lpszNewPassword);
    }
    if (dwRet != NERR_Success)
        CEL_SETERR((CEL_ERR_LIB, _T("Failed call to change user password, return %lu"), dwRet));
    return (dwRet == NERR_Success ? 0 : -1);
}

OsUserInfo *cel_getuserinfo(TCHAR *username)
{
    OsUserInfo *ui;  
#ifndef _UNICODE
    wchar_t UserName[CEL_UNLEN];

    cel_mb2unicode(username, -1, UserName, CEL_UNLEN);
#else
    LPWSTR UserName;
    UserName = username;
#endif
    if (NetUserGetInfo(cel_gethostname_w(), UserName, 1, (LPBYTE *)&ui) != NERR_Success
        && ui != NULL)
    {
        return ui;
    }
    return NULL;
}

BOOL os_getautologon(DWORD *dwAuto, char *username, DWORD *dwSize)
{
    LSTATUS ls;
    HKEY hkResult;
    char autoLogon[2];
    DWORD dataType, autoSize = sizeof(autoLogon);

    if ((ls = RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 
        0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, &hkResult)) == ERROR_SUCCESS)
    {
        if ((ls = RegQueryValueExA(hkResult, 
            "AutoAdminLogon", 0, &dataType, (LPBYTE)autoLogon, &autoSize)) == ERROR_SUCCESS)
        {
            *dwAuto = (autoLogon[0] == '1' ? 1 : 0);
            if ((ls = RegQueryValueExA(hkResult,
                "DefaultUserName", 0, &dataType, (LPBYTE)username, dwSize)) == ERROR_SUCCESS)
            {
                RegCloseKey(hkResult);
                //_tprintf(_T("###%ld\r\n"), *dwAuto);
                return TRUE;
            }
            *dwSize = 0;
        }
        RegCloseKey(hkResult);
    }
    //_tprintf(_T("%ld ls = %ld %s\r\n"), *dwAuto, ls, cel_geterrstr());
    return FALSE;
}

BOOL os_setautologon(DWORD dwAuto, const char *username, const char *password)
{
    LSTATUS ls;
    HKEY hkResult;
    char autoLogon[2];
    wchar_t password_w[PWLEN];

    autoLogon[0] = (dwAuto == 0 ? '0' : '1');
    autoLogon[1] = '\0';
    MultiByteToWideChar(CP_ACP, 0, password, -1, password_w, PWLEN);
    if ((ls = RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 
        0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, &hkResult)) == ERROR_SUCCESS)
    {
        if ((ls = RegSetValueExA(hkResult, "AutoAdminLogon",
            0, REG_SZ, (LPBYTE)&autoLogon, (DWORD)sizeof(autoLogon))) == ERROR_SUCCESS)
        {
            if ((ls = RegSetValueExA(hkResult, "DefaultUserName", 
                0, REG_SZ, (LPBYTE)username, (DWORD)strlen(username))) == ERROR_SUCCESS
                && UpdateDefaultPassword(password_w) == ERROR_SUCCESS)
            {
                RegCloseKey(hkResult);
                return TRUE;
            }
        }
        RegCloseKey(hkResult);
    }

    return FALSE;
}

int os_groupuserforeach(TCHAR *groupname, OsUserEachFunc each_func, void *user_data)
{
    NET_API_STATUS status;
    DWORD i, dwRead, dwTotal;
    DWORD_PTR dwResume = 0;
    LPVOID lpBuffer;
    PLOCALGROUP_MEMBERS_INFO_1 pMembersInfo;
    OsUserInfo *ui;

#ifndef _UNICODE
    wchar_t GroupName[CEL_GNLEN];

    cel_mb2unicode(groupname, -1, GroupName, CEL_GNLEN);
#else
    LPWSTR GroupName;
    GroupName = groupname;
#endif
    status = NetLocalGroupGetMembers(cel_gethostname_w(), GroupName, 
        1, (LPBYTE *)&lpBuffer, MAX_PREFERRED_LENGTH, &dwRead, &dwTotal, &dwResume);
    if (status != NERR_Success && status != ERROR_MORE_DATA) 
        return -1;
    pMembersInfo = (PLOCALGROUP_MEMBERS_INFO_1)lpBuffer;
    for (i = 0; i < dwRead; i++)
    {
        if (pMembersInfo[i].lgrmi1_sidusage != SidTypeUser) continue;
        /* Get user info */
        if (NetUserGetInfo(cel_gethostname_w(), 
            pMembersInfo[i].lgrmi1_name, 1, (LPBYTE *)&ui) != NERR_Success)
            break;
        /* Call each function */
        if (each_func(ui, user_data) != 0)
        {
            NetApiBufferFree(ui);
            break;
        }
        NetApiBufferFree(ui);
        //_tprintf(_T("%s\r\n"), pMembersInfo[i].lgrmi1_name);
    }
    NetApiBufferFree(lpBuffer);

    return 0;
}

#endif