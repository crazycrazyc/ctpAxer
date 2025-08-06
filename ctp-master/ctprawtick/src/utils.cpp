#include <linux/limits.h>
#include <unistd.h>
#include <iostream>
#include <libgen.h>
#include "zlog.h"

using namespace std;

// external global variable
extern zlog_category_t *cat;

string currentPath = "";

// 获得当前进程的绝对路径
void get_self_path()
{
    char exepath[PATH_MAX] = {0};
    if (readlink("/proc/self/exe", exepath, PATH_MAX) == -1) {
        if (cat)
        {
            zlog_error(cat, "can not get work path, return error code: %d", errno);
        }
        else
        {
            cerr << "can not get work path, return error code: " << errno << endl;
        }

        return;
    }
    dirname(exepath);
    currentPath = exepath;
}
