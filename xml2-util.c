/* Note...  Need to make sure GNM_FIRST does the right thing */

#ifndef SKIP_XML_PARTS
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "cli-sub.h"
#include "err_ref.h"

/* --- */

xmlNodePtr source_config_file( int *rc, char *config)

{
    xmlDocPtr doc;
    xmlNodePtr root = 0;

    doc = xmlParseFile( config);
    if( !doc) *rc = ERR_OPT_CONFIG;

    if( *rc == RC_NORMAL)
    {
        root = xmlDocGetRootElement( doc);
        if( !root) *rc = ERR_BAD_FORMAT;
    }

    return root;
}

/* --- */

xmlNodePtr get_node_match( xmlNodePtr head, char *node_name, int match_type)

{
    xmlNodePtr result = 0, walk = 0, last = 0;

    walk = head;
    if( match_type == GNM_NEXT && walk) walk = walk->next;

    for( ; walk && !result; walk = walk->next)
    {
        if( walk->name) if( !strcmp( (char *) walk->name, node_name))
        {
            if( match_type == GNM_LAST) last = walk;
            else result = walk;
	}
    }

    if( match_type == GNM_LAST) result = last;

    return result;
}

/* --- */

xmlNodePtr search_node_list( xmlNodePtr head, char *node_name, char *attrib_name, char *attrib_val)

{
    xmlNodePtr result = 0, walk = 0;
    xmlAttr *attrib_ent = 0;

    for( walk = head; walk && !result; walk = walk->next)
    {
        if( walk->name) if( !strcmp( (char *) walk->name, node_name))
        {
            attrib_ent = search_attrib_list( walk->properties, attrib_name, attrib_val);
            if( attrib_ent) result = walk;
	}
    }

    return result;
}

/* --- */

xmlAttr *search_attrib_list( xmlAttr *head, char *attrib_name, char *attrib_val)

{
    xmlAttr *result = 0, *walk = 0;

    for( walk = head; walk && !result; walk = walk->next)
      if( walk->name && walk->children)
        if( !strcmp( (char *) walk->name, attrib_name))
          if( walk->children->type == XML_TEXT_NODE && walk->children->content)
            if( !strcmp( (char *) walk->children->content, attrib_val))
              result = walk;

    return result;
}

/* --- */

char *dup_attrib_value( int *rc, xmlNodePtr curr, char *attrib_name)

{
    char *result = 0;
    xmlAttr *match = 0, *walk = 0;
    xmlNodePtr val;

    *rc = RC_NORMAL;

    for( walk = curr->properties; walk && !match; walk = walk->next)
      if( walk->name)
        if( !strcmp( (char *) walk->name, attrib_name))
          match = walk;

    if( !match) *rc = ERR_NO_MATCH;
    else if( match->children)
    {
        val = match->children;
        if( val->type == XML_TEXT_NODE && val->content)
        {
            result = strdup( (char *) val->content);
            if( !result) *rc = ERR_MALLOC_FAILED;
	}
    }

    return result;
}

/* --- */

char *get_attrib_value( int *rc, xmlNodePtr curr, char *attrib_name)

{
    char *result = 0;
    xmlAttr *match = 0, *walk = 0;
    xmlNodePtr val;

    *rc = RC_NORMAL;

    for( walk = curr->properties; walk && !match; walk = walk->next)
      if( walk->name)
        if( !strcmp( (char *) walk->name, attrib_name))
          match = walk;

    if( !match) *rc = ERR_NO_MATCH;
    else if( match->children)
    {
        val = match->children;
        if( val->type == XML_TEXT_NODE && val->content)
        {
            result = (char *) val->content;
	}
    }

    return result;
}
#endif
