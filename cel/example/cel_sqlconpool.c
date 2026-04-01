
#include "cel/multithread.h"
#include "cel/sql/sqlconpool.h"
#include "cel/error.h"

int sqlconpool_test(int argc, TCHAR *argv[])
{
    CelSqlConPool sqldb_pool;
    CelSqlCon *conns[68];

    cel_multithread_support();
    cel_sqlconpool_init(&sqldb_pool, CEL_SQLCON_MYSQL,
                        "192.168.23.151", 9076,
                        "iam", "root", "Secur1tyP@ssWD182", 2, 64);

    int i;
    for (i = 0; i < 68; i++) {
        conns[i] = cel_sqlconpool_get(&sqldb_pool);
        if (conns[i] == NULL) {
            puts(cel_geterrstr());
        }
    }

    for (i = 0; i < 68; i++) {
        if (conns[i] != NULL) {
            cel_sqlconpool_return(&sqldb_pool, conns[i]);
        }
    }

    return 0;
}