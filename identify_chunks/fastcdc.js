var hash123;
if(!hash123)
{
    hash123 = require('./hash123');
}

var fastcdc = new function()
{
    var SymbolCount = 256;
    var DigistLength = 16;
    var SeedLength = 64;
    var MaxChunkSizeOffset = 1;
    var MinChunkSizeOffset = 2;

    var g_gear_matrix = new ArrayBuffer(SymbolCount*32);
    //g_gear_matrix_view64 = new BigUint64Array(g_gear_matrix);
    var g_gear_matrix_view32 = new Uint32Array(g_gear_matrix);
    var g_min_chunk_size = 0;
    var g_max_chunk_size = 0;
    var minMask;
    var maxMask;
    var EBlockLength;

    var mask = {
        'Mask_16B': 0x000080010300,
        'Mask_32B': 0x000080050300,
        'Mask_64B': 0x000080070300,
        'Mask_128B': 0x000090100053,
        'Mask_256B': 0x000091300301,
        'Mask_512B': 0x000090055013,
        'Mask_1KB': 0x000083030531,
        'Mask_2KB': 0x0000d9000353,
        'Mask_4KB': 0x0000d9010353,
        'Mask_8KB': 0x0000d9030353,
        'Mask_16KB': 0x0000d9070353,
        'Mask_32KB': 0x0000f9070353,
        'Mask_64KB': 0x0000f90f0353,
        'Mask_128KB': 0x0000f90f0373
    };
    var BlockLen = {
        'Mask_16B': 16,
        'Mask_32B': 32,
        'Mask_64B': 64,
        'Mask_128B': 128,
        'Mask_256B': 256,
        'Mask_512B': 512,
        'Mask_1KB': 1024,
        'Mask_2KB': 2048,
        'Mask_4KB': 4096,
        'Mask_8KB': 8192,
        'Mask_16KB': 16384,
        'Mask_32KB': 32768,
        'Mask_64KB': 65536,
        'Mask_128KB': 131072
    };


    function Init(expectCS)
    {
        seed = new ArrayBuffer(SeedLength) ;
        seedView = new Uint8Array(seed);

        for(i=0; i<SymbolCount; i++){
            for(j=0; j<SeedLength; j++){
                seedView[j] = i;
            }
            g_gear_matrix[i] = 0;

            md5_result = hash123.md5(seedView,0,0,SeedLength);
            g_gear_matrix_view32[i] = md5_result[0];
        }
        if (expectCS == 'Mask_32B')
        {
            minMask = mask['Mask_16B'];
            maxMask = mask['Mask_64B'];
        }
        else if (expectCS == 'Mask_64B')
        {
            minMask = mask['Mask_32B'];
            maxMask = mask['Mask_128B'];
        }
        else if (expectCS == 'Mask_128B')
        {
            minMask = mask['Mask_64B'];
            maxMask = mask['Mask_256B'];
        }
        else if (expectCS == 'Mask_256B')
        {
            minMask = mask['Mask_128B'];
            maxMask = mask['Mask_512B'];
        }
        else if (expectCS == 'Mask_512B')
        {
            minMask = mask['Mask_256B'];
            maxMask = mask['Mask_1KB'];
        }
        else if (expectCS == 'Mask_1KB')
        {
            minMask = mask['Mask_512B'];
            maxMask = mask['Mask_2KB'];
        }
        else if (expectCS == 'Mask_2KB')
        {
            minMask = mask['Mask_1KB'];
            maxMask = mask['Mask_4KB'];
        }
        else if (expectCS == 'Mask_4KB')
        {
            minMask = mask['Mask_2KB'];
            maxMask = mask['Mask_8KB'];
        }
        else if (expectCS == 'Mask_8KB')
        {
            minMask = mask['Mask_4KB'];
            maxMask = mask['Mask_16KB'];
        }
        else if (expectCS == 'Mask_16KB')
        {
            minMask = mask['Mask_8KB'];
            maxMask = mask['Mask_32KB'];
        }
        else if (expectCS == 'Mask_32KB')
        {
            minMask = mask['Mask_16KB'];
            maxMask = mask['Mask_64KB'];
        }

        EBlockLength = BlockLen[expectCS];

    }


    // function ChunkData(dataView, start, length){
    //     var fp = 0;
    //     for(var i=start; i<length; i++) {
    //         fp = (fp << 1) + g_gear_matrix_view32[dataView[i]];
    //         if (!(fp & Mask)) {
    //             return [i + 1, (fp & 0xffffffff)];
    //         }
    //     }
    //     return [i, (fp&0xffffffff)];
    // }

    function ChunkData(dataView, start, length){
        var fp = 0;
        var gearHash = 0;
        var Mask = maxMask;

        for(var i=start; i<length; i++) {
            if((i - start) == EBlockLength);
                Mask = minMask;

            if(((i-start) & 0x0000000f) == 0 )
            {
                gearHash += fp;
            }
            fp = (fp << 1) ^ g_gear_matrix_view32[dataView[i]];

            if (!(fp & Mask)) {
                var fastFp =  gearHash + fp ;
                return [i + 1, fastFp];
            }
        }

        fastFp =  gearHash + fp ;
        return [i, fastFp];
    }

    this.Init = Init;
    this.ChunkData = ChunkData;
}

if(((typeof require) != "undefined") &&
    ((typeof module) != "undefined") &&
    ((typeof module.exports) != "undefined"))
    module.exports = fastcdc;