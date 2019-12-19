

typedef struct kvdb_element{
    int key;
    char* val;
} kvdb_element_t;


typedef struct kvdb {
    int elements;
} kvdb_t;


kvdb_t *kvdb_create(int elements);

char* kvdb_get(int key);

int kvdb_set(int key, char* val);

void kvdb_destroy(kvdb_t db);
