/**
 * @file
 *
 * @brief Document tree API
 *
 * Data structures and functions to build, modify, query and
 * serialize XML and HTML document trees. Also contains the
 * buffer API.
 *
 * @copyright See Copyright for the status of this software.
 *
 * @author Daniel Veillard
 */

#ifndef XML_TREE_INTERNALS

/*
 * Emulate circular dependency for backward compatibility
 */
#include <libxml/parser.h>

#else /* XML_TREE_INTERNALS */

#ifndef __XML_TREE_H__
#define __XML_TREE_H__

#include <stdio.h>
#include <limits.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlregexp.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Backward compatibility
 */
#define xmlBufferAllocScheme XML_BUFFER_ALLOC_EXACT
#define xmlDefaultBufferSize 4096
#define XML_GET_CONTENT(n) \
    ((n)->type == XML_ELEMENT_NODE ? NULL : (n)->content)
#define XML_GET_LINE(n)	xmlGetLineNo(n)

/*
 * Some of the basic types pointer to structures:
 */
/* xmlIO.h */
typedef struct _xmlParserInputBuffer xmlParserInputBuffer;
typedef xmlParserInputBuffer *xmlParserInputBufferPtr;

typedef struct _xmlOutputBuffer xmlOutputBuffer;
typedef xmlOutputBuffer *xmlOutputBufferPtr;

/* parser.h */
typedef struct _xmlParserInput xmlParserInput;
typedef xmlParserInput *xmlParserInputPtr;

typedef struct _xmlParserCtxt xmlParserCtxt;
typedef xmlParserCtxt *xmlParserCtxtPtr;

typedef struct _xmlSAXLocator xmlSAXLocator;
typedef xmlSAXLocator *xmlSAXLocatorPtr;

typedef struct _xmlSAXHandler xmlSAXHandler;
typedef xmlSAXHandler *xmlSAXHandlerPtr;

/* entities.h */
typedef struct _xmlEntity xmlEntity;
typedef xmlEntity *xmlEntityPtr;

/*
 * Removed, buffers always use XML_BUFFER_ALLOC_IO now.
 */
typedef enum {
    XML_BUFFER_ALLOC_DOUBLEIT,	/* double each time one need to grow */
    XML_BUFFER_ALLOC_EXACT,	/* grow only to the minimal size */
    XML_BUFFER_ALLOC_IMMUTABLE, /* immutable buffer, deprecated */
    XML_BUFFER_ALLOC_IO,	/* special allocation scheme used for I/O */
    XML_BUFFER_ALLOC_HYBRID,	/* exact up to a threshold, and doubleit thereafter */
    XML_BUFFER_ALLOC_BOUNDED	/* limit the upper size of the buffer */
} xmlBufferAllocationScheme;

typedef struct _xmlBuffer xmlBuffer;
typedef xmlBuffer *xmlBufferPtr;
/**
 * A buffer structure, this old construct is limited to 2GB and
 * is being deprecated, use API with xmlBuf instead.
 */
struct _xmlBuffer {
    /* The buffer content UTF8 */
    xmlChar *content XML_DEPRECATED_MEMBER;
    /* The buffer size used */
    unsigned int use XML_DEPRECATED_MEMBER;
    /* The buffer size */
    unsigned int size XML_DEPRECATED_MEMBER;
    /* The realloc method */
    xmlBufferAllocationScheme alloc XML_DEPRECATED_MEMBER;
    /* in IO mode we may have a different base */
    xmlChar *contentIO XML_DEPRECATED_MEMBER;
};

/**
 * A buffer structure, new one, the actual structure internals are not public
 */
typedef struct _xmlBuf xmlBuf;
typedef xmlBuf *xmlBufPtr;

/*
 * Macro used to express that the API use the new buffers for
 * xmlParserInputBuffer and xmlOutputBuffer. The change was
 * introduced in 2.9.0.
 */
#define LIBXML2_NEW_BUFFER

/**
 * This is the namespace for the special xml: prefix predefined in the
 * XML Namespace specification.
 */
#define XML_XML_NAMESPACE \
    (const xmlChar *) "http://www.w3.org/XML/1998/namespace"

/**
 * This is the name for the special xml:id attribute
 */
#define XML_XML_ID (const xmlChar *) "xml:id"

/**
 * The different element types carried by an XML tree.
 *
 * NOTE: This is synchronized with DOM Level1 values
 *       See http://www.w3.org/TR/REC-DOM-Level-1/
 *
 * Actually this had diverged a bit, and now XML_DOCUMENT_TYPE_NODE should
 * be deprecated to use an XML_DTD_NODE.
 */
typedef enum {
    /** element */
    XML_ELEMENT_NODE=		1,
    /** attribute */
    XML_ATTRIBUTE_NODE=		2,
    /** text */
    XML_TEXT_NODE=		3,
    /** CDATA section */
    XML_CDATA_SECTION_NODE=	4,
    /** entity reference */
    XML_ENTITY_REF_NODE=	5,
    /** unused */
    XML_ENTITY_NODE=		6,
    /** processing instruction */
    XML_PI_NODE=		7,
    /** comment */
    XML_COMMENT_NODE=		8,
    /** document */
    XML_DOCUMENT_NODE=		9,
    /** unused */
    XML_DOCUMENT_TYPE_NODE=	10,
    /** document fragment */
    XML_DOCUMENT_FRAG_NODE=	11,
    /** notation, unused */
    XML_NOTATION_NODE=		12,
    /** HTML document */
    XML_HTML_DOCUMENT_NODE=	13,
    /** DTD */
    XML_DTD_NODE=		14,
    /** element declaration */
    XML_ELEMENT_DECL=		15,
    /** attribute declaration */
    XML_ATTRIBUTE_DECL=		16,
    /** entity declaration */
    XML_ENTITY_DECL=		17,
    /** XPath namespace node */
    XML_NAMESPACE_DECL=		18,
    /** XInclude start marker */
    XML_XINCLUDE_START=		19,
    /** XInclude end marker */
    XML_XINCLUDE_END=		20
    /* XML_DOCB_DOCUMENT_NODE=	21 */ /* removed */
} xmlElementType;

/** @cond IGNORE */
/* For backward compatibility */
#define XML_DOCB_DOCUMENT_NODE 21
/** @endcond */

typedef struct _xmlNotation xmlNotation;
typedef xmlNotation *xmlNotationPtr;
/**
 * A DTD Notation definition.
 */
struct _xmlNotation {
    /** Notation name */
    const xmlChar               *name;
    /** Public identifier, if any */
    const xmlChar               *PublicID;
    /** System identifier, if any */
    const xmlChar               *SystemID;
};

/*
 * A DTD Attribute type definition.
 */
typedef enum {
    XML_ATTRIBUTE_CDATA = 1,
    XML_ATTRIBUTE_ID,
    XML_ATTRIBUTE_IDREF	,
    XML_ATTRIBUTE_IDREFS,
    XML_ATTRIBUTE_ENTITY,
    XML_ATTRIBUTE_ENTITIES,
    XML_ATTRIBUTE_NMTOKEN,
    XML_ATTRIBUTE_NMTOKENS,
    XML_ATTRIBUTE_ENUMERATION,
    XML_ATTRIBUTE_NOTATION
} xmlAttributeType;

/*
 * A DTD Attribute default definition.
 */
typedef enum {
    XML_ATTRIBUTE_NONE = 1,
    XML_ATTRIBUTE_REQUIRED,
    XML_ATTRIBUTE_IMPLIED,
    XML_ATTRIBUTE_FIXED
} xmlAttributeDefault;

typedef struct _xmlEnumeration xmlEnumeration;
typedef xmlEnumeration *xmlEnumerationPtr;
/**
 * List structure used when there is an enumeration in DTDs.
 */
struct _xmlEnumeration {
    struct _xmlEnumeration    *next;	/* next one */
    const xmlChar            *name;	/* Enumeration name */
};

typedef struct _xmlAttribute xmlAttribute;
typedef xmlAttribute *xmlAttributePtr;
/**
 * An Attribute declaration in a DTD.
 */
struct _xmlAttribute {
    /** application data */
    void           *_private;
    /** XML_ATTRIBUTE_DECL */
    xmlElementType          type;
    /** attribute name */
    const xmlChar          *name;
    /** NULL */
    struct _xmlNode    *children;
    /** NULL */
    struct _xmlNode        *last;
    /** DTD */
    struct _xmlDtd       *parent;
    /** next sibling */
    struct _xmlNode        *next;
    /** previous sibling */
    struct _xmlNode        *prev;
    /** containing document */
    struct _xmlDoc          *doc;

    /** next in hash table */
    struct _xmlAttribute  *nexth;
    /** attribute type */
    xmlAttributeType       atype;
    /** attribute default */
    xmlAttributeDefault      def;
    /** default value */
    const xmlChar  *defaultValue;
    /** enumeration tree if any */
    xmlEnumerationPtr       tree;
    /** namespace prefix if any */
    const xmlChar        *prefix;
    /** element name */
    const xmlChar          *elem;
};

/*
 * Possible definitions of element content types.
 */
typedef enum {
    XML_ELEMENT_CONTENT_PCDATA = 1,
    XML_ELEMENT_CONTENT_ELEMENT,
    XML_ELEMENT_CONTENT_SEQ,
    XML_ELEMENT_CONTENT_OR
} xmlElementContentType;

/*
 * Possible definitions of element content occurrences.
 */
typedef enum {
    XML_ELEMENT_CONTENT_ONCE = 1,
    XML_ELEMENT_CONTENT_OPT,
    XML_ELEMENT_CONTENT_MULT,
    XML_ELEMENT_CONTENT_PLUS
} xmlElementContentOccur;

typedef struct _xmlElementContent xmlElementContent;
typedef xmlElementContent *xmlElementContentPtr;
/**
 * An XML Element content as stored after parsing an element definition
 * in a DTD.
 */
struct _xmlElementContent {
    /** PCDATA, ELEMENT, SEQ or OR */
    xmlElementContentType     type;
    /** ONCE, OPT, MULT or PLUS */
    xmlElementContentOccur    ocur;
    /** element name */
    const xmlChar             *name;
    /** first child */
    struct _xmlElementContent *c1;
    /** second child */
    struct _xmlElementContent *c2;
    /** parent */
    struct _xmlElementContent *parent;
    /** namespace prefix */
    const xmlChar             *prefix;
};

/*
 * The different possibilities for an element content type.
 */
typedef enum {
    XML_ELEMENT_TYPE_UNDEFINED = 0,
    XML_ELEMENT_TYPE_EMPTY = 1,
    XML_ELEMENT_TYPE_ANY,
    XML_ELEMENT_TYPE_MIXED,
    XML_ELEMENT_TYPE_ELEMENT
} xmlElementTypeVal;

typedef struct _xmlElement xmlElement;
typedef xmlElement *xmlElementPtr;
/**
 * An XML Element declaration from a DTD.
 */
struct _xmlElement {
    /** application data */
    void           *_private;
    /** XML_ELEMENT_DECL */
    xmlElementType          type;
    /** element name */
    const xmlChar          *name;
    /** NULL */
    struct _xmlNode    *children;
    /** NULL */
    struct _xmlNode        *last;
    /** -> DTD */
    struct _xmlDtd       *parent;
    /** next sibling */
    struct _xmlNode        *next;
    /** previous sibling */
    struct _xmlNode        *prev;
    /** containing document */
    struct _xmlDoc          *doc;

    /** element type */
    xmlElementTypeVal      etype;
    /** allowed element content */
    xmlElementContentPtr content;
    /** list of declared attributes */
    xmlAttributePtr   attributes;
    /** namespace prefix if any */
    const xmlChar        *prefix;
#ifdef LIBXML_REGEXP_ENABLED
    /** validating regexp */
    xmlRegexpPtr       contModel;
#else
    void	      *contModel;
#endif
};


/**
 * A namespace declaration node.
 */
#define XML_LOCAL_NAMESPACE XML_NAMESPACE_DECL
typedef xmlElementType xmlNsType;

typedef struct _xmlNs xmlNs;
typedef xmlNs *xmlNsPtr;
/**
 * An XML namespace.
 * Note that prefix == NULL is valid, it defines the default namespace
 * within the subtree (until overridden).
 *
 * xmlNsType is unified with xmlElementType.
 *
 * Note that the XPath engine returns XPath namespace nodes as
 * xmlNs cast to xmlNode. This is a terrible design decision that
 * can easily cause type confusion errors.
 */
struct _xmlNs {
    /** next namespace */
    struct _xmlNs  *next;
    /** XML_NAMESPACE_DECL */
    xmlNsType      type;
    /** namespace URI */
    const xmlChar *href;
    /** namespace prefix */
    const xmlChar *prefix;
    /** application data */
    void           *_private;
    /** normally an xmlDoc */
    struct _xmlDoc *context;
};

typedef struct _xmlDtd xmlDtd;
typedef xmlDtd *xmlDtdPtr;
/**
 * An XML DTD, as defined by <!DOCTYPE ... There is actually one for
 * the internal subset and for the external subset.
 */
struct _xmlDtd {
    /** application data */
    void           *_private;
    /** XML_DTD_NODE */
    xmlElementType  type;
    /** name of the DTD */
    const xmlChar *name;
    /** first child */
    struct _xmlNode *children;
    /** last child */
    struct _xmlNode *last;
    /** parent node */
    struct _xmlDoc  *parent;
    /** next sibling */
    struct _xmlNode *next;
    /** previous sibling */
    struct _xmlNode *prev;
    /** containing document */
    struct _xmlDoc  *doc;

    /* End of common part */

    /** hash table for notations if any */
    void          *notations;
    /** hash table for elements if any */
    void          *elements;
    /** hash table for attributes if any */
    void          *attributes;
    /** hash table for entities if any */
    void          *entities;
    /** public identifier */
    const xmlChar *ExternalID;
    /** system identifier */
    const xmlChar *SystemID;
    /** hash table for parameter entities if any */
    void          *pentities;
};

typedef struct _xmlAttr xmlAttr;
typedef xmlAttr *xmlAttrPtr;
/**
 * An attribute on an XML node.
 */
struct _xmlAttr {
    /** application data */
    void           *_private;
    /** XML_ATTRIBUTE_NODE */
    xmlElementType   type;
    /** local name */
    const xmlChar   *name;
    /** first child */
    struct _xmlNode *children;
    /** last child */
    struct _xmlNode *last;
    /** parent node */
    struct _xmlNode *parent;
    /** next sibling */
    struct _xmlAttr *next;
    /** previous sibling */
    struct _xmlAttr *prev;
    /** containing document */
    struct _xmlDoc  *doc;
    /** namespace if any */
    xmlNs           *ns;
    /** attribute type if validating */
    xmlAttributeType atype;
    /** for type/PSVI information */
    void            *psvi;
    /** ID struct if any */
    struct _xmlID   *id;
};

typedef struct _xmlID xmlID;
typedef xmlID *xmlIDPtr;
/*
 * An XML ID instance.
 */
struct _xmlID {
    /* next ID */
    struct _xmlID    *next;
    /* The ID name */
    const xmlChar    *value;
    /* The attribute holding it */
    xmlAttrPtr        attr;
    /* The attribute if attr is not available */
    const xmlChar    *name;
    /* The line number if attr is not available */
    int               lineno;
    /* The document holding the ID */
    struct _xmlDoc   *doc;
};

typedef struct _xmlRef xmlRef;
typedef xmlRef *xmlRefPtr;
/*
 * An XML IDREF instance.
 */
struct _xmlRef {
    struct _xmlRef    *next;	/* next Ref */
    const xmlChar     *value;	/* The Ref name */
    xmlAttrPtr        attr;	/* The attribute holding it */
    const xmlChar    *name;	/* The attribute if attr is not available */
    int               lineno;	/* The line number if attr is not available */
};

typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;
/**
 * A node in an XML or HTML tree.
 *
 * This is used for
 *
 * - XML_ELEMENT_NODE
 * - XML_TEXT_NODE
 * - XML_CDATA_SECTION_NODE
 * - XML_ENTITY_REF_NODE
 * - XML_PI_NODE
 * - XML_COMMENT_NODE
 * - XML_XINCLUDE_START_NODE
 * - XML_XINCLUDE_END_NODE
 */
struct _xmlNode {
    /** application data */
    void           *_private;
    /** type enum */
    xmlElementType   type;
    /** local name for elements */
    const xmlChar   *name;
    /** first child */
    struct _xmlNode *children;
    /** last child */
    struct _xmlNode *last;
    /** parent node */
    struct _xmlNode *parent;
    /** next sibling */
    struct _xmlNode *next;
    /** previous sibling */
    struct _xmlNode *prev;
    /** containing document */
    struct _xmlDoc  *doc;

    /* End of common part */

    /** namespace if any */
    xmlNs           *ns;
    /** content of text, comment, PI nodes */
    xmlChar         *content;
    /** attributes for elements */
    struct _xmlAttr *properties;
    /** namespace definitions on this node */
    xmlNs           *nsDef;
    /** for type/PSVI information */
    void            *psvi;
    /** line number */
    unsigned short   line;
    /** extra data for XPath/XSLT */
    unsigned short   extra;
};

/**
 * Set of properties of the document as found by the parser
 * Some of them are linked to similarly named xmlParserOption
 */
typedef enum {
    /** document is XML well formed */
    XML_DOC_WELLFORMED		= 1<<0,
    /** document is Namespace valid */
    XML_DOC_NSVALID		= 1<<1,
    /** parsed with old XML-1.0 parser */
    XML_DOC_OLD10		= 1<<2,
    /** DTD validation was successful */
    XML_DOC_DTDVALID		= 1<<3,
    /** XInclude substitution was done */
    XML_DOC_XINCLUDE		= 1<<4,
    /** Document was built using the API and not by parsing an instance */
    XML_DOC_USERBUILT		= 1<<5,
    /** built for internal processing */
    XML_DOC_INTERNAL		= 1<<6,
    /** parsed or built HTML document */
    XML_DOC_HTML		= 1<<7
} xmlDocProperties;

typedef struct _xmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;
/**
 * An XML or HTML document.
 */
struct _xmlDoc {
    /** application data */
    void           *_private;
    /** XML_DOCUMENT_NODE or XML_HTML_DOCUMENT_NODE */
    xmlElementType  type;
    /** NULL */
    char           *name;
    /** first child */
    struct _xmlNode *children;
    /** last child */
    struct _xmlNode *last;
    /** parent node */
    struct _xmlNode *parent;
    /** next sibling */
    struct _xmlNode *next;
    /** previous sibling */
    struct _xmlNode *prev;
    /** reference to itself */
    struct _xmlDoc  *doc;

    /* End of common part */

    /** level of zlib compression */
    int             compression;
    /**
     * standalone document (no external refs)
     *
     * - 1 if standalone="yes",
     * - 0 if standalone="no",
     * - -1 if there is no XML declaration,
     * - -2 if there is an XML declaration, but no
     *   standalone attribute was specified
     */
    int             standalone;
    /** internal subset */
    struct _xmlDtd  *intSubset;
    /** external subset */
    struct _xmlDtd  *extSubset;
    /** used to hold the XML namespace if needed */
    struct _xmlNs   *oldNs;
    /** version string from XML declaration */
    const xmlChar  *version;
    /** actual encoding if any */
    const xmlChar  *encoding;
    /** hash table for ID attributes if any */
    void           *ids;
    /** hash table for IDREFs attributes if any */
    void           *refs;
    /** URI of the document */
    const xmlChar  *URL;
    /** unused */
    int             charset;
    /** dict used to allocate names if any */
    struct _xmlDict *dict;
    /** for type/PSVI information */
    void           *psvi;
    /** xmlParserOption enum used to parse the document */
    int             parseFlags;
    /** xmlDocProperties of the document */
    int             properties;
};


typedef struct _xmlDOMWrapCtxt xmlDOMWrapCtxt;
typedef xmlDOMWrapCtxt *xmlDOMWrapCtxtPtr;

/**
 * A function called to acquire namespaces (xmlNs) from the wrapper.
 *
 * @param ctxt  a DOM wrapper context
 * @param node  the context node (element or attribute)
 * @param nsName  the requested namespace name
 * @param nsPrefix  the requested namespace prefix
 * @returns an xmlNsPtr or NULL in case of an error.
 */
typedef xmlNsPtr (*xmlDOMWrapAcquireNsFunction) (xmlDOMWrapCtxtPtr ctxt,
						 xmlNodePtr node,
						 const xmlChar *nsName,
						 const xmlChar *nsPrefix);

/*
 * Context for DOM wrapper-operations.
 */
struct _xmlDOMWrapCtxt {
    void * _private;
    /*
    * The type of this context, just in case we need specialized
    * contexts in the future.
    */
    int type;
    /*
    * Internal namespace map used for various operations.
    */
    void * namespaceMap;
    /*
    * Use this one to acquire an xmlNsPtr intended for node->ns.
    * (Note that this is not intended for elem->nsDef).
    */
    xmlDOMWrapAcquireNsFunction getNsForNodeFunc;
};

/**
 * Signature for the registration callback of a created node
 *
 * @param node  the current node
 */
typedef void (*xmlRegisterNodeFunc) (xmlNodePtr node);

/**
 * Signature for the deregistration callback of a discarded node
 *
 * @param node  the current node
 */
typedef void (*xmlDeregisterNodeFunc) (xmlNodePtr node);

/**
 * Macro for compatibility naming layer with libxml1. Maps
 * to "children."
 */
#ifndef xmlChildrenNode
#define xmlChildrenNode children
#endif

/**
 * Macro for compatibility naming layer with libxml1. Maps
 * to "children".
 */
#ifndef xmlRootNode
#define xmlRootNode children
#endif

/*
 * Variables.
 */

XML_DEPRECATED
XMLPUBFUN xmlRegisterNodeFunc *__xmlRegisterNodeDefaultValue(void);
XML_DEPRECATED
XMLPUBFUN xmlDeregisterNodeFunc *__xmlDeregisterNodeDefaultValue(void);

#ifndef XML_GLOBALS_NO_REDEFINITION
  #define xmlRegisterNodeDefaultValue \
    (*__xmlRegisterNodeDefaultValue())
  #define xmlDeregisterNodeDefaultValue \
    (*__xmlDeregisterNodeDefaultValue())
#endif

/*
 * Some helper functions
 */
XMLPUBFUN int
		xmlValidateNCName	(const xmlChar *value,
					 int space);

XMLPUBFUN int
		xmlValidateQName	(const xmlChar *value,
					 int space);
XMLPUBFUN int
		xmlValidateName		(const xmlChar *value,
					 int space);
XMLPUBFUN int
		xmlValidateNMToken	(const xmlChar *value,
					 int space);

XMLPUBFUN xmlChar *
		xmlBuildQName		(const xmlChar *ncname,
					 const xmlChar *prefix,
					 xmlChar *memory,
					 int len);
XMLPUBFUN xmlChar *
		xmlSplitQName2		(const xmlChar *name,
					 xmlChar **prefix);
XMLPUBFUN const xmlChar *
		xmlSplitQName3		(const xmlChar *name,
					 int *len);

/*
 * Creating/freeing new structures.
 */
XMLPUBFUN xmlDtdPtr
		xmlCreateIntSubset	(xmlDocPtr doc,
					 const xmlChar *name,
					 const xmlChar *ExternalID,
					 const xmlChar *SystemID);
XMLPUBFUN xmlDtdPtr
		xmlNewDtd		(xmlDocPtr doc,
					 const xmlChar *name,
					 const xmlChar *ExternalID,
					 const xmlChar *SystemID);
XMLPUBFUN xmlDtdPtr
		xmlGetIntSubset		(const xmlDoc *doc);
XMLPUBFUN void
		xmlFreeDtd		(xmlDtdPtr cur);
XMLPUBFUN xmlNsPtr
		xmlNewNs		(xmlNodePtr node,
					 const xmlChar *href,
					 const xmlChar *prefix);
XMLPUBFUN void
		xmlFreeNs		(xmlNsPtr cur);
XMLPUBFUN void
		xmlFreeNsList		(xmlNsPtr cur);
XMLPUBFUN xmlDocPtr
		xmlNewDoc		(const xmlChar *version);
XMLPUBFUN void
		xmlFreeDoc		(xmlDocPtr cur);
XMLPUBFUN xmlAttrPtr
		xmlNewDocProp		(xmlDocPtr doc,
					 const xmlChar *name,
					 const xmlChar *value);
XMLPUBFUN xmlAttrPtr
		xmlNewProp		(xmlNodePtr node,
					 const xmlChar *name,
					 const xmlChar *value);
XMLPUBFUN xmlAttrPtr
		xmlNewNsProp		(xmlNodePtr node,
					 xmlNsPtr ns,
					 const xmlChar *name,
					 const xmlChar *value);
XMLPUBFUN xmlAttrPtr
		xmlNewNsPropEatName	(xmlNodePtr node,
					 xmlNsPtr ns,
					 xmlChar *name,
					 const xmlChar *value);
XMLPUBFUN void
		xmlFreePropList		(xmlAttrPtr cur);
XMLPUBFUN void
		xmlFreeProp		(xmlAttrPtr cur);
XMLPUBFUN xmlAttrPtr
		xmlCopyProp		(xmlNodePtr target,
					 xmlAttrPtr cur);
XMLPUBFUN xmlAttrPtr
		xmlCopyPropList		(xmlNodePtr target,
					 xmlAttrPtr cur);
XMLPUBFUN xmlDtdPtr
		xmlCopyDtd		(xmlDtdPtr dtd);
XMLPUBFUN xmlDocPtr
		xmlCopyDoc		(xmlDocPtr doc,
					 int recursive);
/*
 * Creating new nodes.
 */
XMLPUBFUN xmlNodePtr
		xmlNewDocNode		(xmlDocPtr doc,
					 xmlNsPtr ns,
					 const xmlChar *name,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewDocNodeEatName	(xmlDocPtr doc,
					 xmlNsPtr ns,
					 xmlChar *name,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewNode		(xmlNsPtr ns,
					 const xmlChar *name);
XMLPUBFUN xmlNodePtr
		xmlNewNodeEatName	(xmlNsPtr ns,
					 xmlChar *name);
XMLPUBFUN xmlNodePtr
		xmlNewChild		(xmlNodePtr parent,
					 xmlNsPtr ns,
					 const xmlChar *name,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewDocText		(const xmlDoc *doc,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewText		(const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewDocPI		(xmlDocPtr doc,
					 const xmlChar *name,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewPI		(const xmlChar *name,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewDocTextLen	(xmlDocPtr doc,
					 const xmlChar *content,
					 int len);
XMLPUBFUN xmlNodePtr
		xmlNewTextLen		(const xmlChar *content,
					 int len);
XMLPUBFUN xmlNodePtr
		xmlNewDocComment	(xmlDocPtr doc,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewComment		(const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewCDataBlock	(xmlDocPtr doc,
					 const xmlChar *content,
					 int len);
XMLPUBFUN xmlNodePtr
		xmlNewCharRef		(xmlDocPtr doc,
					 const xmlChar *name);
XMLPUBFUN xmlNodePtr
		xmlNewReference		(const xmlDoc *doc,
					 const xmlChar *name);
XMLPUBFUN xmlNodePtr
		xmlCopyNode		(xmlNodePtr node,
					 int recursive);
XMLPUBFUN xmlNodePtr
		xmlDocCopyNode		(xmlNodePtr node,
					 xmlDocPtr doc,
					 int recursive);
XMLPUBFUN xmlNodePtr
		xmlDocCopyNodeList	(xmlDocPtr doc,
					 xmlNodePtr node);
XMLPUBFUN xmlNodePtr
		xmlCopyNodeList		(xmlNodePtr node);
XMLPUBFUN xmlNodePtr
		xmlNewTextChild		(xmlNodePtr parent,
					 xmlNsPtr ns,
					 const xmlChar *name,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewDocRawNode	(xmlDocPtr doc,
					 xmlNsPtr ns,
					 const xmlChar *name,
					 const xmlChar *content);
XMLPUBFUN xmlNodePtr
		xmlNewDocFragment	(xmlDocPtr doc);

/*
 * Navigating.
 */
XMLPUBFUN long
		xmlGetLineNo		(const xmlNode *node);
XMLPUBFUN xmlChar *
		xmlGetNodePath		(const xmlNode *node);
XMLPUBFUN xmlNodePtr
		xmlDocGetRootElement	(const xmlDoc *doc);
XMLPUBFUN xmlNodePtr
		xmlGetLastChild		(const xmlNode *parent);
XMLPUBFUN int
		xmlNodeIsText		(const xmlNode *node);
XMLPUBFUN int
		xmlIsBlankNode		(const xmlNode *node);

/*
 * Changing the structure.
 */
XMLPUBFUN xmlNodePtr
		xmlDocSetRootElement	(xmlDocPtr doc,
					 xmlNodePtr root);
XMLPUBFUN void
		xmlNodeSetName		(xmlNodePtr cur,
					 const xmlChar *name);
XMLPUBFUN xmlNodePtr
		xmlAddChild		(xmlNodePtr parent,
					 xmlNodePtr cur);
XMLPUBFUN xmlNodePtr
		xmlAddChildList		(xmlNodePtr parent,
					 xmlNodePtr cur);
XMLPUBFUN xmlNodePtr
		xmlReplaceNode		(xmlNodePtr old,
					 xmlNodePtr cur);
XMLPUBFUN xmlNodePtr
		xmlAddPrevSibling	(xmlNodePtr cur,
					 xmlNodePtr elem);
XMLPUBFUN xmlNodePtr
		xmlAddSibling		(xmlNodePtr cur,
					 xmlNodePtr elem);
XMLPUBFUN xmlNodePtr
		xmlAddNextSibling	(xmlNodePtr cur,
					 xmlNodePtr elem);
XMLPUBFUN void
		xmlUnlinkNode		(xmlNodePtr cur);
XMLPUBFUN xmlNodePtr
		xmlTextMerge		(xmlNodePtr first,
					 xmlNodePtr second);
XMLPUBFUN int
		xmlTextConcat		(xmlNodePtr node,
					 const xmlChar *content,
					 int len);
XMLPUBFUN void
		xmlFreeNodeList		(xmlNodePtr cur);
XMLPUBFUN void
		xmlFreeNode		(xmlNodePtr cur);
XMLPUBFUN int
		xmlSetTreeDoc		(xmlNodePtr tree,
					 xmlDocPtr doc);
XMLPUBFUN int
		xmlSetListDoc		(xmlNodePtr list,
					 xmlDocPtr doc);
/*
 * Namespaces.
 */
XMLPUBFUN xmlNsPtr
		xmlSearchNs		(xmlDocPtr doc,
					 xmlNodePtr node,
					 const xmlChar *nameSpace);
XMLPUBFUN xmlNsPtr
		xmlSearchNsByHref	(xmlDocPtr doc,
					 xmlNodePtr node,
					 const xmlChar *href);
XMLPUBFUN int
		xmlGetNsListSafe	(const xmlDoc *doc,
					 const xmlNode *node,
					 xmlNsPtr **out);
XMLPUBFUN xmlNsPtr *
		xmlGetNsList		(const xmlDoc *doc,
					 const xmlNode *node);

XMLPUBFUN void
		xmlSetNs		(xmlNodePtr node,
					 xmlNsPtr ns);
XMLPUBFUN xmlNsPtr
		xmlCopyNamespace	(xmlNsPtr cur);
XMLPUBFUN xmlNsPtr
		xmlCopyNamespaceList	(xmlNsPtr cur);

/*
 * Changing the content.
 */
XMLPUBFUN xmlAttrPtr
		xmlSetProp		(xmlNodePtr node,
					 const xmlChar *name,
					 const xmlChar *value);
XMLPUBFUN xmlAttrPtr
		xmlSetNsProp		(xmlNodePtr node,
					 xmlNsPtr ns,
					 const xmlChar *name,
					 const xmlChar *value);
XMLPUBFUN int
		xmlNodeGetAttrValue	(const xmlNode *node,
					 const xmlChar *name,
					 const xmlChar *nsUri,
					 xmlChar **out);
XMLPUBFUN xmlChar *
		xmlGetNoNsProp		(const xmlNode *node,
					 const xmlChar *name);
XMLPUBFUN xmlChar *
		xmlGetProp		(const xmlNode *node,
					 const xmlChar *name);
XMLPUBFUN xmlAttrPtr
		xmlHasProp		(const xmlNode *node,
					 const xmlChar *name);
XMLPUBFUN xmlAttrPtr
		xmlHasNsProp		(const xmlNode *node,
					 const xmlChar *name,
					 const xmlChar *nameSpace);
XMLPUBFUN xmlChar *
		xmlGetNsProp		(const xmlNode *node,
					 const xmlChar *name,
					 const xmlChar *nameSpace);
XMLPUBFUN xmlNodePtr
		xmlStringGetNodeList	(const xmlDoc *doc,
					 const xmlChar *value);
XMLPUBFUN xmlNodePtr
		xmlStringLenGetNodeList	(const xmlDoc *doc,
					 const xmlChar *value,
					 int len);
XMLPUBFUN xmlChar *
		xmlNodeListGetString	(xmlDocPtr doc,
					 const xmlNode *list,
					 int inLine);
XMLPUBFUN xmlChar *
		xmlNodeListGetRawString	(const xmlDoc *doc,
					 const xmlNode *list,
					 int inLine);
XMLPUBFUN int
		xmlNodeSetContent	(xmlNodePtr cur,
					 const xmlChar *content);
XMLPUBFUN int
		xmlNodeSetContentLen	(xmlNodePtr cur,
					 const xmlChar *content,
					 int len);
XMLPUBFUN int
		xmlNodeAddContent	(xmlNodePtr cur,
					 const xmlChar *content);
XMLPUBFUN int
		xmlNodeAddContentLen	(xmlNodePtr cur,
					 const xmlChar *content,
					 int len);
XMLPUBFUN xmlChar *
		xmlNodeGetContent	(const xmlNode *cur);

XMLPUBFUN int
		xmlNodeBufGetContent	(xmlBufferPtr buffer,
					 const xmlNode *cur);
XMLPUBFUN int
		xmlBufGetNodeContent	(xmlBufPtr buf,
					 const xmlNode *cur);

XMLPUBFUN xmlChar *
		xmlNodeGetLang		(const xmlNode *cur);
XMLPUBFUN int
		xmlNodeGetSpacePreserve	(const xmlNode *cur);
XMLPUBFUN int
		xmlNodeSetLang		(xmlNodePtr cur,
					 const xmlChar *lang);
XMLPUBFUN int
		xmlNodeSetSpacePreserve (xmlNodePtr cur,
					 int val);
XMLPUBFUN int
		xmlNodeGetBaseSafe	(const xmlDoc *doc,
					 const xmlNode *cur,
					 xmlChar **baseOut);
XMLPUBFUN xmlChar *
		xmlNodeGetBase		(const xmlDoc *doc,
					 const xmlNode *cur);
XMLPUBFUN int
		xmlNodeSetBase		(xmlNodePtr cur,
					 const xmlChar *uri);

/*
 * Removing content.
 */
XMLPUBFUN int
		xmlRemoveProp		(xmlAttrPtr cur);
XMLPUBFUN int
		xmlUnsetNsProp		(xmlNodePtr node,
					 xmlNsPtr ns,
					 const xmlChar *name);
XMLPUBFUN int
		xmlUnsetProp		(xmlNodePtr node,
					 const xmlChar *name);

#ifdef LIBXML_OUTPUT_ENABLED
XMLPUBFUN void xmlAttrSerializeTxtContent(xmlBufferPtr buf,
					 xmlDocPtr doc,
					 xmlAttrPtr attr,
					 const xmlChar *string);
#endif /* LIBXML_OUTPUT_ENABLED */

/*
 * Namespace handling.
 */
XMLPUBFUN int
		xmlReconciliateNs	(xmlDocPtr doc,
					 xmlNodePtr tree);

#ifdef LIBXML_OUTPUT_ENABLED
/*
 * Saving.
 */
XMLPUBFUN void
		xmlDocDumpFormatMemory	(xmlDocPtr cur,
					 xmlChar **mem,
					 int *size,
					 int format);
XMLPUBFUN void
		xmlDocDumpMemory	(xmlDocPtr cur,
					 xmlChar **mem,
					 int *size);
XMLPUBFUN void
		xmlDocDumpMemoryEnc	(xmlDocPtr out_doc,
					 xmlChar **doc_txt_ptr,
					 int * doc_txt_len,
					 const char *txt_encoding);
XMLPUBFUN void
		xmlDocDumpFormatMemoryEnc(xmlDocPtr out_doc,
					 xmlChar **doc_txt_ptr,
					 int * doc_txt_len,
					 const char *txt_encoding,
					 int format);
XMLPUBFUN int
		xmlDocFormatDump	(FILE *f,
					 xmlDocPtr cur,
					 int format);
XMLPUBFUN int
		xmlDocDump		(FILE *f,
					 xmlDocPtr cur);
XMLPUBFUN void
		xmlElemDump		(FILE *f,
					 xmlDocPtr doc,
					 xmlNodePtr cur);
XMLPUBFUN int
		xmlSaveFile		(const char *filename,
					 xmlDocPtr cur);
XMLPUBFUN int
		xmlSaveFormatFile	(const char *filename,
					 xmlDocPtr cur,
					 int format);
XMLPUBFUN size_t
		xmlBufNodeDump		(xmlBufPtr buf,
					 xmlDocPtr doc,
					 xmlNodePtr cur,
					 int level,
					 int format);
XMLPUBFUN int
		xmlNodeDump		(xmlBufferPtr buf,
					 xmlDocPtr doc,
					 xmlNodePtr cur,
					 int level,
					 int format);

XMLPUBFUN int
		xmlSaveFileTo		(xmlOutputBufferPtr buf,
					 xmlDocPtr cur,
					 const char *encoding);
XMLPUBFUN int
		xmlSaveFormatFileTo     (xmlOutputBufferPtr buf,
					 xmlDocPtr cur,
				         const char *encoding,
				         int format);
XMLPUBFUN void
		xmlNodeDumpOutput	(xmlOutputBufferPtr buf,
					 xmlDocPtr doc,
					 xmlNodePtr cur,
					 int level,
					 int format,
					 const char *encoding);

XMLPUBFUN int
		xmlSaveFormatFileEnc    (const char *filename,
					 xmlDocPtr cur,
					 const char *encoding,
					 int format);

XMLPUBFUN int
		xmlSaveFileEnc		(const char *filename,
					 xmlDocPtr cur,
					 const char *encoding);

#endif /* LIBXML_OUTPUT_ENABLED */
/*
 * XHTML
 */
XMLPUBFUN int
		xmlIsXHTML		(const xmlChar *systemID,
					 const xmlChar *publicID);

/*
 * Compression.
 */
XMLPUBFUN int
		xmlGetDocCompressMode	(const xmlDoc *doc);
XMLPUBFUN void
		xmlSetDocCompressMode	(xmlDocPtr doc,
					 int mode);
XML_DEPRECATED
XMLPUBFUN int
		xmlGetCompressMode	(void);
XML_DEPRECATED
XMLPUBFUN void
		xmlSetCompressMode	(int mode);

/*
* DOM-wrapper helper functions.
*/
XMLPUBFUN xmlDOMWrapCtxtPtr
		xmlDOMWrapNewCtxt	(void);
XMLPUBFUN void
		xmlDOMWrapFreeCtxt	(xmlDOMWrapCtxtPtr ctxt);
XMLPUBFUN int
	    xmlDOMWrapReconcileNamespaces(xmlDOMWrapCtxtPtr ctxt,
					 xmlNodePtr elem,
					 int options);
XMLPUBFUN int
	    xmlDOMWrapAdoptNode		(xmlDOMWrapCtxtPtr ctxt,
					 xmlDocPtr sourceDoc,
					 xmlNodePtr node,
					 xmlDocPtr destDoc,
					 xmlNodePtr destParent,
					 int options);
XMLPUBFUN int
	    xmlDOMWrapRemoveNode	(xmlDOMWrapCtxtPtr ctxt,
					 xmlDocPtr doc,
					 xmlNodePtr node,
					 int options);
XMLPUBFUN int
	    xmlDOMWrapCloneNode		(xmlDOMWrapCtxtPtr ctxt,
					 xmlDocPtr sourceDoc,
					 xmlNodePtr node,
					 xmlNodePtr *clonedNode,
					 xmlDocPtr destDoc,
					 xmlNodePtr destParent,
					 int deep,
					 int options);

/*
 * 5 interfaces from DOM ElementTraversal, but different in entities
 * traversal.
 */
XMLPUBFUN unsigned long
            xmlChildElementCount        (xmlNodePtr parent);
XMLPUBFUN xmlNodePtr
            xmlNextElementSibling       (xmlNodePtr node);
XMLPUBFUN xmlNodePtr
            xmlFirstElementChild        (xmlNodePtr parent);
XMLPUBFUN xmlNodePtr
            xmlLastElementChild         (xmlNodePtr parent);
XMLPUBFUN xmlNodePtr
            xmlPreviousElementSibling   (xmlNodePtr node);

XML_DEPRECATED
XMLPUBFUN xmlRegisterNodeFunc
	    xmlRegisterNodeDefault	(xmlRegisterNodeFunc func);
XML_DEPRECATED
XMLPUBFUN xmlDeregisterNodeFunc
	    xmlDeregisterNodeDefault	(xmlDeregisterNodeFunc func);
XML_DEPRECATED
XMLPUBFUN xmlRegisterNodeFunc
            xmlThrDefRegisterNodeDefault(xmlRegisterNodeFunc func);
XML_DEPRECATED
XMLPUBFUN xmlDeregisterNodeFunc
            xmlThrDefDeregisterNodeDefault(xmlDeregisterNodeFunc func);

/*
 * Handling Buffers, the old ones see `xmlBuf` for the new ones.
 */

XML_DEPRECATED
XMLPUBFUN void
		xmlSetBufferAllocationScheme(xmlBufferAllocationScheme scheme);
XML_DEPRECATED
XMLPUBFUN xmlBufferAllocationScheme
		xmlGetBufferAllocationScheme(void);

XMLPUBFUN xmlBufferPtr
		xmlBufferCreate		(void);
XMLPUBFUN xmlBufferPtr
		xmlBufferCreateSize	(size_t size);
XMLPUBFUN xmlBufferPtr
		xmlBufferCreateStatic	(void *mem,
					 size_t size);
XML_DEPRECATED
XMLPUBFUN int
		xmlBufferResize		(xmlBufferPtr buf,
					 unsigned int size);
XMLPUBFUN void
		xmlBufferFree		(xmlBufferPtr buf);
XMLPUBFUN int
		xmlBufferDump		(FILE *file,
					 xmlBufferPtr buf);
XMLPUBFUN int
		xmlBufferAdd		(xmlBufferPtr buf,
					 const xmlChar *str,
					 int len);
XMLPUBFUN int
		xmlBufferAddHead	(xmlBufferPtr buf,
					 const xmlChar *str,
					 int len);
XMLPUBFUN int
		xmlBufferCat		(xmlBufferPtr buf,
					 const xmlChar *str);
XMLPUBFUN int
		xmlBufferCCat		(xmlBufferPtr buf,
					 const char *str);
XML_DEPRECATED
XMLPUBFUN int
		xmlBufferShrink		(xmlBufferPtr buf,
					 unsigned int len);
XML_DEPRECATED
XMLPUBFUN int
		xmlBufferGrow		(xmlBufferPtr buf,
					 unsigned int len);
XMLPUBFUN void
		xmlBufferEmpty		(xmlBufferPtr buf);
XMLPUBFUN const xmlChar*
		xmlBufferContent	(const xmlBuffer *buf);
XMLPUBFUN xmlChar*
		xmlBufferDetach         (xmlBufferPtr buf);
XMLPUBFUN void
		xmlBufferSetAllocationScheme(xmlBufferPtr buf,
					 xmlBufferAllocationScheme scheme);
XMLPUBFUN int
		xmlBufferLength		(const xmlBuffer *buf);
XMLPUBFUN void
		xmlBufferWriteCHAR	(xmlBufferPtr buf,
					 const xmlChar *string);
XMLPUBFUN void
		xmlBufferWriteChar	(xmlBufferPtr buf,
					 const char *string);
XMLPUBFUN void
		xmlBufferWriteQuotedString(xmlBufferPtr buf,
					 const xmlChar *string);

/*
 * A few public routines for xmlBuf. As those are expected to be used
 * mostly internally the bulk of the routines are internal in buf.h
 */
XMLPUBFUN xmlChar*       xmlBufContent	(const xmlBuf* buf);
XMLPUBFUN xmlChar*       xmlBufEnd      (xmlBufPtr buf);
XMLPUBFUN size_t         xmlBufUse      (const xmlBufPtr buf);
XMLPUBFUN size_t         xmlBufShrink	(xmlBufPtr buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __XML_TREE_H__ */

#endif /* XML_TREE_INTERNALS */

