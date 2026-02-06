//
// Created by diogo on 27/01/26.
//
#include "Nova/parse/ast.h"

#include "Nova/memory/bumpAllocator.h"

using namespace nova;
    struct ASTNode {

        ASTNode() = default;
    };

    NovaNode nova_parse(NovaTokenSlice tok, NovaArena* arena ) {
        const Arena allocator(arena);
        return  static_cast<NovaNode>(allocator.alloc(sizeof(ASTNode)));
    }

