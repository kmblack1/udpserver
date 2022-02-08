/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	SQLite3数据清理和检查函数
*****************************************************************************/
#ifndef KC_8BDFEB1E_915F_7C45_B5AF_22BC1EE8CABF
#define KC_8BDFEB1E_915F_7C45_B5AF_22BC1EE8CABF

#include <sqlcipher/sqlite3.h>

/*****************************************************************************
*   关闭SQLITE3数据库
*****************************************************************************/
#define KC_SQLITE3_SAFE_CLOSE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		sqlite3_close_v2((ptr)); (ptr) = NULL; \
	} \
} while (0)

/*****************************************************************************
*   SQLITE3执行完成sql后销毁statement object.
*****************************************************************************/
#define KC_SQLITE3_SAFE_STMT_DESTROY(ptr) do {\
	if(NULL !=(ptr)  ) { \
		sqlite3_finalize((ptr)); (ptr) = NULL; \
	} \
} while (0)

/*****************************************************************************
*   释放由SQLITE3创建的对象
*****************************************************************************/
#define KC_SQLITE3_SAFE_FREE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		sqlite3_free((ptr)); (ptr) = NULL; \
	} \
} while (0)

/*****************************************************************************
*   检查SQLITE3函数是否成功
*****************************************************************************/
#define KC_SQLITE3_CHECK_RC(rc,funname,str) do{\
	if (SQLITE_OK  != (rc) ){ \
		resetStringBuffer((str)); \
		appendStringBuffer((str),gettext("function \"%s\" fail.\n\"%s\".\nfile:%s(%d)"),(funname),sqlite3_errstr(rc),__FILE__,__LINE__); \
		goto KC_ERROR_CLEAR; \
	}\
}while (0)

/*****************************************************************************
*   检查sqlite3_exec函数是否执行成功
*****************************************************************************/
#define KC_SQLITE3_CHECK_EXEC(rc,errMessag,str) do{\
	if (SQLITE_OK  != (rc)){ \
		resetStringBuffer((str)); \
		appendStringBuffer((str),gettext("function \"%s\" fail.\n\"%s\".\nfile:%s(%d)"),("sqlite3_exec"),errMessag,__FILE__,__LINE__); \
		goto KC_ERROR_CLEAR; \
	}\
}while (0)

/*****************************************************************************
*   检查sqlite3_step函数是否执行成功
*****************************************************************************/
#define KC_SQLITE3_CHECK_STEP_FAIL(rc,str) do{\
	if ( SQLITE_DONE  != (rc) ){ \
		resetStringBuffer((str)); \
		appendStringBuffer((str),gettext("function \"%s\" fail.\n\"%s\".\nfile:%s(%d)"),("sqlite3_step"),sqlite3_errstr(rc),__FILE__,__LINE__); \
		goto KC_ERROR_CLEAR; \
	} \
}while (0)


/*****************************************************************************
*   优化数据库
*****************************************************************************/
#define KC_SQLITE3_VACUUM(db,sqlite3Error) do { \
	if(NULL!=(db)) { \
		sqlite3_exec((db), "vacuum;", NULL, NULL, &(sqlite3Error)); \
	} \
} while (0)

#define KC_SQLITE3_ANALYSIS(db,sqlite3Error) do { \
	if(NULL!=(db)) { \
		sqlite3_exec((db), "PRAGMA analysis_limit=1000;", NULL, NULL, &(sqlite3Error)); \
		sqlite3_exec((db), "PRAGMA optimize;", NULL, NULL, &(sqlite3Error)); \
	} \
} while (0)

#endif	/* KC_8BDFEB1E_915F_7C45_B5AF_22BC1EE8CABF */
