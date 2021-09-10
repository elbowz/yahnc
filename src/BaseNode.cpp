#include "BaseNode.hpp"

const char BaseNode::cIndent[] = "  • ";

BaseNode::BaseNode(const char *id, const char *name, const char *type)
        : HomieNode(id, name, type) {
}
