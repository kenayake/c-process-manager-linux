#pragma once


#include "gio/gio.h"
void process_model_refresh(GListStore *store);

GListModel *process_model_new(void);