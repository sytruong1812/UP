# TraceLogger
TraceLogger is a C++ library that supports logging for programs and applications.
## Logging Levels
* NOTSET : If this level is set, the function log displays the line number and log messages
* DEBUG  : If this level is set, the function log displays the line number, function name and log messages
* INFO   : If this level is set, the function log displays the line number, function name and log messages
* WARN   : If this level is set, the function log displays the line number, function name and log messages
* ERR    : If this level is set, the function log displays the line number, function name, file name and log messages
* CRIT   : If this level is set, the function log displays the line number, function name, file name and log messages
## Output
* WRITE_FILE   : Write log data to file
* SHOW_CONSOLE : Show console
* OUTPUT_DEBUG : Show log data in debug output
### Enable/Disable trace and log
```c++
    TraceLogger::instance()->EnableLog(BOOL);
    TraceLogger::instance()->EnableTrace(BOOL);
```
### Set log function output (Console, Write file, Output debug)
```c++
    TraceLogger::instance()->SetLogOut(LOG_OPT);
```
### Set log level
```c++
    TraceLogger::instance()->SetLogLevel(LOG_LEVEL);
```
### Module log
```c++
    TraceLogger::instance()->LogA(__LINE__,"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->DebugA(__FUNCTION__,__LINE__,"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->InfoA(__FUNCTION__,__LINE__,"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->WarningA(__FUNCTION__,__LINE__,"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->ErrorA(__FUNCTION__,__LINE__,__FILE__,"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->CriticalA(__FUNCTION__,__LINE__,__FILE__,"Text: %c %d \n", 'a', 65);

    TraceLogger::instance()->LogW(__LINE__,L"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->DebugW(__FUNCTION__,__LINE__,L"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->InfoW(__FUNCTION__,__LINE__,L"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->WarningW(__FUNCTION__,__LINE__,L"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->ErrorW(__FUNCTION__,__LINE__,__FILE__,L"Text: %c %d \n", 'a', 65);
    TraceLogger::instance()->CriticalW(__FUNCTION__,__LINE__,__FILE__,L"Text: %c %d \n", 'a', 65);
```
## Macro
### Enable/Disable trace and log
```c++
    ENABLE_LOG(BOOL);
    ENABLE_TRACE(BOOL);
```
### Set log function output (Console, Write file, Output debug)
```c++
    SET_LOG_OUT(SHOW_CONSOLE);      
    SET_LOG_OUT(WRITE_FILE);
    SET_LOG_OUT(OUTPUT_DEBUG);
```
### Set log level
```c++
    SET_LOG_LEVEL(CRIT);
```
### Module log
```c++
    LOG("TEST LOG MACRO");
    LOG_DEBUG("Level %d", 1);
    LOG_INFO("Level %d", 2);
    LOG_WARN("Level %d", 3);
    LOG_ERROR("Level %d", 4);
    LOG_CRITICAL("Level % d", 5);

    TRACE("TEST TRACE MACRO");
    TRACE_IN("Number = %d", n);
    TRACE_OUT("Number = %d", n);
    TRACE_CALL(function, param);
```
#### Usage samples
```c++
int factorial(int n, double m, string str) {
    int res = 1;
    TraceLogger::instance()->TraceInA(__FUNCTION__, __LINE__, "n = %d", n);
    if (n > 1) {
        res = n * factorial(n - 1, m, str);
    }
    TraceLogger::instance()->TraceOutA(__FUNCTION__, __LINE__, "res = %d", res);
    return res;
}

int main()
{
    TraceLogger::instance()->EnableLog(TRUE);
    TraceLogger::instance()->EnableTrace(TRUE);
    TraceLogger::instance()->SetLogOut(SHOW_CONSOLE);
    TraceLogger::instance()->SetLogLevel(CRIT);

    TraceLogger::instance()->LogA(__LINE__,"TEST LOG FUNCTION]");
    TraceLogger::instance()->DebugW(__FUNCTION__,__LINE__,L"Level %d", 1);
    TraceLogger::instance()->InfoA(__FUNCTION__,__LINE__,"Level %d", 2);
    TraceLogger::instance()->WarningW(__FUNCTION__,__LINE__,L"Level %d", 3);
    TraceLogger::instance()->ErrorA(__FUNCTION__,__LINE__,__FILE__,"Level %d", 4);
    TraceLogger::instance()->CriticalW(__FUNCTION__,__LINE__,__FILE__,L"Level %d", 5);

    TraceLogger::instance()->TraceA("TEST TRACE FUNCTIONTEST %d", 3);
    TraceLogger::instance()->TraceCallA(__FUNCTION__,__LINE__,__FILE__,factorial, 3, 3, "Hello");
    return 0;
}

```

#### Usage samples
```c++
int factorial(int n, double m, string str) {
    int res = 1;
    TRACE_IN("n = %d", n);
    if (n > 1) {
        res = n * factorial(n - 1, m, str);
    }
    TRACE_OUT("res = %d", res);
    return res;
int main()
{
    ENABLE_LOG(TRUE);
    ENABLE_TRACE(TRUE);
    SET_LOG_OUT(SHOW_CONSOLE);      
    SET_LOG_LEVEL(CRIT);

    LOG("TEST LOG MACRO");
    LOG_DEBUG("Level %d", 1);
    LOG_INFO("Level %d", 2);
    LOG_WARN("Level %d", 3);
    LOG_ERROR("Level %d", 4);
    LOG_CRITICAL("Level % d", 5);

    TRACE("TEST TRACE MACRO");
    TRACE_CALL(factorial, 3, 2.5, "Hello");
    return 0;
}
```
