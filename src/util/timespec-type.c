#include <glib-object.h>
#include <time.h>
#include "timespec-type.h"

// Copy helper function
static GTimespec *
timespec_copy (const GTimespec *src)
{
    if (src == NULL) return NULL;
    GTimespec *dest = g_new0 (struct timespec, 1);
    *dest = *src;
    return dest;
}

// Free helper function
static void
timespec_free (GTimespec *ts)
{
    g_free (ts);
}

// Macro to obtain the unique GType ID
GType
timespec_get_type (void)
{
    static GType type = 0;
    if (g_once_init_enter (&type)) {
        GType _type = g_boxed_type_register_static (
            "GTimespec",
            (GBoxedCopyFunc) timespec_copy,
            (GBoxedFreeFunc) timespec_free
        );
        g_once_init_leave (&type, _type);
    }
    return type;
}
