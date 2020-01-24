#include <stdio.h>
#include <zlib.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <string.h>
#include "uthash.h"

#define SymbolCount 256
#define SeedLength 64
#define CacheSize 1024 * 1024

struct chunk_fingerprint
{
    uint32_t fingerprints[5];
    struct chunk_fingerprint *next;
};
struct chunk_info
{
    uint32_t id;
    struct chunk_fingerprint *data;
    UT_hash_handle hh;
};
// global variants
struct chunk_info *users = NULL;
int chunk_dist[30];
uint32_t g_global_matrix[SymbolCount];
uint32_t expectCS;
uint32_t MinMask;
uint32_t MaxMask;
uint32_t MinSize;
uint32_t MaxSize;

// functions to call
struct chunk_fingerprint *find_last(struct chunk_fingerprint *src, struct chunk_fingerprint obj);
void del_value_iter(struct chunk_fingerprint *src);
void add_user(uint32_t weakHash, struct chunk_fingerprint strongHash);
struct chunk_info *find_user(uint32_t user_id);
int delete_all(void);
int output_chunk_hash_info(void);
void fastCDC_init(void);
int fastCDC_chunking(int *src, int buffer_length);

int main(void)
{
    // codes below are just for testing
    FILE *random_file;
    uint8_t SHA1_digest[20];
    uLong adler;
    struct chunk_fingerprint newChunk;
    char *fileCache = (char *)malloc(CacheSize);
    int offset = 0, chunkLength = 8096, readFlag = 0;
    random_file = fopen("./random_file", "r");
    fread(fileCache, CacheSize, 1, random_file);

    for (;;)
    {
        // calculate the fingerprints
        adler = adler32(0L, Z_NULL, 0);
        adler = adler32(adler, fileCache + offset, chunkLength);
        SHA1(fileCache + offset, chunkLength, SHA1_digest);
        // update the fingerprints of chunks
        memcpy(newChunk.fingerprints, SHA1_digest, 20);
        newChunk.next = NULL;
        add_user(adler, newChunk);
        offset += chunkLength;
        if (CacheSize - offset < MaxSize)
        {
            memcpy(fileCache, fileCache + offset, CacheSize - offset);
            if (fread(fileCache + CacheSize - offset, offset, 1, random_file) < offset && readFlag == 0)
            {
                // all the files are read
                readFlag = 1;
            }
            offset = 0;
        }
    }

    // clear the items
    output_chunk_hash_info();
    delete_all();
    free(fileCache);
    fileCache = NULL;
    return 0;
}

// functions
struct chunk_fingerprint *find_last(struct chunk_fingerprint *src, struct chunk_fingerprint obj)
{
    if (memcmp(src->fingerprints, obj.fingerprints, 20) == 0)
    {
        // already exist
        return NULL;
    }
    else if (src->next == NULL)
    {
        // not found
        return src;
    }
    else
    {
        return find_last(src->next, obj);
    }
}
void del_value_iter(struct chunk_fingerprint *src)
{
    struct chunk_fingerprint *tmp = NULL, *present = NULL;
    present = src;
    for (;;)
    {
        tmp = present->next;
        free(present);
        present = tmp;
        if (tmp == NULL)
            break;
    }
}
// add the chunk items to the hash table if it's not a member of the hash table
void add_user(uint32_t weakHash, struct chunk_fingerprint strongHash)
{
    struct chunk_info *tmp = NULL, *s = NULL;
    struct chunk_fingerprint *data = NULL;
    HASH_FIND_INT(users, &weakHash, tmp);
    if (tmp == NULL)
    {
        // this key is never used before
        s = malloc(sizeof(struct chunk_info));
        data = malloc(sizeof(struct chunk_fingerprint));
        memcpy(data->fingerprints, strongHash.fingerprints, 20);
        data->next = NULL;
        s->data = data;
        s->id = weakHash;
        HASH_ADD_INT(users, id, s);
    }
    else
    {
        struct chunk_fingerprint *tmpPtr;
        tmpPtr = find_last(tmp->data, strongHash);
        if (tmpPtr)
        {
            data = malloc(sizeof(struct chunk_fingerprint));
            memcpy(data->fingerprints, strongHash.fingerprints, 20);
            data->next = NULL;
            tmpPtr->next = data;
        }
        else
        {
            printf("chunk already in hash table\n");
        }
    }
}
// find chunk items
struct chunk_info *find_user(uint32_t user_id)
{
    struct chunk_info *s;
    HASH_FIND_INT(users, &user_id, s);
    return s;
}
// delete chunk items
int delete_user(uint32_t weakHash)
{
    struct chunk_info *s;
    s = find_user(weakHash);
    if (s == NULL)
        return NULL;
    del_value_iter(s->data);
    HASH_DEL(users, s);
    free(s);
    s = NULL;
    return 0;
}
// delete all data
int delete_all(void)
{
    struct chunk_info *s, *tmp;
    HASH_ITER(hh, users, s, tmp)
    {
        delete_user(s->id);
    }
}
// walk the hash table and get the collision status of weak hash
int output_chunk_hash_info(void)
{
    struct chunk_info *current_user, *tmp;
    struct chunk_fingerprint *tmp_data;
    int count = 0;
    for (int i = 1; i < 30; i++)
    {
        chunk_dist[i] = 0;
    }
    HASH_ITER(hh, users, current_user, tmp)
    {
        // do operations on each key
        tmp_data = current_user->data;
        for (;;)
        {
            count += 1;
            if (tmp_data->next)
            {
                tmp_data = tmp_data->next;
            }
            else
            {
                break;
            }
        }
        chunk_dist[count - 1] += 1;
        count = 0;
    }
    for (int i = 1; i < 30; i++)
    {
        printf("the num for %d duplicate weak hash is %d\n", i + 1, chunk_dist[i]);
    }
}

void fastCDC_init(void)
{
    uint32_t md5_digest[4];
    uint8_t seed[SeedLength];
    for (int i = 0; i < SymbolCount; i++)
    {
        for (int j = 0; j < SeedLength; j++)
        {
            seed[j] = i;
        }
        g_global_matrix[i] = 0;
        md5(seed, SeedLength, md5_digest);
        memcpy(&(g_global_matrix[i]), md5_digest, 4);
    }

    MinSize = 2028;
    MaxSize = 32768;
    MinMask = 0x0000f9070353; //2^15
    MaxMask = 0x0000d9000353; // 2^11
}

int fastCDC_chunking(int *src, int buffer_length)
{
    // set the expected chunk size as 8KB
    expectCS = 8192;
    int i = 0;
    uint32_t fp = 0;
    int n = buffer_length;
    if (n <= MinSize)
        return n;
    if (n >= MaxSize)
        n = MaxSize;
    else if (buffer_length <= expectCS)
        expectCS = n;
    for (; i < expectCS; i++)
    {
        fp = (fp << 1) + g_global_matrix[*(src + i)];
        if (!(fp & MinMask))
            return i;
    }
    for (; i < n; i++)
    {
        fp = (fp << 1) + g_global_matrix[*(src + i)];
        if (!(fp & MaxMask))
            return i;
    }
    return i;
}