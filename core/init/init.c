#include "spider.h"
#include "downloader.h"

/*
  init the cspider
*/

cspider_t *init_cspider() {
  cspider_t *spider = (cspider_t *)malloc(sizeof(cspider_t));
  spider->task_queue = initTaskQueue();
  spider->download_thread_max = 5;
  spider->pipeline_thread_max = 5;
  spider->download_thread = 1;
  spider->pipeline_thread = 1;
  spider->process = NULL;
  spider->save = NULL;
  spider->process_user_data = NULL;
  spider->save_user_data = NULL;
  spider->loop = uv_default_loop();
  spider->task_queue_doing = initTaskQueue();
  spider->data_queue = initDataQueue();
  spider->data_queue_doing = initDataQueue();
  spider->idler = (uv_idle_t*)malloc(sizeof(uv_idle_t));
  spider->lock = (uv_rwlock_t*)malloc(sizeof(uv_rwlock_t));
  uv_rwlock_init(spider->lock);
  spider->save_lock = (uv_rwlock_t*)malloc(sizeof(uv_rwlock_t));
  uv_rwlock_init(spider->save_lock);
  spider->idler->data = spider;
  spider->site = (site_t*)malloc(sizeof(site_t));
  spider->site->user_agent = NULL;
  spider->site->proxy = NULL;
  spider->site->cookie = NULL;
  spider->site->timeout = 0;
  spider->log = NULL;
  return spider;
}
/*
 初始设置要抓取的url
 add urls to task queue
*/
void cs_setopt_url(cspider_t *cspider, char *url){
  assert(cspider != NULL);
  assert(url != NULL);
  createTask(cspider->task_queue, url);
}

/*
设置cookie
set cookie
*/
void cs_setopt_cookie(cspider_t *cspider, char *cookie) {
  ((site_t*)cspider->site)->cookie = cookie;
}

/*
  设置user agent
  set user agent
*/
void cs_setopt_useragent(cspider_t *cspider, char *agent) {
  ((site_t*)cspider->site)->user_agent = agent;
}

/*
  设置proxy
  set proxy
*/
void cs_setopt_proxy(cspider_t *cspider, char *proxy) {
  ((site_t*)cspider->site)->proxy = proxy;
}

/*
 设置超时时间
 set timeout(ms)
*/
void cs_setopt_timeout(cspider_t *cspider, long timeout) {
  ((site_t*)cspider->site)->timeout = timeout;
}
/*
  设置日志
  set log file
*/
void cs_setopt_logfile(cspider_t *cspider, FILE *log) {
  cspider->log = log;
  cspider->log_lock = (uv_rwlock_t*)malloc(sizeof(uv_rwlock_t));
  uv_rwlock_init(cspider->log_lock);
}

void cs_setopt_process(cspider_t *cspider, void (*process)(cspider_t *, char*, void*), void *user_data) {
  assert(cspider != NULL);
  cspider->process = process;
  cspider->process_user_data = user_data;
}

void cs_setopt_save(cspider_t *cspider, void (*save)(void*, void*), void *user_data){
  assert(cspider != NULL);
  cspider->save = save;
  cspider->save_user_data = user_data;
}

/*
  set thread's number
*/
void cs_setopt_threadnum(cspider_t *cspider, int flag, int number) {
  assert(flag == DOWNLOAD || flag == SAVE);
  assert(number > 0);
  if (flag == DOWNLOAD) {
    cspider->download_thread_max = number;
  } else {
    cspider->pipeline_thread_max = number;
  }
}

int cs_run(cspider_t *cspider) {
  if (cspider->process == NULL) {
    printf("warn : need to init process function(use cs_setopt_process)\n");
    return 0;
  }
  if (cspider->save == NULL) {
    printf("warn : need to init data persistence function(use cs_setopt_save)\n");
    return 0;
  }
  uv_idle_init(cspider->loop, cspider->idler);
  uv_idle_start(cspider->idler, watcher);
  
  return uv_run(cspider->loop, UV_RUN_DEFAULT);
}
