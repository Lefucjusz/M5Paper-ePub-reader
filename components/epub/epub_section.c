#include "epub_section.h"
#include "mxml.h"
#include <esp_log.h>
#include <stdbool.h>
#include "utils.h"

static void sax_callback(mxml_node_t *node, mxml_sax_event_t event, void *data);
static bool is_heading(const char *element);
static bool is_paragraph(const char *element);

vec_void_t *epub_section_parse(const char *raw)
{
    vec_void_t *section = malloc(sizeof(vec_void_t));
    vec_init(section);
    mxml_node_t *tree = mxmlSAXLoadString(NULL, raw, MXML_OPAQUE_CALLBACK, sax_callback, (void *)section);
    mxmlDelete(tree);

    return section;
}

void epub_section_destroy(vec_void_t *section)
{
    size_t i;
    epub_paragraph_t *paragraph;
    vec_foreach(section, paragraph, i) {
        free(paragraph->data);
        free(paragraph);
    }
}

static void sax_callback(mxml_node_t *node, mxml_sax_event_t event, void *data)
{
    static bool heading = false;
    static bool paragraph = false;

    switch (event) {
        case MXML_SAX_ELEMENT_OPEN:
            // ESP_LOGI("", "Got opening element %s", mxmlGetElement(node));
            if (is_heading(mxmlGetElement(node))) {
                heading = true;
            } 
            else if (is_paragraph(mxmlGetElement(node))) {
                paragraph = true;
            }
            break;
        case MXML_SAX_DATA:
            if (heading) {
                // ESP_LOGI("", "Got heading: '%s'", mxmlGetOpaque(node));
                epub_paragraph_t *par = malloc(sizeof(epub_paragraph_t));
                par->data = malloc(strlen(mxmlGetOpaque(node)) + 1);
                memcpy(par->data, mxmlGetOpaque(node), strlen(mxmlGetOpaque(node)) + 1);
                par->type = EPUB_FONT_BOLD;
                vec_push((vec_void_t *)data, par);
            }
            else if (paragraph) {
                epub_paragraph_t *par = malloc(sizeof(epub_paragraph_t));
                par->data = malloc(strlen(mxmlGetOpaque(node)) + 1);
                memcpy(par->data, mxmlGetOpaque(node), strlen(mxmlGetOpaque(node)) + 1);
                par->type = EPUB_FONT_NORMAL;
                vec_push((vec_void_t *)data, par);
            }
            break;
        case MXML_SAX_ELEMENT_CLOSE:
            // ESP_LOGI("", "Got closing element %s", mxmlGetElement(node));
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
