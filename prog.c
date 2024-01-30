#include <stdio.h>               // FILE, fopen, fread, fwrite, fclose, feof
#include <stdint.h>              // uint64_t
#include <stdlib.h>              // abort
#include <stdbool.h>             // bool, true, false
#include <dlfcn.h>               // dlopen, dlsym, dlclose

#define NOPLIB "libnop.so"
#define TMPLIB "libtmp.so"

static void abortwhy(char const *const p)
{
    printf("\n========== abort() called - '%s' ==========\n", p);
    abort();
}

static bool Seek_To_MyCode(FILE *const f)
{
    fseek(f, 0, SEEK_SET);

    static char unsigned const mybytes[16u] = {
        // 0xf3, 0x0f, 0x1e, 0xfa,    // endbr64
        0x48, 0x83, 0xc4, 0x01,    // add rsp, 1
        0x48, 0x83, 0xec, 0x01,    // sub rsp, 1
        0x90, 0x90, 0x90, 0x90,    // nop, nop, nop, nop
        0x90, 0x90, 0x90, 0x90,    // nop, nop, nop, nop
    };

    unsigned i = 0u;
    for (; /* ever */ ;)
    {
        char unsigned const c = (char unsigned)fgetc(f);
        if ( ferror(f) || feof(f) ) return false;
        for (; /* ever */ ;)
        {
            if ( c == mybytes[i++] )
            {
                if ( i >= 16u )
                {
                    fseek(f,-16,SEEK_CUR);
                    return true;
                }
                break;
            }
            else
            {
                unsigned const tmp = i;
                i = 0;
                if ( 0u != tmp  ) break;
            }
        }
    }
}

int main(void)
{
    FILE *const in = fopen(NOPLIB, "rb");
    if ( NULL == in ) abortwhy("Unable to open file '" NOPLIB "'");
    FILE *const out = fopen(TMPLIB, "wb+");
    if ( NULL == out ) abortwhy("Unable to open file '" TMPLIB "'");

    for (; /* ever */ ;)
    {
        char unsigned const c = (char unsigned)fgetc(in);  // cast for compiler warning
        if ( feof(in) ) break;
        fwrite(&c,1u,1u,out);
    }

    fclose(in);

    if ( false == Seek_To_MyCode(out) ) abortwhy("Unable to find CPU instructions inside '" TMPLIB "'");

    static char unsigned const replacement[] = {
        0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,  // movabs rax, 0x1122334455667788
        0xc3                                                         // ret
    };

    fwrite(replacement, 1u, sizeof replacement, out);

    fclose(out);

    void *const h = dlopen("./"TMPLIB, RTLD_LAZY);
    if ( NULL == h )
    {
        printf("dlerror() returned '%s'\n", dlerror());
        abortwhy("Could not dlopen(\"" TMPLIB "\")");
    }
    uint64_t (*const MyCode)(void) = (uint64_t(*)(void))dlsym(h,"MyCode");
    if ( NULL == MyCode ) abortwhy("Could not find symbol MyCode in '" TMPLIB "'");
    uint64_t const retval = MyCode();
    dlclose(h);
    printf("MyCode returned 0x%llx\n", (long long unsigned)retval);
    return 0;
}
