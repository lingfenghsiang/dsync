# -*- coding: UTF-8 -*-
import os
import re

slidingDistance = ['8', '16', '32']
file_num = ['1', '2', '3', '4', '5']
operationDict={'XOR':'^', "PLUS":'+'}
operation = ['XOR', 'PLUS']

filename = "search_chunk.c"
pattern_distance = re.compile(r'#define SlidingDis .*')
pattern_operation = re.compile(r'gearHash .=.*')
pattern_file_num = re.compile(r'random_data_.')


def contentSubstitute(filename, pattern, dst):
    srcCode = open(filename, "r")
    dstCode = open(filename + "tmp", "w")
    codeLines = srcCode.readlines()
    dstCode.seek(0, 0)
    for singleLine in codeLines:
        m = pattern.search(singleLine)
        try:
            repalcedLine = singleLine.replace(m.group(0), dst)
            print(singleLine)
        except:
            repalcedLine = singleLine
        dstCode.write(repalcedLine)
    srcCode.close()
    dstCode.close()
    os.system("mv "+filename + "tmp "+filename)


# for dist in slidingDistance:
#     contentSubstitute(filename, pattern_distance, "#define SlidingDis "+dist)
#     for oper in operation:
#         contentSubstitute(filename, pattern_operation,
#                           "gearHash " + operationDict[oper] + "= fp;")
#         for num in file_num:
#             contentSubstitute(filename, pattern_file_num, "random_data_" + num)
#             os.system("make;mv walk_chunks walk_chunks" + oper + dist + "_" + num)

for num in file_num:
    contentSubstitute(filename, pattern_file_num, "random_data_" + num)
    os.system("make;mv walk_chunks walk_chunks_adler_" + num)
