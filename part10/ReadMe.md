# 1 本节说明
pthread_cond_timedwait的使用方法，不建议使用Sleep了，推荐使用pthread_cond_timedwait

# 2 引用库
## 2.1 WINDOWS使用到的库
```bash
vcpkg search gettext;
vcpkg search pthread;
vcpkg search gettimeofday;
vcpkg install gettext pthreads gettimeofday;
```