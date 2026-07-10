#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utilities/app_utils.h>

enum { DEFAULT_VEC_CAPACITY = 4, DEFAULT_ARENA_CAPACITY = 256 };

static AppVecStatus app_vec_size_grow(AppVec* v, u32 incoming_string_bytes);

static inline str buffer_offset_to_string(const AppVec* v, sref x);

static inline str buffer_offset_to_string(const AppVec* v, sref x) {
  if (!v || x == SREF_NULL) return NULL;
  return v->arena.buf + x;
}

static AppVecStatus app_vec_size_grow(AppVec* v, u32 incoming_string_bytes) {
  if (!v) return APPVEC_ERR_NULL;

  if (v->len == v->capacity) {
    u32 oldCapacity = v->capacity;
    u32 newCapacity = v->capacity == 0 ? DEFAULT_VEC_CAPACITY : v->capacity * 2;

    App* temp = realloc(v->data, newCapacity * sizeof *temp);
    if (!temp) return APPVEC_ERR_ALLOC;

    v->data = temp;
    v->capacity = newCapacity;

    u32 addedSlots = newCapacity - oldCapacity;
    memset(v->data + oldCapacity, 0, addedSlots * sizeof(App));
  }

  u32 required_bytes = v->arena.size + incoming_string_bytes;

  if (v->arena.capacity < required_bytes) {
    u32 needed = required_bytes - v->arena.capacity;
    u32 blocks = (needed + DEFAULT_ARENA_CAPACITY - 1) / DEFAULT_ARENA_CAPACITY;
    u32 newCapacity = v->arena.capacity + (blocks * DEFAULT_ARENA_CAPACITY);

    mut_str temp = realloc(v->arena.buf, newCapacity);
    if (!temp) return APPVEC_ERR_ALLOC;

    v->arena.buf = temp;
    v->arena.capacity = newCapacity;
  }
  return APPVEC_OK;
}

static AppVecStatus app_vec_size_shrink(AppVec* v) {
  if (!v) return APPVEC_ERR_NULL;

  if (v->capacity > DEFAULT_VEC_CAPACITY && (v->len <= (v->capacity >> 2))) {
    u32 newCapacity = v->capacity >> 1;

    if (newCapacity < DEFAULT_VEC_CAPACITY) {
      newCapacity = DEFAULT_VEC_CAPACITY;
    }

    App* temp = realloc(v->data, newCapacity * sizeof *temp);
    if (!temp) return APPVEC_ERR_ALLOC;
    v->data = temp;
    v->capacity = newCapacity;
  }

  return APPVEC_OK;
}

void app_print(const App* a, const AppVec* v) {
  if (!a) {
    printf("null");
    return;
  }

  printf(
      "{\n"
      "\"id\": \"%s\",\n"
      "\"name\": \"%s\",\n"
      "\"icon\": \"%s\",\n"
      "\"url\": \"%s\"\n"
      "}",
      buffer_offset_to_string(v, a->id), buffer_offset_to_string(v, a->name),
      buffer_offset_to_string(v, a->icon), buffer_offset_to_string(v, a->url));
}

void app_vec_print(const AppVec* a) {
  if (!a) {
    printf("[]\n");
    return;
  }

  printf(
      "{\n\t\"info\": {\n\t\t\"vec-len\": %u,\n\t\t\"vec-cap\": %u,\n\t\t"
      "\"buff-size-bytes\": %u,\n\t\t\"buff-cap-bytes\": %u\n\t},\n\t\"data\": "
      "[",
      a->len, a->capacity, a->arena.size, a->arena.capacity);

  for (u32 i = 0; i < a->len; ++i) {
    printf(
        "\n\t\t{\n\t\t\t\"id\": \"%s\",\n\t\t\t\"name\": "
        "\"%s\",\n\t\t\t\"icon\": \"%s\",\n\t\t\t\"url\": \"%s\"\n\t\t}",
        buffer_offset_to_string(a, a->data[i].id),
        buffer_offset_to_string(a, a->data[i].name),
        buffer_offset_to_string(a, a->data[i].icon),
        buffer_offset_to_string(a, a->data[i].url));
    if (i < a->len - 1) printf(",");
    if (i == a->len - 1) printf("\n\t]");
  }
  printf("\n}");
}

AppVecStatus app_vec_init(AppVec* v) {
  if (!v) return APPVEC_ERR_NULL;

  App* data = calloc(DEFAULT_VEC_CAPACITY, sizeof *data);
  mut_str pool = calloc(DEFAULT_ARENA_CAPACITY, sizeof *pool);

  if (!data || !pool) {
    free(data);
    free(pool);
    *v = (AppVec){0};
    return APPVEC_ERR_ALLOC;
  }

  *v = (AppVec){
      .data = data,
      .arena =
          {
              .buf = pool,
              .capacity = DEFAULT_ARENA_CAPACITY,
              .size = 0,
          },
      .capacity = DEFAULT_VEC_CAPACITY,
      .len = 0,
  };

  return APPVEC_OK;
}

AppVecStatus app_vec_item_insert(AppVec* v, str id, str name, str icon,
                                 str url) {
  if (!v) return APPVEC_ERR_NULL;

  size_t id_len = id ? strlen(id) + 1 : 0;
  size_t name_len = name ? strlen(name) + 1 : 0;
  size_t icon_len = icon ? strlen(icon) + 1 : 0;
  size_t url_len = url ? strlen(url) + 1 : 0;

  u32 total_string_bytes = (u32)(id_len + name_len + icon_len + url_len);

  AppVecStatus grow_status = app_vec_size_grow(v, total_string_bytes);
  if (grow_status != APPVEC_OK) return grow_status;

  App* dest_app = &v->data[v->len];

  u32 current_offset = v->arena.size;
  mut_str write_position = v->arena.buf + current_offset;

  if (id) {
    memcpy(write_position, id, id_len);
    dest_app->id = current_offset;
    write_position += id_len;
    current_offset += id_len;
  } else {
    dest_app->id = SREF_NULL;
  }

  if (name) {
    memcpy(write_position, name, name_len);
    dest_app->name = current_offset;
    write_position += name_len;
    current_offset += name_len;
  } else {
    dest_app->name = SREF_NULL;
  }

  if (icon) {
    memcpy(write_position, icon, icon_len);
    dest_app->icon = current_offset;
    write_position += icon_len;
    current_offset += icon_len;
  } else {
    dest_app->icon = SREF_NULL;
  }

  if (url) {
    memcpy(write_position, url, url_len);
    dest_app->url = current_offset;
    write_position += url_len;
    current_offset += url_len;
  } else {
    dest_app->url = SREF_NULL;
  }

  v->arena.size += total_string_bytes;
  v->len += 1;
  return APPVEC_OK;
}

AppVecStatus app_vec_item_remove_by_id(AppVec* v, str id) {
  if (!v || !id || id[0] == '\0') return APPVEC_ERR_NULL;

  i32 target_index = -1;
  char target_first_char = id[0];

  for (u32 i = 0; i < v->len; i++) {
    sref current = v->data[i].id;

    if (current != SREF_NULL) {
      str current_id = v->arena.buf + current;
      if (current_id[0] == target_first_char && strcmp(current_id, id) == 0) {
        target_index = (int)i;
        break;
      }
    }
  }

  if (target_index == -1) return APPVEC_OK;

  u32 items_after = v->len - (u32)target_index - 1;
  if (items_after > 0) {
    memmove(&v->data[target_index], &v->data[target_index + 1],
            items_after * sizeof(App));
  }

  v->len -= 1;
  app_vec_size_shrink(v);
  return APPVEC_OK;
}

AppVecStatus app_vec_reset(AppVec* v) {
  if (!v) return APPVEC_ERR_NULL;
  v->len = 0;
  v->arena.size = 0;

  if (v->data) memset(v->data, 0, v->capacity * sizeof(App));
  if (v->arena.buf) memset(v->arena.buf, 0, v->arena.capacity);
  return APPVEC_OK;
}

AppVecStatus app_vec_free(AppVec* v) {
  if (!v) return APPVEC_ERR_NULL;

  free(v->data);
  free(v->arena.buf);

  *v = (AppVec){0};

  return APPVEC_OK;
}