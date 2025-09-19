#include "cel/timepolicy.h"
#include "cel/error.h"

void timepolicy_print(CelTimePolicy *tp)
{
	int i, n;
	CelTimeRange *time_range;

	printf("\r\n##");
	if (tp->is_everyday) {
		printf("* ? ");
	}
	else if (tp->is_weekly) {
		printf("? ");
		for (i = 0; i < 7; i++) {
			if (tp->wdays[i])
				printf(" %d", i);
		}
		printf(" ");
	}
	else {
		for (i = 0; i < 31; i++) {
			if (tp->mdays[i])
				printf("%d ", i);
		}
		printf("? ");
	}

	n = cel_arraylist_get_size(&(tp->time_ranges));
	for (i = 0; i < n; i++)
	{
		time_range = (CelTimeRange *)cel_arraylist_get_by_index(
			&(tp->time_ranges), i);
		printf("%02d:%02d:%02d-%02d:%02d:%02d,", time_range->start_hour,
			time_range->star_min,
			time_range->start_sec,
			time_range->end_hour,
			time_range->end_min,
			time_range->end_sec);
	}
	printf("##\r\n");
}

int timepolicy_test(int argc, const char *argv[])
{
	CelTimePolicy tp;
	CelTime t;
	char timeStr[32];
	
	cel_time_init_now(&t);
	cel_time_asc(&t, timeStr, 32);

	if (cel_timepolicy_init(&tp, _T("* ? 09:50:00-10:00:00")) == -1)
		printf("%s\r\n", cel_geterrstr());
	timepolicy_print(&tp);
	printf("%s  - %d\r\n", timeStr, cel_timepolicy_is_allow(&tp, &t));
	cel_timepolicy_destroy(&tp);

	if (cel_timepolicy_init(&tp, _T("? Off-Day 00:00:00-23:59:59")) == -1)
		printf("%s\r\n", cel_geterrstr());
	timepolicy_print(&tp);
	printf("%s  - %d\r\n", timeStr, cel_timepolicy_is_allow(&tp, &t));
	cel_timepolicy_destroy(&tp);

	if (cel_timepolicy_init(&tp, _T("? Working-Day 12:00:00-23:59:59,00:00:01-06:00:00")) == -1)
		printf("%s\r\n", cel_geterrstr());
	timepolicy_print(&tp);
	printf("%s  - %d\r\n", timeStr, cel_timepolicy_is_allow(&tp, &t));
	cel_timepolicy_destroy(&tp);

	if (cel_timepolicy_init(&tp, _T("? TUE,THU,SAT,SUN 00:00:00-23:59:59")) == -1)
		printf("%s\r\n", cel_geterrstr());
	timepolicy_print(&tp);
	printf("%s  - %d\r\n", timeStr, cel_timepolicy_is_allow(&tp, &t));
	cel_timepolicy_destroy(&tp);

	if (cel_timepolicy_init(&tp, _T("1,2,3,31 ? 00:00:00-23:59:59 ")) == -1)
		printf("%s\r\n", cel_geterrstr());
	timepolicy_print(&tp);
	printf("%s  - %d\r\n", timeStr, cel_timepolicy_is_allow(&tp, &t));
	cel_timepolicy_destroy(&tp);

	if (cel_timepolicy_init(&tp, _T("* ? *")) == -1)
		printf("%s\r\n", cel_geterrstr());
	timepolicy_print(&tp);
	printf("%s  - %d\r\n", timeStr, cel_timepolicy_is_allow(&tp, &t));
	cel_timepolicy_destroy(&tp);
	return 0;
}