using System;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using System.IO;
using System.Globalization;
using System.Threading;

namespace TraceLogger
{
    public static class Logger
    {
        public static void Log(
            string format,
            [CallerFilePath] string filePath = "",
            [CallerLineNumber] int lineNumber = 0,
            [CallerMemberName] string memberName = "",
            params object[] args)
        {
            string exeName = AppDomain.CurrentDomain.FriendlyName;
            int threadId = Thread.CurrentThread.ManagedThreadId; 
            string fileName = Path.GetFileName(filePath);

            string formattedMessage = string.Format(CultureInfo.InvariantCulture, format, args);

            string logMessage = $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] " +
                                $"[Thread:{threadId}] " +
                                $"[Exe:{exeName}] " +
                                $"[File:{fileName}] " +
                                $"[Line:{lineNumber}] " +
                                $"[Method:{memberName}] " +
                                $"{formattedMessage}";

            Console.WriteLine(logMessage);
        }

        public static void Log_v2(string format, params object[] args)
        {
            var stackFrame = new StackFrame(1, true);
            var method = stackFrame.GetMethod();
            var fileName = Path.GetFileName(stackFrame.GetFileName()) ?? "Unknown File";
            var lineNumber = stackFrame.GetFileLineNumber();

            string exeName = AppDomain.CurrentDomain.FriendlyName;
            int threadId = Thread.CurrentThread.ManagedThreadId;

            string formattedMessage = string.Format(CultureInfo.InvariantCulture, format, args);

            string logMessage = $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] " +
                                $"[Thread:{threadId}] " +
                                $"[Exe:{exeName}] " +
                                $"[File:{fileName}] " +
                                $"[Line:{lineNumber}] " +
                                $"[Method:{method?.Name}] " +
                                $"{formattedMessage}";

            Console.WriteLine(logMessage);
        }

        public static void Trace(
            string message = "Tracing method execution...",
            [CallerFilePath] string filePath = "",
            [CallerLineNumber] int lineNumber = 0,
            [CallerMemberName] string memberName = "")
        {
            LogMessage(message, filePath, lineNumber, memberName);
        }

        public static void Trace_In(
            string methodName,
            object[] parameters,
            [CallerFilePath] string filePath = "",
            [CallerLineNumber] int lineNumber = 0,
            [CallerMemberName] string memberName = "")
        {
            string paramDetails = FormatParameters(parameters);
            LogMessage($"Entering {methodName} with parameters: {paramDetails}", filePath, lineNumber, memberName);
        }

        public static void Trace_Out(
            string methodName,
            object returnValue,
            [CallerFilePath] string filePath = "",
            [CallerLineNumber] int lineNumber = 0,
            [CallerMemberName] string memberName = "")
        {
            string returnDetails = returnValue != null ? returnValue.ToString() : "void";
            LogMessage($"Exiting {methodName} with return value: {returnDetails}", filePath, lineNumber, memberName);
        }

        private static void LogMessage(
            string message,
            string filePath,
            int lineNumber,
            string memberName)
        {
            string exeName = AppDomain.CurrentDomain.FriendlyName;
            int threadId = Thread.CurrentThread.ManagedThreadId;
            string fileName = Path.GetFileName(filePath);

            string logMessage = $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] " +
                                $"[Thread:{threadId}] " +
                                $"[Exe:{exeName}] " +
                                $"[File:{fileName}] " +
                                $"[Line:{lineNumber}] " +
                                $"[Method:{memberName}] " +
                                $"{message}";

            Console.WriteLine(logMessage);
        }

        private static string FormatParameters(object[] parameters)
        {
            if (parameters == null || parameters.Length == 0)
                return "None";

            string[] formattedParams = new string[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                formattedParams[i] = parameters[i] != null
                    ? $"{parameters[i].GetType().Name}: {parameters[i]}"
                    : "null";
            }

            return string.Join(", ", formattedParams);
        }
    }
}
