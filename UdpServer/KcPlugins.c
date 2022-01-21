#include <jansson.h>
#include "KcShared.h"
#include "KcPlugins.h"




#pragma region 从插件配置文件加载插件

int32_t kcServerPluginsLoadRead(const struct KC_CONFIG* const config, StringBuffer error, const char* const fullfile, struct KC_PLUGINS** ptr);
int32_t kcServerPluginsParse(const struct KC_CONFIG* const config, StringBuffer error, const json_t* const root, struct KC_PLUGINS** ptr);
int32_t kcServerPluginsLoadLibrary(struct KC_PLUGIN_ITEM* plugin, StringBuffer error);


int32_t kcServerPluginsLoad(const struct KC_CONFIG* const config, StringBuffer str, struct KC_PLUGINS** ptr) {
	resetStringBuffer(str);
	appendStringBuffer(str, gettext("%s/plugins.json"), config->runing_path);
	//检查配置文件是否存在和是否有读取权限
	KC_FILE_ACCESS_CHECK(str->data, 4, 1,str);
	KC_CHECK_RCV2(kcServerPluginsLoadRead(config, str, str->data, ptr));
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

int32_t kcServerPluginsLoadRead(const struct KC_CONFIG* const config, StringBuffer error, const char* const fullfile, struct KC_PLUGINS** ptr) {
	FILE* stream = NULL;
	json_t* doc = NULL;
	json_error_t jerror;
	size_t nbytes = 0, rbytes = 0, offset = 0;
	int32_t index = 0;

	//KC_VALIDATE_PARA_PTR(fullfile, error);
	//KC_VALIDATE_PARA_PTR(conf, error);

	KC_FILE_OPEN_AND_CHECK(stream, fullfile, PG_BINARY_R, error);
	KC_FILE_DESCRIPTORS_CHECK(fullfile, error);
	KC_FILE_SEEK_CHECK(fseeko(stream, 0, SEEK_SET), fullfile, error);
	resetStringBuffer(error);
	while (!(feof(stream) || ferror(stream))) {
		//如果不是文件末尾，并且 error->len 等于 MaxAllocSize - 1，则文件太大，或者没有要读取的内容。尝试再读取一个字节以查看是否已到达文件末尾。如果不是，则文件太大，抛出错误消息。
		if (error->len == MaxAllocSize - 1) {
			char		rbuf[1];
			if (fread(rbuf, 1, 1, stream) != 0 || !feof(stream)) {
				resetStringBuffer(error);
				appendStringBuffer(error, "%s",gettext("file length too large"));
				goto KC_ERROR_CLEAR;
			} else {
				break;
			}
		}
		// 确保我们至少可以读取256字节,enlargeStringBuffer重新调整内存大小为 error->len*2，所以在这之前必须要检查error->len == MaxAllocSize - 1
		enlargeStringBuffer(error, 256);
		rbytes = fread(error->data + error->len, 1, (size_t)(error->maxlen - error->len - 1), stream);
		error->len += rbytes;
		nbytes += rbytes;
	}
	if (ferror(stream)) {
		resetStringBuffer(error);
		appendStringBuffer(error, gettext("could not read file \"%s\""), fullfile);
		goto KC_ERROR_CLEAR;
	}

	//检查UTF-8文件中是否包含BOM
	if (UTF8BOM[0] == error->data[0] && UTF8BOM[1] == error->data[1] && UTF8BOM[2] == error->data[2])
		offset = 3;
	doc = json_loadb(error->data + offset, error->len - offset, JSON_REJECT_DUPLICATES, &jerror);//不允许有重复的key
	KC_JSON_LOAD_CHECK(doc, jerror, error);
	KC_CHECK_RCV2(kcServerPluginsParse(config, error, doc, ptr));

	KC_JSON_FREE(doc);
	KC_FILE_CLOSE(stream);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_JSON_FREE(doc);
	KC_FILE_CLOSE(stream);
	return KC_FAIL;
}


int32_t kcServerPluginsParse(const struct KC_CONFIG* const config, StringBuffer error, const json_t* const root, struct KC_PLUGINS** ptr) {
	size_t index;
	struct KC_PLUGINS* plugins = NULL;
	struct KC_PLUGIN_ITEM* firstAdd;
	json_t* item;
	json_t* identifier, * library;
	json_t* entry_process_data;
	char* strLibrary = NULL, * hex = NULL;
	size_t hexLen;




	plugins = (struct KC_PLUGINS*)calloc(1, sizeof(struct KC_PLUGINS));
	KC_CHECK_MALLOC(plugins, error);
	plugins->count = json_array_size(root);
	plugins->items = (struct KC_PLUGIN_ITEM**)calloc(plugins->count, sizeof(struct KC_PLUGIN_ITEM));
	KC_CHECK_MALLOC(plugins->items, error);
	firstAdd = (struct KC_PLUGIN_ITEM*)plugins->items;
	json_array_foreach(root, index, item) {
		identifier = json_object_get(item, KC_2STR(identifier));
		library = json_object_get(item, KC_2STR(library));
		entry_process_data = json_object_get(item, KC_2STR(entry_process_data));

		KC_JSON_NODE_ISNULL(identifier, KC_2STR(identifier), error);
		KC_JSON_NODE_ISNULL(library, KC_2STR(library), error);
		KC_JSON_NODE_ISNULL(entry_process_data, KC_2STR(entry_process_data), error);

		KC_CHECK_RCV2(kcJsonGetStringN(KC_2STR(identifier), identifier, &hex, &hexLen, error));
		KC_CHECK_RCV2(kcHexStringToBytes(hex, hexLen, error, &(firstAdd->identifier), &(firstAdd->identifierLen)));
		KC_CHECK_RCV2(kcJsonGetString(KC_2STR(library), library, &strLibrary, error));
		KC_CHECK_RCV2(kcJsonGetString(KC_2STR(entry_process_data), entry_process_data, &(firstAdd->entry_process_data), error));

		resetStringBuffer(error);
		appendStringBuffer(error, "%s/%s", config->runing_path, strLibrary);
		KC_CHECK_RCV2(kcStringCopyV2(error->data, error->len, error, &(firstAdd->library)));

		KC_CHECK_RCV2(kcServerPluginsLoadLibrary(firstAdd, error));
		++firstAdd;
	}
	KC_SAFE_MODULE_FREE(hex);
	KC_SAFE_MODULE_FREE(strLibrary);
	(*ptr) = plugins;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_MODULE_FREE(hex);
	KC_SAFE_MODULE_FREE(strLibrary);
	KC_SAFE_UNLOAD_SERVER_PLUGINS(plugins);
	return KC_FAIL;
}

int32_t kcServerPluginsLoadLibrary(struct KC_PLUGIN_ITEM* plugin, StringBuffer error) {

#ifdef __GNUC__
	char* load_error;
#endif

#ifdef _WIN32
	plugin->handler = LoadLibrary(plugin->library);
	if (NULL == plugin->handler)
		KC_GOTOERR(error, gettext("load library \"%s\" fail."), plugin->library);

	plugin->funProcessData = (kcPluginProcessData)GetProcAddress(plugin->handler, plugin->entry_process_data);
	if (NULL == plugin->funProcessData)
		KC_GOTOERR(error, gettext("get the entry function \"%s\" fail from the module."), plugin->entry_process_data);
#else
	plugin->handler = dlopen(plugin->library, RTLD_LAZY);
	if (NULL == plugin->handler) {
		load_error = dlerror();
		resetStringBuffer(error);
		appendStringBuffer(error, gettext("load library \"%s\" fail,%s."), plugin->library, load_error);
	}

	plugin->funProcessData = (kcPluginProcessData)dlsym(plugin->handler, plugin->entry_process_data);
	if (NULL == plugin->funProcessData)
		KC_GOTOERR(error, gettext("get the entry function \"%s\" fail from the module."), plugin->entry_process_data);
#endif
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

#pragma endregion

#pragma region 卸载模块
void kcServerPluginsUnloadItem(struct KC_PLUGIN_ITEM* pluItem) {
#ifdef _WIN32
	BOOL isOk;
#else
	int32_t rc;
	char* load_error;
#endif


	if (NULL != pluItem->handler) {
#ifdef _WIN32
		isOk = FreeLibrary(pluItem->handler);
		if (!isOk)
			fprintf(stderr, gettext("unload module \"%s\" fail.\n"), pluItem->library);
#else
		rc = dlclose(pluItem->handler);
		if (rc) {
			load_error = dlerror();
			fprintf(stderr, gettext("unload module \"%s\" fail,%s.\n"), pluItem->library, load_error);
		}
#endif
	}
	KC_SAFE_MODULE_FREE(pluItem->identifier);
	KC_SAFE_MODULE_FREE(pluItem->library);
	KC_SAFE_MODULE_FREE(pluItem->entry_process_data);
}

void kcServerPluginsUnload(struct KC_PLUGINS* plugins) {
	struct KC_PLUGIN_ITEM* firstAdd;
	if (NULL == plugins)
		return;
	firstAdd = (struct KC_PLUGIN_ITEM*)plugins->items;
	for (size_t i = 0; i < plugins->count; ++i, ++firstAdd)
		kcServerPluginsUnloadItem(firstAdd);
	if (NULL != plugins->items)
		free(plugins->items);
	free(plugins);
}
#pragma endregion

#pragma region 查找模块
const struct KC_PLUGIN_ITEM* kcServerPluginsFind(const StringBuffer recv, const struct KC_PLUGINS* plugins, StringBuffer error) {
	const struct KC_PLUGIN_ITEM* firstAdd, * item = NULL;

	KC_VALIDATE_PARA_PTR(recv, error);
	KC_VALIDATE_PARA_PTR(plugins, error);

	firstAdd = (struct KC_PLUGIN_ITEM*)plugins->items;
	for (size_t i = 0; i < plugins->count; ++i, ++firstAdd) {
		if (0 == memcmp(firstAdd->identifier, recv->data, firstAdd->identifierLen))
			return firstAdd;
	}
	return NULL;
KC_ERROR_CLEAR:
	return NULL;
}
#pragma endregion
