#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

int main( int narg, char **opts) 

{
    char *conf = 0;
    xmlDocPtr doc;
    xmlNodePtr curr, child, root;

    if( narg < 2) exit( 1);

    conf = opts[ 1];

    doc = xmlParseFile( conf);
    if( !doc)
    {
        fprintf( stderr, "Parse error.\n");
        exit( 1);
    }

    root = xmlDocGetRootElement( doc);
    if( !root)
    {
        fprintf( stderr, "No document root.\n");
        xmlFreeDoc( doc);
        exit( 1);
    }

    printf( "Document root, name '%s'\n", root->name);

    for( child = root->children; child; child = child->next)
    {
        printf( "  Child '%s'\n", child->name);
    }

    exit( 0);
}
