#include "cel/error.h"

int error_test(int argc, TCHAR *argv[])
{
	int i = 1000;

	while (i-- > 0)
	{
		CEL_SETERR((CEL_ERR_USER, _T("1(%s)"), cel_geterrstr()));
		_tprintf(_T("%s\r\n"), cel_geterrstr());
		CEL_SETERR((CEL_ERR_USER, _T("2(%s)"), cel_geterrstr()));
		_tprintf(_T("%s\r\n"), cel_geterrstr());
		CEL_SETERR((CEL_ERR_USER, _T("3(%s)"), cel_geterrstr()));
		_tprintf(_T("%s\r\n"), cel_geterrstr());
		CEL_SETERR((CEL_ERR_USER, _T("4(%s)"), cel_geterrstr()));
		_tprintf(_T("%s\r\n"), cel_geterrstr());
		sleep(1);
	}

	return 0;
}