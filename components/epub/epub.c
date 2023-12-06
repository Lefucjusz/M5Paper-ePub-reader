#include "epub.h"
#include "epub_utils.h"
#include "../third_party/miniz/miniz.h" // TODO fix this better way
#include "mxml.h"
#include <esp_log.h>

struct epub_ctx_t
{
    mz_zip_archive zip;
    map_str_t manifest;
};

static struct epub_ctx_t epub_ctx;
   
epub_err_t epub_open(const char *path)
{
    mz_zip_zero_struct(&epub_ctx.zip);

    /* Open file */
    if (!mz_zip_reader_init_file(&epub_ctx.zip, path, 0)) {
        return EPUB_IO_ERROR; // TODO add better error handling
    }

    /* Read content.opf and parse it TODO error handling */
    char *opf = mz_zip_reader_extract_file_to_heap(&epub_ctx.zip, "content.opf", NULL, 0);
    mxml_node_t *tree = mxmlLoadString(NULL, opf, MXML_NO_CALLBACK);
    free(opf);

    /* Parse manifest looking for application/xhtml+xml */
    mxml_node_t *node = mxmlFindElement(tree, tree, "manifest", NULL, NULL, MXML_DESCEND);

    map_str_create(&epub_ctx.manifest);

    for (mxml_node_t *el = mxmlGetFirstChild(node); el != NULL; el = mxmlGetNextSibling(el)) {
        const char *id = mxmlElementGetAttr(el, "id");
        const char *href = mxmlElementGetAttr(el, "href");
        if (id != NULL && href != NULL) {
            map_str_set(&epub_ctx.manifest, id, href);
        }
    }

    mxmlDelete(tree);

    map_iter_t it = map_str_iter(&epub_ctx.manifest);
    const char *key;
    while ((key = map_str_next(&epub_ctx.manifest, &it)) != NULL) {
        ESP_LOGI("", "%s -> %s", key, map_str_get(&epub_ctx.manifest, key));
    }
    
    map_str_destroy(&epub_ctx.manifest);

    /* Parse spine and create list of elements */

    

    return 0;
}

epub_err_t epub_close(void)
{
    if (!mz_zip_reader_end(&epub_ctx.zip)) {
        return EPUB_IO_ERROR;
    }
    return EPUB_OK;
}
