#include "epub.h"
#include "mxml.h"
#include "map.h"
#include "cvec.h"
#include "path_utils.h"
#include <esp_log.h>

#define TAG "EPUB"

/* Private functions prototypes */
static char *epub_get_content_opf_path(epub_t *epub);
static char *epub_get_root_path(const char *content_opf_path);
static epub_err_t epub_parse_content_opf(epub_t *epub, const char *opf_path, const char *root_path, char **ncx_path);
static epub_err_t epub_parse_toc_ncx(epub_t *epub, const char *toc_ncx_path, const char *root_path);
   
/* Public functions */
epub_err_t epub_open(epub_t *epub, const char *path)
{
    if ((epub == NULL) || (path == NULL)) {
        return EPUB_INVALID_ARG;
    }

    epub_err_t err = EPUB_OK;

    /* Open file */
    mz_zip_zero_struct(&epub->zip);
    if (!mz_zip_reader_init_file(&epub->zip, path, 0)) {
        ESP_LOGE(TAG, "Failed to open file '%s'", path);
        err = EPUB_IO_ERROR; // TODO add better error handling
        goto exit;
    }

    /* Get OPF file path */
    char *opf_path = epub_get_content_opf_path(epub);
    if (opf_path == NULL) {
        ESP_LOGE(TAG, "Failed to get OPF file path");
        err = EPUB_PARSING_ERROR;
        goto exit_opf_path_fail;
    }

    /* Get root directory path */
    char *root_path = epub_get_root_path(opf_path);
    if (root_path == NULL) {
        ESP_LOGE(TAG, "Failed to get EPUB root path from OPF path '%s'", opf_path);
        err = EPUB_NO_MEMORY;
        goto exit_root_path_fail;
    }

    /* Parse OPF file */
    char *toc_ncx_path = NULL;
    err = epub_parse_content_opf(epub, opf_path, root_path, &toc_ncx_path);
    if (err) {
        ESP_LOGE(TAG, "Failed to parse OPF file '%s'", opf_path);
        goto exit_opf_parse_fail;
    }
    free(opf_path);
    opf_path = NULL;

    /* Parse NCX file */
    err = epub_parse_toc_ncx(epub, toc_ncx_path, root_path);
    if (err) {
        ESP_LOGE(TAG, "Failed to parse NCX file '%s'", toc_ncx_path);
        goto exit_ncx_parse_fail;
    }
    ESP_LOGI(TAG, "NCX path: %s", toc_ncx_path);
    ESP_LOGI(TAG, "Root path: %s", root_path);
    free(toc_ncx_path);
    free(root_path);

    size_t i;
    const char *spine_entry;
    vec_foreach(&epub->spine, spine_entry, i) {
        ESP_LOGI(TAG, "Spine entry %zu: %s", i, spine_entry);
    }

    epub_toc_entry_t *toc_entry;
    vec_foreach(&epub->toc, toc_entry, i) {
        ESP_LOGI(TAG, "TOC entry %zu: %s | %s", i, toc_entry->title, toc_entry->path);
    }

    goto exit;

exit_ncx_parse_fail:
    cvec_destroy(&epub->spine);
    free(toc_ncx_path);
exit_opf_parse_fail:
    free(root_path);
exit_root_path_fail:    
    free(opf_path);
exit_opf_path_fail:    
    mz_zip_reader_end(&epub->zip);
exit:
    return err;
}

epub_err_t epub_close(epub_t *epub)
{
    if (epub == NULL) {
        return EPUB_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Closing epub file...");

    /* Clean up TOC */
    size_t i;
    epub_toc_entry_t *entry;
    vec_foreach(&epub->toc, entry, i) {
        epub_toc_entry_destroy(entry);
    }
    vec_deinit(&epub->toc);

    /* Clean up spine */
    cvec_destroy(&epub->spine);

    /* Close EPUB */
    if (!mz_zip_reader_end(&epub->zip)) {
        ESP_LOGE(TAG, "Failed to close EPUB file");
        return EPUB_IO_ERROR;
    }
    return EPUB_OK;
}

const epub_toc_t *epub_get_toc(epub_t *epub)
{
    if (epub == NULL) {
        return NULL;
    }
    return &epub->toc;
}

ssize_t epub_get_spine_entry_index(epub_t *epub, const char *href)
{
    if ((epub == NULL) || (href == NULL)) {
        return EPUB_INVALID_ARG;
    }

    size_t i;
    const char *path;
    vec_foreach(&epub->spine, path, i) {
        if (strcmp(path, href) == 0) {
            ESP_LOGI(TAG, "Found spine entry index '%zu' for href '%s'", i, href);
            return i;
        }
    }
    ESP_LOGI(TAG, "Spine entry index for href '%s' not found", href);
    return EPUB_NOT_FOUND;
}

ssize_t epub_get_spine_size(epub_t *epub)
{
    if (epub == NULL) {
        return EPUB_INVALID_ARG;
    }
    return epub->spine.length;
}

char *epub_get_section_content(epub_t *epub, size_t spine_entry)
{
    if ((epub == NULL) || (spine_entry >= epub_get_spine_size(epub))) {
        return NULL;
    }

    const char *path = epub->spine.data[spine_entry];
    return mz_zip_reader_extract_file_to_heap(&epub->zip, path, NULL, 0);
}

epub_section_t *epub_get_section(epub_t *epub, size_t spine_entry)
{
    char *section_content = epub_get_section_content(epub, spine_entry);
    if (section_content == NULL) {
        return NULL;
    }

    return epub_section_create(section_content);
}

/* Private functions definitions */
static char *epub_get_content_opf_path(epub_t *epub)
{
    /* Read container file from the archive and parse it */
    const char *container_xml_path = "META-INF/container.xml";
    char *container_xml_raw = mz_zip_reader_extract_file_to_heap(&epub->zip, container_xml_path, NULL, 0);
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

static epub_err_t epub_parse_content_opf(epub_t *epub, const char *opf_path, const char *root_path, char **ncx_path)
{
    /* Read OPF file and parse it */
    char *opf_raw = mz_zip_reader_extract_file_to_heap(&epub->zip, opf_path, NULL, 0);
    if (opf_raw == NULL) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", opf_path);
        return EPUB_IO_ERROR;
    }
    mxml_node_t *opf_tree = mxmlLoadString(NULL, opf_raw, MXML_NO_CALLBACK);
    free(opf_raw);
    if (opf_tree == NULL) {
        ESP_LOGE(TAG, "Failed to parse '%s'", opf_path);
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
            ESP_LOGE(TAG, "Failed to find manifest tag in '%s'", opf_path);
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
                *ncx_path = path_concatenate(root_path, href);
            }
        }

        /* Find spine tag */
        mxml_node_t *spine_tag = mxmlFindElement(opf_tree, opf_tree, "spine", NULL, NULL, MXML_DESCEND);
        if (spine_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find spine tag in '%s'", opf_path);
            free(*ncx_path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Iterate through every item in spine */
        cvec_create(&epub->spine);
        for (mxml_node_t *item = mxmlGetFirstChild(spine_tag); item != NULL; item = mxmlGetNextSibling(item)) {
            if (mxmlGetType(item) != MXML_ELEMENT) {
                continue;
            }

            /* Add path to spine vector */
            const char *idref = mxmlElementGetAttr(item, "idref");
            char **item_path_ptr = map_get(&manifest, idref);
            if (item_path_ptr != NULL) {
                char *path_full = path_concatenate(root_path, *item_path_ptr);
                cvec_push_string(&epub->spine, path_full);
                free(path_full);
            }
        }
    } while (0);

    /* Cleanup */
    map_deinit(&manifest);
    mxmlDelete(opf_tree);

    return err;
}

static epub_err_t epub_parse_toc_ncx(epub_t *epub, const char *toc_ncx_path, const char *root_path)
{
    /* Read NCX file and parse it */
    char *ncx_raw = mz_zip_reader_extract_file_to_heap(&epub->zip, toc_ncx_path, NULL, 0);
    if (ncx_raw == NULL) {
        ESP_LOGE(TAG, "Failed to extract '%s' from archive", toc_ncx_path);
        return EPUB_IO_ERROR;
    }
    mxml_node_t *ncx_tree = mxmlLoadString(NULL, ncx_raw, MXML_OPAQUE_CALLBACK);
    free(ncx_raw);
    if (ncx_tree == NULL) {
        ESP_LOGE(TAG, "Failed to parse '%s'", toc_ncx_path);
        return EPUB_PARSING_ERROR;
    }

    /* Parse TOC entries */
    epub_err_t err = EPUB_OK;
    do
    {
        /* Find ncx tag */
        mxml_node_t *ncx_tag = mxmlFindElement(ncx_tree, ncx_tree, "ncx", NULL, NULL, MXML_DESCEND);
        if (ncx_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find ncx tag in '%s'", toc_ncx_path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Find navMap tag */
        mxml_node_t *navmap_tag = mxmlFindElement(ncx_tag, ncx_tree, "navMap", NULL, NULL, MXML_DESCEND);
        if (navmap_tag == NULL) {
            ESP_LOGE(TAG, "Failed to find navMap tag in '%s'", toc_ncx_path);
            err = EPUB_PARSING_ERROR;
            break;
        }

        /* Iterate through every item in navMap and add it to the vector */
        vec_init(&epub->toc);
        for (mxml_node_t *item = mxmlGetFirstChild(navmap_tag); item != NULL; item = mxmlGetNextSibling(item)) {
            if (mxmlGetType(item) != MXML_ELEMENT) {
                continue;
            }

            /* Extract title */
            mxml_node_t *navlabel_tag = mxmlFindElement(item, ncx_tree, "navLabel", NULL, NULL, MXML_DESCEND);
            if (navlabel_tag == NULL) {
                ESP_LOGE(TAG, "Failed to find navLabel tag in '%s'", toc_ncx_path);
                err = EPUB_PARSING_ERROR;
                break;
            }
            mxml_node_t *text_tag = mxmlFindElement(navlabel_tag, ncx_tree, "text", NULL, NULL, MXML_DESCEND);
            if (text_tag == NULL) {
                ESP_LOGE(TAG, "Failed to find text tag in '%s'", toc_ncx_path);
                err = EPUB_PARSING_ERROR;
                break;
            }
            const char *title = mxmlGetOpaque(text_tag);

            /* Extract path */
            mxml_node_t *content_tag = mxmlFindElement(item, ncx_tree, "content", NULL, NULL, MXML_DESCEND);
            if (content_tag == NULL) {
                ESP_LOGE(TAG, "Failed to find content tag in '%s'", toc_ncx_path);
                err = EPUB_PARSING_ERROR;
                break;
            }
            const char *src_tag_data = mxmlElementGetAttr(content_tag, "src");
            char *anchor_start = strrchr(src_tag_data, '#'); // TODO anchor-based navigation is not supported yet

            /* Add to TOC */
            if (anchor_start != NULL) {
                *anchor_start = '\0'; // Dirty hack
            }
            char *path_full = path_concatenate(root_path, src_tag_data);
            epub_toc_entry_t *entry = epub_toc_entry_create(title, path_full);
            free(path_full);
            vec_push(&epub->toc, entry);
        }
    } while (0);

    /* Cleanup */
    mxmlDelete(ncx_tree);

    return err;
}
