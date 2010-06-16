
#include <usual/string.h>

#include "test_common.h"

/*
 * strlcpy
 */

static char *run_strlcpy(char *dst, const char *src, int size, int expres)
{
	int res;
	strcpy(dst, "XXX");
	res = strlcpy(dst, src, size);
	if (res != expres)
		return "FAIL";
	return dst;
}

static void test_strlcpy(void *ptr)
{
	char buf[128];
	str_check(run_strlcpy(buf, "", 16, 0), "");
	str_check(run_strlcpy(buf, "", 0, 0), "XXX");
	str_check(run_strlcpy(buf, "", 16, 0), "");
	str_check(run_strlcpy(buf, "abc", 16, 3), "abc");
	str_check(run_strlcpy(buf, "abc", 4, 3), "abc");
	str_check(run_strlcpy(buf, "abc", 3, 3), "ab");
	str_check(run_strlcpy(buf, "abc", 2, 3), "a");
	str_check(run_strlcpy(buf, "abc", 1, 3), "");
	str_check(run_strlcpy(buf, "abc", 0, 3), "XXX");
end:;
}

/*
 * strlcat
 */

static char *run_strlcat(char *dst, const char *src, int size, int expres)
{
	int res;
	strcpy(dst, "PFX");
	res = strlcat(dst, src, size);
	if (res != expres)
		return "FAIL";
	return dst;
}

static void test_strlcat(void *ptr)
{
	char buf[128];
	str_check(run_strlcat(buf, "", 16, 3), "PFX");
	str_check(run_strlcat(buf, "abc", 16, 6), "PFXabc");
	str_check(run_strlcat(buf, "abc", 7, 6), "PFXabc");
	str_check(run_strlcat(buf, "abc", 6, 6), "PFXab");
	str_check(run_strlcat(buf, "abc", 5, 6), "PFXa");
	str_check(run_strlcat(buf, "abc", 4, 6), "PFX");
	str_check(run_strlcat(buf, "abc", 3, 6), "PFX");
	str_check(run_strlcat(buf, "abc", 2, 5), "PFX");
	str_check(run_strlcat(buf, "abc", 1, 4), "PFX");
	str_check(run_strlcat(buf, "abc", 0, 3), "PFX");
end:;
}

/*
 * strerror_r()
 */

static void test_strerror_r(void *p)
{
	char buf[128];
	/* "int" vs. "const char *" */
	tt_assert(strerror_r(EINTR, buf, sizeof(buf)) != 0);
	tt_assert(strlen(strerror_r(EINTR, buf, sizeof(buf))) != 0);
end:;
}

/*
 * fls
 */

static void test_fls(void *p)
{
	int_check(fls(0), 0);
	int_check(fls(1), 1);
	int_check(fls(3), 2);
	int_check(fls((int)-1), 32);
end:;
}

static void test_flsl(void *p)
{
	int_check(flsl(0), 0);
	int_check(flsl(1), 1);
	int_check(flsl(3), 2);
	if (sizeof(long) == 4)
		int_check(flsl((long)-1), 32);
	else
		int_check(flsl((long)-1), 64);
end:;
}

static void test_flsll(void *p)
{
	int_check(flsll(0), 0);
	int_check(flsll(1), 1);
	int_check(flsll(3), 2);
	int_check(flsll((long long)-1), 64);
end:;
}

/*
 * memrchr
 */

static void test_memrchr(void *p)
{
	static const char data[] = "abcdabc";
	tt_assert(memrchr(data, 'a', 8) == data + 4);
	tt_assert(memrchr(data, 'a', 4) == data + 0);
	tt_assert(memrchr(data, 'd', 8) == data + 3);
	tt_assert(memrchr(data, 'x', 8) == NULL);
end:;
}

/*
 * basename
 */

static const char *run_basename(const char *path)
{
	char buf[128];
	const char *res;
	if (!path)
		return basename(path);
	strlcpy(buf, path, sizeof(buf));
	res = basename(buf);
	if (strcmp(buf, path) != 0)
		return "MODIFIES";
	return res;
}

static void test_basename(void *p)
{
	str_check(run_basename("/usr/lib"), "lib");
	str_check(run_basename("/usr/"), "usr");
	str_check(run_basename("/"), "/");
	str_check(run_basename("///"), "/");
	str_check(run_basename("//usr//lib//"), "lib");
	str_check(run_basename(""), ".");
	str_check(run_basename("a/"), "a");
	str_check(run_basename(NULL), ".");
end:;
}

/*
 * dirname
 */

static const char *run_dirname(const char *path)
{
	char buf[128];
	const char *res;
	if (!path)
		return dirname(path);
	strlcpy(buf, path, sizeof(buf));
	res = dirname(buf);
	if (strcmp(buf, path) != 0)
		return "MODIFIES";
	return res;
}

static void test_dirname(void *p)
{
	str_check(run_dirname("/usr/lib"), "/usr");
	str_check(run_dirname("/usr/"), "/");
	str_check(run_dirname("usr"), ".");
	str_check(run_dirname("/usr/"), "/");
	str_check(run_dirname("/"), "/");
	str_check(run_dirname("/"), "/");
	str_check(run_dirname(".."), ".");
	str_check(run_dirname("."), ".");
	str_check(run_dirname(""), ".");
	str_check(run_dirname("a/"), ".");
	str_check(run_dirname("a//"), ".");
	str_check(run_dirname(NULL), ".");
end:;
}


/*
 * Describe
 */

struct testcase_t string_tests[] = {
	{ "strlcpy", test_strlcpy },
	{ "strlcat", test_strlcat },
	{ "strerror_r", test_strerror_r },
	{ "fls", test_fls },
	{ "flsl", test_flsl },
	{ "flsll", test_flsll },
	{ "memrchr", test_memrchr },
	{ "basename", test_basename },
	{ "dirname", test_dirname },
	END_OF_TESTCASES
};
