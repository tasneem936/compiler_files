cmake_minimum_required(VERSION 3.20)
project(compiler_files)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(compiler_files
        CodeGenerator.c
        CodeGenerator.h
        config.h
        ctree.c
        ctree.h
        dsm_extension.c
        dsm_extension.h
        f2.cactivity_gen.cpp
        f2.cactivity_gen.h
        f2.cactivity_gen_spawn.cpp
        f3.cactivity_gen.cpp
        f3.cactivity_gen.h
        f3.cactivity_gen_spawn.cpp
        globals.h
        gram.c
        gram.h
        gram_parser.cpp.h
        head.h
        heap.c
        heap.h
        identify.h
        lexer.c
        lexer.h
        nmetab.c
        nmetab.h
        prnttree.c
        prnttree.h
        sample1.c
        symtab.c
        symtab.h
        token.c
        token.h
        tree.c
        tree.h
        treestk.c
        treestk.h
        utype.h)
