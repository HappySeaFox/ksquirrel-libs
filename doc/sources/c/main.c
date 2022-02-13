#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#define PATH_LENGTH 256
      
int main(int argc, char * argv[])
{
    char path[PATH_LENGTH], *msg = NULL;
    const char* (*fmt)();
    void *module;
      
    getcwd(path, PATH_LENGTH);
    strcat(path, "/");
    strcat(path, "module.so");

    /* Load module */
    module = dlopen(path, RTLD_NOW);
      
    /* Error ! */
    if(!module)
    {
        msg = dlerror();

	if(msg != NULL)
        {
              dlclose(module);
              exit(1);
        }
    }

    /* Try to resolve function "fmt_info()" */
    fmt = dlsym(module, "fmt_info");

    msg = dlerror();

    if(msg != NULL)
    {
        perror(msg);
        dlclose(module);
        exit(1);
    }
      
    /* call fmt_info() through a pointer*/
    printf("%s\n", fmt());

    /* close module */
    if(dlclose(module))
    {
        perror("error");
        exit(1);
    }
      
    return 0;
}
