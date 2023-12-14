#include "epub.h"
#include "epub_toc_entry.h"
#include "../third_party/miniz/miniz.h" // TODO fix this better way
#include "mxml.h"
#include "map.h"
#include "vec.h"
#include "cvec.h"
#include <esp_log.h>

#define TAG "EPUB"

typedef struct
{
    mz_zip_archive zip;
    
    vec_void_t spine;
    vec_void_t toc;
    
    char *root_path;
    char *toc_ncx_path;
} epub_ctx_t;

static epub_ctx_t epub_ctx;

/* Private functions prototypes */
static char *epub_get_content_opf_path(void);
static char *epub_get_root_path(const char *content_opf_path);
static epub_err_t epub_parse_content_opf(const char *path);
static epub_err_t epub_parse_toc_ncx(const char *path);
   
/* Public functions */
epub_err_t epub_open(const char *path)
{
    /* Open file */
    mz_zip_zero_struct(&epub_ctx.zip);
    if (!mz_zip_reader_init_file(&epub_ctx.zip, path, 0)) {
        ESP_LOGE(TAG, "Failed to open file '%s'", path);
        return EPUB_IO_ERROR; // TODO add better error handling
    }

    /* Get OPF file path */
    char *opf_path = epub_get_content_opf_path();
    if (opf_path == NULL) {
        ESP_LOGE(TAG, "Failed to get OPF file path");
        mz_zip_reader_end(&epub_ctx.zip);
        return EPUB_GENERAL_ERROR;
    }

    /* Get root directory path */
    epub_ctx.root_path = epub_get_root_path(opf_path);
    if (epub_ctx.root_path == NULL) {
        ESP_LOGE(TAG, "Failed to get EPUB root path from OPF path '%s'", opf_path);
        free(opf_path);
        mz_zip_reader_end(&epub_ctx.zip);
        return EPUB_GENERAL_ERROR;
    }

    /* Parse OPF file */
    epub_err_t err = epub_parse_content_opf(opf_path);
    if (err) {
        ESP_LOGE(TAG, "Failed to parse OPF file '%s'", opf_path);
        free(epub_ctx.root_path);
        free(opf_path);
        mz_zip_reader_end(&epub_ctx.zip);
        return err;
    }
    free(opf_path);

    /* Parse NCX file */
    err = epub_parse_toc_ncx(epub_ctx.toc_ncx_path);
    if (err) {
        ESP_LOGE(TAG, "Failed to parse NCX file '%s'", epub_ctx.toc_ncx_path);
        mz_zip_reader_end(&epub_ctx.zip);
        return err;
    }
    free(epub_ctx.toc_ncx_path);

    size_t i;
    epub_toc_entry_t *item;
    vec_foreach(&epub_ctx.toc, item, i) {
        ESP_LOGI(TAG, "%s | %s", item->title, item->path);
        // ESP_LOGI("", "%x", (unsigned)item);
    }

    // ESP_LOGI(TAG, "Root path: %s", epub_ctx.root_path);
    // ESP_LOGI(TAG, "NCX path: %s", epub_ctx.toc_ncx_path);

    return EPUB_OK;
}

epub_err_t epub_close(void)
{
    /* Clean up TOC */
    size_t i;
    epub_toc_entry_t *entry;
    vec_foreach(&epub_ctx.toc, entry, i) {
        epub_toc_entry_destroy(entry);
    }

    /* Clean up spine */
    cvec_destroy(&epub_ctx.spine);

    /* Clean up root path */
    free(epub_ctx.root_path);

    /* Close EPUB */
    if (!mz_zip_reader_end(&epub_ctx.zip)) {
        ESP_LOGE(TAG, "Failed to close EPUB file");
        return EPUB_IO_ERROR;
    }
    return EPUB_OK;
}

/* Private functions definitions */
static char *epub_get_content_opf_path(void)
{
    /* Read container file from the archive and parse it */
    const char *container_xml_path = "META-INF/container.xml";
    char *container_xml_raw = mz_zip_reader_extract_file_to_heap(&epub_ctx.zip, container_xml_path, NULL, 0);
    if (container_xml_raw == NULL) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", container_xml_path);
        return NULL;
    }
    mxml_node_t *container_xml_root = mxmlLoadString(NULL, container_xml_raw, MXML_NO_CALLBACK);
    free(container_xml_raw);
    if (container_xml_root == NULL) {
        ESP_LOGE(TAG, "Failed to parse '%s'", container_xml_path);
        return NULL;
    }

    /* Find path to rootfile with media-type="application/oebps-package+xml" */
    char *path = NULL; 
    do {
        mxml_node_t *container_tag = mxmlFindElement(container_xml_root, container_xml_root, "container", NULL, NULL, MXML_DESCEND);
        if (container_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find container tag in '%s'", container_xml_path);
            break;
        }

        mxml_node_t *rootfiles_tag = mxmlFindElement(container_tag, container_xml_root, "rootfiles", NULL, NULL, MXML_DESCEND);
        if (rootfiles_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find rootfiles tag in '%s'", container_xml_path);
            break;
        }

        const char *attribute_name = "media-type";
        const char *media_type = "application/oebps-package+xml";
        mxml_node_t *rootfile_tag = mxmlFindElement(rootfiles_tag, container_xml_root, "rootfile", attribute_name, media_type, MXML_DESCEND);
        if (rootfile_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find rootfile tag in '%s'", container_xml_path);
            break;
        }

        const char *full_path = mxmlElementGetAttr(rootfile_tag, "full-path");
        const size_t length = strlen(full_path) + 1;
        path = malloc(length);
        if (path == NULL) {
            break;
        }
        memcpy(path, full_path, length);
    } while (0);

    mxmlDelete(container_xml_root);
    return path;
}

static char *epub_get_root_path(const char *content_opf_path)
{
    const char *last_slash = strrchr(content_opf_path, '/');
    size_t path_length;
    if (last_slash == NULL) {
        path_length = 0;
    }
    else {
        path_length = last_slash - content_opf_path;
    }

    const size_t size = path_length + 1;
    char *root_path = malloc(size);
    if (root_path == NULL) {
        return NULL;
    }
    strlcpy(root_path, content_opf_path, size);

    return root_path;
}

static epub_err_t epub_parse_content_opf(const char *path)
{
    /* Read OPF file and parse it */
    char *opf_raw = mz_zip_reader_extract_file_to_heap(&epub_ctx.zip, path, NULL, 0);
    if (opf_raw == NULL) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", path);
        return EPUB_IO_ERROR;
    }
    mxml_node_t *opf_tree = mxmlLoadString(NULL, opf_raw, MXML_NO_CALLBACK);
    free(opf_raw);
    if (opf_tree == NULL) {
        ESP_LOGE(TAG, "Failed to parse '%s'", path);
        return EPUB_PARSING_ERROR;
    }

    /* Parse manifest and spine */
    epub_err_t err = EPUB_OK;
    map_str_t manifest;
    map_init(&manifest);
    do {
        /* Find manifest tag */
        mxml_node_t *manifest_tag = mxmlFindElement(opf_tree, opf_tree, "manifest", NULL, NULL, MXML_DESCEND);
        if (manifest_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find manifest tag in '%s'", path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Iterate through every item in manifest */
        for (mxml_node_t *item = mxmlGetFirstChild(manifest_tag); item != NULL; item = mxmlGetNextSibling(item)) {
            if (mxmlGetType(item) != MXML_ELEMENT) {
                continue;
            }

            /* Add every entry to map */
            const char *id = mxmlElementGetAttr(item, "id");
            const char *href = mxmlElementGetAttr(item, "href");
            map_set(&manifest, id, (char *)href); // Cast to suppress discarded qualifier warning

            /* Store path to NCX file */
            if (strcmp(id, "ncx") == 0) {
                const size_t length = strlen(href) + 1;
                epub_ctx.toc_ncx_path = malloc(length); // TODO check if not null
                strlcpy(epub_ctx.toc_ncx_path, href, length);
            }
        }

        /* Find spine tag */
        mxml_node_t *spine_tag = mxmlFindElement(opf_tree, opf_tree, "spine", NULL, NULL, MXML_DESCEND);
        if (spine_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find spine tag in '%s'", path);
            free(epub_ctx.toc_ncx_path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Iterate through every item in spine */
        cvec_create(&epub_ctx.spine);
        for (mxml_node_t *item = mxmlGetFirstChild(spine_tag); item != NULL; item = mxmlGetNextSibling(item)) {
            if (mxmlGetType(item) != MXML_ELEMENT) {
                continue;
            }

            /* Add path to spine vector */
            const char *idref = mxmlElementGetAttr(item, "idref");
            char **item_path_ptr = map_get(&manifest, idref);
            if (item_path_ptr != NULL) {
                cvec_push_string(&epub_ctx.spine, *item_path_ptr);
            }
        }
    } while (0);

    /* Cleanup */
    map_deinit(&manifest);
    mxmlDelete(opf_tree);

    return err;
}

static epub_err_t epub_parse_toc_ncx(const char *path)
{
    /* Read NCX file and parse it */
    char *ncx_raw = mz_zip_reader_extract_file_to_heap(&epub_ctx.zip, path, NULL, 0);
    if (ncx_raw == NULL) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", path);
        return EPUB_IO_ERROR;
    }
    mxml_node_t *ncx_tree = mxmlLoadString(NULL, ncx_raw, MXML_OPAQUE_CALLBACK);
    free(ncx_raw);
    if (ncx_tree == NULL) {
        ESP_LOGE(TAG, "Failed to parse '%s'", path);
        return EPUB_PARSING_ERROR;
    }

    /* Parse TOC entries */
    epub_err_t err = EPUB_OK;
    do
    {
        /* Find ncx tag */
        mxml_node_t *ncx_tag = mxmlFindElement(ncx_tree, ncx_tree, "ncx", NULL, NULL, MXML_DESCEND);
        if (ncx_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find ncx tag in '%s'", path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Find navMap tag */
        mxml_node_t *navmap_tag = mxmlFindElement(ncx_tag, ncx_tree, "navMap", NULL, NULL, MXML_DESCEND);
        if (navmap_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find navMap tag in '%s'", path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Iterate through every item in navMap and add it to the vector */
        vec_init(&epub_ctx.toc);
        for (mxml_node_t *item = mxmlGetFirstChild(navmap_tag); item != NULL; item = mxmlGetNextSibling(item)) {
            if (mxmlGetType(item) != MXML_ELEMENT) {
                continue;
            }

            /* Extract title */
            mxml_node_t *navlabel_tag = mxmlFindElement(mxmlGetFirstChild(item), ncx_tree, "navLabel", NULL, NULL, MXML_NO_DESCEND);
            if (navlabel_tag == NULL) {
                ESP_LOGE(TAG, "Failed to find navLabel tag in '%s'", path);
                err = EPUB_PARSING_ERROR;
                break;
            }
            mxml_node_t *text_tag = mxmlFindElement(mxmlGetFirstChild(navlabel_tag), ncx_tree, "text", NULL, NULL, MXML_NO_DESCEND);
            if (text_tag == NULL) {
                ESP_LOGE(TAG, "Failed to find text tag in '%s'", path);
                err = EPUB_PARSING_ERROR;
                break;
            }
            const char *title = mxmlGetOpaque(text_tag);

            /* Extract path */
            mxml_node_t *content_tag = mxmlFindElement(mxmlGetFirstChild(item), ncx_tree, "content", NULL, NULL, MXML_NO_DESCEND);
            if (content_tag == NULL) {
                ESP_LOGE(TAG, "Failed to find content tag in '%s'", path);
                err = EPUB_PARSING_ERROR;
                break;
            }
            const char *src_tag_data = mxmlElementGetAttr(content_tag, "src");
            char *anchor_start = strrchr(src_tag_data, '#');

            /* Add to TOC */
            if (anchor_start != NULL) {
                *anchor_start = '\0'; // Dirty hack
            }
            epub_toc_entry_t *entry = epub_toc_entry_create(title, src_tag_data);
            vec_push(&epub_ctx.toc, entry);
            // ESP_LOGE("", "%x", (unsigned)entry);
        }
    } while (0);

    /* Cleanup */
    mxmlDelete(ncx_tree);

    return err;
}
