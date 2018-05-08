# dbproxy
本软件为数据库类的代理程序，支持mysql,redis等数据库的代理。

工程采用automake实现编译，请参见https://www.ibm.com/developerworks/cn/linux/l-makefile/

文件结构说明：
1、common
   常用工具包，如配置文件，网络通讯(socket)，调试日志(trace)，工具包(util)
2、config
   软件的配置文件模板
   proxy/log/proxy 日志记录模式配置
         log_path 生产日志文件的路径
         log_filename 日志文件名
         trace_ids 写日志的id, 对应debug，error等，-1表示记录所有，多种类型使用英文逗号分隔
         trace_filter 只有输出日志中包含此关键字才会输出
         trace_filter_exclude 只有输出日志中不包含此关键字才会输出
         print_tty 输出到tty
         short_prefix 日志前缀 0 %H:%M:%S, 非0 %Y-%m-%d %H:%M:%S
         max_file_size 文件最大字节数
  proxy/log/control 协议代理配置
         type 代理类型，write：读写代理，readonly：只读代理，synchro：同步代理，sysncro只有mysql有效.
         audit 是否审计sql语句，yes：审计，no：不审计，审计时将发送到syslog中配置的所有服务器，无论此项是yes还是no，登录与登出的会话日志都会发送。
         protocol 代理协议，目前支持mysql和redis
         url 代理元数据获取接口，程序通过数据库账号来向url主动获取目标地址。
         access_id accessid, 有url所在的系统颁发
         access_key access_key, 有url所在的系统颁发
         cache_file, 当url宕机时，可以通过cache file来实现高可用，url连接不通，会读取本地cache file的数据来实现代理，不填写将不在本地缓存。
  event/server 目前无用，请填写本地IP
  event/syslog/item 日志服务器地址配置，目前发送三种日志（认证，退出和审计），详情查看doc/设计文档/数据库任务接口规范.docx中的3.3.2.6
         ip 日志服务器的地址
         port 日志服务器的端口
 3、processor
    主要代码，这里实现了协议代理功能,
    {protocol}_cache 为cache处理相关函数
    {protocol}_proc 为协议代理函数
    {protocol}_decoder 为协议解析处理相关函数
    proto_parser_conf 为支持协议的配置
 4、proxy
    为主程序，读取配置，根据配置调用processor中对应的协议代理程序
 5、responsor
    为代理会话相关函数，这里定义了会话数据的结构，并提供日志发送相关函数。

通过make编译后的可执行文件为proxy/proxy，只需要拷贝此文件既可以发布。


本程序可以根据需求扩展所有的反向协议代理，添加新协议，只需要在processor中添加cache, proc,decoder三个文件即可。
特别说明：
   1) mysql jdbc 客户端jdbc >= 5.1.44是判断了mysql_proc中的mysql_client_auth函数中的server_version的值，客户端会根据这个值的版本号来实现不同的协议，
   因此在未来需要根据不同版本的mysql提供独特的代理，mysql_proc中的mysql_client_auth固定值(302-307行)，需要通过抓取协议来获取。
   2) 在软件升级的过程中，由于proxy采用多进程方式运行，主进程退出后，但proxy程序依然被锁定，可以将新proxy重名为proxy_yyyymmdd, 将启动proxy_yyyymmdd既可
   
