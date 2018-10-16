#include "cel/_command.h"

CEL_CLI(test1_func, test1_element, _T("test1 argv1 argv2"), _T("Test1 help string!"))
{
    cel_clisession_out(cli_si, _T("Hello, cli test1!\r\n"));

    return 0;
}

CEL_CLI(test2_func, test2_element, _T("test1 argv1 argv01"), _T("Test2 help string!"))
{
    cel_clisession_out(cli_si, _T("Hello, cli test2!\r\n"));

    return 0;
}

CEL_CLI(test3_func, test3_element, _T("test1 argv2 (argv|a)"), _T("Test3 help string!"))
{
    cel_clisession_out(cli_si, _T("Hello, cli test3!\r\n"));

    return 0;
}

CEL_CLI(test4_func, test4_element, _T("test1 argv1 [argv]"), _T("Test3 help string!"))
{
    cel_clisession_out(cli_si, _T("Hello, cli test4!\r\n"));

    return 0;
}

CEL_CLI(test5_func, test5_element, 
        _T("test2 argv1 {always|metric <0-16777214>|metric-type (1|2)|route-map WORD}"),
        _T("Test5 help string!"))
{
    cel_clisession_out(cli_si, _T("Hello, cli test5!\r\n"));

    return 0;
}

int cli_test_write(void *buf, size_t size)
{
    return _tprintf(_T("%s"), (TCHAR *)buf);
}

int command_test(int argc, const char *argv[])
{
    CelCommand cli;
    CelCommandSession cli_si;

    cel_command_init(&cli);

    cel_command_install_element(&cli, &test1_element);
    cel_command_install_element(&cli, &test2_element);
    cel_command_install_element(&cli, &test3_element);
    cel_command_install_element(&cli, &test4_element);
    cel_command_install_element(&cli, &test5_element);
    
    cel_clisession_init(&cli_si, &cli, cli_test_write);
    cel_clisession_describe(&cli_si, NULL);

    cel_clisession_execute(&cli_si, _T("test1 argv1 argv01"));
    cel_clisession_execute(&cli_si, _T("test1 argv1 argv01 xxx"));
    cel_clisession_destroy(&cli_si);

    cel_command_destroy(&cli);

    return 0;
}
