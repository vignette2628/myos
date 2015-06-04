#include <io.h>
#include <start32.h>
#include <system.h>

void default_handle(void);

void do_default_handle()
{
    /*do nothing by now.*/
    printk("default handle\n");

    return;
}

void trap_init(void)
{
    int i;
    /*why 48? 32~40 is used by 8259-1, and 40~48 is used by 8259-2*/
    for( i = 0; i < 48; i++ )
    {
        set_trap_gate(i, &default_handle);
    }
}


