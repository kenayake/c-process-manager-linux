#pragma once

#include "glib-object.h"

typedef struct timespec GTimespec;

#define TYPE_TIMESPEC (timespec_get_type ())

GType
timespec_get_type (void) G_GNUC_CONST;

