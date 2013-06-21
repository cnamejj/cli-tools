#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

int print_xml_attrib( int, xmlAttr *);
int print_xml_element( int, char *, xmlNodePtr);

int print_xml_attrib( int level, xmlAttr *attrib)

{
    int ind, result = 0;

    if( attrib)
    {
        printf( "\n");
        for( ind=0; ind<level; ind++) printf( "  ");
        printf( "..Attr '%s'", attrib->name);
        if( attrib->children) print_xml_element( 0, "", attrib->children);
        else printf( "\n"); 

        result = print_xml_attrib( level, attrib->next);
    }

    return( result);
}

int print_xml_element( int level, char *mark, xmlNodePtr node) 

{
    int ind, result = 0;
    unsigned char *st = 0;

    if( node)
    {
        if( node->type != 3)
        {
            printf( "\n");
            for( ind=0; ind<level; ind++) printf( "  ");
            printf( "%s t=%d Name '%s'", mark, node->type, node->name);
	}

        if( node->content)
        {
            for( st=node->content; *st && (*st == ' ' || *st == '\n'); st++) ;
            if( *st)
            {
                printf( " '");
                for( ; *st; st++)
                  if( *st != '\n') printf( "%c", *st);
                printf( "'");
	    }
	}

        print_xml_attrib( level, node->properties);
        print_xml_element( level + 1, "Child", node->children);
        result = print_xml_element( level, "Next", node->next);
    }

    return( result);
}

int main( int narg, char **opts) 

{
    char *conf = 0;
    xmlDocPtr doc;
    xmlNodePtr root;

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

    print_xml_element( 1, "Next", root);
    printf( "\n");

    exit( 0);
}
