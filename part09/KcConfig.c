#include <libintl.h>
#ifdef _MSC_VER
#	include <direct.h>
#	include <io.h>
#else
#	include <unistd.h>
#endif
#include <jansson.h>

#include  "pg_bswap.h"
#include "KcUtility.h"
#include "KcConfig.h"
#include "KcShared.h"


int32_t kcLoadConfig(const char* const fullfile, struct KC_CONFIG* conf, StringBuffer error);
int32_t kcParseRoot(const json_t* const root, struct KC_CONFIG* conf, StringBuffer error);


void kcConfigFree(struct KC_CONFIG* ptr) {
	if (NULL == ptr)
		return;
	KC_SAFE_MODULE_FREE(ptr->log_path);
	KC_SAFE_MODULE_FREE(ptr->runing_path);
	free(ptr);
}


int32_t kcConfigCreate(const char* const stdPath, StringBuffer str, struct KC_CONFIG** ptr) {
	struct KC_CONFIG* config = NULL;

	KC_VALIDATE_PARA_PTR(stdPath, str);

	//配置文件完整文件名
	resetStringBuffer(str);
	appendStringBuffer(str, "%s/config.json", stdPath);
	//检查配置文件是否存在和是否有读取权限
	KC_FILE_ACCESS_CHECK(str->data, 4,str);
	//为KC_CONFIG分配存储空间
	config = (struct KC_CONFIG*)calloc(1, sizeof(struct KC_CONFIG));
	KC_CHECK_MALLOC(config, str);
	//复制运行路径
	KC_CHECK_RCV2(kcStringCopyV3(stdPath, str, &config->runing_path));
	//加载并解析配置文件
	KC_CHECK_RCV2(kcLoadConfig(str->data, config, str));
	//测试配置文件中的目录是否存在及权限测试
	KC_FILE_ACCESS_CHECK(config->log_path, 6, str);			//读写
	(*ptr) = config;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FEEE_CONFIG(config);
	return KC_FAIL;
}

int32_t kcLoadConfig(const char* const fullfile, struct KC_CONFIG* conf, StringBuffer error) {
	FILE* stream = NULL;
	json_t* doc = NULL;
	json_error_t jerror;
	size_t nbytes = 0, rbytes = 0, offset = 0;
	int32_t index = 0;


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
				appendStringBuffer(error, "file length too large");
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
	KC_CHECK_RCV2(kcParseRoot(doc, conf, error));

	KC_JSON_FREE(doc);
	KC_FILE_CLOSE(stream);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_JSON_FREE(doc);
	KC_FILE_CLOSE(stream);
	return KC_FAIL;
}

int32_t kcParseRoot(const json_t* const root, struct KC_CONFIG* conf, StringBuffer error) {
	char* strAddress = NULL, * strLogPath = NULL;
	int32_t nPort;
	json_t* listen_addresses, * port, * send_timeout, * recv_timeout, * queue_max, * thread_pool;
	json_t* log_path;

	//根据节点名称获取节点对象
	listen_addresses = json_object_get(root, KC_2STR(listen_addresses));
	port = json_object_get(root, KC_2STR(port));
	send_timeout = json_object_get(root, KC_2STR(send_timeout));
	recv_timeout = json_object_get(root, KC_2STR(recv_timeout));
	queue_max = json_object_get(root, KC_2STR(queue_max));
	thread_pool = json_object_get(root, KC_2STR(thread_pool));

	log_path = json_object_get(root, KC_2STR(log_path));

	//检查节点对象是否为NULL	
	KC_JSON_NODE_ISNULL(listen_addresses, KC_2STR(listen_addresses), error);
	KC_JSON_NODE_ISNULL(port, KC_2STR(port), error);
	KC_JSON_NODE_ISNULL(send_timeout, KC_2STR(send_timeout), error);
	KC_JSON_NODE_ISNULL(recv_timeout, KC_2STR(recv_timeout), error);
	KC_JSON_NODE_ISNULL(queue_max, KC_2STR(queue_max), error);
	KC_JSON_NODE_ISNULL(thread_pool, KC_2STR(thread_pool), error);

	KC_JSON_NODE_ISNULL(log_path, KC_2STR(log_path), error);

	//获取节点值
	KC_CHECK_RCV2(kcJsonGetString(KC_2STR(listen_addresses), listen_addresses, &strAddress, error));
	KC_CHECK_RCV2(kcString2NetworkAddressV4(strAddress, &conf->addres.sin_addr, error));
	KC_CHECK_RCV2(kcJsonGetInt32(KC_2STR(port), port, &nPort, error));
	KC_CHECK_RCV2(kcJsonGetInt32(KC_2STR(send_timeout), send_timeout, &conf->sendTimeout, error));
	KC_CHECK_RCV2(kcJsonGetInt32(KC_2STR(recv_timeout), recv_timeout, &conf->recvTimeout, error));
	KC_CHECK_RCV2(kcJsonGetInt32(KC_2STR(queue_max), queue_max, &conf->queue_max, error));
	KC_CHECK_RCV2(kcJsonGetInt32(KC_2STR(thread_pool), thread_pool, &conf->threadPoolSize, error));

	KC_CHECK_RCV2(kcJsonGetString(KC_2STR(log_path), log_path, &strLogPath, error));
	KC_CHECK_RCV2(kcStringReplace(strLogPath, "\\", "/", error, &conf->log_path));
	

	
	if (conf->sendTimeout < -999999 || conf->sendTimeout > 120)
		KC_GOTOERR(error, "config item \"%s\" value range is -999999-120.", "send_timeout");
	if (conf->recvTimeout < -999999 || conf->recvTimeout > 120)
		KC_GOTOERR(error, "config item \"%s\" value range is -999999-120.", "recv_timeout");
	if (conf->queue_max < 1 || conf->queue_max > 8192)
		KC_GOTOERR(error, "config item \"%s\" value range is 1-8192.", "queue_max");
	if (conf->threadPoolSize < 1 || conf->threadPoolSize > 1024)
		KC_GOTOERR(error, "config item \"%s\" value range is 1-1024.", "thread_pool");

	conf->addres.sin_family = AF_INET;  //使用IPv4地址
	conf->addres.sin_port = pg_bswap16(nPort);//端口转换
	conf->sendTimeout *= 1000;
	conf->recvTimeout *= 1000;

	KC_SAFE_MODULE_FREE(strAddress);
	KC_SAFE_MODULE_FREE(strLogPath);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_MODULE_FREE(strAddress);
	KC_SAFE_MODULE_FREE(strLogPath);
	return KC_FAIL;
}
