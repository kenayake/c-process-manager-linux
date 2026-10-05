#include "util/cpu-usage-type.h"
#include <glib-object.h>
#include <time.h>

// Copy helper function
static CpuUsage *
cpu_usage_copy (const CpuUsage *src)
{
    if (src == NULL) return NULL;
    CpuUsage *dest = g_new0 (CpuUsage, 1);
    *dest = *src;
    return dest;
}

// Free helper function
static void
cpu_usage (CpuUsage *ts)
{
    g_free (ts);
}

// Macro to obtain the unique GType ID
GType
cpu_usage_get_type (void)
{
    static GType type = 0;
    if (g_once_init_enter (&type)) {
        GType _type = g_boxed_type_register_static (
            "CpuUsage",
            (GBoxedCopyFunc) cpu_usage_copy,
            (GBoxedFreeFunc) cpu_usage
        );
        g_once_init_leave (&type, _type);
    }
    return type;
}
