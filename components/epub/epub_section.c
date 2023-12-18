#include "epub_section.h"
#include "mxml.h"
#include <esp_log.h>
#include <stdbool.h>
#include "utils.h"

static void sax_callback(mxml_node_t *node, mxml_sax_event_t event, void *data);
static bool is_heading(const char *element);
static bool is_paragraph(const char *element);

epub_section_t *epub_section_create(const char *raw_text)
{
    if (raw_text == NULL) {
        return NULL;
        
    }
    epub_section_t *section = malloc(sizeof(epub_section_t));
    if (section == NULL) {
        return NULL;
    }
    vec_init(section);

    mxml_node_t *tree = mxmlSAXLoadString(NULL, raw_text, MXML_OPAQUE_CALLBACK, sax_callback, (void *)section);
    mxmlDelete(tree);

    return section;
}

void epub_section_destroy(epub_section_t *section)
{
    if (section == NULL) {
        return;
    }

    size_t i;
    epub_text_block_t *block;
    vec_foreach(section, block, i) {
        epub_text_block_destroy(block);
    }
}

static void sax_callback(mxml_node_t *node, mxml_sax_event_t event, void *data)
{
    static bool heading = false;
    static bool paragraph = false;

    switch (event) {
        case MXML_SAX_ELEMENT_OPEN:
            if (is_heading(mxmlGetElement(node))) {
                heading = true;
            } 
            else if (is_paragraph(mxmlGetElement(node))) {
                paragraph = true;
            }
            break;
        case MXML_SAX_DATA:
            if (paragraph) {
                epub_text_block_t *block = epub_text_block_create(mxmlGetOpaque(node), EPUB_FONT_NORMAL);
                epub_section_t *section = (epub_section_t *)data;
                vec_push(section, block);
            }
            else if (heading) {
                epub_text_block_t *block = epub_text_block_create(mxmlGetOpaque(node), EPUB_FONT_BOLD);
                epub_section_t *section = (epub_section_t *)data;
                vec_push(section, block);
            }
            break;
        case MXML_SAX_ELEMENT_CLOSE:
            if (is_heading(mxmlGetElement(node))) {
                heading = false;
            } 
            else if (is_paragraph(mxmlGetElement(node))) {
                paragraph = false;
            }
            break;
        default:
            break;
    }
}

static bool is_heading(const char *element)
{
    static const char *headings[]  = {"h1", "h2", "h3", "h4", "h5", "h6"};
    for (size_t i = 0; i < ARRAY_SIZE(headings); ++i) {
        if (strcmp(element, headings[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_paragraph(const char *element)
{
    return (strcmp(element, "p") == 0);
}
