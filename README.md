#How to generate a random file
----
Execute the "make" command in data_gen folder, the file size is defined in #define FILE_SIZE
The unit is MiB. Default file name is random_data
----
#How to compare the collision ratio of different algorithm
----
See main function in search_chunk.c
When you use Adler32
>gFingerprint = fastCDC_chunking(fileCache + offset, CacheSize - offset + 1);
>        // use fastFp
>        // weakHash = gFingerprint.g_fingerprint;
>        chunkLength = gFingerprint.length;
>        // printf("chunk size is %d\n", chunkLength);
>        // use Adler32 weak hash
>        weakHash = adler32(0L, Z_NULL, 0);
>        weakHash = adler32(weakHash, fileCache + offset, chunkLength);

When you use GearHash
>gFingerprint = fastCDC_chunking(fileCache + offset, CacheSize - offset + 1);
>        // use fastFp
>        weakHash = gFingerprint.g_fingerprint;
>        chunkLength = gFingerprint.length;
>        // printf("chunk size is %d\n", chunkLength);
>        // use Adler32 weak hash
>        // weakHash = adler32(0L, Z_NULL, 0);
>        // weakHash = adler32(weakHash, fileCache + offset, chunkLength);

If you need to compile several binary executable files, compile.py may help