#include "core/Parser.h"
#include <boost/algorithm/string/classification.hpp>

Parser::Parser(std::string html)
    : html_(std::move(html)),
      root(gumbo_parse(html_.c_str())->root)
{}

Parser::LinkContainer Parser::getLinks(const ParserSearchParams &params)
{
    LinkContainer container{};
    getLinks(root, params, container);
    return container;
}

void Parser::getLinks(const GumboNode *node, const ParserSearchParams &params, Parser::LinkContainer &container)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    GumboAttribute *href;
    if (href = gumbo_get_attribute(&node->v.element.attributes, params.attr.c_str());
        boost::is_any_of(params.tags)(node->v.element.tag) && href) {

        std::string link = href->value;
        if (!link.empty()) {
            container.emplace(std::move(link));
        }
    }

    const GumboVector *children = &node->v.element.children;
    for (size_t i = 0; i < children->length; ++i) {
        getLinks(static_cast<GumboNode *>(children->data[i]),
                 params,
                 container);
    }
}
