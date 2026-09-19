#include <string.h>
#include "reflux_runtime.h"
void reflux_log_reset(RefluxLog *log) { memset(log, 0, sizeof(*log)); }
void reflux_log_dispatch(RefluxLog *log, int type, int a, int b, int c) {
    RefluxAction *x = &log->actions[log->total_dispatched % REFLUX_LOG_CAPACITY];
    x->action_type = type; x->a = a; x->b = b; x->c = c;
    log->total_dispatched++;
}
int reflux_log_oldest(const RefluxLog *log) {
    return log->total_dispatched > REFLUX_LOG_CAPACITY ? log->total_dispatched - REFLUX_LOG_CAPACITY : 0;
}
const RefluxAction *reflux_log_at(const RefluxLog *log, int abs_index) {
    if (abs_index < reflux_log_oldest(log) || abs_index >= log->total_dispatched) return 0;
    return &log->actions[abs_index % REFLUX_LOG_CAPACITY];
}
