
#include "util/disk-usage-type.h"
#include <glib-object.h>
#include <time.h>

// Copy helper function
static DiskUsage *
disk_usage_copy (const DiskUsage *src)
{
    if (src == NULL) return NULL;
    DiskUsage *dest = g_new0 (DiskUsage, 1);
    *dest = *src;
    return dest;
}

// Free helper function
static void
disk_usage (DiskUsage *ts)
{
    g_free (ts);
}

// Macro to obtain the unique GType ID
GType
disk_usage_get_type (void)
{
    static GType type = 0;
    if (g_once_init_enter (&type)) {
        GType _type = g_boxed_type_register_static (
            "DiskUsage",
            (GBoxedCopyFunc) disk_usage_copy,
            (GBoxedFreeFunc) disk_usage
        );
        g_once_init_leave (&type, _type);
    }
    return type;
}
