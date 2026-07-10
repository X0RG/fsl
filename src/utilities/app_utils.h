#ifndef APP_UTILS_H
#define APP_UTILS_H

#include <fsl.h>

void app_print(const App* a, const AppVec* v);
void app_vec_print(const AppVec* v);

AppVecStatus app_vec_init(AppVec* v);
AppVecStatus app_vec_item_insert(AppVec* v, str id, str name, str icon,
                                 str url);
AppVecStatus app_vec_item_remove_by_id(AppVec* v, str id);
AppVecStatus app_vec_reset(AppVec* v);
AppVecStatus app_vec_free(AppVec* v);

#endif