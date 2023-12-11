#include "epub.h"
#include "epub_utils.h"
#include "../third_party/miniz/miniz.h" // TODO fix this better way
#include "mxml.h"
#include <esp_log.h>

#define EPUB_TAG "EPUB"

struct epub_ctx_t
{
    mz_zip_archive zip;
    map_str_t manifest;
    vec_str_t spine;
};

static struct epub_ctx_t epub_ctx;

/* Private functions prototypes */
static char *epub_get_content_opf_path(void);
static epub_err_t epub_parse_content_opf(const char *path);
   
/* Public functions */
epub_err_t epub_open(const char *path)
{
    /* Open file */
    mz_zip_zero_struct(&epub_ctx.zip);
    if (!mz_zip_reader_init_file(&epub_ctx.zip, path, 0)) {
        ESP_LOGE(EPUB_TAG, "Failed to open file '%s'", path);
        return EPUB_IO_ERROR; // TODO add better error handling
    }

    /* Get OPF file path */
    char *opf_path = epub_get_content_opf_path();
    if (opf_path == NULL) {
        ESP_LOGE(EPUB_TAG, "Failed to get OPF file path");
        mz_zip_reader_end(&epub_ctx.zip);
        return EPUB_GENERAL_ERROR;
    }

    /* Parse OPF file */
    const epub_err_t err = epub_parse_content_opf(opf_path);
    if (err) {
        ESP_LOGE(EPUB_TAG, "Failed to parse OPF file '%s'", opf_path);
        mz_zip_reader_end(&epub_ctx.zip);
    }
    free(opf_path);

    size_t i;
    char *item;
    vec_foreach(&epub_ctx.spine, item, i) {
        ESP_LOGI(EPUB_TAG, "Item %zu: %s", i, item);
    }

    return err;
}

epub_err_t epub_close(void)
{
    map_str_destroy(&epub_ctx.manifest);
    vec_str_destroy(&epub_ctx.spine);
    if (!mz_zip_reader_end(&epub_ctx.zip)) {
        ESP_LOGE(EPUB_TAG, "Failed to close EPUB file");
        return EPUB_IO_ERROR;
    }
    return EPUB_OK;
}

/* Private functions definitions */
static char *epub_get_content_opf_path(void)
{
    /* Extract file from the archive */
    const char *container_xml_path = "META-INF/container.xml";
    char *container_xml_raw = mz_zip_reader_extract_file_to_heap(&epub_ctx.zip, container_xml_path, NULL, 0);
    if (container_xml_raw == NULL) {
        ESP_LOGE(EPUB_TAG, "Failed to extract '%s' from archive", container_xml_path);
        return NULL;
    }

    /* Parse container.xml */
    mxml_node_t *container_xml_root = mxmlLoadString(NULL, container_xml_raw, MXML_NO_CALLBACK);
    free(container_xml_raw); // Original content is no longer used
    if (container_xml_root == NULL) {
        ESP_LOGE(EPUB_TAG, "Failed to parse '%s'", container_xml_path);
        return NULL;
    }

    /* Find path to rootfile with media-type="application/oebps-package+xml" */
    char *path = NULL; 
    do {
        mxml_node_t *container_tag = mxmlFindElement(container_xml_root, container_xml_root, "container", NULL, NULL, MXML_DESCEND);
        if (container_tag == NULL) {
            ESP_LOGE(EPUB_TAG, "Failed to find container tag in '%s'", container_xml_path);
            break;
        }

        mxml_node_t *rootfiles_tag = mxmlFindElement(container_tag, container_xml_root, "rootfiles", NULL, NULL, MXML_DESCEND);
        if (rootfiles_tag == NULL) {
            ESP_LOGE(EPUB_TAG, "Failed to find rootfiles tag in '%s'", container_xml_path);
            break;
        }

        const char *attribute_name = "media-type";
        const char *media_type = "application/oebps-package+xml";
        mxml_node_t *rootfile_tag = mxmlFindElement(rootfiles_tag, container_xml_root, "rootfile", attribute_name, media_type, MXML_DESCEND);
        if (rootfile_tag == NULL) {
            ESP_LOGE(EPUB_TAG, "Failed to find rootfile tag in '%s'", container_xml_path);
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

static epub_err_t epub_parse_content_opf(const char *path)
{
    /* Read OPF file and parse it */
    char *opf_raw = mz_zip_reader_extract_file_to_heap(&epub_ctx.zip, path, NULL, 0);
    if (opf_raw == NULL) {
        ESP_LOGE(EPUB_TAG, "Failed to extract '%s' from archive", path);
        return EPUB_IO_ERROR;
    }
    mxml_node_t *opf_tree = mxmlLoadString(NULL, opf_raw, MXML_NO_CALLBACK);
    free(opf_raw);
    if (opf_tree == NULL) {
        ESP_LOGE(EPUB_TAG, "Failed to parse '%s'", path);
        return EPUB_PARSING_ERROR;
    }

    /* Parse manifest and spine */
    epub_err_t err = EPUB_OK;
    do {
        /* Find manifest tag */
        mxml_node_t *manifest_tag = mxmlFindElement(opf_tree, opf_tree, "manifest", NULL, NULL, MXML_DESCEND);
        if (manifest_tag == NULL) {
            ESP_LOGE(EPUB_TAG, "Failed to find manifest tag in '%s'", path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Iterate through every item in manifest and add it to the map TODO do it only for needed ones */
        map_str_create(&epub_ctx.manifest);
        for (mxml_node_t *item = mxmlGetFirstChild(manifest_tag); item != NULL; item = mxmlGetNextSibling(item)) {
            const char *id = mxmlElementGetAttr(item, "id");
            const char *href = mxmlElementGetAttr(item, "href");
            if ((id != NULL) && (href != NULL)) {
                map_str_set(&epub_ctx.manifest, id, href);
            }
        }

        /* Find spine tag */
        mxml_node_t *spine_tag = mxmlFindElement(opf_tree, opf_tree, "spine", NULL, NULL, MXML_DESCEND);
        if (spine_tag == NULL) {
            ESP_LOGE(EPUB_TAG, "Failed to find spine tag in '%s'", path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Iterate through every item in spine and add it to the vector */
        vec_str_create(&epub_ctx.spine);
        for (mxml_node_t *item = mxmlGetFirstChild(spine_tag); item != NULL; item = mxmlGetNextSibling(item)) {
            const char *idref = mxmlElementGetAttr(item, "idref");
            if (idref != NULL) {
                vec_str_push(&epub_ctx.spine, idref);
            }
        }
    } while (0);

    /* Cleanup */
    mxmlDelete(opf_tree);
    if (err) {
        map_str_destroy(&epub_ctx.manifest);
    }
    return err;
}
